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

#include "GUIBorderedImage.h"

CGUIBorderedImage::CGUIBorderedImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture, const CTextureInfo& borderTexture, const CRect &borderSize)
   : CGUIImage(parentID, controlID, posX + borderSize.x1, posY + borderSize.y1, width - borderSize.x1 - borderSize.x2, height - borderSize.y1 - borderSize.y2, texture),
     m_borderImage(posX, posY, width, height, borderTexture),
     m_borderSize(borderSize)
{
  ControlType = GUICONTROL_BORDEREDIMAGE;
}

CGUIBorderedImage::CGUIBorderedImage(const CGUIBorderedImage &right)
: CGUIImage(right), m_borderImage(right.m_borderImage)
{
  m_borderSize = right.m_borderSize;
  ControlType = GUICONTROL_BORDEREDIMAGE;
}

CGUIBorderedImage::~CGUIBorderedImage(void)
{
}

void CGUIBorderedImage::Render()
{
  if (!m_borderImage.GetFileName().IsEmpty())
  {
    CRect rect = CRect(m_texture.GetXPosition(), m_texture.GetYPosition(), m_texture.GetXPosition() + m_texture.GetWidth(), m_texture.GetYPosition() + m_texture.GetHeight());
    rect.Intersect(m_texture.GetRenderRect());
    m_borderImage.SetPosition(rect.x1 - m_borderSize.x1, rect.y1 - m_borderSize.y1);
    m_borderImage.SetWidth(rect.Width() + m_borderSize.x1 + m_borderSize.x2);
    m_borderImage.SetHeight(rect.Height() + m_borderSize.y1 + m_borderSize.y2);
    m_borderImage.SetRenderBorderOnly(true);
    m_borderImage.Render();
  }
  CGUIImage::Render();
}

void CGUIBorderedImage::AllocResources()
{
  m_borderImage.AllocResources();
  CGUIImage::AllocResources();
}

void CGUIBorderedImage::FreeResources(bool immediately)
{
  m_borderImage.FreeResources(immediately);
  CGUIImage::FreeResources(immediately);
}

void CGUIBorderedImage::DynamicResourceAlloc(bool bOnOff)
{
  m_borderImage.DynamicResourceAlloc(bOnOff);
  CGUIImage::DynamicResourceAlloc(bOnOff);
}
