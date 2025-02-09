#if 1//defined(_DLL) || defined(_USRDLL)
#include "ximage.h"
#include "ximajpg.h"

#if defined(_LINUX) || defined(__APPLE__)
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#define strcmpi strcasecmp
#else //win32
#include <sys/types.h>
#include <sys/stat.h>
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#define strcmpi _stricmp
#endif

#define RESAMPLE_QUALITY 0
#undef USE_EXIF_THUMBS

// MS DirectX routines don't like loading .jpg less than 8x8 pixels.
#define MIN_THUMB_WIDTH 8
#define MIN_THUMB_HEIGHT 8
#define MAX_WIDTH 4096
#define MAX_HEIGHT 4096

extern "C" struct ImageInfo
{
  unsigned int width;
  unsigned int height;
  unsigned int originalwidth;
  unsigned int originalheight;
  EXIFINFO exifInfo;
  BYTE *texture;
  void *context;
  BYTE *alpha;
};

#if defined(_LINUX) || defined(__APPLE__)
static void DeleteFile(const char* name)
{
  unlink(name);
}
#endif

// helper functions
DWORD GetImageType(const char *file, const char *contentType)
  {
  if ( !file || ( 0 == *file ) ) // ensure file is not NULL and has non-zero length
    return CXIMAGE_FORMAT_UNKNOWN;

  // try to determine extension using '.' or use entire filename
  // if '.' is absent.
  const char *ext = strrchr(file, '.');
  if (ext == NULL)
    ext = (char*)file;
  else
    ext++;

  if ( 0 == *ext ) // if filename ends in '.', we can't identify based on extension
    return CXIMAGE_FORMAT_UNKNOWN;

  if ( 0 == strcmpi(ext, "bmp") ) return CXIMAGE_FORMAT_BMP;
  else if ( 0 == strcmpi(ext, "bitmap") ) return CXIMAGE_FORMAT_BMP;
  else if ( 0 == strcmpi(ext, "gif") ) return CXIMAGE_FORMAT_GIF;
  else if ( 0 == strcmpi(ext, "jpg") ) return CXIMAGE_FORMAT_JPG;
  else if ( 0 == strcmpi(ext, "tbn") ) return CXIMAGE_FORMAT_JPG;
  else if ( 0 == strcmpi(ext, "jpeg") ) return CXIMAGE_FORMAT_JPG;
  else if ( 0 == strcmpi(ext, "png") ) return CXIMAGE_FORMAT_PNG;
  else if ( 0 == strcmpi(ext, "ico") ) return CXIMAGE_FORMAT_ICO;
  else if ( 0 == strcmpi(ext, "tif") ) return CXIMAGE_FORMAT_TIF;
  else if ( 0 == strcmpi(ext, "tiff") ) return CXIMAGE_FORMAT_TIF;
  else if ( 0 == strcmpi(ext, "tga") ) return CXIMAGE_FORMAT_TGA;
  else if ( 0 == strcmpi(ext, "pcx") ) return CXIMAGE_FORMAT_PCX;

  // RAW camera formats
  else if ( 0 == strcmpi(ext, "cr2") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "nef") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "dng") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "crw") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "orf") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "arw") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "erf") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "3fr") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "dcr") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "x3f") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "mef") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "raf") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "mrw") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "pef") ) return CXIMAGE_FORMAT_RAW;
  else if ( 0 == strcmpi(ext, "sr2") ) return CXIMAGE_FORMAT_RAW;

  else if ( contentType && 0 == strcmpi(contentType, "image/gif") ) return  CXIMAGE_FORMAT_GIF;
  else if ( contentType && 0 == strcmpi(contentType, "image/jpg") ) return  CXIMAGE_FORMAT_JPG;
  else if ( contentType && 0 == strcmpi(contentType, "image/jpeg") ) return  CXIMAGE_FORMAT_JPG;
  else if ( contentType && 0 == strcmpi(contentType, "image/jp2") ) return  CXIMAGE_FORMAT_JPG;
  else if ( contentType && 0 == strcmpi(contentType, "image/png") ) return  CXIMAGE_FORMAT_PNG;
  else if ( contentType && 0 == strcmpi(contentType, "image/tiff") ) return  CXIMAGE_FORMAT_TIF;
  else if ( contentType && 0 == strcmpi(contentType, "image/tiff-fx") ) return  CXIMAGE_FORMAT_TIF;

  // fallback to unknown
  return CXIMAGE_FORMAT_UNKNOWN;
}

int DetectFileType(const BYTE* pBuffer, int nBufSize)
{
  if (nBufSize <= 5)
    return CXIMAGE_FORMAT_UNKNOWN;
  if (pBuffer[1] == 'P' && pBuffer[2] == 'N' && pBuffer[3] == 'G')
    return CXIMAGE_FORMAT_PNG;
  if (pBuffer[0] == 'B' && pBuffer[1] == 'M')
    return CXIMAGE_FORMAT_BMP;
  if (pBuffer[0] == 0xFF && pBuffer[1] == 0xD8 && pBuffer[2] == 0xFF && (pBuffer[3] == 0xE0 || pBuffer[3] == 0xE1))
    return CXIMAGE_FORMAT_JPG;
  return CXIMAGE_FORMAT_UNKNOWN;
}

bool CopyFile(const char *src, const char *dest)
{
  const unsigned int bufferSize = 16384;
  char buffer[bufferSize];
  FILE *hSrc = fopen(src, "rb");
  if (!hSrc) return false;
  FILE *hDest = fopen(dest, "wb");
  if (!hDest)
  {
    fclose(hSrc);
    return false;
  }
  bool ret = true;
  while (ret)
  {
    int sizeRead = fread(buffer, 1, bufferSize, hSrc);
    if (sizeRead > 0)
    {
      int sizeWritten = fwrite(buffer, 1, sizeRead, hDest);
      if (sizeWritten != sizeRead)
      {
        printf("PICTURE:Error writing file in copy");
        ret = false;
      }
    }
    else if (sizeRead < 0)
    {
      printf("PICTURE:Error reading file for copy");
      ret = false;
    }
    else
      break;  // we're done
  }
  fclose(hSrc);
  fclose(hDest);
  return ret;
}

int ResampleKeepAspect(CxImage &image, unsigned int width, unsigned int height, bool checkTooSmall = false)
{
  bool bResize = false;
  float fAspect = ((float)image.GetWidth()) / ((float)image.GetHeight());
  unsigned int newwidth = image.GetWidth();
  unsigned int newheight = image.GetHeight();
  if (newwidth > width)
  {
    bResize = true;
    newwidth = width;
    newheight = (DWORD)( ( (float)newwidth) / fAspect);
  }
  if (newheight > height)
  {
    bResize = true;
    newheight = height;
    newwidth = (DWORD)( fAspect * ( (float)newheight) );
  }
  if (checkTooSmall && newwidth < MIN_THUMB_WIDTH)
  {
    bResize = true;
    newwidth = MIN_THUMB_HEIGHT;
    newheight = (DWORD)( ( (float)newwidth) / fAspect);
  }
  if (checkTooSmall && newheight < MIN_THUMB_HEIGHT)
  {
    bResize = true;
    newheight = MIN_THUMB_HEIGHT;
    newwidth = (DWORD)( fAspect * ( (float)newheight) );
  }
  if (bResize)
  {
    if (!image.Resample(newwidth, newheight, RESAMPLE_QUALITY) || !image.IsValid())
    {
      printf("PICTURE::SaveThumb: Unable to resample picture: Error:%s\n", image.GetLastError());
      return -1;
    }
  }
  return bResize ? 1 : 0;
}

int ResampleKeepAspectArea(CxImage &image, unsigned int area)
{
  bool bResize = false;
  float fAspect = ((float)image.GetWidth()) / ((float)image.GetHeight());
  unsigned int width = (unsigned int)sqrt(area * fAspect);
  unsigned int height = (unsigned int)sqrt(area / fAspect);
  if (width > MAX_WIDTH) width = MAX_WIDTH;
  if (height > MAX_HEIGHT) height = MAX_HEIGHT;
  return ResampleKeepAspect(image, width, height, true);
}

bool SaveThumb(CxImage &image, const char *file, const char *thumb, int maxWidth, int maxHeight, bool bNeedToConvert = true, bool autoRotate = true)
{
  long nBkgndIndex;
  RGBQUAD nBkgndColor;

  nBkgndIndex = image.GetTransIndex();
  nBkgndColor = image.GetTransColor();

  // ok, now resample the image down if necessary
  int ret = ResampleKeepAspectArea(image, maxWidth * maxHeight);
  if (ret < 0) return false;
  if (ret) bNeedToConvert = true;

  // if we don't have a png but have a < 24 bit image, then convert to 24bits
  // also check if we have background color. If so we need to create alpha channel
  if ( image.GetNumColors() || nBkgndIndex != -1)
  {
    if (!image.IncreaseBpp(24) || !image.IsValid())
    {
      printf("PICTURE::SaveThumb: Unable to convert to 24bpp: Error:%s\n", image.GetLastError());
      return false;
    }
    bNeedToConvert = true;
  }

  if (nBkgndIndex != -1)
  {
    printf("Found background %s\n", file);

    if(!image.AlphaIsValid())
      image.AlphaCreate();

    BYTE* buffer = image.GetBits();
    RGBQUAD srcRGB;

    int width = image.GetWidth();
    int height = image.GetHeight();

    for (unsigned int y = 0; y < height; y++)
    {
      for (unsigned int x = 0; x < width; x++)
      {
        srcRGB = image.GetPixelColor(x, y, false);
        if(srcRGB.rgbBlue == nBkgndColor.rgbBlue &&
           srcRGB.rgbGreen == nBkgndColor.rgbGreen &&
           srcRGB.rgbRed == nBkgndColor.rgbRed)
        {
          printf("Found matching background color %d %d %d\n",
              nBkgndColor.rgbBlue, nBkgndColor.rgbGreen, nBkgndColor.rgbRed);
          image.AlphaSet(x, y, 0x00);
        }
      }
    }
  }

  if ( autoRotate && image.GetExifInfo()->Orientation > 1)
  {
    image.RotateExif(image.GetExifInfo()->Orientation);
    bNeedToConvert = true;
  }

#ifndef _LINUX
  ::DeleteFile(thumb);
#else
  unlink(thumb);
#endif

#undef SAVE_THUMB_AS_DXT
#ifdef SAVE_THUMB_AS_DXT

 //  printf("SaveThumb: File: %s Width: %ld Height: %ld\n", thumb, image.GetWidth(), image.GetHeight());

  if (image.DDS_SaveAsDXT5(thumb))
  {
    return true;
  }

  // Fallback to non DXT processing...
#endif

  // only resave the image if we have to (quality of the JPG saver isn't too hot!)
  if (bNeedToConvert)
  {
    // May as well have decent quality thumbs
    image.SetJpegQuality(90);
    if (!image.Save(thumb, image.AlphaIsValid() ? CXIMAGE_FORMAT_PNG : CXIMAGE_FORMAT_JPG))
    {
      printf("PICTURE::SaveThumb: Unable to save image: %s Error:%s\n", thumb, image.GetLastError());
      ::DeleteFile(thumb);
      return false;
    }
  }
  else
  { // Don't need to convert the file - copy it instead
    if (!CopyFile(file, thumb))
    {
      printf("PICTURE::SaveThumb: Unable to copy file %s\n", file);
      ::DeleteFile(thumb);
      return false;
    }
  }

  return true;
}

#ifdef LoadImage
#undef LoadImage
#endif

#if defined(_LINUX) || defined(__APPLE__)
#define __declspec(x) 
#endif

extern "C"
{
  bool IsDir(const char* file)
  {
    struct stat holder;
    if (stat(file, &holder) == -1)
      return false;
    if (S_ISDIR(holder.st_mode))
      return true;

    return false;
  }

  bool __declspec(dllexport) ReleaseImage(ImageInfo *info) 
  {
    if (info && info->context)
    {
      delete ((CxImage *)info->context);
      return true;
    }

	  return false;
  }

  bool __declspec(dllexport) LoadImageWithCT(const char *file, const char* contentType, unsigned int maxwidth, unsigned int maxheight, ImageInfo *info)
  {
    if (!file || !info) return false;

	  if (IsDir(file))
		  return false;

	  // load the image
    DWORD dwImageType = GetImageType(file, contentType);
    CxImage *image = new CxImage(dwImageType);
    if (!image) return false;

    int actualwidth = maxwidth;
    int actualheight = maxheight;
    try
    {
      if (!image->Load(file, dwImageType, actualwidth, actualheight) || !image->IsValid())
      {
#if !defined(_LINUX) && !defined(__APPLE__)
	    int nErr = GetLastError();
#else
	    int nErr = errno;
#endif
        //printf("PICTURE::LoadImage: Unable to open image: %s Error:%s (%d)\n", file, image->GetLastError(),nErr);
        delete image;
        return false;
      }
    }
    catch (...)
    {
      //printf("PICTURE::LoadImage: Unable to open image: %s\n", file);
      delete image;
      return false;
    }
    
    // ok, now resample the image down if necessary
    if (ResampleKeepAspect(*image, maxwidth, maxheight) < 0)
    {
      printf("PICTURE::LoadImage: Unable to resample picture: %s\n", file);
      delete image;
      return false;
    }

    if (info->width && info->height)
    {
      CxImage *resampledImage = new CxImage();
      image->Resample(info->width, info->height, 1, resampledImage);
      delete image;
      image = resampledImage;
    }

    // make sure our image is 24bit minimum
    image->IncreaseBpp(24);

    // fill in our struct
    info->width = image->GetWidth();
    info->height = image->GetHeight();
    info->originalwidth = actualwidth;
    info->originalheight = actualheight;
    memcpy(&info->exifInfo, image->GetExifInfo(), sizeof(EXIFINFO));

    // create our texture
    info->context = image;
    info->texture = image->GetBits();
    info->alpha = image->AlphaGetBits();
    return (info->texture != NULL);
  };

#ifndef WIN32
  #include <pthread.h>
  static pthread_mutex_t a_mutex = PTHREAD_MUTEX_INITIALIZER;

  bool __declspec(dllexport) LoadImage(const char *file, unsigned int maxwidth, unsigned int maxheight, ImageInfo *info)
  {
    pthread_mutex_lock( &a_mutex );
    bool result = LoadImageWithCT(file, "", maxwidth, maxheight, info);
    pthread_mutex_unlock( &a_mutex );
    return result;
  }
#else
static HANDLE a_mutex = CreateMutex(NULL, FALSE, NULL); // TODO: Need to release this resource...
bool __declspec(dllexport) LoadImage(const char *file, unsigned int maxwidth, unsigned int maxheight, ImageInfo *info)
{
  if (WAIT_OBJECT_0 != WaitForSingleObject( a_mutex, INFINITE ))
    return false;
  bool result = LoadImageWithCT(file, "", maxwidth, maxheight, info);
  ReleaseMutex( a_mutex );
  return result;
}
#endif

  bool __declspec(dllexport) CreateThumbnailWithCT(const char *file, const char* contentType, const char *thumb, int maxWidth, int maxHeight, bool rotateExif)
  {
    if (!file || !thumb) return false;

	if (IsDir(file))
		return false;

    // load the image
    DWORD dwImageType = GetImageType(file, contentType);
    CxImage image(dwImageType);
    int actualwidth = maxWidth * maxHeight;
    int actualheight = 0;
    try
    {
      // jpeg's may contain an EXIF preview image
      // we don't use it though, as the resolution is normally too low
#ifdef USE_EXIF_THUMBS
      if ((dwImageType == CXIMAGE_FORMAT_JPG || dwImageType == CXIMAGE_FORMAT_RAW) && image.GetExifThumbnail(file, thumb, dwImageType))
      {
        return true;
      }
#endif

      if (!image.Load(file, dwImageType, maxWidth, maxHeight) || !image.IsValid())
      {
        printf("PICTURE::CreateThumbnail: Unable to open image: %s Error:%s\n", file, image.GetLastError());
        return false;
      }
    }
    catch (...)
    {
      printf("PICTURE::CreateThumbnail: Unable to open image: %s\n", file);
      return false;
    }

    // we need to convert if we don't have a jpeg or png.
    bool bNeedToConvert = (dwImageType != CXIMAGE_FORMAT_JPG && dwImageType != CXIMAGE_FORMAT_PNG);
    if (actualwidth > maxWidth || actualheight > maxHeight)
      bNeedToConvert = true;

    // save png thumbs as png, but all others as jpeg
    return SaveThumb(image, file, thumb, maxWidth, maxHeight, bNeedToConvert, rotateExif);
  };

  bool __declspec(dllexport) DecodeFromMemory(BYTE *buffer, unsigned int size, ImageInfo *info)
  {
    if (!buffer || !size ) 
      return false;
    
    // load the image
    CxImage *image = new CxImage(CXIMAGE_FORMAT_UNKNOWN);
    try
    {
      bool success = image->Decode(buffer, size, CXIMAGE_FORMAT_UNKNOWN);
      if (!success || !image->IsValid())
      {
        printf("PICTURE::DecodeFromMemory: Unable to decode image. Error:%s\n", image->GetLastError());
        return false;
      }
    }
    catch (...)
    {
      printf("PICTURE::DecodeFromMemory: Unable to decode image.");
      return false;
    }
    image->IncreaseBpp(24);
    
    // fill in our struct
    info->width = image->GetWidth();
    info->height = image->GetHeight();
    info->originalwidth = info->width;
    info->originalheight = info->height;
    memcpy(&info->exifInfo, image->GetExifInfo(), sizeof(EXIFINFO));
    
    // create our texture
    info->context = image;
    info->texture = image->GetBits();
    info->alpha = image->AlphaGetBits();

    return info->texture != NULL;
  };
  
  bool __declspec(dllexport) CreateThumbnailFromMemory(BYTE *buffer, unsigned int size, const char *ext, const char *thumb, int maxWidth, int maxHeight)
  {
    if (!buffer || !size || !ext || !thumb) return false;
    // load the image
    DWORD dwImageType = CXIMAGE_FORMAT_UNKNOWN;
    if (strlen(ext)) {
      dwImageType = GetImageType(ext, "");
      if (dwImageType == CXIMAGE_FORMAT_UNKNOWN)
        dwImageType = DetectFileType(buffer, size);
    }
    else
      dwImageType = DetectFileType(buffer, size);
    if (dwImageType == CXIMAGE_FORMAT_UNKNOWN)
    {
      printf("PICTURE::CreateThumbnailFromMemory: Unable to determine image type.");
      return false;
    }
    CxImage image(dwImageType);
    try
    {
      bool success = image.Decode(buffer, size, dwImageType);
      if (!success && dwImageType != CXIMAGE_FORMAT_UNKNOWN)
      { // try to decode with unknown imagetype
        success = image.Decode(buffer, size, CXIMAGE_FORMAT_UNKNOWN);
      }
      if (!success || !image.IsValid())
      {
        printf("PICTURE::CreateThumbnailFromMemory: Unable to decode image. Error:%s\n", image.GetLastError());
        return false;
      }
    }
    catch (...)
    {
      printf("PICTURE::CreateThumbnailFromMemory: Unable to decode image.");
      return false;
    }

    return SaveThumb(image, "", thumb, maxWidth, maxHeight);
  };

  bool __declspec(dllexport) CreateFolderThumbnail(const char **file, const char *thumb, int maxWidth, int maxHeight)
  {
    if (!file || !file[0] || !file[1] || !file[2] || !file[3] || !thumb) return false;

    CxImage folderimage(maxWidth, maxHeight, 32, CXIMAGE_FORMAT_PNG);
    folderimage.AlphaCreate();
    int iWidth = maxWidth / 2;
    int iHeight = maxHeight / 2;
    for (int i = 0; i < 2; i++)
    {
      for (int j = 0; j < 2; j++)
      {
        int width = iWidth;
        int height = iHeight;
        bool bBlank = false;
        if (strlen(file[i*2 + j]) == 0)
          bBlank = true;
        if (!bBlank)
        {
          CxImage image;
          if (image.Load(file[i*2 + j], CXIMAGE_FORMAT_JPG, width, height))
          {
            // resize image to iWidth
            if (ResampleKeepAspect(image, iWidth - 2, iHeight - 2) >= 0)
            {
              int iOffX = (iWidth - 2 - image.GetWidth()) / 2;
              int iOffY = (iHeight - 2 - image.GetHeight()) / 2;
              for (int x = 0; x < iWidth; x++)
              {
                for (int y = 0; y < iHeight; y++)
                {
                  RGBQUAD rgb;
                  if (x < iOffX || x >= iOffX + (int)image.GetWidth() || y < iOffY || y >= iOffY + (int)image.GetHeight())
                  {
                    rgb.rgbBlue = 0; rgb.rgbGreen = 0; rgb.rgbRed = 0; rgb.rgbReserved = 0;
                  }
                  else
                  {
                    rgb = image.GetPixelColor(x - iOffX, y - iOffY);
                    rgb.rgbReserved = 255;
                  }
                  folderimage.SetPixelColor(x + j*iWidth, y + (1 - i)*iHeight, rgb, true);
                }
              }
            }
            else
              bBlank = true;
          }
          else
            bBlank = true;
        }
        if (bBlank)
        { // no image - just fill with black alpha
          for (int x = 0; x < iWidth; x++)
          {
            for (int y = 0; y < iHeight; y++)
            {
              RGBQUAD rgb;
              rgb.rgbBlue = 0; rgb.rgbGreen = 0; rgb.rgbRed = 0; rgb.rgbReserved = 0;
              folderimage.SetPixelColor(x + j*iWidth, y + (1 - i)*iHeight, rgb, true);
            }
          }
        }
      }
    }
    ::DeleteFile(thumb);
    if (!folderimage.Save(thumb, CXIMAGE_FORMAT_PNG))
    {
      printf("Unable to save thumb file");
      ::DeleteFile(thumb);
      return false;
    }
    return true;
  };

  bool __declspec(dllexport) CreateThumbnail(const char *file, const char *thumb, int maxWidth, int maxHeight, bool rotateExif)
  {
    return CreateThumbnailWithCT(file, "", thumb, maxWidth, maxHeight, rotateExif);
  }

  bool __declspec(dllexport) CreateThumbnailFromSurface(BYTE *buffer, unsigned int width, unsigned int height, unsigned int stride, const char *thumb)
  {
    if (!buffer || !thumb) return false;
    // creates an image, and copies the surface data into it.
    CxImage image(width, height, 24, CXIMAGE_FORMAT_PNG);
    if (!image.IsValid()) return false;
    image.AlphaCreate();
    if (!image.AlphaIsValid()) return false;
    bool fullyTransparent(true);
    for (unsigned int y = 0; y < height; y++)
    {
      BYTE *ptr = buffer + (y * stride);
      for (unsigned int x = 0; x < width; x++)
      {
        BYTE b = *ptr++;
        BYTE g = *ptr++;
        BYTE r = *ptr++;
        BYTE a = *ptr++;  // alpha
        if (a) fullyTransparent = false;
        image.SetPixelColor(x, height - 1 - y, RGB(r, g, b));
        image.AlphaSet(x, height - 1 - y, a);
      }
    }
    if (fullyTransparent)
      image.AlphaDelete();
    image.SetJpegQuality(90);
    if (!image.Save(thumb, image.AlphaIsValid() ? CXIMAGE_FORMAT_PNG : CXIMAGE_FORMAT_JPG))
    {
      printf("PICTURE::CreateThumbnailFromSurface: Unable to save thumb to %s", thumb);
      return false;
    }
    return true;
  };
   int __declspec(dllexport) ConvertFile(const char *srcfile, const char *destfile, float rotateDegrees, int destwidth, int destheight, unsigned int destquality, bool mirror)
  {
    if (!srcfile || !destfile || (destwidth ==-1 && destheight==-1)) return false;
    DWORD dwImageType = GetImageType(srcfile, "");
    DWORD dwDestImageType = GetImageType(destfile, "");
    CxImage image(dwImageType);
    try
    {
      if (!image.Load(srcfile, dwImageType) || !image.IsValid())
      {
        printf("PICTURE::ConvertFile: Unable to open image: %s Error:%s\n", srcfile, image.GetLastError());
        return 7;
      }
    }
    catch (...)
    {
      printf("PICTURE::ConvertFile: Unable to open image: %s\n", srcfile);
      return 2;
    }
    if (destheight==-1) {
      destheight = (int) ((float)destwidth * ((float)image.GetHeight()/ (float)image.GetWidth())) ;
    }
    if (destwidth==-1)
      destwidth = (int) ((float)destheight * ((float)image.GetWidth()/(float)image.GetHeight())) ;
    if (!image.Resample(destwidth, destheight, RESAMPLE_QUALITY) || !image.IsValid())
    {
      printf("PICTURE::ConvertFile: Unable to resample picture: Error:%s\n", image.GetLastError());
      return 3;
    }
    if (!rotateDegrees==0.0)
      if (!image.Rotate(rotateDegrees) || !image.IsValid())
      {
        printf("PICTURE::ConvertFile: Unable to rotate picture: Error:%s\n", image.GetLastError());
        return 4;
      }
    if (mirror)
      image.Mirror(false,false);
    if (dwDestImageType==CXIMAGE_FORMAT_JPG)
      image.SetJpegQuality(destquality);
    if (!image.Save(destfile, dwDestImageType))
    {
      printf("PICTURE::ConvertFile: Unable to save image: %s Error:%s\n", destfile, image.GetLastError());
      ::DeleteFile(destfile);
      return 5;
    }
    return 0;
  };

  bool __declspec(dllexport) GetBackgroundColor(ImageInfo *info, RGBQUAD* rgbQuad, long*  nBkgndIndex)
  {
    if(!info || !rgbQuad || !nBkgndIndex)
      return false;

    CxImage *cxImage = (CxImage *)info->context;
    *nBkgndIndex = cxImage->GetTransIndex();
    *rgbQuad = cxImage->GetTransColor();
    return true;
  }
}

#endif
