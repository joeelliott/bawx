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

#include "IDirectory.h"
#include "FileSmb.h"
#include "MediaSource.h"

namespace DIRECTORY
{
class CSMBDirectory : public IDirectory
{
public:
  CSMBDirectory(void);
  virtual ~CSMBDirectory(void);
  virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
  virtual DIR_CACHE_TYPE GetCacheType(const CStdString &strPath) const { return DIR_CACHE_ALWAYS; };
  virtual bool Create(const char* strPath);
  virtual bool Exists(const char* strPath);
  virtual bool Remove(const char* strPath);

  int Open(const CURI &url);

  //MountShare will try to mount the smb share and return the path to the mount point (or empty string if failed)
  static CStdString MountShare(const CStdString &smbPath, const CStdString &strType, const CStdString &strName, 
    const CStdString &strUser, const CStdString &strPass);

  static void UnMountShare(const CStdString &strType, const CStdString &strName);
  static CStdString GetMountPoint(const CStdString &strType, const CStdString &strName);
  static bool ProcessPath(const CStdString& strPath, CFileItemList &items);

  static bool MountShare(const CStdString &strType, CMediaSource &share);

private:
  int OpenDir(const CURI &url, CStdString& strAuth);
  static bool GetComputers(CFileItemList &items);
  static bool GetComputerDevices(const CStdString& strPath,const CStdString& selectedComp, CFileItemList &items);
  static CStdString GetComputerName(const CStdString& computer);
};
}
