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


#include "WINSMBDirectory.h"
#include "Util.h"
#include "URL.h"
#include "GUISettings.h"
#include "FileItem.h"
#include "WIN32Util.h"
#include "AutoPtrHandle.h"
#include "log.h"
#include "CharsetConverter.h"
#include "BrowserService.h"
#include "Application.h"
#include "DirectoryCache.h"
#include "SMBUtils.h"

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) -1)
#endif


using namespace AUTOPTR;
using namespace DIRECTORY;

CWINSMBDirectory::CWINSMBDirectory(void)
{
  m_bHost=false;
}

CWINSMBDirectory::~CWINSMBDirectory(void)
{
}

CStdString CWINSMBDirectory::GetLocal(const CStdString& strPath)
{
  CURI url(strPath);
  CStdString path( url.GetFileName() );
  if( url.GetProtocol().Equals("smb", false) )
  {
    CStdString host( url.GetHostName() );

    if(host.size() > 0)
    {
      path = "//" + host + "/" + path;
    }
  }
  path.Replace('/', '\\');
  return path;
}

bool IsCredentialsError(DWORD dwRet) {
	return ((dwRet == ERROR_ACCESS_DENIED) || (dwRet == ERROR_INVALID_PASSWORD) || (dwRet == ERROR_LOGON_FAILURE) || (dwRet == ERROR_LOGON_TYPE_NOT_GRANTED));
}

bool CWINSMBDirectory::GetDirectory(const CStdString& strPath1, CFileItemList &items)
{
	if(SMBUtils::ProcessPath(strPath1, items, this->m_allowPrompting))
  {
    return true;
  }

  CBrowserService* pBrowser = g_application.GetBrowserService();
  if( strPath1 == "smb://" && pBrowser )
  {
    pBrowser->GetShare( CBrowserService::SMB_SHARE, items );    
    return true;
  }

  // If we have the items in the cache, return them
	if (g_directoryCache.GetDirectory(strPath1, items)) 
		return true;
  
  if (pBrowser)
  {
    CURI u(strPath1);
    if (!pBrowser->IsHostAvailable(u.GetHostName()))
    {
      CLog::Log(LOGDEBUG,"Host <%s> not available according to browser-service",u.GetHostName().c_str());
      return false;
    }
  }

  WIN32_FIND_DATAW wfd;
  CStdString strPath=strPath1;

  CURI url(strPath);

  if(url.GetShareName().empty())
  {
    LPNETRESOURCEW lpnr = NULL;
    bool ret;
    if(!url.GetHostName().empty())
    {
      lpnr = (LPNETRESOURCEW) GlobalAlloc(GPTR, 16384);
      if(lpnr == NULL)
        return false;

      ConnectToShare(url);
      CStdString strHost = "\\\\" + url.GetHostName();
      CStdStringW strHostW;
      g_charsetConverter.utf8ToW(strHost,strHostW);
      lpnr->lpRemoteName = (LPWSTR)strHostW.c_str();
      m_bHost = true;
	  bool need_creds=false;
      ret = EnumerateFunc(lpnr, items, &need_creds);
	  if(need_creds)	// In-order that the item-loader would pop the password dialog the next time.
		  items.SetProperty("getcredentials", true);
      GlobalFree((HGLOBAL) lpnr);
      m_bHost = false;
    }
    else
      ret = EnumerateFunc(lpnr, items);  
 
    return ret; 
  }

  memset(&wfd, 0, sizeof(wfd));
  //rebuild the URL
  CStdString strUNCShare = "\\\\" + url.GetHostName() + "\\" + url.GetFileName();
  strUNCShare.Replace("/", "\\");
  if(!CUtil::HasSlashAtEnd(strUNCShare))
    strUNCShare.append("\\");

  CStdStringW strSearchMask;
  g_charsetConverter.utf8ToW(strUNCShare, strSearchMask, false); 
  strSearchMask += "*";

  FILETIME localTime;
  CAutoPtrFind hFind ( FindFirstFileW(strSearchMask.c_str(), &wfd));
  
  // on error, check if path exists at all, this will return true if empty folder
  if (!hFind.isValid()) 
  {
    if(IsCredentialsError(GetLastError()))
    {
		bool need_creds=false;
      if(ConnectToShare(url, &need_creds) == false) {
		if(need_creds)	// In-order that the item-loader would pop the password dialog the next time.
		  items.SetProperty("getcredentials", true);
        return false;
	  }
      hFind.attach(FindFirstFileW(strSearchMask.c_str(), &wfd));
    }
    else
      return Exists(strPath1);
  }

  if (hFind.isValid())
  {
    do
    {
      if (wfd.cFileName[0] != 0)
      {
        CStdString strLabel;
        g_charsetConverter.wToUTF8(wfd.cFileName,strLabel);
        if ( (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
          if (strLabel != "." && strLabel != "..")
          {
            CFileItemPtr pItem(new CFileItem(strLabel));
            pItem->m_strPath = strPath;
            CUtil::AddSlashAtEnd(pItem->m_strPath);
            pItem->m_strPath += strLabel;
            pItem->m_bIsFolder = true;
            CUtil::AddSlashAtEnd(pItem->m_strPath);
            FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &localTime);
            pItem->m_dateTime=localTime;

            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
              pItem->SetProperty("file:hidden", true);

              items.Add(pItem);
          }
        }
        else
        {
          CFileItemPtr pItem(new CFileItem(strLabel));
          pItem->m_strPath = strPath;
          CUtil::AddSlashAtEnd(pItem->m_strPath);
          pItem->m_strPath += strLabel;
          pItem->m_bIsFolder = false;
          pItem->m_dwSize = CUtil::ToInt64(wfd.nFileSizeHigh, wfd.nFileSizeLow);
          FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &localTime);
          pItem->m_dateTime=localTime;

          if (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            pItem->SetProperty("file:hidden", true);
            items.Add(pItem);
          }
        }
      }
    while (FindNextFileW((HANDLE)hFind, &wfd));
  }
  return true;
}

bool CWINSMBDirectory::Create(const char* strPath)
{
  CStdString strPath1 = GetLocal(strPath);
  CStdStringW strWPath1;
  g_charsetConverter.utf8ToW(strPath1, strWPath1, false);
  if(::CreateDirectoryW(strWPath1, NULL))
    return true;
  else if(GetLastError() == ERROR_ALREADY_EXISTS)
    return true;

  return false;
}

bool CWINSMBDirectory::Remove(const char* strPath)
{
  CStdStringW strWPath;
  CStdString strPath1 = GetLocal(strPath);
  g_charsetConverter.utf8ToW(strPath1, strWPath, false);
  return ::RemoveDirectoryW(strWPath) ? true : false;
}

bool CWINSMBDirectory::Exists(const char* strPath)
{
  CStdString strReplaced=GetLocal(strPath);
  CStdStringW strWReplaced;
  g_charsetConverter.utf8ToW(strReplaced, strWReplaced, false);
  // this will fail on shares, needs a subdirectory inside a share
  DWORD attributes = GetFileAttributesW(strWReplaced);
  if(attributes == INVALID_FILE_ATTRIBUTES)
    return false;
  if (FILE_ATTRIBUTE_DIRECTORY & attributes) 
    return true;
  return false;
}

bool CWINSMBDirectory::EnumerateFunc(LPNETRESOURCEW lpnr, CFileItemList &items, bool *need_creds /*=NULL*/)
{
  DWORD dwResult, dwResultEnum;
  HANDLE hEnum;
  DWORD cbBuffer = 16384;     // 16K is a good size
  LPNETRESOURCEW lpnrLocal;    // pointer to enumerated structures
  DWORD cEntries = -1;        // enumerate all possible entries
  DWORD i;

  if(need_creds != NULL)
	  (*need_creds)=false;
  //
  // Call the WNetOpenEnum function to begin the enumeration.
  //
  dwResult = WNetOpenEnumW(RESOURCE_GLOBALNET, // all network resources
                            RESOURCETYPE_DISK,   // all disk resources
                          0,  // enumerate all resources
                          lpnr,       // NULL first time the function is called
                          &hEnum);    // handle to the resource

  if (dwResult != NO_ERROR) 
  {
	  CLog::Log(LOGWARNING,"WnetOpenEnum failed with code %d. Network path is: %ws", dwResult, lpnr->lpRemoteName);
	if(IsCredentialsError(dwResult) && (need_creds != NULL))
	  (*need_creds)=true;
    if(dwResult == ERROR_EXTENDED_ERROR)
    {
      DWORD dwWNetResult, dwLastError;   
      CHAR szDescription[256]; 
      CHAR szProvider[256]; 
      dwWNetResult = WNetGetLastError(&dwLastError, // error code
                            (LPSTR) szDescription,  // buffer for error description 
                            sizeof(szDescription),  // size of error buffer
                            (LPSTR) szProvider,     // buffer for provider name 
                            sizeof(szProvider));    // size of name buffer
      if(dwWNetResult == NO_ERROR) 
        CLog::Log(LOGWARNING,"%s failed with code %ld; %s", szProvider, dwLastError, szDescription);
    }
    return false;
  }
  //
  // Call the GlobalAlloc function to allocate resources.
  //
  lpnrLocal = (LPNETRESOURCEW) GlobalAlloc(GPTR, cbBuffer);
  if (lpnrLocal == NULL) 
  {
    CLog::Log(LOGERROR,"Can't allocate buffer %d", cbBuffer);
    return false;
  }

  do 
  {
    //
    // Initialize the buffer.
    //
    ZeroMemory(lpnrLocal, cbBuffer);
    //
    // Call the WNetEnumResource function to continue
    //  the enumeration.
    //
    dwResultEnum = WNetEnumResourceW(hEnum,  // resource handle
                                    &cEntries,      // defined locally as -1
                                    lpnrLocal,      // LPNETRESOURCE
                                    &cbBuffer);     // buffer size
    //
    // If the call succeeds, loop through the structures.
    //
    if (dwResultEnum == NO_ERROR) 
    {
      for (i = 0; i < cEntries; i++) 
      {
        DWORD dwDisplayType = lpnrLocal[i].dwDisplayType;
        DWORD dwType = lpnrLocal[i].dwType;

        if((((dwDisplayType == RESOURCEDISPLAYTYPE_SERVER) && (m_bHost == false)) || 
           ((dwDisplayType == RESOURCEDISPLAYTYPE_SHARE) && m_bHost)) &&
           (dwType != RESOURCETYPE_PRINT))
        {
          CStdString strurl = "smb:";
          CStdStringW strNameW = lpnrLocal[i].lpComment;
          CStdStringW strRemoteNameW = lpnrLocal[i].lpRemoteName;
          CStdString  strName,strRemoteName;

          g_charsetConverter.wToUTF8(strRemoteNameW,strRemoteName);
          g_charsetConverter.wToUTF8(strNameW,strName);
          CLog::Log(LOGDEBUG,"Found Server/Share: %s", strRemoteName.c_str());

          strurl.append(strRemoteName);
          strurl.Replace("\\","/");
          CURI rooturl(strurl);
          rooturl.SetFileName("");

          if(strName.empty())
          {
            if(!rooturl.GetShareName().empty())
              strName = rooturl.GetShareName();
            else
              strName = rooturl.GetHostName();

            strName.Replace("\\","");
          }

          CFileItemPtr pItem(new CFileItem(strName));        
          pItem->m_strPath = strurl;
          CUtil::AddSlashAtEnd(pItem->m_strPath);
          pItem->m_bIsFolder = true;
            items.Add(pItem);
        }

        // If the NETRESOURCE structure represents a container resource, 
        //  call the EnumerateFunc function recursively.
        if (RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER))
          EnumerateFunc(&lpnrLocal[i], items);
      }
    }
    // Process errors.
    //
    else if (dwResultEnum != ERROR_NO_MORE_ITEMS) 
    {
      CLog::Log(LOGERROR,"WNetEnumResource failed with error %d", dwResultEnum);
      break;
    }
  }
  //
  // End do.
  //
  while (dwResultEnum != ERROR_NO_MORE_ITEMS);
  //
  // Call the GlobalFree function to free the memory.
  //
  GlobalFree((HGLOBAL) lpnrLocal);
  //
  // Call WNetCloseEnum to end the enumeration.
  //
  dwResult = WNetCloseEnum(hEnum);

  if (dwResult != NO_ERROR) 
  {
      //
      // Process errors.
      //
      CLog::Log(LOGERROR,"WNetCloseEnum failed with error %d", dwResult);
      return false;
  }

  return true;
}

bool CWINSMBDirectory::ConnectToShare(const CURI& url, bool *need_creds /*=NULL*/)
{
  NETRESOURCE nr;
  CURI urlIn(url);
  DWORD dwRet=-1;
  CStdString strUNC("\\\\"+url.GetHostName());
  if(!url.GetShareName().empty())
    strUNC.append("\\"+url.GetShareName());

  CStdString strPath;
  memset(&nr,0,sizeof(nr));
  nr.dwType = RESOURCETYPE_ANY;
  nr.lpRemoteName = (char*)strUNC.c_str();

  if(need_creds != NULL)
	  (*need_creds) = false;

  // in general we shouldn't need the password manager as we won't disconnect from shares yet
  IMAPPASSWORDS it = g_passwordManager.m_mapSMBPasswordCache.find(strUNC);
  if(it != g_passwordManager.m_mapSMBPasswordCache.end())
  {
    // if share found in cache use it to supply username and password
    CURI murl(it->second);		// map value contains the full url of the originally authenticated share. map key is just the share
    CStdString strPassword = murl.GetPassWord();
    CStdString strUserName = murl.GetUserName();
    urlIn.SetPassword(strPassword);
    urlIn.SetUserName(strUserName);
  }
  else if(urlIn.GetUserNameA().empty() && !g_guiSettings.GetString("smb.username").IsEmpty())
  {
    urlIn.SetPassword(g_guiSettings.GetString("smb.password"));
    urlIn.SetUserName(g_guiSettings.GetString("smb.username"));
  }

  CStdString strAuth = URLEncode(urlIn);

  while(dwRet != NO_ERROR)
  {
    strPath = URLEncode(urlIn);
    LPCTSTR pUser = urlIn.GetUserNameA().empty() ? NULL : (LPCTSTR)urlIn.GetUserNameA().c_str();
    LPCTSTR pPass = urlIn.GetPassWord().empty() ? NULL : (LPCTSTR)urlIn.GetPassWord().c_str();
	CLog::Log(LOGDEBUG,"Trying to connect to %s with username(%s) and password(%s)", strUNC.c_str(), urlIn.GetUserNameA().c_str(), urlIn.GetPassWord().c_str());
    dwRet = WNetAddConnection2(&nr, pPass, pUser, CONNECT_TEMPORARY);
    if(IsCredentialsError(dwRet))
    {
      CLog::Log( LOGDEBUG, "Couldn't connect to %s, access denied", strUNC.c_str() ) ;
      if (m_allowPrompting)
      {
        g_passwordManager.SetSMBShare(strPath);
        if (!g_passwordManager.GetSMBShareUserPassword())  // Do this bit via a threadmessage?
        	break;

        CURI urlnew( g_passwordManager.GetSMBShare() );
        urlIn.SetUserName(urlnew.GetUserName());
        urlIn.SetPassword(urlnew.GetPassWord());
      }
      else {
		if(need_creds != NULL)
			(*need_creds) = true;
        break;
	  }
    }
    else if(dwRet == ERROR_SESSION_CREDENTIAL_CONFLICT)
    {
      DWORD dwRet2=-1;
      CStdString strRN = nr.lpRemoteName;
      do
      {
        dwRet2 = WNetCancelConnection2((LPCSTR)strRN.c_str(), 0, false);
        strRN.erase(strRN.find_last_of("\\"),CStdString::npos);
      } 
      while(dwRet2 == ERROR_NOT_CONNECTED && !strRN.Equals("\\\\"));
    }
    else if(dwRet != NO_ERROR)
    {
      break;
    }
	else {	// dwRet == NO_ERROR
		// When you have a non-password protected share with a password-protected directory you're trying to access, WNetAddConnection2() would return NO_ERROR, even-though credentials are still needed.
		// We need to check if new credentials are needed, by trying to open a handle to the UNC directory and see if we get a credetials-related error.
		HANDLE dummy = CreateFile(nr.lpRemoteName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if(INVALID_HANDLE_VALUE == dummy) {
			dwRet = GetLastError();
			if(IsCredentialsError(dwRet)) {
				if (m_allowPrompting)
				  {
					g_passwordManager.SetSMBShare(strPath);
					if (!g_passwordManager.GetSMBShareUserPassword())  // Do this bit via a threadmessage?
        				break;

					CURI urlnew( g_passwordManager.GetSMBShare() );
					CLog::Log(LOGDEBUG,"Disconnecting from: %s: invalid username(%s) or password(%s)", nr.lpRemoteName, urlIn.GetUserNameA().c_str(), urlIn.GetPassWord().c_str());
					
					CStdString strRN = nr.lpRemoteName;
					DWORD dwRet2=0;
				  do
				  {
					  // Need to disconnect recursively from all the UNC paths in this share, in-order to connect again with a different user-name and password in the next iteration of the "big loop".
					dwRet2 = WNetCancelConnection2((LPCSTR)strRN.c_str(), 0, false);
					strRN.erase(strRN.find_last_of("\\"),CStdString::npos);
				  } 
				  while((dwRet2 != ERROR_NOT_CONNECTED) && !strRN.Equals("\\\\"));

					urlIn.SetUserName(urlnew.GetUserName());
					urlIn.SetPassword(urlnew.GetPassWord());
				  }
				  else {	// m_allowPrompting
					if(need_creds != NULL)
						(*need_creds) = true;
					break;
				  }
			}
			else	// IsCredentialsError()
				dwRet = NO_ERROR;	// Return the original value, so we won't break the functionality by changing the return code from true to false. Note that if we're trying to access a computer (= \\ip-addr) - then GetLastError()=161 (invalid path).
		}
		else	// INVALID_HANDLE_VALUE == dummy
			CloseHandle(dummy);
	}
  }

  if(dwRet != NO_ERROR)
  {
    CLog::Log( LOGDEBUG, "Couldn't connect to %s, error code %d", strUNC.c_str(), dwRet );
    return false;
  }
  else if (strPath != strAuth && !strUNC.IsEmpty()) // we succeeded so, if path was changed, return the correct one and cache it
  {
    g_passwordManager.m_mapSMBPasswordCache[strUNC] = strPath;
  }  
  return true;
}

CStdString CWINSMBDirectory::URLEncode(const CURI &url)
{
  /* due to smb wanting encoded urls we have to build it manually */

  CStdString flat = "smb://";

  /* samba messes up of password is set but no username is set. don't know why yet */
  /* probably the url parser that goes crazy */
  if(url.GetUserName().length() > 0 /* || url.GetPassWord().length() > 0 */)
  {
    flat += url.GetUserName();
    flat += ":";
    flat += url.GetPassWord();
    flat += "@";
  }
  else if( !url.GetHostName().IsEmpty() && !g_guiSettings.GetString("smb.username").IsEmpty() )
  {
    /* okey this is abit uggly to do this here, as we don't really only url encode */
    /* but it's the simplest place to do so */
    flat += g_guiSettings.GetString("smb.username");
    flat += ":";
    flat += g_guiSettings.GetString("smb.password");
    flat += "@";
  }

  flat += url.GetHostName();

  /* okey sadly since a slash is an invalid name we have to tokenize */
  std::vector<CStdString> parts;
  std::vector<CStdString>::iterator it;
  CUtil::Tokenize(url.GetFileName(), parts, "/");
  for( it = parts.begin(); it != parts.end(); it++ )
  {
    flat += "/";
    flat += (*it);
  }

  /* okey options should go here, thou current samba doesn't support any */

  return flat;
}