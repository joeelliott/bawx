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

#include "DateTime.h"
#include "LangInfo.h"
#include "LocalizeStrings.h"
#include "StringUtils.h"

#define SECONDS_PER_DAY 86400UL
#define SECONDS_PER_HOUR 3600UL
#define SECONDS_PER_MINUTE 60UL
#define SECONDS_TO_FILETIME 10000000UL

/////////////////////////////////////////////////
//
// CDateTimeSpan
//

CDateTimeSpan::CDateTimeSpan()
{
  m_timeSpan.dwHighDateTime=0;
  m_timeSpan.dwLowDateTime=0;
}

CDateTimeSpan::CDateTimeSpan(const CDateTimeSpan& span)
{
  m_timeSpan.dwHighDateTime=span.m_timeSpan.dwHighDateTime;
  m_timeSpan.dwLowDateTime=span.m_timeSpan.dwLowDateTime;
}

CDateTimeSpan::CDateTimeSpan(int day, int hour, int minute, int second)
{
  SetDateTimeSpan(day, hour, minute, second);
}

bool CDateTimeSpan::operator >(const CDateTimeSpan& right) const
{
  return CompareFileTime(&m_timeSpan, &right.m_timeSpan)>0;
}

bool CDateTimeSpan::operator >=(const CDateTimeSpan& right) const
{
  return operator >(right) || operator ==(right);
}

bool CDateTimeSpan::operator <(const CDateTimeSpan& right) const
{
  return CompareFileTime(&m_timeSpan, &right.m_timeSpan)<0;
}

bool CDateTimeSpan::operator <=(const CDateTimeSpan& right) const
{
  return operator <(right) || operator ==(right);
}

bool CDateTimeSpan::operator ==(const CDateTimeSpan& right) const
{
  return CompareFileTime(&m_timeSpan, &right.m_timeSpan)==0;
}

bool CDateTimeSpan::operator !=(const CDateTimeSpan& right) const
{
  return !operator ==(right);
}

CDateTimeSpan CDateTimeSpan::operator +(const CDateTimeSpan& right) const
{
  CDateTimeSpan left(*this);

  ULARGE_INTEGER timeLeft;
  left.ToULargeInt(timeLeft);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeLeft.QuadPart+=timeRight.QuadPart;

  left.FromULargeInt(timeLeft);

  return left;
}

CDateTimeSpan CDateTimeSpan::operator -(const CDateTimeSpan& right) const
{
  CDateTimeSpan left(*this);

  ULARGE_INTEGER timeLeft;
  left.ToULargeInt(timeLeft);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeLeft.QuadPart-=timeRight.QuadPart;

  left.FromULargeInt(timeLeft);

  return left;
}

const CDateTimeSpan& CDateTimeSpan::operator +=(const CDateTimeSpan& right)
{
  ULARGE_INTEGER timeThis;
  ToULargeInt(timeThis);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeThis.QuadPart+=timeRight.QuadPart;

  FromULargeInt(timeThis);

  return *this;
}

const CDateTimeSpan& CDateTimeSpan::operator -=(const CDateTimeSpan& right)
{
  ULARGE_INTEGER timeThis;
  ToULargeInt(timeThis);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeThis.QuadPart-=timeRight.QuadPart;

  FromULargeInt(timeThis);

  return *this;
}

void CDateTimeSpan::ToULargeInt(ULARGE_INTEGER& time) const
{
  time.u.HighPart=m_timeSpan.dwHighDateTime;
  time.u.LowPart=m_timeSpan.dwLowDateTime;
}

void CDateTimeSpan::FromULargeInt(const ULARGE_INTEGER& time)
{
  m_timeSpan.dwHighDateTime=time.u.HighPart;
  m_timeSpan.dwLowDateTime=time.u.LowPart;
}

void CDateTimeSpan::SetDateTimeSpan(int day, int hour, int minute, int second)
{
  ULARGE_INTEGER time;
  ToULargeInt(time);

  time.QuadPart=(LONGLONG)day*SECONDS_PER_DAY*SECONDS_TO_FILETIME;
  time.QuadPart+=(LONGLONG)hour*SECONDS_PER_HOUR*SECONDS_TO_FILETIME;
  time.QuadPart+=(LONGLONG)minute*SECONDS_PER_MINUTE*SECONDS_TO_FILETIME;
  time.QuadPart+=(LONGLONG)second*SECONDS_TO_FILETIME;

  FromULargeInt(time);
}

int CDateTimeSpan::GetDays() const
{
  ULARGE_INTEGER time;
  ToULargeInt(time);

  return (int)(time.QuadPart/SECONDS_TO_FILETIME)/SECONDS_PER_DAY;
}

int CDateTimeSpan::GetHours() const
{
  ULARGE_INTEGER time;
  ToULargeInt(time);

  return (int)((time.QuadPart/SECONDS_TO_FILETIME)%SECONDS_PER_DAY)/SECONDS_PER_HOUR;
}

int CDateTimeSpan::GetMinutes() const
{
  ULARGE_INTEGER time;
  ToULargeInt(time);

  return (int)((time.QuadPart/SECONDS_TO_FILETIME%SECONDS_PER_DAY)%SECONDS_PER_HOUR)/SECONDS_PER_MINUTE;
}

int CDateTimeSpan::GetSeconds() const
{
  ULARGE_INTEGER time;
  ToULargeInt(time);

  return (int)(((time.QuadPart/SECONDS_TO_FILETIME%SECONDS_PER_DAY)%SECONDS_PER_HOUR)%SECONDS_PER_MINUTE)%SECONDS_PER_MINUTE;
}

void CDateTimeSpan::SetFromPeriod(const CStdString &period)
{
  long days = atoi(period.c_str());
  // find the first non-space and non-number
  int pos = period.find_first_not_of("0123456789 ", 0);
  if (pos >= 0)
  {
    CStdString units = period.Mid(pos, 3);
    if (units.CompareNoCase("wee") == 0)
      days *= 7;
    else if (units.CompareNoCase("mon") == 0)
      days *= 31;
  }

  SetDateTimeSpan(days, 0, 0, 0);
}

/////////////////////////////////////////////////
//
// CDateTime
//

CDateTime::CDateTime()
{
  Reset();
}

CDateTime::CDateTime(const SYSTEMTIME &time)
{
  // we store internally as a FILETIME
  m_state = ToFileTime(time, m_time) ? valid : invalid;
}

CDateTime::CDateTime(const FILETIME &time)
{
  m_time=time;
  SetValid(true);
}

CDateTime::CDateTime(const CDateTime& time) : IArchivable(time)
{
  m_time=time.m_time;
  m_state=time.m_state;
}

CDateTime::CDateTime(const time_t& time)
{
  m_state = ToFileTime(time, m_time) ? valid : invalid;
}

CDateTime::CDateTime(const tm& time)
{
  m_state = ToFileTime(time, m_time) ? valid : invalid;
}

CDateTime::CDateTime(int year, int month, int day, int hour, int minute, int second)
{
  SetDateTime(year, month, day, hour, minute, second);
}

CDateTime CDateTime::GetCurrentDateTime()
{
  // get the current time
  SYSTEMTIME time;
  GetLocalTime(&time);

  return CDateTime(time);
}

CDateTime CDateTime::GetUTCDateTime()
{
  TIME_ZONE_INFORMATION tz;

  CDateTime time(GetCurrentDateTime());
  switch(GetTimeZoneInformation(&tz))
  {
    case TIME_ZONE_ID_DAYLIGHT:
        time += CDateTimeSpan(0, 0, tz.Bias + tz.DaylightBias, 0);
        break;
    case TIME_ZONE_ID_STANDARD:
        time += CDateTimeSpan(0, 0, tz.Bias + tz.StandardBias, 0);
        break;
    case TIME_ZONE_ID_UNKNOWN:
        time += CDateTimeSpan(0, 0, tz.Bias, 0);
        break;
  }

  return time;
}

const CDateTime& CDateTime::operator =(const SYSTEMTIME& right)
{
  m_state = ToFileTime(right, m_time) ? valid : invalid;

  return *this;
}

const CDateTime& CDateTime::operator =(const FILETIME& right)
{
  m_time=right;
  SetValid(true);

  return *this;
}

const CDateTime& CDateTime::operator =(const time_t& right)
{
  m_state = ToFileTime(right, m_time) ? valid : invalid;

  return *this;
}

const CDateTime& CDateTime::operator =(const tm& right)
{
  m_state = ToFileTime(right, m_time) ? valid : invalid;

  return *this;
}

bool CDateTime::operator >(const CDateTime& right) const
{
  return operator >(right.m_time);
}

bool CDateTime::operator >=(const CDateTime& right) const
{
  return operator >(right) || operator ==(right);
}

bool CDateTime::operator <(const CDateTime& right) const
{
  return operator <(right.m_time);
}

bool CDateTime::operator <=(const CDateTime& right) const
{
  return operator <(right) || operator ==(right);
}

bool CDateTime::operator ==(const CDateTime& right) const
{
  return operator ==(right.m_time);
}

bool CDateTime::operator !=(const CDateTime& right) const
{
  return !operator ==(right);
}

bool CDateTime::operator >(const FILETIME& right) const
{
  return CompareFileTime(&m_time, &right)>0;
}

bool CDateTime::operator >=(const FILETIME& right) const
{
  return operator >(right) || operator ==(right);
}

bool CDateTime::operator <(const FILETIME& right) const
{
  return CompareFileTime(&m_time, &right)<0;
}

bool CDateTime::operator <=(const FILETIME& right) const
{
  return operator <(right) || operator ==(right);
}

bool CDateTime::operator ==(const FILETIME& right) const
{
  return CompareFileTime(&m_time, &right)==0;
}

bool CDateTime::operator !=(const FILETIME& right) const
{
  return !operator ==(right);
}

bool CDateTime::operator >(const SYSTEMTIME& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator >(time);
}

bool CDateTime::operator >=(const SYSTEMTIME& right) const
{
  return operator >(right) || operator ==(right);
}

bool CDateTime::operator <(const SYSTEMTIME& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator <(time);
}

bool CDateTime::operator <=(const SYSTEMTIME& right) const
{
  return operator <(right) || operator ==(right);
}

bool CDateTime::operator ==(const SYSTEMTIME& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator ==(time);
}

bool CDateTime::operator !=(const SYSTEMTIME& right) const
{
  return !operator ==(right);
}

bool CDateTime::operator >(const time_t& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator >(time);
}

bool CDateTime::operator >=(const time_t& right) const
{
  return operator >(right) || operator ==(right);
}

bool CDateTime::operator <(const time_t& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator <(time);
}

bool CDateTime::operator <=(const time_t& right) const
{
  return operator <(right) || operator ==(right);
}

bool CDateTime::operator ==(const time_t& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator ==(time);
}

bool CDateTime::operator !=(const time_t& right) const
{
  return !operator ==(right);
}

bool CDateTime::operator >(const tm& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator >(time);
}

bool CDateTime::operator >=(const tm& right) const
{
  return operator >(right) || operator ==(right);
}

bool CDateTime::operator <(const tm& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator <(time);
}

bool CDateTime::operator <=(const tm& right) const
{
  return operator <(right) || operator ==(right);
}

bool CDateTime::operator ==(const tm& right) const
{
  FILETIME time;
  ToFileTime(right, time);

  return operator ==(time);
}

bool CDateTime::operator !=(const tm& right) const
{
  return !operator ==(right);
}

CDateTime CDateTime::operator +(const CDateTimeSpan& right) const
{
  CDateTime left(*this);

  ULARGE_INTEGER timeLeft;
  left.ToULargeInt(timeLeft);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeLeft.QuadPart+=timeRight.QuadPart;

  left.FromULargeInt(timeLeft);

  return left;
}

CDateTime CDateTime::operator -(const CDateTimeSpan& right) const
{
  CDateTime left(*this);

  ULARGE_INTEGER timeLeft;
  left.ToULargeInt(timeLeft);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeLeft.QuadPart-=timeRight.QuadPart;

  left.FromULargeInt(timeLeft);

  return left;
}

const CDateTime& CDateTime::operator +=(const CDateTimeSpan& right)
{
  ULARGE_INTEGER timeThis;
  ToULargeInt(timeThis);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeThis.QuadPart+=timeRight.QuadPart;

  FromULargeInt(timeThis);

  return *this;
}

const CDateTime& CDateTime::operator -=(const CDateTimeSpan& right)
{
  ULARGE_INTEGER timeThis;
  ToULargeInt(timeThis);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeThis.QuadPart-=timeRight.QuadPart;

  FromULargeInt(timeThis);

  return *this;
}

CDateTimeSpan CDateTime::operator -(const CDateTime& right) const
{
  CDateTimeSpan left;

  ULARGE_INTEGER timeLeft;
  left.ToULargeInt(timeLeft);

  ULARGE_INTEGER timeThis;
  ToULargeInt(timeThis);

  ULARGE_INTEGER timeRight;
  right.ToULargeInt(timeRight);

  timeLeft.QuadPart=timeThis.QuadPart-timeRight.QuadPart;

  left.FromULargeInt(timeLeft);

  return left;
}

CDateTime::operator FILETIME() const
{
  return m_time;
}

void CDateTime::Archive(CArchive& ar)
{
  if (ar.IsStoring())
  {
    ar<<(int)m_state;
    if (m_state==valid)
    {
      SYSTEMTIME st;
      GetAsSystemTime(st);
      ar<<st;
    }
  }
  else
  {
    Reset();
    int state;
    ar >> (int &)state;
    m_state = CDateTime::STATE(state);
    if (m_state==valid)
    {
      SYSTEMTIME st;
      ar>>st;
      ToFileTime(st, m_time);
    }
  }
}

void CDateTime::Reset()
{
  SetDateTime(1601, 1, 1, 0, 0, 0);
  SetValid(false);
}

void CDateTime::SetValid(bool yesNo)
{
  m_state=yesNo ? valid : invalid;
}

bool CDateTime::IsValid() const
{
  return m_state==valid;
}

bool CDateTime::ToFileTime(const SYSTEMTIME& time, FILETIME& fileTime) const
{
  return SystemTimeToFileTime(&time, &fileTime)==TRUE;
}

bool CDateTime::ToFileTime(const time_t& time, FILETIME& fileTime) const
{
  LONGLONG ll = Int32x32To64(time, 10000000)+0x19DB1DED53E8000LL;
  
  fileTime.dwLowDateTime  = (DWORD)(ll & 0xFFFFFFFF);
  fileTime.dwHighDateTime = (DWORD)(ll >> 32);
  
  return true;
}

bool CDateTime::ToFileTime(const tm& time, FILETIME& fileTime) const
{
  SYSTEMTIME st;
  ZeroMemory(&st, sizeof(SYSTEMTIME));

  st.wYear=time.tm_year+1900;
  st.wMonth=time.tm_mon+1;
  st.wDayOfWeek=time.tm_wday;
  st.wDay=time.tm_mday;
  st.wHour=time.tm_hour;
  st.wMinute=time.tm_min;
  st.wSecond=time.tm_sec;

  return SystemTimeToFileTime(&st, &fileTime)==TRUE;
}

void CDateTime::ToULargeInt(ULARGE_INTEGER& time) const
{
  time.u.HighPart=m_time.dwHighDateTime;
  time.u.LowPart=m_time.dwLowDateTime;
}

void CDateTime::FromULargeInt(const ULARGE_INTEGER& time)
{
  m_time.dwHighDateTime=time.u.HighPart;
  m_time.dwLowDateTime=time.u.LowPart;
}

void CDateTime::SetFromDateString(const CStdString &date)
{
  const char* months[] = {"january","february","march","april","may","june","july","august","september","october","november","december",NULL};
  int j=0;
  int iDayPos = date.Find("day");
  int iPos = date.Find(" ");
  if (iDayPos < iPos && iDayPos > -1)
  {
    iDayPos = iPos+1;
    iPos = date.Find(" ",iPos+1);
  }
  else
    iDayPos = 0;

  CStdString strMonth = date.Mid(iDayPos,iPos-iDayPos);
  if (strMonth.IsEmpty()) // assume dbdate format
  {
    SetFromDBDate(date);
    return;
  }

  int iPos2 = date.Find(",");
  CStdString strDay = date.Mid(iPos,iPos2-iPos);
  CStdString strYear = date.Mid(date.Find(" ",iPos2)+1);
  while (months[j] && stricmp(strMonth.c_str(),months[j]) != 0)
    j++;
  if (!months[j])
    return;

  SetDateTime(atol(strYear.c_str()),j+1,atol(strDay.c_str()),0,0,0);
}

int CDateTime::GetDay() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  return st.wDay;
}

int CDateTime::GetMonth() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  return st.wMonth;
}

int CDateTime::GetYear() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  return st.wYear;
}

int CDateTime::GetHour() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  return st.wHour;
}

int CDateTime::GetMinute() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  return st.wMinute;
}

int CDateTime::GetSecond() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  return st.wSecond;
}

int CDateTime::GetDayOfWeek() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  return st.wDayOfWeek;
}

int CDateTime::GetMinuteOfDay() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);
  return st.wHour*60+st.wMinute;
}

void CDateTime::SetDateTime(int year, int month, int day, int hour, int minute, int second)
{
  SYSTEMTIME st;
  ZeroMemory(&st, sizeof(SYSTEMTIME));

  st.wYear=year;
  st.wMonth=month;
  st.wDay=day;
  st.wHour=hour;
  st.wMinute=minute;
  st.wSecond=second;

  m_state = ToFileTime(st, m_time) ? valid : invalid;
}

void CDateTime::SetDate(int year, int month, int day)
{
  SetDateTime(year, month, day, 0, 0, 0);
}

void CDateTime::SetTime(int hour, int minute, int second)
{
  // 01.01.1601 00:00:00 is 0 as filetime
  SetDateTime(1601, 1, 1, hour, minute, second);
}

void CDateTime::GetAsSystemTime(SYSTEMTIME& time) const
{
  FileTimeToSystemTime(&m_time, &time);
}

#define UNIX_BASE_TIME 116444736000000000LL /* nanoseconds since epoch */
void CDateTime::GetAsTime(time_t& time) const
{
  LONGLONG ll;
  ll = ((LONGLONG)m_time.dwHighDateTime << 32) + m_time.dwLowDateTime;
  time=(time_t)((ll - UNIX_BASE_TIME) / 10000000);
  
}

void CDateTime::GetAsTm(tm& time) const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  time.tm_year=st.wYear-1900;
  time.tm_mon=st.wMonth-1;
  time.tm_wday=st.wDayOfWeek;
  time.tm_mday=st.wDay;
  time.tm_hour=st.wHour;
  time.tm_min=st.wMinute;
  time.tm_sec=st.wSecond;

  mktime(&time);
}

void CDateTime::GetAsTimeStamp(FILETIME& time) const
{
  ::LocalFileTimeToFileTime(&m_time, &time);
}

CStdString CDateTime::GetAsddMMMYYYYDate()
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  CStdString date;
  date.Format("%02i %s %04i  ", st.wDay, ConvertMonthToString(st.wMonth).c_str(), st.wYear);

  return date;
}

CStdString CDateTime::ConvertMonthToString(int month)
{

  /* By XBMC
  const char *mon[] =  {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  */

  //Boxee
  const char *mon[] =  {
      g_localizeStrings.Get(57121), g_localizeStrings.Get(57122), g_localizeStrings.Get(57123), g_localizeStrings.Get(57124), g_localizeStrings.Get(57125), g_localizeStrings.Get(57126),
      g_localizeStrings.Get(57127), g_localizeStrings.Get(57128), g_localizeStrings.Get(57129), g_localizeStrings.Get(57130), g_localizeStrings.Get(57131), g_localizeStrings.Get(57132)
    };
  //End Boxee

  return mon[month - 1];
}

CStdString CDateTime::ConvertDayofWeekToString(int day)
{

  /* By XBMC
  const char *DAY_NAMES[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  */

  //Boxee
  const char *days[] =  { g_localizeStrings.Get(17), g_localizeStrings.Get(11), g_localizeStrings.Get(12), g_localizeStrings.Get(13), g_localizeStrings.Get(14), g_localizeStrings.Get(15), g_localizeStrings.Get(16)};
  //End Boxee

  return days[day];
}


CStdString CDateTime::GetAsDBDate() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  CStdString date;
  date.Format("%04i-%02i-%02i", st.wYear, st.wMonth, st.wDay);

  return date;
}

CStdString CDateTime::GetAsDBDateTime() const
{
  SYSTEMTIME st;
  GetAsSystemTime(st);

  CStdString date;
  date.Format("%04i-%02i-%02i %02i:%02i:%02i", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

  return date;
}

void CDateTime::SetFromDBDateTime(const CStdString &date)
{
  int year = 0, month = 0, day = 0;
  year = atoi(date.Mid(0,4).c_str());
  month = atoi(date.Mid(5,2).c_str());
  day = atoi(date.Mid(8,2).c_str());

  int hour = 0, minutes = 0, seconds = 0;
  hour = atoi(date.Mid(11,2));
  minutes = atoi(date.Mid(14,2));
  seconds = atoi(date.Mid(17,2));

  SetDateTime(year,month,day,hour,minutes,seconds);
}

void CDateTime::SetFromDBDate(const CStdString &date)
{
  // assumes format:
  // YYYY-MM-DD
  int year = 0, month = 0, day = 0;
  year = atoi(date.Mid(0,4).c_str());
  month = atoi(date.Mid(5,2).c_str());
  day = atoi(date.Mid(8,2).c_str());
  SetDate(year, month, day);
}

CStdString CDateTime::GetAsLocalizedTime(const CStdString &format, bool withSeconds) const
{
  CStdString strOut;
  const CStdString& strFormat = format.IsEmpty() ? g_langInfo.GetTimeFormat() : format;

  SYSTEMTIME dateTime;
  GetAsSystemTime(dateTime);

  // Prefetch meridiem symbol
  const CStdString& strMeridiem=g_langInfo.GetMeridiemSymbol(dateTime.wHour > 11 ? CLangInfo::MERIDIEM_SYMBOL_PM : CLangInfo::MERIDIEM_SYMBOL_AM);

  int length=strFormat.GetLength();
  for (int i=0; i<length; ++i)
  {
    char c=strFormat[i];
    if (c=='\'')
    {
      // To be able to display a "'" in the string,
      // find the last "'" that doesn't follow a "'"
      int pos=i+1;
      while(((pos=strFormat.Find(c,pos+1))>-1 && pos<strFormat.GetLength()) && strFormat[pos+1]=='\'') {}

      CStdString strPart;
      if (pos>-1)
      {
        // Extract string between ' '
        strPart=strFormat.Mid(i+1, pos-i-1);
        i=pos;
      }
      else
      {
        strPart=strFormat.Mid(i+1, length-i-1);
        i=length;
      }

      strPart.Replace("''", "'");

      strOut+=strPart;
    }
    else if (c=='h' || c=='H') // parse hour (H="24 hour clock")
    {
      int partLength=0;

      int pos=strFormat.find_first_not_of(c,i+1);
      if (pos>-1)
      {
        // Get length of the hour mask, eg. HH
        partLength=pos-i;
        i=pos-1;
      }
      else
      {
        // mask ends at the end of the string, extract it
        partLength=length-i;
        i=length;
      }

      int hour=dateTime.wHour;
      if (c=='h')
      { // recalc to 12 hour clock
        if (hour > 11)
          hour -= (12 * (hour > 12));
        else
          hour += (12 * (hour < 1));
      }

      // Format hour string with the length of the mask
      CStdString str;
      if (partLength==1)
        str.Format("%d", hour);
      else
        str.Format("%02d", hour);

      strOut+=str;
    }
    else if (c=='m') // parse minutes
    {
      int partLength=0;

      int pos=strFormat.find_first_not_of(c,i+1);
      if (pos>-1)
      {
        // Get length of the minute mask, eg. mm
        partLength=pos-i;
        i=pos-1;
      }
      else
      {
        // mask ends at the end of the string, extract it
        partLength=length-i;
        i=length;
      }

      // Format minute string with the length of the mask
      CStdString str;
      if (partLength==1)
        str.Format("%d", dateTime.wMinute);
      else
        str.Format("%02d", dateTime.wMinute);

      strOut+=str;
    }
    else if (c=='s') // parse seconds
    {
      int partLength=0;

      int pos=strFormat.find_first_not_of(c,i+1);
      if (pos>-1)
      {
        // Get length of the seconds mask, eg. ss
        partLength=pos-i;
        i=pos-1;
      }
      else
      {
        // mask ends at the end of the string, extract it
        partLength=length-i;
        i=length;
      }

      if (withSeconds)
      {
        // Format seconds string with the length of the mask
        CStdString str;
        if (partLength==1)
          str.Format("%d", dateTime.wSecond);
        else
          str.Format("%02d", dateTime.wSecond);

        strOut+=str;
      }
      else
        strOut.Delete(strOut.GetLength()-1,1);
    }
    else if (c=='x') // add meridiem symbol
    {
      int partLength=0;

      int pos=strFormat.find_first_not_of(c,i+1);
      if (pos>-1)
      {
        // Get length of the meridiem mask
        partLength=pos-i;
        i=pos-1;
      }
      else
      {
        // mask ends at the end of the string, extract it
        partLength=length-i;
        i=length;
      }

      strOut+=strMeridiem;
    }
    else // everything else pass to output
      strOut+=c;
  }

  return strOut;
}

CStdString CDateTime::GetAsLocalizedDate(bool longDate/*=false*/) const
{
  CStdString strOut;

  const CStdString& strFormat=g_langInfo.GetDateFormat(longDate);

  time_t t;
  GetAsTime(t);
  if (t == 0)
    return StringUtils::EmptyString;
  
  SYSTEMTIME dateTime;
  GetAsSystemTime(dateTime);

  int length=strFormat.GetLength();

  for (int i=0; i<length; ++i)
  {
    char c=strFormat[i];
    if (c=='\'')
    {
      // To be able to display a "'" in the string,
      // find the last "'" that doesn't follow a "'"
      int pos=i+1;
      while(((pos=strFormat.Find(c,pos+1))>-1 && pos<strFormat.GetLength()) && strFormat[pos+1]=='\'') {}

      CStdString strPart;
      if (pos>-1)
      {
        // Extract string between ' '
        strPart=strFormat.Mid(i+1, pos-i-1);
        i=pos;
      }
      else
      {
        strPart=strFormat.Mid(i+1, length-i-1);
        i=length;
      }
      strPart.Replace("''", "'");
      strOut+=strPart;
    }
    else if (c=='D') // parse days
    {
      int partLength=0;

      int pos=strFormat.find_first_not_of(c,i+1);
      if (pos>-1)
      {
        // Get length of the day mask, eg. DDDD
        partLength=pos-i;
        i=pos-1;
      }
      else
      {
        // mask ends at the end of the string, extract it
        partLength=length-i;
        i=length;
      }

      // Format string with the length of the mask
      CStdString str;
      if (partLength==1) // single-digit number
        str.Format("%d", dateTime.wDay);
      else if (partLength==2) // two-digit number
        str.Format("%02d", dateTime.wDay);
      else // Day of week string
      {
        switch (dateTime.wDayOfWeek)
        {
          case 1 : str = g_localizeStrings.Get(11); break;
          case 2 : str = g_localizeStrings.Get(12); break;
          case 3 : str = g_localizeStrings.Get(13); break;
          case 4 : str = g_localizeStrings.Get(14); break;
          case 5 : str = g_localizeStrings.Get(15); break;
          case 6 : str = g_localizeStrings.Get(16); break;
          default: str = g_localizeStrings.Get(17); break;
        }
      }
      strOut+=str;
    }
    else if (c=='M') // parse month
    {
      int partLength=0;

      int pos=strFormat.find_first_not_of(c,i+1);
      if (pos>-1)
      {
        // Get length of the month mask, eg. MMMM
        partLength=pos-i;
        i=pos-1;
      }
      else
      {
        // mask ends at the end of the string, extract it
        partLength=length-i;
        i=length;
      }

      // Format string with the length of the mask
      CStdString str;
      if (partLength==1) // single-digit number
        str.Format("%d", dateTime.wMonth);
      else if (partLength==2) // two-digit number
        str.Format("%02d", dateTime.wMonth);
      else // Month string
      {
        switch (dateTime.wMonth)
        {
          case 1 : str = g_localizeStrings.Get(21); break;
          case 2 : str = g_localizeStrings.Get(22); break;
          case 3 : str = g_localizeStrings.Get(23); break;
          case 4 : str = g_localizeStrings.Get(24); break;
          case 5 : str = g_localizeStrings.Get(25); break;
          case 6 : str = g_localizeStrings.Get(26); break;
          case 7 : str = g_localizeStrings.Get(27); break;
          case 8 : str = g_localizeStrings.Get(28); break;
          case 9 : str = g_localizeStrings.Get(29); break;
          case 10: str = g_localizeStrings.Get(30); break;
          case 11: str = g_localizeStrings.Get(31); break;
          default: str = g_localizeStrings.Get(32); break;
        }
      }
      strOut+=str;
    }
    else if (c=='Y') // parse year
    {
      int partLength=0;

      int pos=strFormat.find_first_not_of(c,i+1);
      if (pos>-1)
      {
        // Get length of the year mask, eg. YYYY
        partLength=pos-i;
        i=pos-1;
      }
      else
      {
        // mask ends at the end of the string, extract it
        partLength=length-i;
        i=length;
      }

      // Format string with the length of the mask
      CStdString str;
      str.Format("%d", dateTime.wYear); // four-digit number
      if (partLength<=2)
        str.Delete(0, 2); // two-digit number

      strOut+=str;
    }
    else // everything else pass to output
      strOut+=c;
  }

  return strOut;
}

CStdString CDateTime::GetAsLocalizedDateTime(bool longDate/*=false*/, bool withSeconds/*=true*/) const
{
  return GetAsLocalizedDate(longDate)+" "+GetAsLocalizedTime("", withSeconds);
}

CDateTime CDateTime::GetAsUTCDateTime() const
{
  TIME_ZONE_INFORMATION tz;

  CDateTime time(m_time);
  switch(GetTimeZoneInformation(&tz))
  {
    case TIME_ZONE_ID_DAYLIGHT:
        time += CDateTimeSpan(0, 0, tz.Bias + tz.DaylightBias, 0);
        break;
    case TIME_ZONE_ID_STANDARD:
        time += CDateTimeSpan(0, 0, tz.Bias + tz.StandardBias, 0);
        break;
    case TIME_ZONE_ID_UNKNOWN:
        time += CDateTimeSpan(0, 0, tz.Bias, 0);
        break;
  }

  return time;
}

static const char *DAY_NAMES[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *MONTH_NAMES[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

CStdString CDateTime::GetAsRFC1123DateTime() const
{
  CDateTime time(GetAsUTCDateTime());
  CStdString result;
  result.Format("%s, %02i %s %04i %02i:%02i:%02i GMT", DAY_NAMES[time.GetDayOfWeek()], time.GetDay(), MONTH_NAMES[time.GetMonth()-1], time.GetYear(), time.GetHour(), time.GetMinute(), time.GetSecond());
  return result;
}
