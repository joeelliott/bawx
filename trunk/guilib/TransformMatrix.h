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

#include <math.h>
#include <memory>
#include <string.h>
#include <stdint.h>
#include "StdString.h"

#ifdef __GNUC__
// under gcc, inline will only take place if optimizations are applied (-O). this will force inline even whith optimizations.
#define XBMC_FORCE_INLINE __attribute__((always_inline))
#else
#define XBMC_FORCE_INLINE
#endif

typedef uint32_t color_t;

class TransformMatrix
{
public:
  TransformMatrix()
  {
    m_bColumnMajor = false;
    Reset();
  };
  void Reset()
  {
    memset(m, 0, 16 * sizeof(float));
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
    alpha = 1.0f;
  };

  void SetColumnMajor(bool first)
  {
    m_bColumnMajor = first;
  }
  
  bool IsColumnMajor()
  {
    return m_bColumnMajor;
  }

  void Transpose()
  {
    TransformMatrix tmp = *this;

    for(int i = 0; i < 4; i++)
      for(int j = 0; j < 4; j++)
        m[i][j] = tmp.m[j][i];

    m_bColumnMajor = !m_bColumnMajor;
  }

  static TransformMatrix CreateTranslation(float transX, float transY, float transZ = 0)
  {
    TransformMatrix translation;
    translation.m[0][3] = transX;
    translation.m[1][3] = transY;
    translation.m[2][3] = transZ;
    return translation;
  }
  void SetTranslation(float transX, float transY, float transZ)
  {
    m[0][1] = m[0][2] = 0.0f; m[0][0] = 1.0f; m[0][3] = transX;
    m[1][0] = m[1][2] = 0.0f; m[1][1] = 1.0f; m[1][3] = transY;
    m[2][0] = m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = transZ;
    alpha = 1.0f;
  }
  static TransformMatrix CreateScaler(float scaleX, float scaleY, float scaleZ = 1.0f)
  {
    TransformMatrix scaler;
    scaler.m[0][0] = scaleX;
    scaler.m[1][1] = scaleY;
    scaler.m[2][2] = scaleZ;
    return scaler;
  };
  void SetScaler(float scaleX, float scaleY, float centerX, float centerY)
  {
    // Trans(centerX,centerY,centerZ)*Scale(scaleX,scaleY,scaleZ)*Trans(-centerX,-centerY,-centerZ)
    float centerZ = 0.0f, scaleZ = 1.0f;
    m[0][0] = scaleX;  m[0][1] = 0.0f;    m[0][2] = 0.0f;    m[0][3] = centerX*(1-scaleX);
    m[1][0] = 0.0f;    m[1][1] = scaleY;  m[1][2] = 0.0f;    m[1][3] = centerY*(1-scaleY);
    m[2][0] = 0.0f;    m[2][1] = 0.0f;    m[2][2] = scaleZ;  m[2][3] = centerZ*(1-scaleZ);
    alpha = 1.0f;
  };
  void SetXRotation(float angle, float y, float z, float ar = 1.0f)
  { // angle about the X axis, centered at y,z where our coordinate system has aspect ratio ar.
    // Trans(0,y,z)*Scale(1,1/ar,1)*RotateX(angle)*Scale(ar,1,1)*Trans(0,-y,-z);
    float c = cos(angle); float s = sin(angle);
    m[0][0] = ar;    m[0][1] = 0.0f;  m[0][2] = 0.0f;   m[0][3] = 0.0f;
    m[1][0] = 0.0f;  m[1][1] = c/ar;  m[1][2] = -s/ar;  m[1][3] = (-y*c+s*z)/ar + y;
    m[2][0] = 0.0f;  m[2][1] = s;     m[2][2] = c;      m[2][3] = (-y*s-c*z) + z;
    angle = 1.0f;
  }
  void SetYRotation(float angle, float x, float z, float ar = 1.0f)
  { // angle about the Y axis, centered at x,z where our coordinate system has aspect ratio ar.
    // Trans(x,0,z)*Scale(1/ar,1,1)*RotateY(angle)*Scale(ar,1,1)*Trans(-x,0,-z);
    float c = cos(angle); float s = sin(angle);
    m[0][0] = c;     m[0][1] = 0.0f;  m[0][2] = -s/ar;  m[0][3] = -x*c + s*z/ar + x;
    m[1][0] = 0.0f;  m[1][1] = 1.0f;  m[1][2] = 0.0f;   m[1][3] = 0.0f;
    m[2][0] = ar*s;  m[2][1] = 0.0f;  m[2][2] = c;      m[2][3] = -ar*x*s - c*z + z;
    angle = 1.0f;
  }
  static TransformMatrix CreateZRotation(float angle, float x, float y, float ar = 1.0f)
  { // angle about the Z axis, centered at x,y where our coordinate system has aspect ratio ar.
    // Trans(x,y,0)*Scale(1/ar,1,1)*RotateZ(angle)*Scale(ar,1,1)*Trans(-x,-y,0)
    float c = cos(angle); float s = sin(angle);
    TransformMatrix rot;
    rot.m[0][0] = c;    rot.m[0][1] = -s/ar; rot.m[0][3] = -x*c + s*y/ar + x;
    rot.m[1][0] = s*ar; rot.m[1][1] = c;     rot.m[1][3] = -ar*x*s - c*y + y;
    return rot;
  }
  void SetZRotation(float angle, float x, float y, float ar = 1.0f)
  { // angle about the Z axis, centered at x,y where our coordinate system has aspect ratio ar.
    // Trans(x,y,0)*Scale(1/ar,1,1)*RotateZ(angle)*Scale(ar,1,1)*Trans(-x,-y,0)
    float c = cos(angle); float s = sin(angle);
    m[0][0] = c;     m[0][1] = -s/ar;  m[0][2] = 0.0f;  m[0][3] = -x*c + s*y/ar + x;
    m[1][0] = s*ar;  m[1][1] = c;      m[1][2] = 0.0f;  m[1][3] = -ar*x*s - c*y + y;
    m[2][0] = 0.0f;  m[2][1] = 0.0f;   m[2][2] = 1.0f;  m[2][3] = 0.0f;
    angle = 1.0f;
  }
  static TransformMatrix CreateFader(float a)
  {
    TransformMatrix fader;
    fader.alpha = a;
    return fader;
  }
  void SetFader(float a)
  {
    Reset();
    alpha = a;
  }
  // assignment operator
  const TransformMatrix &operator =(const TransformMatrix &right)
  {
    if (this != &right)
    {
      memcpy(m, right.m, 16 * sizeof(float));
      alpha = right.alpha;
      m_bColumnMajor = right.m_bColumnMajor;
    }
    return *this;
  }

  // multiplication operators
  const TransformMatrix &operator *=(const TransformMatrix &right)
  {
    TransformMatrix result;

    result.m[0][0] = m[0][0] * right.m[0][0] + m[0][1] * right.m[1][0] + m[0][2] * right.m[2][0] + m[0][3] * right.m[3][0];
    result.m[0][1] = m[0][0] * right.m[0][1] + m[0][1] * right.m[1][1] + m[0][2] * right.m[2][1] + m[0][3] * right.m[3][1];
    result.m[0][2] = m[0][0] * right.m[0][2] + m[0][1] * right.m[1][2] + m[0][2] * right.m[2][2] + m[0][3] * right.m[3][2];
    result.m[0][3] = m[0][0] * right.m[0][3] + m[0][1] * right.m[1][3] + m[0][2] * right.m[2][3] + m[0][3] * right.m[3][3];
    result.m[1][0] = m[1][0] * right.m[0][0] + m[1][1] * right.m[1][0] + m[1][2] * right.m[2][0] + m[1][3] * right.m[3][0];
    result.m[1][1] = m[1][0] * right.m[0][1] + m[1][1] * right.m[1][1] + m[1][2] * right.m[2][1] + m[1][3] * right.m[3][1];
    result.m[1][2] = m[1][0] * right.m[0][2] + m[1][1] * right.m[1][2] + m[1][2] * right.m[2][2] + m[1][3] * right.m[3][2];
    result.m[1][3] = m[1][0] * right.m[0][3] + m[1][1] * right.m[1][3] + m[1][2] * right.m[2][3] + m[1][3] * right.m[3][3];
    result.m[2][0] = m[2][0] * right.m[0][0] + m[2][1] * right.m[1][0] + m[2][2] * right.m[2][0] + m[2][3] * right.m[3][0]; 
    result.m[2][1] = m[2][0] * right.m[0][1] + m[2][1] * right.m[1][1] + m[2][2] * right.m[2][1] + m[2][3] * right.m[3][1];
    result.m[2][2] = m[2][0] * right.m[0][2] + m[2][1] * right.m[1][2] + m[2][2] * right.m[2][2] + m[2][3] * right.m[3][2];
    result.m[2][3] = m[2][0] * right.m[0][3] + m[2][1] * right.m[1][3] + m[2][2] * right.m[2][3] + m[2][3] * right.m[3][3];
    result.m[3][0] = m[3][0] * right.m[0][0] + m[3][1] * right.m[1][0] + m[3][2] * right.m[2][0] + m[3][3] * right.m[3][0];
    result.m[3][1] = m[3][0] * right.m[0][1] + m[3][1] * right.m[1][1] + m[3][2] * right.m[2][1] + m[3][3] * right.m[3][1];
    result.m[3][2] = m[3][0] * right.m[0][2] + m[3][1] * right.m[1][2] + m[3][2] * right.m[2][2] + m[3][3] * right.m[3][2];
    result.m[3][3] = m[3][0] * right.m[0][3] + m[3][1] * right.m[1][3] + m[3][2] * right.m[2][3] + m[3][3] * right.m[3][3];

    result.alpha = alpha * right.alpha;

    *this = result;

    return *this;
  }

  TransformMatrix operator *(const TransformMatrix &right) const
  {
    TransformMatrix result;

    result.m[0][0] = m[0][0] * right.m[0][0] + m[0][1] * right.m[1][0] + m[0][2] * right.m[2][0] + m[0][3] * right.m[3][0];
    result.m[0][1] = m[0][0] * right.m[0][1] + m[0][1] * right.m[1][1] + m[0][2] * right.m[2][1] + m[0][3] * right.m[3][1];
    result.m[0][2] = m[0][0] * right.m[0][2] + m[0][1] * right.m[1][2] + m[0][2] * right.m[2][2] + m[0][3] * right.m[3][2];
    result.m[0][3] = m[0][0] * right.m[0][3] + m[0][1] * right.m[1][3] + m[0][2] * right.m[2][3] + m[0][3] * right.m[3][3];
    result.m[1][0] = m[1][0] * right.m[0][0] + m[1][1] * right.m[1][0] + m[1][2] * right.m[2][0] + m[1][3] * right.m[3][0];
    result.m[1][1] = m[1][0] * right.m[0][1] + m[1][1] * right.m[1][1] + m[1][2] * right.m[2][1] + m[1][3] * right.m[3][1];
    result.m[1][2] = m[1][0] * right.m[0][2] + m[1][1] * right.m[1][2] + m[1][2] * right.m[2][2] + m[1][3] * right.m[3][2];
    result.m[1][3] = m[1][0] * right.m[0][3] + m[1][1] * right.m[1][3] + m[1][2] * right.m[2][3] + m[1][3] * right.m[3][3];
    result.m[2][0] = m[2][0] * right.m[0][0] + m[2][1] * right.m[1][0] + m[2][2] * right.m[2][0] + m[2][3] * right.m[3][0]; 
    result.m[2][1] = m[2][0] * right.m[0][1] + m[2][1] * right.m[1][1] + m[2][2] * right.m[2][1] + m[2][3] * right.m[3][1];
    result.m[2][2] = m[2][0] * right.m[0][2] + m[2][1] * right.m[1][2] + m[2][2] * right.m[2][2] + m[2][3] * right.m[3][2];
    result.m[2][3] = m[2][0] * right.m[0][3] + m[2][1] * right.m[1][3] + m[2][2] * right.m[2][3] + m[2][3] * right.m[3][3];
    result.m[3][0] = m[3][0] * right.m[0][0] + m[3][1] * right.m[1][0] + m[3][2] * right.m[2][0] + m[3][3] * right.m[3][0];
    result.m[3][1] = m[3][0] * right.m[0][1] + m[3][1] * right.m[1][1] + m[3][2] * right.m[2][1] + m[3][3] * right.m[3][1];
    result.m[3][2] = m[3][0] * right.m[0][2] + m[3][1] * right.m[1][2] + m[3][2] * right.m[2][2] + m[3][3] * right.m[3][2];
    result.m[3][3] = m[3][0] * right.m[0][3] + m[3][1] * right.m[1][3] + m[3][2] * right.m[2][3] + m[3][3] * right.m[3][3];

    result.alpha = alpha * right.alpha;
    return result;
  }


  inline void TransformPosition(float &x, float &y, float &z) const XBMC_FORCE_INLINE
  {
    float newX = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
    float newY = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
    z = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
    y = newY;
    x = newX;
  }

  inline void TransformPositionUnscaled(float &x, float &y, float &z) const XBMC_FORCE_INLINE
  {
    float n;
    // calculate the norm of the transformed (but not translated) vectors involved
    n = sqrt(m[0][0]*m[0][0] + m[0][1]*m[0][1] + m[0][2]*m[0][2]);
    float newX = (m[0][0] * x + m[0][1] * y + m[0][2] * z)/n + m[0][3];
    n = sqrt(m[1][0]*m[1][0] + m[1][1]*m[1][1] + m[1][2]*m[1][2]);
    float newY = (m[1][0] * x + m[1][1] * y + m[1][2] * z)/n + m[1][3];
    n = sqrt(m[2][0]*m[2][0] + m[2][1]*m[2][1] + m[2][2]*m[2][2]);
    float newZ = (m[2][0] * x + m[2][1] * y + m[2][2] * z)/n + m[2][3];
    z = newZ;
    y = newY;
    x = newX;
  }

  inline void InverseTransformPosition(float &x, float &y) const XBMC_FORCE_INLINE
  { // used for mouse - no way to find z
    x -= m[0][3]; y -= m[1][3];
    float detM = m[0][0]*m[1][1] - m[0][1]*m[1][0];
    if(detM == 0)
		return;
    float newX = (m[1][1] * x - m[0][1] * y)/detM;
    y = (-m[1][0] * x + m[0][0] * y)/detM;
    x = newX;
  }

  void MatrixOrtho(float left,float right,float bottom,float top,float zNear,float zFar)
  {	
    float tx=-(right+left)/(right-left);
    float ty=-(top+bottom)/(top-bottom);
    float tz=-(zFar+zNear)/(zFar-zNear);
    Reset();
    float* _t = (float*)m; 
    _t[0]=2/(right-left);
    _t[5]=2/(top-bottom);
    _t[10]=-2/(zFar-zNear);	
    _t[12]=tx;
    _t[13]=ty;
    _t[14]=tz;
  }

  void MatrixFrustum(float l, float r, float b, float t, float n, float f)
  {
    Reset();

    float* m1 = (float *)m;
    float rightMinusLeftInv, topMinusBottomInv, farMinusNearInv, twoNear;

    rightMinusLeftInv = 1.0f / (r - l);
    topMinusBottomInv = 1.0f / (t - b);
    farMinusNearInv = 1.0f / (f - n);
    twoNear = 2.0f * n;

    m1[ 0] = twoNear * rightMinusLeftInv;
    m1[ 1] = 0.0f;
    m1[ 2] = 0.0f;
    m1[ 3] = 0.0f;

    m1[ 4] = 0.0f;
    m1[ 5] = twoNear * topMinusBottomInv;
    m1[ 6] = 0.0f;
    m1[ 7] = 0.0f;

    m1[ 8] = (r + l) * rightMinusLeftInv;
    m1[ 9] = (t + b) * topMinusBottomInv;
    m1[10] = -(f + n) * farMinusNearInv;
    m1[11] = -1.0f;

    m1[12] = 0.0f;
    m1[13] = 0.0f;
    m1[14] = -(twoNear * f) * farMinusNearInv;
    m1[15] = 0.0f;
  };

  inline color_t TransformAlpha(color_t colour) const XBMC_FORCE_INLINE
  {
    return (color_t)(colour * alpha);
  }

  bool m_bColumnMajor;
  float m[4][4];
  float alpha;
};
