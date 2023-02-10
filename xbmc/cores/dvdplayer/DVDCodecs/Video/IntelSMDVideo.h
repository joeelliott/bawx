#pragma once
/*
 *      Copyright (C) 2005-2009 Team XBMC
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

#if defined(HAS_INTEL_SMD)

#include "libavcodec/avcodec.h"
#include <ismd_core.h>
#include <ismd_viddec.h>
#include <deque>
#include <vector>

#include "DVDVideoCodec.h"
#include "utils/Thread.h"

struct H264BSFContext {
  uint8_t  length_size;
  uint8_t  first_idr;
  uint8_t *sps_pps_data;
  uint32_t size;
} ;


#define MAX_SIZE_PES_SEQUENCE_HEADER 16

typedef struct H264Viddec {
    ismd_codec_type_t    vCodec;
    // H.264 header information extracted from caps
    unsigned char              *h264_codec_priv_pushed_data_ptr;
    unsigned int                h264_priv_len;
    unsigned char               sizeOfNALULength;
    bool             first;
    // For FOURCC headers, PVT data is not available
    bool           pvt_data_avail;
    // For seek to work properly
    unsigned long long        ptsOffset;
    unsigned long long        lastFlipPts;
    unsigned long long        calculated_offset;

    // AVI: SPS/PPS
    unsigned char     *avi_sps_pps;
    unsigned int        sps_pps_len;
}H264Viddec;

typedef struct Vc1Viddec {
  ismd_codec_type_t    vCodec;

  // size of the buffer is:
  // start code = 4
  // sequence header = 2 + 2 + 4 = 8 ==> ((8+3)/3)*4 == maximum 12 after encapsulation.
  unsigned char SPMP_PESpacket_PayloadFormatHeader[MAX_SIZE_PES_SEQUENCE_HEADER]; // max size of a sequence header
  unsigned int size_SPMP_PESpacket_PayloadFormatHeader; // actual size
  int flag; // boolean indicates if sequence header should be sent in next buffer
  unsigned char *AP_Buffer;
  unsigned int size_AP_Buffer;
  unsigned char *SPMP_EndOfSequence;
} Vc1Viddec;



////////////////////////////////////////////////////////////////////////////////////////////

#define INTEL_SMD_FIELD_FULL        0x00
#define INTEL_SMD_FIELD_EVEN        0x01
#define INTEL_SMD_FIELD_ODD         0x02

#define INPUT_LIST_SIZE 20

class CIntelSMDVideo
{
public:
  static void RemoveInstance(void);
  static CIntelSMDVideo* GetInstance(void);

  bool OpenDecoder(CodecID ffmpegCodedId, ismd_codec_type_t codec_type, int extradata_size, void *extradata);
  void CloseDecoder(void);
  void SetDefaults();

  void Pause();
  void Resume();
  void Reset();
  void Resync(double pts) {}

  bool AddInput(unsigned char *pData, size_t size, double dts, double pts);

  bool GetPicture(DVDVideoPicture* pDvdVideoPicture);
  void SetDropState(bool bDrop);

  void SetWidth(unsigned int width) { m_width = m_dwidth = width; }
  void SetHeight(unsigned int height) { m_height = m_dheight = height; }

  int WriteToInputPort(unsigned char* data, unsigned int length, double pts, int bufSize);
  int WriteToRenderer();
  unsigned char *Vc1_viddec_convert_AP (Vc1Viddec *viddec, unsigned char *buf,int size, bool need_seq_hdr,unsigned int *outbuf_size);
  unsigned char *Vc1_viddec_convert_SPMP (Vc1Viddec *viddec, unsigned char *buf, int size, bool need_seq_hdr, unsigned int *outbuf_size);\
  int vc1_viddec_encapsulate_and_write_ebdu ( unsigned char* pDes, unsigned int SizeDes, unsigned char* pRbdu, unsigned int SizeRBDU );
  void vc1_viddec_SPMP_PESpacket_PayloadFormatHeader (Vc1Viddec *viddec, unsigned char *pCodecData, int width, int height);
  bool vc1_viddec_init (Vc1Viddec *viddec);

  bool GetInputPortStatus(unsigned int& curDepth, unsigned int& maxDepth);
  void DisablePtsCorrection(bool bDisable) { m_bDisablePtsCorrection = bDisable; }

protected:
  virtual ~CIntelSMDVideo();
  bool CheckStartRunning();

  bool          m_IsConfigured;
  bool          m_drop_state;
  unsigned int  m_OutputTimeout;
  unsigned int  m_field;
  unsigned int  m_width;
  unsigned int  m_height;
  unsigned int  m_dwidth;
  unsigned int  m_dheight;
  Vc1Viddec       m_vc1_converter;
  H264Viddec      m_H264_converter;
  ismd_codec_type_t m_codec_type;
  CodecID         m_ffmpegCodedId;
  bool            m_bNeedWMV3Conversion;
  bool            m_bNeedVC1Conversion;
  bool            m_bNeedH264Conversion;

  bool m_bRunning;
  bool m_bDiscontinuity;
  bool m_bFlushFlag;
  ismd_pts_t m_lastPTS;
  ismd_pts_t m_ptsCorrection;

  bool m_bDisablePtsCorrection;

private:
  CIntelSMDVideo();
  static CIntelSMDVideo *m_pInstance;
};

#endif
