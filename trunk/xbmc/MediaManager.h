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

#include "MediaSource.h" // for VECSOURCES
#include <map>
#ifdef HAS_DVD_DRIVE
#include "FileSystem/cdioSupport.h"
#endif
#include "URL.h"
#include "utils/CriticalSection.h"
#include "IStorageProvider.h"

#ifdef HAS_DVD_DRIVE
using namespace MEDIA_DETECT;
#endif

class CNetworkLocation
{
public:
  CNetworkLocation() { id = 0; };
  int id;
  CStdString path;
};

class CMediaManager
{
public:

  static CMediaManager& GetInstance();

  bool LoadSources();
  bool SaveSources();

  void GetLocalDrives(VECSOURCES &localDrives, bool includeQ = true);
  void GetRemovableDrives(VECSOURCES &removableDrives);
  void GetNetworkLocations(VECSOURCES &locations);

  bool AddNetworkLocation(const CStdString &path);
  bool HasLocation(const CStdString& path) const;
  bool RemoveLocation(const CStdString& path);
  bool SetLocationPath(const CStdString& oldPath, const CStdString& newPath);

  void AddAutoSource(const CMediaSource &share, bool bAutorun=false);
  void RemoveAutoSource(const CMediaSource &share);
  bool IsDiscInDrive(const CStdString& devicePath="");
  bool IsAudio(const CStdString& devicePath="");
  CStdString TranslateDevicePath(const CStdString& devicePath, bool bReturnAsDevice=false);
  DWORD GetDriveStatus(const CStdString& devicePath="");
#ifdef HAS_DVD_DRIVE
  CCdInfo* GetCdInfo(const CStdString& devicePath="");
  bool RemoveCdInfo(const CStdString& devicePath="");
#endif
  CStdString GetDiskLabel(const CStdString& devicePath="");
  void SetHasOpticalDrive(bool bstatus);

  void ProcessEvents();

  std::vector<CStdString> GetDiskUsage();
protected:
  CMediaManager();

  std::vector<CNetworkLocation> m_locations;

  CCriticalSection m_muAutoSource;
#ifdef HAS_DVD_DRIVE
  std::map<CStdString,CCdInfo*> m_mapCdInfo;
#endif
  bool m_bhasoptical;

private:
  IStorageProvider *m_platformStorage;
};

#define g_mediaManager CMediaManager::GetInstance()

