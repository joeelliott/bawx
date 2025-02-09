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

#include "Texture.h"
#include "Picture.h"
#include "WindowingFactory.h"
#include "utils/log.h"
#include "DllImageLib.h"
#include "DDSImage.h"
#include "Util.h"

#ifndef NO_XBMC_FILESYSTEM
#include "FileSystem/File.h"
using namespace XFILE;
#else
#include "SimpleFS.h"
#endif

/************************************************************************/
/*                                                                      */
/************************************************************************/
CBaseTexture::CBaseTexture(unsigned int width, unsigned int height, unsigned int format)
{
#ifndef HAS_DX 
  m_texture = 0; 
#endif
  m_textureOwner = true;
  m_pixels = NULL;
  m_loadedToGPU = false;
  m_imageInitialWidth = 0;
  m_imageInitialHeight = 0;

  if (width > 0 && height > 0)
  {
    Allocate(width, height, format);
  }
}

bool CBaseTexture::LoadFromTexture(unsigned int width, unsigned int height, unsigned int format, XBMC::TexturePtr texture)
{
#ifndef HAS_DX
  if (m_pixels)
  {
    delete [] m_pixels;
  }
  m_texture = texture;
  m_textureOwner = false;
  m_pixels = NULL;
  m_loadedToGPU = true;
  m_imageWidth = m_textureWidth = width;
  m_imageHeight = m_textureHeight = height;
  m_orientation = 0;
#endif
  
  return true;
}

CBaseTexture::~CBaseTexture()
{
  if (m_pixels)
  {
    delete[] m_pixels;
    m_pixels = NULL;
  }
}
  
void CBaseTexture::Allocate(unsigned int width, unsigned int height, unsigned int format)
{
  m_imageWidth = width;
  m_imageHeight = height;
  m_format = format;
  m_orientation = 0;

  if (!g_Windowing.SupportsNPOT((m_format & XB_FMT_DXT_MASK) != 0))
  {
    m_textureWidth = PadPow2(m_imageWidth);
    m_textureHeight = PadPow2(m_imageHeight);
}
  else
  {
    m_textureWidth = m_imageWidth;
    m_textureHeight = m_imageHeight;
  }
  if (m_format & XB_FMT_DXT_MASK)
  { // DXT textures must be a multiple of 4 in width and height
    m_textureWidth = ((m_textureWidth + 3) / 4) * 4;
    m_textureHeight = ((m_textureHeight + 3) / 4) * 4;
}

  // check for max texture size
  #define CLAMP(x, y) { if (x > y) x = y; }
  CLAMP(m_textureWidth, g_Windowing.GetMaxTextureSize());
  CLAMP(m_textureHeight, g_Windowing.GetMaxTextureSize());
  CLAMP(m_imageWidth, m_textureWidth);
  CLAMP(m_imageHeight, m_textureHeight);

  if (m_pixels)
  {
    delete[] m_pixels;
    m_pixels = NULL;
  }

  m_pixels = new unsigned char[GetPitch() * GetRows()];
  memset(m_pixels, 0, GetPitch() * GetRows());
}

void CBaseTexture::Update(unsigned int width, unsigned int height, unsigned int pitch, unsigned int format, const unsigned char *pixels, bool loadToGPU) 
{
  if (pixels == NULL)
    return;
 
  Allocate(width, height, format);

  unsigned int srcPitch = pitch ? pitch : GetPitch(width);
  unsigned int srcRows = GetRows(height);
  unsigned int dstPitch = GetPitch(m_textureWidth);
  unsigned int dstRows = GetRows(m_textureHeight);

  if (srcPitch == dstPitch)
    memcpy(m_pixels, pixels, srcPitch * std::min(srcRows, dstRows));
  else
  {
    const unsigned char *src = pixels;
    unsigned char* dst = m_pixels;
    for (unsigned int y = 0; y < srcRows && y < dstRows; y++)
    {
      memcpy(dst, src, std::min(srcPitch, dstPitch));
      src += srcPitch;
      dst += dstPitch;
    }
  }
  ClampToEdge();

  if (loadToGPU)
    LoadToGPU();
}

void CBaseTexture::ClampToEdge()
  {
  unsigned int imagePitch = GetPitch(m_imageWidth);
  unsigned int imageRows = GetRows(m_imageHeight);
  unsigned int texturePitch = GetPitch(m_textureWidth);
  unsigned int textureRows = GetRows(m_textureHeight);
  if (imagePitch < texturePitch)
  {
    unsigned int blockSize = GetBlockSize();
    unsigned char *src = m_pixels + imagePitch - blockSize;
    unsigned char *dst = m_pixels;
    for (unsigned int y = 0; y < imageRows; y++)
    {
      for (unsigned int x = imagePitch; x < texturePitch; x += blockSize)
        memcpy(dst + x, src, blockSize);
      dst += texturePitch;
  }
  }

  if (imageRows < textureRows)
  {
    unsigned char *dst = m_pixels + imageRows * texturePitch;
    for (unsigned int y = imageRows; y < textureRows; y++)
    {
      memcpy(dst, dst - texturePitch, texturePitch);
      dst += texturePitch;
  }
}
}

bool CBaseTexture::LoadFromFile(const CStdString& texturePath, unsigned int maxWidth, unsigned int maxHeight,
                                bool autoRotate, unsigned int *originalWidth, unsigned int *originalHeight,
                                unsigned int dstWidth, unsigned int dstHeight)
{
  //Quick and dirty check if the file is DDS file...

  CFile file;
  if (!file.Open(texturePath))
  {  
    CLog::Log(LOGERROR, "%s - Error opening texture file: %s", __FUNCTION__, texturePath.c_str());
    return false;
  }

  // read the header
  uint32_t magic;
  if (file.Read(&magic, 4) != 4)
  {
    CLog::Log(LOGERROR, "%s - Can't read signature from file: %s", __FUNCTION__, texturePath.c_str());
    return false;
  }

  if (strncmp((const char *)&magic, "DDS ", 4) == 0 || CUtil::GetExtension(texturePath).Equals(".dds"))
  { // special case for DDS images
    CDDSImage image;
    CLog::Log(LOGDEBUG, "%s - loading dds file: %s", __FUNCTION__, texturePath.c_str());
    image.ReadFile(texturePath);
    Update(image.GetWidth(), image.GetHeight(), 0, image.GetFormat(), image.GetData(), false);
    return true;
  }

  DllImageLib dll;
  if (!dll.Load())
    return false;

  ImageInfo image;
  memset(&image, 0, sizeof(image));
  //image.width = dstWidth;
  //image.height = dstHeight;

  unsigned int width = maxWidth ? std::min(maxWidth, g_Windowing.GetMaxTextureSize()) : g_Windowing.GetMaxTextureSize();
  unsigned int height = maxHeight ? std::min(maxHeight, g_Windowing.GetMaxTextureSize()) : g_Windowing.GetMaxTextureSize();

  if(!dll.LoadImage(texturePath.c_str(), width, height, &image))
  {
    CLog::Log(LOGERROR, "Texture manager unable to load file: %s", texturePath.c_str());
    return false;
  }

  Allocate(image.width, image.height, XB_FMT_B8G8R8A8);
  if (autoRotate && image.exifInfo.Orientation)
    m_orientation = image.exifInfo.Orientation - 1;
  if (originalWidth)
    *originalWidth = image.originalwidth;
  if (originalHeight)
    *originalHeight = image.originalheight;

  unsigned int destPitch = GetPitch();
  unsigned int srcPitch = ((image.width + 1)* 3 / 4) * 4; // bitmap row length is aligned to 4 bytes

  // test background color for alpha blending
  long  nBkgndIndex;
  RGBQUAD nBkgndColor;
  dll.GetBackgroundColor(&image, &nBkgndColor, &nBkgndIndex);

  for (unsigned int y = 0; y < m_imageHeight; y++)
  {
    unsigned char *dst = m_pixels + y * destPitch;
    unsigned char *src = image.texture + (m_imageHeight - 1 - y) * srcPitch;
    unsigned char *alpha = image.alpha + (m_imageHeight - 1 - y) * m_imageWidth;
    for (unsigned int x = 0; x < m_imageWidth; x++)
    {
      *dst++ = *src++;
      *dst++ = *src++;
      *dst++ = *src++;
      *dst++ = (image.alpha) ? *alpha++ : 0xff;
    }
  }

  if (nBkgndIndex != -1)
  {
    for (unsigned int y = 0; y < m_imageHeight; y++)
    {
      unsigned char *dst = m_pixels + y * destPitch;
      //unsigned char *src = image.texture + (m_imageHeight - 1 - y) * srcPitch;
      //unsigned char *alpha = image.alpha + (m_imageHeight - 1 - y) * m_imageWidth;
      for (unsigned int x = 0; x < m_imageWidth; x++)
      {
        if (*dst == nBkgndColor.rgbBlue
            && *(dst + 1) == nBkgndColor.rgbGreen
            && *(dst + 2) == nBkgndColor.rgbRed)
        {
          *(dst + 3) = 0x00;
        }
        dst += 4;
      }
    }
  }

  dll.ReleaseImage(&image);

  ClampToEdge();

  return true;
}

bool CBaseTexture::LoadFromMemory(unsigned int width, unsigned int initialWidth, unsigned int height, unsigned int initialHeight, unsigned int pitch, unsigned int format, unsigned char* pixels)
{
  m_imageWidth = width;
  m_imageInitialWidth = initialWidth;
  m_imageHeight = height;
  m_imageInitialHeight = initialHeight;
  m_format = format;
  Update(width, height, pitch, format, pixels, false);
  return true;
}

bool CBaseTexture::LoadPaletted(unsigned int width, unsigned int height, unsigned int pitch, unsigned int format, const unsigned char *pixels, const COLOR *palette)
{
  if (pixels == NULL || palette == NULL)
    return false;

  Allocate(width, height, format);

  for (unsigned int y = 0; y < m_imageHeight; y++)
  {
    unsigned char *dest = m_pixels + y * GetPitch();
    const unsigned char *src = pixels + y * pitch;
    for (unsigned int x = 0; x < m_imageWidth; x++)
    {
      COLOR col = palette[*src++];
      *dest++ = col.b;
      *dest++ = col.g;
      *dest++ = col.r;
      *dest++ = col.x;
    }
    }
  ClampToEdge();
  return true;
}

unsigned int CBaseTexture::PadPow2(unsigned int x) 
{
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return ++x;
}

unsigned int CBaseTexture::GetPitch(unsigned int width) const
{
  switch (m_format)
  {
  case XB_FMT_DXT1:
    return ((width + 3) / 4) * 8;
  case XB_FMT_DXT3:
  case XB_FMT_DXT5:
  case XB_FMT_DXT5_YCoCg:
    return ((width + 3) / 4) * 16;
  case XB_FMT_A8:
    return width;
  case XB_FMT_PVR2:
    return width / 4;
  case XB_FMT_PVR4:
    return width / 2;
  case XB_FMT_B8G8R8A8:
  default:
    return width*4;
  }
}

unsigned int CBaseTexture::GetRows(unsigned int height) const
{
  switch (m_format)
  {
  case XB_FMT_DXT1:
    return (height + 3) / 4;
  case XB_FMT_DXT3:
  case XB_FMT_DXT5:
  case XB_FMT_DXT5_YCoCg:
    return (height + 3) / 4;
  default:
    return height;
  }
}

unsigned int CBaseTexture::GetBlockSize() const
{
  switch (m_format)
  {
  case XB_FMT_DXT1:
    return 8;
  case XB_FMT_DXT3:
  case XB_FMT_DXT5:
  case XB_FMT_DXT5_YCoCg:
    return 16;
  case XB_FMT_A8:
    return 1;
  case XB_FMT_PVR2:
    return 2;
  case XB_FMT_PVR4:
    return 4;
  default:
    return 4;
  }
}
