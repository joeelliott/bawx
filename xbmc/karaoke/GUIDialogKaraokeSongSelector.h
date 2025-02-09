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

#include "GUIDialog.h"
#include "MusicDatabase.h"
#include "Song.h"


class CGUIDialogKaraokeSongSelector: public CGUIDialog
{
public:
  CGUIDialogKaraokeSongSelector( int id, const char *xmlFile );
  virtual ~CGUIDialogKaraokeSongSelector(void);

protected:
  // Those functions control the selection process
  virtual void OnInitWindow();
  virtual void OnDeinitWindow(int nextWindowID);
  virtual bool OnAction(const CAction &action);
  virtual void Render();

  void OnButtonNumeric( unsigned int code, bool reset_autotimer = true ); // 0x00 - 0x09
  void OnButtonSelect(); // Song is selected
  void OnBackspace(); // Backspace pressed
  void UpdateData();

  // Configuration
  //! Auto-close timeout
  unsigned int  m_autoCloseTimeout;

  //! Start playing song as soon as it's selected?
  bool      m_startPlaying;

protected:
  //! Currently selected number
  unsigned int  m_selectedNumber;

  //! True if the number above did select some song and the info is in m_karaokeData
  bool      m_songSelected;

  //! True if we need to update fields before rendering
  bool      m_updateData;

  //! Database stuff
  CMusicDatabase m_musicdatabase;
  CSong          m_karaokeSong;
};


// A 'small' version of dialog using DialogKaraokeSongSelector.xml
class CGUIDialogKaraokeSongSelectorSmall : public CGUIDialogKaraokeSongSelector
{
  public:
    CGUIDialogKaraokeSongSelectorSmall();
    void DoModal(unsigned int startcode, int iWindowID = WINDOW_INVALID, const CStdString &param = "");
};


// A 'large' version of dialog using DialogKaraokeSongSelectorLarge.xml
class CGUIDialogKaraokeSongSelectorLarge : public CGUIDialogKaraokeSongSelector
{
  public:
    CGUIDialogKaraokeSongSelectorLarge();
    void DoModal(int iWindowID = WINDOW_INVALID, const CStdString &param = "");
};
