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
#include "FileSystem/VirtualDirectory.h"
#include "FileSystem/DirectoryHistory.h"
#include "GUIViewControl.h"
#include "PictureThumbLoader.h"

class CFileItem;
class CFileItemList;

class CGUIDialogFileBrowser : public CGUIDialog, public IBackgroundLoaderObserver
{
public:
  CGUIDialogFileBrowser(void);
  CGUIDialogFileBrowser(int id, const CStdString &xmlFile);
  void InitializeFileBrowser();
  virtual ~CGUIDialogFileBrowser(void);
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);
  virtual void Render();
  virtual void OnWindowLoaded();
  virtual void OnWindowUnload();
  bool IsConfirmed() { return m_bConfirmed; };
  void SetHeading(const CStdString &heading);
  void SetPathLabel(const CStdString &heading);

  static bool ShowAndGetDirectory(const VECSOURCES &shares, const CStdString &heading, CStdString &path, bool bWriteOnly=false);
  static bool ShowAndGetFile(const VECSOURCES &shares, const CStdString &mask, const CStdString &heading, CStdString &path, bool useThumbs = false, bool useFileDirectories = false, bool allowNonLocalSources = true);
  static bool ShowAndGetFile(const CStdString &directory, const CStdString &mask, const CStdString &heading, CStdString &path, bool useThumbs = false, bool useFileDirectories = false, bool allowNonLocalSources = true);
  static bool ShowAndGetSource(CStdString &path, bool allowNetworkShares, VECSOURCES* additionalShare = NULL, const CStdString& strType="");
  static bool ShowAndGetImage(const VECSOURCES &shares, const CStdString &heading, CStdString &path);
  static bool ShowAndGetImage(const CFileItemList &items, VECSOURCES &shares, const CStdString &heading, CStdString &path, bool* flip=NULL);

  void SetSources(const VECSOURCES &shares);

  virtual void OnItemLoaded(CFileItem *item) {};

  virtual bool HasListItems() const { return true; };
  virtual CFileItemPtr GetCurrentListItem(int offset = 0);
  int GetViewContainerID() const { return m_viewControl.GetCurrentControl(); };

protected:
  void GoParentFolder();
  void OnClick(int iItem);
  void OnSort();
  void ClearFileItems();
  void Update(const CStdString &strDirectory);
  bool HaveDiscOrConnection( CStdString& strPath, int iDriveType );
  bool OnPopupMenu(int iItem);
  void OnAddNetworkLocation();
  void OnAddMediaSource();
  void OnEditMediaSource(CFileItem* pItem);
  CGUIControl *GetFirstFocusableControl(int id);

  VECSOURCES m_shares;
  DIRECTORY::CVirtualDirectory m_rootDir;
  CFileItemList* m_vecItems;
  CFileItem* m_Directory;
  CStdString m_strParentPath;
  CStdString m_selectedPath;
  CDirectoryHistory m_history;
  int m_browsingForFolders; // 0 - no, 1 - yes, 2 - yes, only writable
  bool m_bConfirmed;
  int m_bFlip;
  bool m_addNetworkShareEnabled;
  bool m_flipEnabled;
  CStdString m_addSourceType;
  bool m_browsingForImages;
  bool m_useFileDirectories;
  bool m_singleList;              // if true, we have no shares or anything

  CPictureThumbLoader m_thumbLoader;
  CGUIViewControl m_viewControl;
};
