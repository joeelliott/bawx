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

#include "system.h" // for HAS_DVD_DRIVE

#ifdef HAS_DVD_DRIVE

#include "IFile.h"
#include "cdioSupport.h"

namespace XFILE
{
class CFileCDDA : public IFile
{
public:
  CFileCDDA(void);
  virtual ~CFileCDDA(void);
  virtual bool Open(const CURI& url);
  virtual bool Exists(const CURI& url);
  virtual int Stat(const CURI& url, struct __stat64* buffer);

  virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
  virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
  virtual void Close();
  virtual int64_t GetPosition();
  virtual int64_t GetLength();
  virtual int GetChunkSize();

protected:
  bool IsValidFile(const CURI& url);
  int GetTrackNum(const CURI& url);

protected:
  CdIo_t* m_pCdIo;
  lsn_t m_lsnStart;  // Start of m_iTrack in logical sector number
  lsn_t m_lsnCurrent; // Position inside the track in logical sector number
  lsn_t m_lsnEnd;   // End of m_iTrack in logical sector number
  MEDIA_DETECT::CLibcdio* m_cdio;
};
}

#endif

