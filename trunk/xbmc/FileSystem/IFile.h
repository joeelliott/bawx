/*
* XBMC Media Center
* Copyright (c) 2002 Frodo
* Portions Copyright (c) by the authors of ffmpeg and xvid
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
// IFile.h: interface for the IFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_)
#define AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _LINUX
#include "PlatformDefs.h" // for __stat64
#endif

#include "URL.h"

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

#define SEEK_POSSIBLE 0x10 // flag used to check if protocol allows seeks

namespace XFILE
{

class ICacheInterface;

class IFile
{
public:
  IFile();
  virtual ~IFile();

  virtual bool Open(const CURI& url) = 0;
  virtual bool OpenForWrite(const CURI& url, bool bOverWrite = false) { return false; };
  virtual bool Exists(const CURI& url) = 0;
  virtual bool HasFileError(CStdString* error) { return false; };  
  virtual int Stat(const CURI& url, struct __stat64* buffer) = 0;
  virtual int Stat(struct __stat64* buffer);
  virtual unsigned int Read(void* lpBuf, int64_t uiBufSize) = 0;
  virtual int Write(const void* lpBuf, int64_t uiBufSize) { return -1;};
  virtual bool ReadString(char *szLine, int iLineLength);
  virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET) = 0;
  virtual void Close() = 0;
  virtual int64_t GetPosition() = 0;
  virtual int64_t GetLength() = 0;
  virtual void Flush() { }
  /* Returns the minium size that can be read from input stream.   *
   * For example cdrom access where access could be sector based.  *
   * This will cause file system to buffer read requests, to       *
   * to meet the requirement of CFile.                             *
   * It can also be used to indicate a file system is non buffered *
   * but accepts any read size, have it return the value 1         */

  virtual int  GetChunkSize() {return 0;}
  virtual bool SkipNext(){return false;}

  virtual bool Delete(const CURI& url) { return false; }
  virtual bool Rename(const CURI& url, const CURI& urlnew) { return false; }

  virtual ICacheInterface* GetCache() {return NULL;} 
  virtual int IoControl(int request, void* param) { return -1; }

  virtual CStdString GetContent()                            { return "application/octet-stream"; }
  virtual __int64	GetAvailableRead() { return 0; }

protected:
  bool m_bSkipEol;
};

class CRedirectException
{
public:
  IFile *m_pNewFileImp;

  CRedirectException() : m_pNewFileImp(NULL) { }
  CRedirectException(IFile *pNewFileImp) : m_pNewFileImp(pNewFileImp) { }
};

}

#endif // !defined(AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_)


