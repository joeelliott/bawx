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

// FilePipe.h: interface for the CFilePipe class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEPIPE_H__DD2B0A9E_4971_4A29_B525_78CEFCDAF4A1__INCLUDED_)
#define AFX_FILEPIPE_H__DD2B0A9E_4971_4A29_B525_78CEFCDAF4A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IFile.h"
#include "AutoPtrHandle.h"
#include "StdString.h"
#include "Event.h"
#include "utils/CriticalSection.h"
#include "RingBuffer.h"
#include "PipesManager.h"

namespace XFILE
{
  
class CFilePipe : public IFile, public IPipeListener
{
public:
  CFilePipe();
  virtual ~CFilePipe();
  virtual int64_t GetPosition();
  virtual int64_t GetLength();
  virtual void SetLength(int64_t len);
  virtual bool Open(const CURI& url);
  virtual bool Exists(const CURI& url);
  virtual int Stat(const CURI& url, struct __stat64* buffer);
  virtual int Stat(struct __stat64* buffer);
  virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
  virtual int Write(const void* lpBuf, int64_t uiBufSize);
  virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
  virtual void Close();
  virtual void Flush();
  virtual __int64	GetAvailableRead();

  virtual bool OpenForWrite(const CURI& url, bool bOverWrite = false);

  virtual bool Delete(const CURI& url);
  virtual bool Rename(const CURI& url, const CURI& urlnew);
  virtual int IoControl(int request, void* param);
  
  CStdString GetName() const;
  
  virtual void OnPipeOverFlow();
  virtual void OnPipeUnderFlow();

  void AddListener(IPipeListener *l);
  void RemoveListener(IPipeListener *l);

  void SetEof();
  bool IsEof();
  bool IsEmpty();
  bool IsClosed();
  
  void SetOpenThreashold(int threashold);

protected:
  int64_t m_pos;
  int64_t m_length;
  
  XFILE::Pipe *m_pipe;
  
  CCriticalSection m_lock;
  std::vector<XFILE::IPipeListener *> m_listeners;
};

}
#endif // !defined(AFX_FILEPIPE_H__DD2B0A9E_4971_4A29_B525_78CEFCDAF4A1__INCLUDED_)
