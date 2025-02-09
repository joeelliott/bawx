#include "GUIVisualisationControl.h"
#include "GUIWindowManager.h"
#include "GUIUserMessages.h"
#include "Application.h"
#include "visualizations/Visualisation.h"
#include "visualizations/VisualisationFactory.h"
#include "visualizations/fft.h"
#include "Util.h"
#include "utils/CriticalSection.h"
#include "utils/log.h"
#include "utils/SingleLock.h"
#include "utils/GUIInfoManager.h"
#include "GUISettings.h"

using namespace std;
using namespace MUSIC_INFO;

#define LABEL_ROW1 10
#define LABEL_ROW2 11
#define LABEL_ROW3 12

// uggly hack, we can only allow one visualisation at one time or stuff starts crashing
static CCriticalSection m_critSection;
static bool m_globalvis = false;

CAudioBuffer::CAudioBuffer(int iSize)
{
  m_iLen = iSize;
  m_pBuffer = new short[iSize];
}

CAudioBuffer::~CAudioBuffer()
{
  delete [] m_pBuffer;
}

const short* CAudioBuffer::Get() const
{
  return m_pBuffer;
}

void CAudioBuffer::Set(const unsigned char* psBuffer, int iSize, int iBitsPerSample)
{
  if (iSize<0) 
  {
    return;
  }

  if (iBitsPerSample == 16)
  {
    iSize /= 2;
    for (int i = 0; i < iSize && i < m_iLen; i++)
    { // 16 bit -> convert to short directly
      m_pBuffer[i] = ((short *)psBuffer)[i];
    }
  }
  else if (iBitsPerSample == 8)
  {
    for (int i = 0; i < iSize && i < m_iLen; i++)
    { // 8 bit -> convert to signed short by multiplying by 256
      m_pBuffer[i] = ((short)((char *)psBuffer)[i]) << 8;
    }
  }
  else // assume 24 bit data
  {
    iSize /= 3;
    for (int i = 0; i < iSize && i < m_iLen; i++)
    { // 24 bit -> ignore least significant byte and convert to signed short
      m_pBuffer[i] = (((int)psBuffer[3 * i + 1]) << 0) + (((int)((char *)psBuffer)[3 * i + 2]) << 8);
    }
  }
  for (int i = iSize; i < m_iLen;++i) m_pBuffer[i] = 0;
}

CGUIVisualisationControl::CGUIVisualisationControl(int parentID, int controlID, float posX, float posY, float width, float height)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
{
  m_pVisualisation = NULL;
  m_bInitialized = false;
  m_iNumBuffers = 0;
  m_currentVis = "";
  ControlType = GUICONTROL_VISUALISATION;
}

CGUIVisualisationControl::CGUIVisualisationControl(const CGUIVisualisationControl &from)
: CGUIControl(from), IAudioCallback(from)
{
  m_pVisualisation = NULL;
  m_bInitialized = false;
  m_iNumBuffers = 0;
  m_currentVis = "";
  ControlType = GUICONTROL_VISUALISATION;
}

CGUIVisualisationControl::~CGUIVisualisationControl(void)
{
}

void CGUIVisualisationControl::FreeVisualisation()
{
  if (!m_bInitialized) return;
  m_bInitialized = false;
  // tell our app that we're going
  CGUIMessage msg(GUI_MSG_VISUALISATION_UNLOADING, 0, 0);
  g_windowManager.SendMessage(msg);

  CSingleLock lock (m_critSection);

  CLog::Log(LOGDEBUG, "FreeVisualisation() started");
  if (g_application.m_pPlayer)
    g_application.m_pPlayer->UnRegisterAudioCallback();
  if (m_pVisualisation)
  {
    OutputDebugString("Visualisation::Stop()\n");
    g_graphicsContext.CaptureStateBlock();
    m_pVisualisation->Stop();
    g_graphicsContext.ApplyStateBlock();

    OutputDebugString("delete Visualisation()\n");
    delete m_pVisualisation;

    /* we released the global vis spot */
    m_globalvis = false;
  }
  m_pVisualisation = NULL;
  ClearBuffers();
  CLog::Log(LOGDEBUG, "FreeVisualisation() done");
}

void CGUIVisualisationControl::LoadVisualisation()
{
  CSingleLock lock (m_critSection);
  if (m_pVisualisation)
    FreeVisualisation();

  m_bInitialized = false;

  /* check if any other control beat us to the punch */
  if(m_globalvis)
    return;

  CVisualisationFactory factory;
  CStdString strVisz, strModule;
  m_currentVis = g_guiSettings.GetString("mymusic.visualisation");
	
    if (m_currentVis.Equals("None"))
    return;
  
  // check if it's a multi-vis and if it is , get it's module name
  {
    int colonPos = m_currentVis.ReverseFind(":");
    if ( colonPos > 0 )
    {
      strModule = m_currentVis.Mid( colonPos+1 );
      strVisz = m_currentVis.Mid( 0, colonPos );
      m_pVisualisation = factory.LoadVisualisation(strVisz, strModule);
  }
    else
    {
      strVisz = m_currentVis;
  m_pVisualisation = factory.LoadVisualisation(strVisz);
    }
  }
  if (m_pVisualisation)
  {
    float z = 0;

    g_graphicsContext.CaptureStateBlock();

    
    float x = GetXPosition();
    float y = GetYPosition();
    float w = GetXPosition() + GetWidth();
    float h = GetYPosition() + GetHeight() - y;

    g_graphicsContext.ScaleFinalCoords(x, y, z);
    g_graphicsContext.ScaleFinalCoords(w, h, z);
    
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + w > g_graphicsContext.GetWidth()) w = g_graphicsContext.GetWidth() - x;
    if (y + h > g_graphicsContext.GetHeight()) h = g_graphicsContext.GetHeight() - y;

    m_pVisualisation->Create((int)(x+0.5f), (int)(y+0.5f), (int)(w+0.5f), (int)(h+0.5f));
    g_graphicsContext.ApplyStateBlock();
    VerifyGLState();
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->RegisterAudioCallback(this);

    // Create new audio buffers
    CreateBuffers();

    m_globalvis = true;
    m_bInitialized = true;
  }

  // tell our app that we're back
  CGUIMessage msg(GUI_MSG_VISUALISATION_LOADED, 0, 0, 0, 0, m_pVisualisation);
  g_windowManager.SendMessage(msg);
}

void CGUIVisualisationControl::UpdateVisibility(const CGUIListItem *item)
{
  CGUIControl::UpdateVisibility(item);
  if (!IsVisible() && m_bInitialized)
    FreeVisualisation();    
  }

void CGUIVisualisationControl::SetInvalid()
{
  FreeResources();
}

void CGUIVisualisationControl::Render()
{
  if (m_pVisualisation == NULL)
  { // check if we need to load
    if (g_application.IsPlayingAudio())
    {
      LoadVisualisation();
    }

    // just clear if we don't have any visual
    g_graphicsContext.Clear();

    CGUIControl::Render();

    return;
  }
  else
  { // check if we need to unload
    if (!g_application.IsPlayingAudio())
    { // deinit
      FreeVisualisation();
      CGUIControl::Render();
      return;
    }
    else if (!m_currentVis.Equals(g_guiSettings.GetString("mymusic.visualisation")))
    { // vis changed - reload
      LoadVisualisation();

      CGUIControl::Render();

      return;
    }
  }
  CSingleLock lock (m_critSection);
  if (m_pVisualisation)
  {
    if (m_bInitialized)
    {
      // set the viewport - note: We currently don't have any control over how
      // the visualisation renders, so the best we can do is attempt to define
      // a viewport??
      TransformMatrix mat = g_graphicsContext.GetGuiTransform();

      g_graphicsContext.PushTransform(mat, true);
      g_graphicsContext.PushViewPort(m_posX, m_posY, m_width, m_height);
      bool clip = g_graphicsContext.SetClipRegion(m_posX, m_posY, m_width, m_height);
      try
      {
        m_pVisualisation->Render();
      }
      catch (...)
      {
        CLog::Log(LOGERROR, "Exception in Visualisation::Render()");
      }
      // clear the viewport
      g_graphicsContext.PopTransform();
      g_graphicsContext.PopViewPort();
      if(clip)
        g_graphicsContext.RestoreClipRegion();
    }
  }

  CGUIControl::Render();
}


void CGUIVisualisationControl::OnInitialize(int iChannels, int iSamplesPerSec, int iBitsPerSample)
{
  CSingleLock lock (m_critSection);
  if (!m_pVisualisation)
    return ;
  CLog::Log(LOGDEBUG, "OnInitialize() started");

  m_iChannels = iChannels;
  m_iSamplesPerSec = iSamplesPerSec;
  m_iBitsPerSample = iBitsPerSample;

  // Start the visualisation (this loads settings etc.)
  CStdString strFile = CUtil::GetFileName(g_application.CurrentFile());
  OutputDebugString("Visualisation::Start()\n");
  m_pVisualisation->Start(m_iChannels, m_iSamplesPerSec, m_iBitsPerSample, strFile);
  if (!m_bInitialized)
  {
    UpdateTrack();
  }
  m_bInitialized = true;
  CLog::Log(LOGDEBUG, "OnInitialize() done");
}

void CGUIVisualisationControl::OnAudioData(const unsigned char* pAudioData, int iAudioDataLength)
{
  CSingleLock lock (m_critSection);
  if (!m_pVisualisation)
    return ;
  if (!m_bInitialized) return ;
  
  // FIXME: iAudioDataLength should never be less than 0
  if (iAudioDataLength<0)
    return;
  
  // Save our audio data in the buffers
  auto_ptr<CAudioBuffer> pBuffer ( new CAudioBuffer(2*AUDIO_BUFFER_SIZE) );
  pBuffer->Set(pAudioData, iAudioDataLength, m_iBitsPerSample);
  m_vecBuffers.push_back( pBuffer.release() );

  if ( (int)m_vecBuffers.size() < m_iNumBuffers) return ;

  auto_ptr<CAudioBuffer> ptrAudioBuffer ( m_vecBuffers.front() );
  m_vecBuffers.pop_front();
  // Fourier transform the data if the vis wants it...
  if (m_bWantsFreq)
  {
    // Convert to floats
    const short* psAudioData = ptrAudioBuffer->Get();
    for (int i = 0; i < 2*AUDIO_BUFFER_SIZE; i++)
    {
      m_fFreq[i] = (float)psAudioData[i];
    }

    // FFT the data
    twochanwithwindow(m_fFreq, AUDIO_BUFFER_SIZE);

    // Normalize the data
    float fMinData = (float)AUDIO_BUFFER_SIZE * AUDIO_BUFFER_SIZE * 3 / 8 * 0.5 * 0.5; // 3/8 for the Hann window, 0.5 as minimum amplitude
    float fInvMinData = 1.0f/fMinData;
    for (int i = 0; i < AUDIO_BUFFER_SIZE + 2; i++)
    {
      m_fFreq[i] *= fInvMinData;
    }

    // Transfer data to our visualisation
    try
    {
      m_pVisualisation->AudioData(ptrAudioBuffer->Get(), AUDIO_BUFFER_SIZE, m_fFreq, AUDIO_BUFFER_SIZE);
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "Exception in Visualisation::AudioData()");
    }
  }
  else
  { // Transfer data to our visualisation
    try
    {
      m_pVisualisation->AudioData(ptrAudioBuffer->Get(), AUDIO_BUFFER_SIZE, NULL, 0);
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "Exception in Visualisation::AudioData()");
    }
  }
  return ;
}

bool CGUIVisualisationControl::OnAction(const CAction &action)
{
  if (!m_pVisualisation) return false;
  enum CVisualisation::VIS_ACTION visAction = CVisualisation::VIS_ACTION_NONE;
  if (action.id == ACTION_VIS_PRESET_NEXT)
    visAction = CVisualisation::VIS_ACTION_NEXT_PRESET;
  else if (action.id == ACTION_VIS_PRESET_PREV)
    visAction = CVisualisation::VIS_ACTION_PREV_PRESET;
  else if (action.id == ACTION_VIS_PRESET_LOCK)
    visAction = CVisualisation::VIS_ACTION_LOCK_PRESET;
  else if (action.id == ACTION_VIS_PRESET_RANDOM)
    visAction = CVisualisation::VIS_ACTION_RANDOM_PRESET;
  else if (action.id == ACTION_VIS_RATE_PRESET_PLUS)
    visAction = CVisualisation::VIS_ACTION_RATE_PRESET_PLUS;
  else if (action.id == ACTION_VIS_RATE_PRESET_MINUS)
    visAction = CVisualisation::VIS_ACTION_RATE_PRESET_MINUS;

  return m_pVisualisation->OnAction(visAction);
}

bool CGUIVisualisationControl::UpdateTrack()
{
  bool handled = false;

  if ( m_pVisualisation )
  {
  // get the current album art filename
    m_AlbumThumb = g_infoManager.GetImage(MUSICPLAYER_COVER, WINDOW_INVALID);

  // get the current track tag
  const CMusicInfoTag* tag = g_infoManager.GetCurrentSongTag();

    if (m_AlbumThumb == "DefaultAlbumCover.png")
      m_AlbumThumb = "";
    else
      CLog::Log(LOGDEBUG,"Updating visualisation albumart: %s", m_AlbumThumb.c_str());

    // inform the visulisation of the current album art
    if ( m_pVisualisation->OnAction( CVisualisation::VIS_ACTION_UPDATE_ALBUMART,
                                     (void*)( m_AlbumThumb.c_str() ) ) )
      handled = true;

    // inform the visualisation of the current track's tag information
    if ( tag && m_pVisualisation->OnAction( CVisualisation::VIS_ACTION_UPDATE_TRACK,
                                            (void*)tag ) )
      handled = true;
    }
  return handled;
}

bool CGUIVisualisationControl::OnMessage(CGUIMessage &message)
{
  if (message.GetMessage() == GUI_MSG_GET_VISUALISATION)
  {
    message.SetPointer(GetVisualisation());
    return true;
  }
  else if (message.GetMessage() == GUI_MSG_VISUALISATION_ACTION)
  {
    CAction action;
    action.id = message.GetParam1();
    return OnAction(action);
  }
  else if (message.GetMessage() == GUI_MSG_PLAYBACK_STARTED)
  {
    if (IsVisible() && UpdateTrack()) return true;
  }
  return CGUIControl::OnMessage(message);
}

void CGUIVisualisationControl::CreateBuffers()
{
  CSingleLock lock (m_critSection);
  ClearBuffers();

  // Get the number of buffers from the current vis
  VIS_INFO info;
  m_pVisualisation->GetInfo(&info);
  m_iNumBuffers = info.iSyncDelay + 1;
  m_bWantsFreq = info.bWantsFreq;
  if (m_iNumBuffers > MAX_AUDIO_BUFFERS)
    m_iNumBuffers = MAX_AUDIO_BUFFERS;
  if (m_iNumBuffers < 1)
    m_iNumBuffers = 1;
}


void CGUIVisualisationControl::ClearBuffers()
{
  CSingleLock lock (m_critSection);
  m_bWantsFreq = false;
  m_iNumBuffers = 0;
  
  while (m_vecBuffers.size() > 0)
  {
    CAudioBuffer* pAudioBuffer = m_vecBuffers.front();
    delete pAudioBuffer;
    m_vecBuffers.pop_front();
  }
  for (int j = 0; j < AUDIO_BUFFER_SIZE*2; j++)
  {
    m_fFreq[j] = 0.0f;
  }
}

void CGUIVisualisationControl::FreeResources(bool immediately)
{
  FreeVisualisation();
  CGUIControl::FreeResources(immediately);
}

CVisualisation *CGUIVisualisationControl::GetVisualisation()
{
  CSingleLock lock (m_critSection);
  return m_pVisualisation;
}

bool CGUIVisualisationControl::OnMouseOver(const CPoint &point)
{
  // unfocusable, so return true
  CGUIControl::OnMouseOver(point);
  return false;
}

bool CGUIVisualisationControl::CanFocus() const
{ // unfocusable
  return false;
}

bool CGUIVisualisationControl::CanFocusFromPoint(const CPoint &point, CGUIControl **control, CPoint &controlPoint) const
{ // mouse is allowed to focus this control, but it doesn't actually receive focus
  controlPoint = point;
  m_transform.InverseTransformPosition(controlPoint.x, controlPoint.y);
  if (HitTest(controlPoint))
  {
    *control = (CGUIControl *)this;
    return true;
  }
  *control = NULL;
  return false;
}
