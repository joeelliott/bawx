#ifndef __PERFORMANCE_SAMPLE__
#define __PERFORMANCE_SAMPLE__

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

#ifdef _LINUX
#include "linux/PlatformDefs.h"
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#endif

#include <string>

// disabled for now till we understand the true effect on cpu usage
#define NO_PERFORMANCE_MEASURE

#ifndef NO_PERFORMANCE_MEASURE
#define MEASURE_FUNCTION CPerformanceSample aSample(__FUNCTION__,true);
#define BEGIN_MEASURE_BLOCK(n) { CPerformanceSample aSample(n,true);
#define END_MEASURE_BLOCK }
#else
#define MEASURE_FUNCTION
#define BEGIN_MEASURE_BLOCK(n)
#define END_MEASURE_BLOCK
#endif

class CPerformanceSample
{
public:
  CPerformanceSample(const std::string &statName, bool bCheckWhenDone=true);
  virtual ~CPerformanceSample();

  void Reset();
  void CheckPoint(); // will add a sample to stats and restart counting.

  static double GetEstimatedError();

protected:
  std::string m_statName;
  bool m_bCheckWhenDone;

#ifdef _LINUX
  struct rusage m_usage;
#endif

  int64_t m_tmStart;
  static int64_t m_tmFreq;
};

#endif
