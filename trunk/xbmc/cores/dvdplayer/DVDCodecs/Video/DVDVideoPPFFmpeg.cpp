/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif

#ifndef DISABLE_FFMPEG_POSTPROC

#include "DVDVideoPPFFmpeg.h"
#include "utils/log.h"

CDVDVideoPPFFmpeg::CDVDVideoPPFFmpeg(const CStdString& mType)
{
  m_sType = mType;
  m_pMode = m_pContext = NULL;
  m_pSource = m_pTarget = NULL;
  m_iInitWidth = m_iInitHeight = 0;
  memset(&m_FrameBuffer, 0, sizeof(DVDVideoPicture));
}
CDVDVideoPPFFmpeg::~CDVDVideoPPFFmpeg()
{
  Dispose();
}
void CDVDVideoPPFFmpeg::Dispose()
{
  if (m_pMode)
  {
    m_dll.pp_free_mode(m_pMode);
    m_pMode = NULL;
  }
  if(m_pContext)
  {
    m_dll.pp_free_context(m_pContext);
    m_pContext = NULL;
  }
  
  if( m_FrameBuffer.iFlags & DVP_FLAG_ALLOCATED )
  {
    for( int i = 0; i<4; i++ )
    {
      if( m_FrameBuffer.data[i] )
      {
        _aligned_free(m_FrameBuffer.data[i]);
        m_FrameBuffer.data[i] = NULL;
        m_FrameBuffer.iLineSize[i] = 0;
      }
    }
    m_FrameBuffer.iFlags &= ~DVP_FLAG_ALLOCATED;
  }

  m_iInitWidth = 0;
  m_iInitHeight = 0;

  m_dll.Unload();
}

bool CDVDVideoPPFFmpeg::CheckInit(int iWidth, int iHeight)
{
  if (!m_dll.IsLoaded() && !m_dll.Load()) return false;
  
  if(m_iInitWidth != iWidth || m_iInitHeight != iHeight)
  {
    if(m_pContext || m_pMode)
    {
      Dispose();
    }

    m_pContext = m_dll.pp_get_context(m_pSource->iWidth, m_pSource->iHeight, PP_CPU_CAPS_MMX | PP_CPU_CAPS_MMX2 | PP_FORMAT_420);

    m_iInitWidth = m_pSource->iWidth;
    m_iInitHeight = m_pSource->iHeight;

    m_pMode = m_dll.pp_get_mode_by_name_and_quality((char *)m_sType.c_str(), PP_QUALITY_MAX);
    }


  if(m_pMode) 
    return true;
  else
    return false;
}


bool CDVDVideoPPFFmpeg::Process(DVDVideoPicture* pPicture)
{
  m_pSource =  pPicture;

  if(m_pSource->format != DVDVideoPicture::FMT_YUV420P)
    return false;

  if( !CheckInit(m_pSource->iWidth, m_pSource->iHeight) )
  {
    CLog::Log(LOGERROR, "Initialization of ffmpeg postprocessing failed");
    return false;
  }

  //If no target was set or we are using internal buffer, make sure it's correctly sized
  if(m_pTarget == &m_FrameBuffer || !m_pTarget)
  {
    if(CheckFrameBuffer(m_pSource))
      m_pTarget = &m_FrameBuffer;
    else
    {
      m_pTarget = NULL;
      return false;
    }
  }

  m_dll.pp_postprocess(m_pSource->data, m_pSource->iLineSize, 
                m_pTarget->data, m_pTarget->iLineSize,        
                m_pSource->iWidth, m_pSource->iHeight,
                0, 0,
                m_pMode, m_pContext, 
                PP_PICT_TYPE_QP2); //m_pSource->iFrameType);

  //Copy frame information over to target, but make sure it is set as allocated should decoder have forgotten
  m_pTarget->iFlags = m_pSource->iFlags | DVP_FLAG_ALLOCATED;
  m_pTarget->iFrameType = m_pSource->iFrameType;
  m_pTarget->iRepeatPicture = m_pSource->iRepeatPicture;;
  m_pTarget->iDuration = m_pSource->iDuration;
  m_pTarget->iDisplayHeight = m_pSource->iDisplayHeight;
  m_pTarget->iDisplayWidth = m_pSource->iDisplayWidth;
  m_pTarget->pts = m_pSource->pts;
  m_pTarget->iGroupId = m_pSource->iGroupId;
  m_pTarget->format = DVDVideoPicture::FMT_YUV420P;
  return true;
}
 


bool CDVDVideoPPFFmpeg::CheckFrameBuffer(const DVDVideoPicture* pSource)
{
  if( m_FrameBuffer.iFlags & DVP_FLAG_ALLOCATED && (m_FrameBuffer.iWidth != pSource->iWidth || m_FrameBuffer.iHeight != pSource->iHeight))
  {
    m_FrameBuffer.iFlags &= ~DVP_FLAG_ALLOCATED;
    for(int i = 0;i<3;i++)
      if(m_FrameBuffer.data[i])
      {
        delete[] m_FrameBuffer.data[i];
        m_FrameBuffer.data[i] = NULL;
      }
  }

  if(!(m_FrameBuffer.iFlags & DVP_FLAG_ALLOCATED))
  {
    memset(&m_FrameBuffer, 0, sizeof(DVDVideoPicture));

    m_FrameBuffer.iLineSize[0] = pSource->iLineSize[0];   //Y
    m_FrameBuffer.iLineSize[1] = pSource->iLineSize[1]; //U
    m_FrameBuffer.iLineSize[2] = pSource->iLineSize[2]; //V
    m_FrameBuffer.iLineSize[3] = 0;

    m_FrameBuffer.iWidth = pSource->iWidth;
    m_FrameBuffer.iHeight = pSource->iHeight;
    
    m_FrameBuffer.data[0] = (BYTE*)_aligned_malloc(m_FrameBuffer.iLineSize[0] * m_FrameBuffer.iHeight  , 16);
    m_FrameBuffer.data[1] = (BYTE*)_aligned_malloc(m_FrameBuffer.iLineSize[1] * m_FrameBuffer.iHeight/2, 16);
    m_FrameBuffer.data[2] = (BYTE*)_aligned_malloc(m_FrameBuffer.iLineSize[2] * m_FrameBuffer.iHeight/2, 16);

    if( !m_FrameBuffer.data[0] || !m_FrameBuffer.data[1] || !m_FrameBuffer.data[2])
    {
      CLog::Log(LOGERROR, "CDVDVideoDeinterlace::AllocBufferOfType - Unable to allocate framebuffer, bailing");
      return false;
    }

    m_FrameBuffer.iFlags |= DVP_FLAG_ALLOCATED;
  }

  return true;
}


bool CDVDVideoPPFFmpeg::GetPicture(DVDVideoPicture* pPicture)
{
  if( m_pTarget )
  {
    memmove(pPicture, m_pTarget, sizeof(DVDVideoPicture));
    return true;
  }
  return false;
}

#endif

