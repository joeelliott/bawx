/*!
\file GUIControlGroupList.h
\brief 
*/

#pragma once

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

#include "GUIControlGroup.h"

/*!
 \ingroup controls
 \brief list of controls that is scrollable
 */
class CGUIControlGroupList : public CGUIControlGroup
{
public:
  CGUIControlGroupList(int parentID, int controlID, float posX, float posY, float width, float height, float itemGap, int pageControl, ORIENTATION orientation, bool useControlPositions, uint32_t alignment);
  virtual ~CGUIControlGroupList(void);
  virtual CGUIControlGroupList *Clone() const { return new CGUIControlGroupList(*this); };

  virtual float GetWidth() const;
  virtual float GetHeight() const;
  float GetMaxSize() const { return Size(); }


  virtual void Render();
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool CanFocusFromPoint(const CPoint &point, CGUIControl **control, CPoint &controlPoint) const;
  virtual void UnfocusFromPoint(const CPoint &point);
  virtual bool OnMouseWheel(char wheel, const CPoint &point);  
  virtual void AddControl(CGUIControl *control, int position = -1);
  virtual void ClearAll(bool bDelete = true);

  virtual bool GetCondition(int condition, int data) const;
  
  // based on grouplist orientation pick one value as minSize;
  void SetMinSize(float minWidth, float minHeight);

protected:
  bool IsFirstFocusableControl(const CGUIControl *control) const;
  bool IsLastFocusableControl(const CGUIControl *control) const;
  void ValidateOffset();
  float Size(const CGUIControl *control) const;
  float Size() const;
  void ScrollTo(float offset);
  void Scroll(int amount);
  float GetAlignOffset() const;

  float m_itemGap;
  int m_pageControl;

  float m_offset; // measurement in pixels of our origin
  float m_totalSize;

  float m_scrollSpeed;
  float m_scrollOffset;
  unsigned int m_scrollTime;

  bool m_useControlPositions;
  ORIENTATION m_orientation;
  uint32_t m_alignment;

  // for autosizing
  float m_minSize;
};

