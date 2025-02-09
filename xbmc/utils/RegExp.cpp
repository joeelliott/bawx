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

#include <stdlib.h>
#include <string.h>
#include "RegExp.h"
#include "StdString.h"

#if defined (_WIN32) || defined (__APPLE__)
#include "lib/libpcre/pcre.h"
#else
#include <pcre.h>
#endif
#include "log.h"

using namespace PCRE;


CRegExpDateFormat::CRegExpDateFormat()
{
}
CRegExpDateFormat::~CRegExpDateFormat()
{
}
CRegExpDateFormat::CRegExpDateFormat(const std::string&  dateEx, const std::string& yearPos, const std::string& monthPos, const std::string& dayPos ):
     m_dateEx(dateEx), m_yearPos(yearPos), m_monthPos(monthPos), m_dayPos(dayPos)
{
}

CRegExp::CRegExp(bool caseless)
{
  m_re          = NULL;
  m_iOptions    = PCRE_DOTALL;
  if(caseless)
    m_iOptions |= PCRE_CASELESS;

  m_bMatched    = false;
  m_iMatchCount = 0;
  m_datesExp.clear();

  AddDateEx();
}

CRegExp::~CRegExp()
{
  Cleanup();
}

CRegExp* CRegExp::RegComp(const char *re)
{
  if (!re)
    return NULL;

  m_bMatched         = false;
  m_iMatchCount      = 0;
  const char *errMsg = NULL;
  int errOffset      = 0;

  Cleanup();

  m_re = pcre_compile(re, m_iOptions, &errMsg, &errOffset, NULL);
  if (!m_re)
  {
    CLog::Log(LOGERROR, "PCRE: %s. Compilation failed at offset %d in expression '%s'",
              errMsg, errOffset, re);
    return NULL;
  }

  return this;
}

int CRegExp::RegFind(const char* str, int startoffset)
{
  m_bMatched    = false;
  m_iMatchCount = 0;

  if (!m_re)
  {
    CLog::Log(LOGERROR, "PCRE: Called before compilation");
    return -1;
  }

  if (!str)
  {
    CLog::Log(LOGERROR, "PCRE: Called without a string to match");
    return -1;
  }

  m_subject = str;
  int rc = pcre_exec(m_re, NULL, str, strlen(str), startoffset, 0, m_iOvector, OVECCOUNT);

  if (rc<1)
  {
    switch(rc)
    {
    case PCRE_ERROR_NOMATCH:
      return -1;

    case PCRE_ERROR_MATCHLIMIT:
      CLog::Log(LOGERROR, "PCRE: Match limit reached");
      return -1;

    default:
      CLog::Log(LOGERROR, "PCRE: Unknown error: %d", rc);
      return -1;
    }
  }
  m_bMatched = true;
  m_iMatchCount = rc;
  return m_iOvector[0];
}

char* CRegExp::GetReplaceString( const char* sReplaceExp )
{
  char *src = (char *)sReplaceExp;
  char *buf;
  char c;
  int no;
  size_t len;

  if( sReplaceExp == NULL || !m_bMatched )
    return NULL;


  // First compute the length of the string
  int replacelen = 0;
  while ((c = *src++) != '\0')
  {
    if (c == '&')
      no = 0;
    else if (c == '\\' && isdigit(*src))
      no = *src++ - '0';
    else
      no = -1;

    if (no < 0)
    {
      // Ordinary character.
      if (c == '\\' && (*src == '\\' || *src == '&'))
        c = *src++;
      replacelen++;
    }
    else if (no < m_iMatchCount && (m_iOvector[no*2]>=0))
    {
      // Get tagged expression
      len = m_iOvector[no*2+1] - m_iOvector[no*2];
      replacelen += len;
    }
  }

  // Now allocate buf
  buf = (char *)malloc((replacelen + 1)*sizeof(char));
  if( buf == NULL )
    return NULL;

  char* sReplaceStr = buf;

  // Add null termination
  buf[replacelen] = '\0';

  // Now we can create the string
  src = (char *)sReplaceExp;
  while ((c = *src++) != '\0')
  {
    if (c == '&')
      no = 0;
    else if (c == '\\' && isdigit(*src))
      no = *src++ - '0';
    else
      no = -1;

    if (no < 0)
    {
      // Ordinary character.
      if (c == '\\' && (*src == '\\' || *src == '&'))
        c = *src++;
      *buf++ = c;
    }
    else if (no < m_iMatchCount && (m_iOvector[no*2]>=0))
    {
      // Get tagged expression
      len = m_iOvector[no*2+1] - m_iOvector[no*2];
      strncpy(buf, m_subject.c_str()+m_iOvector[no*2], len);
      buf += len;
    }
  }

  return sReplaceStr;
}

std::string CRegExp::GetMatch(int iSub /* = 0 */)
{
  if (iSub < 0 || iSub > m_iMatchCount)
    return "";

  int pos = m_iOvector[(iSub*2)];
  int len = m_iOvector[(iSub*2)+1] - pos;
  return m_subject.substr(pos, len);
}

bool CRegExp::GetNamedSubPattern(const char* strName, std::string& strMatch)
{
  strMatch.clear();
  int iSub = pcre_get_stringnumber(m_re, strName);
  if (iSub < 0)
    return false;
  strMatch = GetMatch(iSub);
  return true;
}

void CRegExp::DumpOvector(int iLog /* = LOGDEBUG */)
{
  if (iLog < LOGDEBUG || iLog > LOGNONE)
    return;

  CStdString str = "{";
  int size = GetSubCount(); // past the subpatterns is junk
  for (int i = 0; i <= size; i++)
  {
    CStdString t;
    t.Format("[%i,%i]", m_iOvector[(i*2)], m_iOvector[(i*2)+1]);
    if (i != size)
      t += ",";
    str += t;
  }
  str += "}";
  CLog::Log(iLog, "regexp ovector=%s", str.c_str());
}

void CRegExp::AddDateEx()
{
  // yyyy/mm/dd  yy/mm/dd  yyyy.mm.dd  yy-mm-dd etc ...
  m_datesExp.push_back(CRegExpDateFormat("(?:19|20)?(\\d\\d)[- /.](0[1-9]|1[012])[- /.](0[1-9]|[12][0-9]|3[01])","\\1","\\2","\\3"));
  // mm/dd/yyyy  mm.dd.yyyy mm-dd-yyyy etc ...
  m_datesExp.push_back(CRegExpDateFormat("(0[1-9]|1[012])[- /.](0[1-9]|[12][0-9]|3[01])[- /.]((?:19|20)?\\d\\d)","\\3","\\1","\\2"));

}

// find date expression - feel free to extend the function.
bool CRegExp::RegFindDate(const char *str, std::string& year, std::string& month, std::string& day, int& iTagPosition, int& iTagLength)
{

  int iPos = -1;

  for (int i = 0; i < m_datesExp.size() ; i ++ )
  {
    CRegExpDateFormat  dateFmt = m_datesExp[i];
    RegComp(dateFmt.m_dateEx.c_str());
    iPos = RegFind(str);

    if (iPos != -1)
    {
      // get the year
      char* tagStr = NULL;
      tagStr = GetReplaceString(dateFmt.m_yearPos.c_str());

      if (!tagStr)
        return false;

      year.assign(tagStr);;
      free(tagStr);

      // get the month
      tagStr = NULL;
      tagStr = GetReplaceString(dateFmt.m_monthPos.c_str());

      if (!tagStr)
        return false;

      month.assign(tagStr);
      free(tagStr);

      // get the day
      tagStr = NULL;
      tagStr = GetReplaceString(dateFmt.m_dayPos.c_str());

      if (!tagStr)
        return false;

      day.assign(tagStr);
      free(tagStr);

      iTagPosition = iPos;
      iTagLength = GetFindLen();

      return true;
    }
  }
  return false;
}
