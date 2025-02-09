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

#include "GUIFont.h"
#include "GUIFontTTF.h"
#include "GraphicContext.h"
#include "WindowingFactory.h"
#include "GUIWindowManager.h"
#include "utils/SingleLock.h"
#include "utils/TimeUtils.h"
#include "MathUtils.h"
#ifdef HAS_OPENMAX
#include "Application.h"
#endif

#define ROUND(x) (float)(MathUtils::round_int(x))

float CScrollInfo::GetPixelsPerFrame()
{
  static const float alphaEMA = 0.05f;

  if (0 == pixelSpeed)
    return 0; // not scrolling
  unsigned int currentTime = CTimeUtils::GetFrameTime();
  float delta = m_lastFrameTime ? (float)(currentTime - m_lastFrameTime) : m_averageFrameTime;
  if (delta > 100)
    delta = 100; // assume a minimum of 10 fps
  m_lastFrameTime = currentTime;
  // do an exponential moving average of the frame time
  m_averageFrameTime = m_averageFrameTime + (delta - m_averageFrameTime) * alphaEMA;
  // and multiply by pixel speed (per ms) to get number of pixels to move this frame
  return pixelSpeed * m_averageFrameTime;
}

CGUIFont::CGUIFont(const CStdString& strFontName, uint32_t style, color_t textColor, color_t shadowColor, float lineSpacing, CGUIFontTTFBase *font)
{
  m_strFontName = strFontName;
  m_style = style & 3;
  m_textColor = textColor;
  m_shadowColor = shadowColor;
  m_lineSpacing = lineSpacing;
  m_font = font;

  if (m_font)
    m_font->AddReference();
}

CGUIFont::~CGUIFont()
{
  if (m_font)
    m_font->RemoveReference();
}

CStdString& CGUIFont::GetFontName()
{
  return m_strFontName;
}

void CGUIFont::RenderText(FontCoordsIndiced& pData)
{
  if(!m_font)
    return;

  if(pData.m_pCoord.size() == 0)
    return;

  // don't render fonts with zero trans
  if(pData.m_pCoord[0].m_pCoords->a == 0)
    return;

  // fonts are always render at screen res, not skin res
  float xScale = g_graphicsContext.GetGUIScaleX();
  float yScale = g_graphicsContext.GetGUIScaleY();

  TransformMatrix mat = TransformMatrix::CreateScaler(xScale, yScale);
  g_graphicsContext.PushTransform(mat);

  bool useShadow = m_shadowColor == 0 ? false : true;

#ifdef HAS_OPENMAX
  if(g_application.IsPlayingVideo())
    g_Windowing.EnableBlending(true, true);
  else
#endif
  g_Windowing.EnableBlending(true);

  g_Windowing.EnableTexture(true);
  m_font->LoadShaders();
  m_font->Begin();
  m_font->Render(pData, useShadow);
  m_font->End();
  g_Windowing.EnableBlending(false);
  g_Windowing.EnableTexture(false);

  g_graphicsContext.PopTransform();
}

bool CGUIFont::BuildTextCoords( float x, float y, const vecColors &colors, color_t shadowColor,
                const vecText &text, uint32_t alignment, float maxPixelWidth, FontCoordsIndiced& pData, bool clip)
{
  if (!m_font) 
    return false;

  bool clip_width = maxPixelWidth > 0;
  bool clipEmpty = ClippedRegionIsEmpty(x, y, maxPixelWidth, alignment);
  if (clip && clip_width && clipEmpty)
    return false;

  maxPixelWidth = ROUND(maxPixelWidth / g_graphicsContext.GetGUIScaleX());
  vecColors renderColors;
  for (unsigned int i = 0; i < colors.size(); i++)
    renderColors.push_back(g_graphicsContext.MergeAlpha(colors[i] ? colors[i] : m_textColor));
  if (!shadowColor) 
    shadowColor = m_shadowColor;
  else
    m_shadowColor = shadowColor;
 
  m_font->BuildTextCoordinates(0.0, 0.0, renderColors, g_graphicsContext.MergeAlpha(shadowColor),
                              text, alignment, maxPixelWidth, false, pData);

  if (!clipEmpty)
    g_graphicsContext.RestoreClipRegion();

  return true;
}

void CGUIFont::BuildScrollingTextCoords(float x, float y, const vecColors &colors, color_t shadowColor,
                const vecText &text, uint32_t alignment, float maxWidth, CScrollInfo &scrollInfo, FontCoordsIndiced& pData)
{
  if (!m_font) return;
  if (!shadowColor) shadowColor = m_shadowColor;

  float spaceWidth = m_font->GetCharWidthInternal(L' ');
  // max chars on screen + extra margin chars
  vecText::size_type maxChars =
    std::min<vecText::size_type>(
      (text.size() + (vecText::size_type)scrollInfo.suffix.size()),
      (vecText::size_type)((maxWidth * 1.05f) / spaceWidth));

  if (!text.size() || ClippedRegionIsEmpty(x, y, maxWidth, alignment))
    return; // nothing to render

  maxWidth = ROUND(maxWidth / g_graphicsContext.GetGUIScaleX());

  // draw at our scroll position
  // we handle the scrolling as follows:
  //   We scroll on a per-pixel basis up until we have scrolled the first character outside
  //   of our viewport, whereby we cycle the string around, and reset the scroll position.
  //
  //   pixelPos is the amount in pixels to move the string by.
  //   characterPos is the amount in characters to rotate the string by.
  //
  float offset = scrollInfo.pixelPos;
  if (!scrollInfo.waitTime)
  {
    // move along by the appropriate scroll amount
    float scrollAmount = fabs(scrollInfo.GetPixelsPerFrame() * g_graphicsContext.GetGUIScaleX());

    if (scrollInfo.pixelSpeed > 0)
    {
      // we want to move scrollAmount, grab the next character
      float charWidth = m_font->GetCharWidthInternal(scrollInfo.GetCurrentChar(text));
      if (scrollInfo.pixelPos + scrollAmount < charWidth)
        scrollInfo.pixelPos += scrollAmount;  // within the current character
      else
      { // past the current character, decrement scrollAmount by the charWidth and move to the next character
        while (scrollInfo.pixelPos + scrollAmount >= charWidth)
        {
          scrollAmount -= (charWidth - scrollInfo.pixelPos);
          scrollInfo.pixelPos = 0;
          scrollInfo.characterPos++;
          if (scrollInfo.characterPos >= text.size() + scrollInfo.suffix.size())
          {
            scrollInfo.Reset();
            break;
          }
          charWidth = m_font->GetCharWidthInternal(scrollInfo.GetCurrentChar(text));
        }
      }
      offset = scrollInfo.pixelPos;
    }
    else if (scrollInfo.pixelSpeed < 0)
    { // scrolling backwards
      // we want to move scrollAmount, grab the next character
      float charWidth = m_font->GetCharWidthInternal(scrollInfo.GetCurrentChar(text));
      if (scrollInfo.pixelPos + scrollAmount < charWidth)
        scrollInfo.pixelPos += scrollAmount;  // within the current character
      else
      { // past the current character, decrement scrollAmount by the charWidth and move to the next character
        while (scrollInfo.pixelPos + scrollAmount >= charWidth)
        {
          scrollAmount -= (charWidth - scrollInfo.pixelPos);
          scrollInfo.pixelPos = 0;
          if (scrollInfo.characterPos == 0)
          {
            scrollInfo.Reset();
            scrollInfo.characterPos = text.size() + scrollInfo.suffix.size() - 1;
            break;
          }
          scrollInfo.characterPos--;
          charWidth = m_font->GetCharWidthInternal(scrollInfo.GetCurrentChar(text));
        }
      }
      offset = charWidth - scrollInfo.pixelPos;
    }
  }
  else
    scrollInfo.waitTime--;

  // Now rotate our string as needed, only take a slightly larger then visible part of the text.
  unsigned int pos = scrollInfo.characterPos;
  vecText renderText;
  renderText.reserve(maxChars);
  for (vecText::size_type i = 0; i < maxChars; i++)
  {
    if (pos >= text.size() + scrollInfo.suffix.size())
      pos = 0;
    if (pos < text.size())
      renderText.push_back(text[pos]);
    else
      renderText.push_back(scrollInfo.suffix[pos - text.size()]);
    pos++;
  }

  vecColors renderColors;
  for (unsigned int i = 0; i < colors.size(); i++)
    renderColors.push_back(g_graphicsContext.MergeAlpha(colors[i] ? colors[i] : m_textColor));

  bool scroll =  !scrollInfo.waitTime && scrollInfo.pixelSpeed;

  /*
  if (shadowColor)
    m_font->DrawTextInternal(x - offset + 1, y + 1, g_graphicsContext.MergeAlpha(shadowColor), renderText, alignment, maxWidth + scrollInfo.pixelPos + m_font->GetLineHeight(2.0f), scroll);
  */

  m_font->BuildTextCoordinates(offset, 0.0, renderColors, g_graphicsContext.MergeAlpha(shadowColor),
    renderText, alignment, maxWidth + scrollInfo.pixelPos + m_font->GetLineHeight(2.0f), scroll, pData);
}

// remaps unsupported font glpyhs to other suitable ones
wchar_t CGUIFont::RemapGlyph(wchar_t letter)
{
  if (letter == 0x2019 || letter == 0x2018) return 0x0027;  // single quotes
  else if (letter == 0x201c || letter == 0x201d) return 0x0022;
  return 0; // no decent character map
}

bool CGUIFont::ClippedRegionIsEmpty(float x, float y, float width, uint32_t alignment) const
{
  if (alignment & XBFONT_CENTER_X)
    x -= width * 0.5f;
  else if (alignment & XBFONT_RIGHT)
    x -= width;
  if (alignment & XBFONT_CENTER_Y)
    y -= m_font->GetLineHeight(m_lineSpacing);

  return !g_graphicsContext.SetClipRegion(x, y, width, m_font->GetLineHeight(2.0f) * g_graphicsContext.GetGUIScaleY());
}

unsigned int CGUIFont::GetFontTextureWidth()
{
  return m_font->GetTextureWidth();
}

unsigned int CGUIFont::GetFontTextureHeight()
{
  return m_font->GetTextureHeight();
}

float CGUIFont::GetTextWidth( const vecText &text )
{
  if (!m_font) return 0;
  CSingleLock lock(g_graphicsContext);
  return m_font->GetTextWidthInternal(text.begin(), text.end()) * g_graphicsContext.GetGUIScaleX();
}

float CGUIFont::GetCharWidth( character_t ch )
{
  if (!m_font) return 0;
  CSingleLock lock(g_graphicsContext);
  return m_font->GetCharWidthInternal(ch) * g_graphicsContext.GetGUIScaleX();
}

float CGUIFont::GetTextHeight(int numLines) const
{
  if (!m_font) return 0;
  return m_font->GetTextHeight(m_lineSpacing, numLines) * g_graphicsContext.GetGUIScaleY();
}

float CGUIFont::GetLineHeight() const
{
  if (!m_font) return 0;
  return m_font->GetLineHeight(m_lineSpacing) * g_graphicsContext.GetGUIScaleY();
}

void CGUIFont::SetFont(CGUIFontTTFBase *font)
{
  if (m_font == font)
    return; // no need to update the font if we already have it
  if (m_font)
    m_font->RemoveReference();
  m_font = font;
  if (m_font)
    m_font->AddReference();
}
