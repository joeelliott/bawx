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

#include <limits.h>

#include "system.h"
#include "AdvancedSettings.h"
#include "Application.h"
#include "DNSNameCache.h"
#include "FileSystem/File.h"
#include "LangCodeExpander.h"
#include "LangInfo.h"
#include "Util.h"
#include "GUISettings.h"
#include "Settings.h"
#include "StringUtils.h"
#include "SystemInfo.h"
#include "XMLUtils.h"
#include "utils/log.h"

using namespace XFILE;

#define DEFAULT_LOG_FILE_SIZE  (32 * 1024 * 1024)

CAdvancedSettings g_advancedSettings;

CAdvancedSettings::CAdvancedSettings()
{
  m_useMultipaths = true;

  m_audioHeadRoom = 0;
  m_ac3Gain = 12.0f;
  m_audioApplyDrc = true;      

  m_karaokeSyncDelayCDG = 0.0f;
  m_karaokeSyncDelayLRC = 0.0f;
  m_karaokeChangeGenreForKaraokeSongs = false;
  m_karaokeKeepDelay = true;
  m_karaokeStartIndex = 1;
  m_karaokeAlwaysEmptyOnCdgs = 1;
  m_karaokeUseSongSpecificBackground = 0;

  m_audioDefaultPlayer = "paplayer";
  m_audioPlayCountMinimumPercent = 90.0f;
  m_videoVDPAUScaling = false;
  m_audioHost = "default";

  m_videoSubsDelayRange = 200;
  m_videoAudioDelayRange = 25;
  m_videoSmallStepBackSeconds = 7;
  m_videoSmallStepBackTries = 3;
  m_videoSmallStepBackDelay = 300;
  m_videoUseTimeSeeking = true;
  m_videoTimeSeekForward = 30;
  m_videoTimeSeekBackward = -10;
  m_videoTimeSeekForwardBig = 600;
  m_videoTimeSeekBackwardBig = -600;
  m_videoPercentSeekForward = 2;
  m_videoPercentSeekBackward = -2;
  m_videoPercentSeekForwardBig = 10;
  m_videoPercentSeekBackwardBig = -10;
  m_videoBlackBarColour = 1;
  m_videoPPFFmpegType = "linblenddeint";
  m_videoDefaultPlayer = "dvdplayer";
  m_videoDefaultDVDPlayer = "dvdplayer";
  m_videoIgnoreAtStart = 15;
  m_videoIgnoreAtEnd = 5; 
  m_videoPlayCountMinimumPercent = 90.0f;

  m_videoNonLinStretchRatio = 0.5f;
  m_videoAllowLanczos3 = false;
  m_videoAutoScaleMaxFps = 30.0;
  m_videoAllowMpeg4VDPAU = false;
  m_DXVACheckCompatibility = false;
  m_DXVACheckCompatibilityPresent = false;

  m_musicUseTimeSeeking = true;
  m_musicTimeSeekForward = 10;
  m_musicTimeSeekBackward = -10;
  m_musicTimeSeekForwardBig = 60;
  m_musicTimeSeekBackwardBig = -60;
  m_musicPercentSeekForward = 1;
  m_musicPercentSeekBackward = -1;
  m_musicPercentSeekForwardBig = 10;
  m_musicPercentSeekBackwardBig = -10;
  m_musicResample = 0;

  m_slideshowPanAmount = 2.5f;
  m_slideshowZoomAmount = 5.0f;
  m_slideshowBlackBarCompensation = 20.0f;

  m_lcdRows = 4;
  m_lcdColumns = 20;
  m_lcdAddress1 = 0;
  m_lcdAddress2 = 0x40;
  m_lcdAddress3 = 0x14;
  m_lcdAddress4 = 0x54;
  m_lcdScrolldelay = 1;

  m_autoDetectPingTime = 30;

  m_songInfoDuration = 10;
  m_busyDialogDelay = 2000;
  m_cddbAddress = "freedb.freedb.org";
#ifdef HAS_HAL
  m_useHalMount = g_application.IsStandAlone();
#endif
  m_fullScreenOnMovieStart = true;
  m_noDVDROM = false;
  m_cachePath = "special://temp/";
  m_displayRemoteCodes = false;

  m_videoCleanDateTimeRegExp = "(.+[^ _\\,\\.\\(\\)\\[\\]\\-])[ _\\.\\(\\)\\[\\]\\-]+(19[0-9][0-9]|20[0-1][0-9])([ _\\,\\.\\(\\)\\[\\]\\-]|[^0-9]$)";

  m_videoCleanStringRegExps.push_back("[ _\\,\\.\\(\\)\\[\\]\\-](ac3|dts|custom|dc|divx|divx5|dsr|dsrip|dutch|dvd|dvd5|dvd9|dvdrip|dvdscr|dvdscreener|screener|dvdivx|cam|fragment|fs|hdtv|hdrip|hdtvrip|internal|limited|multisubs|ntsc|ogg|ogm|pal|pdtv|proper|repack|rerip|retail|r3|r5|bd5|se|svcd|swedish|german|read.nfo|nfofix|unrated|extended|ws|telesync|ts|telecine|tc|brrip|bdrip|480p|480i|576p|576i|720p|720i|1080p|1080i|hrhd|hrhdtv|hddvd|bluray|x264|h264|xvid|xvidvd|xxx|www.www|subbed|festival|workprint|swesub|nlsub|nl-subs|dvdrscreener|italian|cd[1-9]|\\[.*\\])([ _\\,\\.\\(\\)\\[\\]\\-]|$)");
  m_videoCleanStringRegExps.push_back("(\\[.*\\])");

  m_moviesExcludeFromScanRegExps.push_back("-trailer");
  m_moviesExcludeFromScanRegExps.push_back("[-._ ]sample");
  m_tvshowExcludeFromScanRegExps.push_back("[-._ ]sample[-._ ]");

  m_videoStackRegExps.push_back("()[ _.-]*?(?:cd|dvd|p(?:ar)t|dis[ck])[ _.-]*?([0-9a-d]+)(.*\\....?.?)$");
  m_videoStackRegExps.push_back("()[ ._-]*?([a-c0-3]+)(\\....?.?)$");
  m_videoStackRegExps.push_back("()[ ._-]+(0?[a-c1-3])[ ._-]+(.*?\\....?.?)$");

  // foo_[s01]_[e01]
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"\\[[Ss]([0-9]+)\\]_\\[[Ee]([0-9]+)\\]?([^\\\\/]*)$"));
  // foo.1x09* or just /1x09*
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"[\\\\/\\._ \\[-]([0-9]+)x([0-9]+)([^\\\\/]*)$"));
  // foo.s01.e01, foo.s01_e01, S01E02 foo
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"[Ss]([0-9]+)[\\.-]?[Ee]([0-9]+)([^\\\\/]*)$"));
  // foo.yyyy.mm.dd.* (byDate=true)
  m_tvshowStackRegExps.push_back(TVShowRegexp(true,"([0-9]{4})[\\.-]([0-9]{2})[\\.-]([0-9]{2})"));
  // foo.mm.dd.yyyy.* (byDate=true)
  m_tvshowStackRegExps.push_back(TVShowRegexp(true,"([0-9]{2})[\\.-]([0-9]{2})[\\.-]([0-9]{4})"));
  // foo.103*, 103 foo
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"[\\\\/\\._ -]([0-9]+)([0-9][0-9])([\\._ -][^\\\\/]*)$"));

  m_tvshowMultiPartStackRegExp = "^[-EeXx]+([0-9]+)";

  m_remoteDelay = 3;
  m_controllerDeadzone = 0.2f;

  m_playlistAsFolders = true;
  m_detectAsUdf = false;

  m_thumbSize = DEFAULT_THUMB_SIZE;
  m_useDDSFanart = false;

  m_sambaclienttimeout = 10;
  m_sambadoscodepage = "";
  m_sambastatfiles = true;

  m_bHTTPDirectoryStatFilesize = false;

  m_musicThumbs = "folder.jpg|Folder.jpg|folder.JPG|Folder.JPG|cover.jpg|Cover.jpg|cover.jpeg";
  m_dvdThumbs = "folder.jpg|Folder.jpg|folder.JPG|Folder.JPG";
  m_fanartImages = "fanart.jpg|fanart.png";

  m_bMusicLibraryHideAllItems = false;
  m_bMusicLibraryAllItemsOnBottom = false;
  m_bMusicLibraryAlbumsSortByArtistThenYear = false;
  m_iMusicLibraryRecentlyAddedItems = 25;
  m_strMusicLibraryAlbumFormat = "";
  m_strMusicLibraryAlbumFormatRight = "";
  m_prioritiseAPEv2tags = false;
  m_musicItemSeparator = " / ";
  m_videoItemSeparator = " / ";

  m_bVideoLibraryHideAllItems = false;
  m_bVideoLibraryAllItemsOnBottom = false;
  m_iVideoLibraryRecentlyAddedItems = 25;
  m_bVideoLibraryHideRecentlyAddedItems = false;
  m_bVideoLibraryHideEmptySeries = false;
  m_bVideoLibraryCleanOnUpdate = false;
  m_bVideoLibraryExportAutoThumbs = false;
  m_bVideoLibraryMyMoviesCategoriesToGenres = false;

  m_bUseEvilB = true;

  m_bTuxBoxAudioChannelSelection = false;
  m_bTuxBoxSubMenuSelection = false;
  m_bTuxBoxPictureIcon= true;
  m_bTuxBoxSendAllAPids= false;
  m_iTuxBoxEpgRequestTime = 10; //seconds
  m_iTuxBoxDefaultSubMenu = 4;
  m_iTuxBoxDefaultRootMenu = 0; //default TV Mode
  m_iTuxBoxZapWaitTime = 0; // Time in sec. Default 0:OFF

  m_iMythMovieLength = 0; // 0 == Off

  m_bEdlMergeShortCommBreaks = false;      // Off by default
  m_iEdlMaxCommBreakLength = 8 * 30 + 10;  // Just over 8 * 30 second commercial break.
  m_iEdlMinCommBreakLength = 3 * 30;       // 3 * 30 second commercial breaks.
  m_iEdlMaxCommBreakGap = 4 * 30;          // 4 * 30 second commercial breaks.
  m_iEdlMaxStartGap = 5 * 60;              // 5 minutes.

  m_curlconnecttimeout = 10;
  m_curllowspeedtime = 5;
  m_curlretries = 3;

  m_fullScreen = m_startFullScreen = false;

  m_playlistRetries = 100;
  m_playlistTimeout = 20; // 20 seconds timeout
  m_GLRectangleHack = false;
  m_iSkipLoopFilter = 0;
  m_osx_GLFullScreen = false;
  m_bVirtualShares = true;
  m_sleepBeforeFlip = 0;

  m_AllowD3D9Ex = true;
  m_ForceD3D9Ex = false;
  m_AllowDynamicTextures = true;
  m_RestrictCapsMask = 0;

//caused lots of jerks
//#ifdef _WIN32
//  m_ForcedSwapTime = 2.0;
//#else
  m_ForcedSwapTime = 0.0;
//#endif

  m_cpuTempCmd = "";
  m_gpuTempCmd = "";
#ifdef __APPLE__
  // default for osx is fullscreen always on top
  m_alwaysOnTop = true;
#else
  // default for windows is not always on top
  m_alwaysOnTop = false;
#endif

  m_bgInfoLoaderMaxThreads = 5;
  
  m_measureRefreshrate = false;

  m_bWireFrameMode = false;
  m_bCountPixels = false;

  m_logFileSize = DEFAULT_LOG_FILE_SIZE;

  m_bForceVideoHardwareDecoding = false;
  m_bForceAudioHardwarePassthrough = false;
  m_bForceWMV3Flushing = false;

  m_httpCacheMaxSize = 1024*50 * 1024;
  m_appsMaxSize = 1024*50 * 1024;
  m_thumbsMaxSize = 1024*100 * 1024;

  m_ddplus.Clear();
}

bool CAdvancedSettings::Load()
{
  // NOTE: This routine should NOT set the default of any of these parameters
  //       it should instead use the versions of GetString/Integer/Float that
  //       don't take defaults in.  Defaults are set in the constructor above
  CStdString advancedSettingsXML;
  advancedSettingsXML  = g_settings.GetUserDataItem("advancedsettings.xml");
  TiXmlDocument advancedXML;
  if (!CFile::Exists(advancedSettingsXML))
  { // tell the user it doesn't exist
    CLog::Log(LOGNOTICE, "No advancedsettings.xml to load (%s)", advancedSettingsXML.c_str());
    return false;
  }

  if (!advancedXML.LoadFile(advancedSettingsXML))
  {
    CLog::Log(LOGERROR, "Error loading %s, Line %d\n%s", advancedSettingsXML.c_str(), advancedXML.ErrorRow(), advancedXML.ErrorDesc());
    return false;
  }

  TiXmlElement *pRootElement = advancedXML.RootElement();
  if (!pRootElement || strcmpi(pRootElement->Value(),"advancedsettings") != 0)
  {
    CLog::Log(LOGERROR, "Error loading %s, no <advancedsettings> node", advancedSettingsXML.c_str());
    return false;
  }

  // succeeded - tell the user it worked
  CLog::Log(LOGNOTICE, "Loaded advancedsettings.xml from %s", advancedSettingsXML.c_str());

  // Dump contents of AS.xml to debug log
  TiXmlPrinter printer;
  printer.SetLineBreak("\n");
  printer.SetIndent("  ");
  advancedXML.Accept(&printer);
  CLog::Log(LOGDEBUG, "Contents of %s are...\n%s", advancedSettingsXML.c_str(), printer.CStr());

  TiXmlElement *pElement = pRootElement->FirstChildElement("audio");
  if (pElement)
  {
    XMLUtils::GetFloat(pElement, "ac3downmixgain", m_ac3Gain, -96.0f, 96.0f);
    XMLUtils::GetInt(pElement, "headroom", m_audioHeadRoom, 0, 12);
    XMLUtils::GetString(pElement, "defaultplayer", m_audioDefaultPlayer);
    XMLUtils::GetFloat(pElement, "playcountminimumpercent", m_audioPlayCountMinimumPercent, 0.0f, 100.0f);
    
    XMLUtils::GetBoolean(pElement, "usetimeseeking", m_musicUseTimeSeeking);
    XMLUtils::GetInt(pElement, "timeseekforward", m_musicTimeSeekForward, 0, 6000);
    XMLUtils::GetInt(pElement, "timeseekbackward", m_musicTimeSeekBackward, -6000, 0);
    XMLUtils::GetInt(pElement, "timeseekforwardbig", m_musicTimeSeekForwardBig, 0, 6000);
    XMLUtils::GetInt(pElement, "timeseekbackwardbig", m_musicTimeSeekBackwardBig, -6000, 0);

    XMLUtils::GetInt(pElement, "percentseekforward", m_musicPercentSeekForward, 0, 100);
    XMLUtils::GetInt(pElement, "percentseekbackward", m_musicPercentSeekBackward, -100, 0);
    XMLUtils::GetInt(pElement, "percentseekforwardbig", m_musicPercentSeekForwardBig, 0, 100);
    XMLUtils::GetInt(pElement, "percentseekbackwardbig", m_musicPercentSeekBackwardBig, -100, 0);

    XMLUtils::GetInt(pElement, "resample", m_musicResample, 0, 192000);

    TiXmlElement* pAudioExcludes = pElement->FirstChildElement("excludefromlisting");
    if (pAudioExcludes)
      GetCustomRegexps(pAudioExcludes, m_audioExcludeFromListingRegExps);

    pAudioExcludes = pElement->FirstChildElement("excludefromscan");
    if (pAudioExcludes)
      GetCustomRegexps(pAudioExcludes, m_audioExcludeFromScanRegExps);

    XMLUtils::GetString(pElement, "audiohost", m_audioHost);
    XMLUtils::GetBoolean(pElement, "applydrc", m_audioApplyDrc);  
    
    TiXmlElement* pDDPlusCert = pElement->FirstChildElement("ddpluscert");
    if( pDDPlusCert )
    {
      XMLUtils::GetBoolean(pDDPlusCert, "ltrtmode", m_ddplus.ltmode);  
      XMLUtils::GetBoolean(pDDPlusCert, "rfmode", m_ddplus.rfmode);
      XMLUtils::GetBoolean(pDDPlusCert, "lfemode", m_ddplus.lfemode);
      XMLUtils::GetBoolean(pDDPlusCert, "drc", m_ddplus.drc);
      CStdString mono;
      XMLUtils::GetString(pDDPlusCert, "monomode", mono);
      if( mono == "stereo" )     m_ddplus.monomode = __ddplus::_stereo;
      else if( mono == "left" )  m_ddplus.monomode = __ddplus::_left;
      else if( mono == "right" ) m_ddplus.monomode = __ddplus::_right;
      else if( mono == "mixed" ) m_ddplus.monomode = __ddplus::_mixed;
    }
  }

  pElement = pRootElement->FirstChildElement("karaoke");
  if (pElement)
  {
    XMLUtils::GetFloat(pElement, "syncdelaycdg", m_karaokeSyncDelayCDG, -3.0f, 3.0f); // keep the old name for comp
    XMLUtils::GetFloat(pElement, "syncdelaylrc", m_karaokeSyncDelayLRC, -3.0f, 3.0f);
    XMLUtils::GetBoolean(pElement, "alwaysreplacegenre", m_karaokeChangeGenreForKaraokeSongs );
    XMLUtils::GetBoolean(pElement, "storedelay", m_karaokeKeepDelay );
    XMLUtils::GetInt(pElement, "autoassignstartfrom", m_karaokeStartIndex, 1, 2000000000);
    XMLUtils::GetBoolean(pElement, "nocdgbackground", m_karaokeAlwaysEmptyOnCdgs );
    XMLUtils::GetBoolean(pElement, "lookupsongbackground", m_karaokeUseSongSpecificBackground );

    TiXmlElement* pKaraokeBackground = pElement->FirstChildElement("defaultbackground");
    if (pKaraokeBackground)
    {
      const char* attr = pKaraokeBackground->Attribute("type");
      if ( attr )
        m_karaokeDefaultBackgroundType = attr;

      attr = pKaraokeBackground->Attribute("path");
      if ( attr )
        m_karaokeDefaultBackgroundFilePath = attr;
    }
  }

  pElement = pRootElement->FirstChildElement("video");
  if (pElement)
  {
    XMLUtils::GetFloat(pElement, "subsdelayrange", m_videoSubsDelayRange, 10, 600);
    XMLUtils::GetFloat(pElement, "audiodelayrange", m_videoAudioDelayRange, 10, 600);
    XMLUtils::GetInt(pElement, "blackbarcolour", m_videoBlackBarColour, 0, 255);
    XMLUtils::GetString(pElement, "defaultplayer", m_videoDefaultPlayer);
    XMLUtils::GetString(pElement, "defaultdvdplayer", m_videoDefaultDVDPlayer);
    XMLUtils::GetBoolean(pElement, "fullscreenonmoviestart", m_fullScreenOnMovieStart);
    XMLUtils::GetFloat(pElement, "playcountminimumpercent", m_videoPlayCountMinimumPercent, 0.0f, 100.0f);
    XMLUtils::GetInt(pElement, "ignoreatstart", m_videoIgnoreAtStart, 0, 900);
    XMLUtils::GetInt(pElement, "ignoreatend", m_videoIgnoreAtEnd, 0, 900);

    XMLUtils::GetInt(pElement, "smallstepbackseconds", m_videoSmallStepBackSeconds, 1, INT_MAX);
    XMLUtils::GetInt(pElement, "smallstepbacktries", m_videoSmallStepBackTries, 1, 10);
    XMLUtils::GetInt(pElement, "smallstepbackdelay", m_videoSmallStepBackDelay, 100, 5000); //MS

    XMLUtils::GetBoolean(pElement, "usetimeseeking", m_videoUseTimeSeeking);
    XMLUtils::GetInt(pElement, "timeseekforward", m_videoTimeSeekForward, 0, 6000);
    XMLUtils::GetInt(pElement, "timeseekbackward", m_videoTimeSeekBackward, -6000, 0);
    XMLUtils::GetInt(pElement, "timeseekforwardbig", m_videoTimeSeekForwardBig, 0, 6000);
    XMLUtils::GetInt(pElement, "timeseekbackwardbig", m_videoTimeSeekBackwardBig, -6000, 0);

    XMLUtils::GetInt(pElement, "percentseekforward", m_videoPercentSeekForward, 0, 100);
    XMLUtils::GetInt(pElement, "percentseekbackward", m_videoPercentSeekBackward, -100, 0);
    XMLUtils::GetInt(pElement, "percentseekforwardbig", m_videoPercentSeekForwardBig, 0, 100);
    XMLUtils::GetInt(pElement, "percentseekbackwardbig", m_videoPercentSeekBackwardBig, -100, 0);

    TiXmlElement* pVideoExcludes = pElement->FirstChildElement("excludefromlisting");
    if (pVideoExcludes)
      GetCustomRegexps(pVideoExcludes, m_videoExcludeFromListingRegExps);

    pVideoExcludes = pElement->FirstChildElement("excludefromscan");
    if (pVideoExcludes)
      GetCustomRegexps(pVideoExcludes, m_moviesExcludeFromScanRegExps);

    pVideoExcludes = pElement->FirstChildElement("excludetvshowsfromscan");
    if (pVideoExcludes)
      GetCustomRegexps(pVideoExcludes, m_tvshowExcludeFromScanRegExps);

    pVideoExcludes = pElement->FirstChildElement("cleanstrings");
    if (pVideoExcludes)
      GetCustomRegexps(pVideoExcludes, m_videoCleanStringRegExps);

    XMLUtils::GetString(pElement,"cleandatetime", m_videoCleanDateTimeRegExp);
    XMLUtils::GetString(pElement,"postprocessing",m_videoPPFFmpegType);

	m_DXVACheckCompatibilityPresent = XMLUtils::GetBoolean(pElement,"checkdxvacompatibility", m_DXVACheckCompatibility);
  }

  pElement = pRootElement->FirstChildElement("musiclibrary");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "hideallitems", m_bMusicLibraryHideAllItems);
    XMLUtils::GetInt(pElement, "recentlyaddeditems", m_iMusicLibraryRecentlyAddedItems, 1, INT_MAX);
    XMLUtils::GetBoolean(pElement, "prioritiseapetags", m_prioritiseAPEv2tags);
    XMLUtils::GetBoolean(pElement, "allitemsonbottom", m_bMusicLibraryAllItemsOnBottom);
    XMLUtils::GetBoolean(pElement, "albumssortbyartistthenyear", m_bMusicLibraryAlbumsSortByArtistThenYear);
    XMLUtils::GetString(pElement, "albumformat", m_strMusicLibraryAlbumFormat);
    XMLUtils::GetString(pElement, "albumformatright", m_strMusicLibraryAlbumFormatRight);
    XMLUtils::GetString(pElement, "itemseparator", m_musicItemSeparator);
  }

  pElement = pRootElement->FirstChildElement("videolibrary");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "hideallitems", m_bVideoLibraryHideAllItems);
    XMLUtils::GetBoolean(pElement, "allitemsonbottom", m_bVideoLibraryAllItemsOnBottom);
    XMLUtils::GetInt(pElement, "recentlyaddeditems", m_iVideoLibraryRecentlyAddedItems, 1, INT_MAX);
    XMLUtils::GetBoolean(pElement, "hiderecentlyaddeditems", m_bVideoLibraryHideRecentlyAddedItems);
    XMLUtils::GetBoolean(pElement, "hideemptyseries", m_bVideoLibraryHideEmptySeries);
    XMLUtils::GetBoolean(pElement, "cleanonupdate", m_bVideoLibraryCleanOnUpdate);
    XMLUtils::GetString(pElement, "itemseparator", m_videoItemSeparator);
    XMLUtils::GetBoolean(pElement, "exportautothumbs", m_bVideoLibraryExportAutoThumbs);
    
    TiXmlElement* pMyMovies = pElement->FirstChildElement("mymovies");
    if (pMyMovies)
      XMLUtils::GetBoolean(pMyMovies, "categoriestogenres", m_bVideoLibraryMyMoviesCategoriesToGenres);
  }
  // Backward-compatibility of ExternalPlayer config
  pElement = pRootElement->FirstChildElement("externalplayer");
  if (pElement)
  {
    CLog::Log(LOGWARNING, "External player configuration has been removed from advancedsettings.xml.  It can now be configed in userdata/playercorefactory.xml");
  }
  pElement = pRootElement->FirstChildElement("slideshow");
  if (pElement)
  {
    XMLUtils::GetFloat(pElement, "panamount", m_slideshowPanAmount, 0.0f, 20.0f);
    XMLUtils::GetFloat(pElement, "zoomamount", m_slideshowZoomAmount, 0.0f, 20.0f);
    XMLUtils::GetFloat(pElement, "blackbarcompensation", m_slideshowBlackBarCompensation, 0.0f, 50.0f);
  }

  pElement = pRootElement->FirstChildElement("lcd");
  if (pElement)
  {
    XMLUtils::GetInt(pElement, "rows", m_lcdRows, 1, 4);
    XMLUtils::GetInt(pElement, "columns", m_lcdColumns, 1, 40);
    XMLUtils::GetInt(pElement, "address1", m_lcdAddress1, 0, 0x100);
    XMLUtils::GetInt(pElement, "address2", m_lcdAddress2, 0, 0x100);
    XMLUtils::GetInt(pElement, "address3", m_lcdAddress3, 0, 0x100);
    XMLUtils::GetInt(pElement, "address4", m_lcdAddress4, 0, 0x100);
    XMLUtils::GetBoolean(pElement, "heartbeat", m_lcdHeartbeat);
    XMLUtils::GetInt(pElement, "scrolldelay", m_lcdScrolldelay, -8, 8);
  }
  pElement = pRootElement->FirstChildElement("network");
  if (pElement)
  {
    XMLUtils::GetInt(pElement, "autodetectpingtime", m_autoDetectPingTime, 1, 240);
    XMLUtils::GetInt(pElement, "curlclienttimeout", m_curlconnecttimeout, 1, 1000);
    XMLUtils::GetInt(pElement, "curllowspeedtime", m_curllowspeedtime, 1, 1000);
    XMLUtils::GetInt(pElement, "curlretries", m_curlretries, 0, 10);
  }

  pElement = pRootElement->FirstChildElement("samba");
  if (pElement)
  {
    XMLUtils::GetString(pElement,  "doscodepage",   m_sambadoscodepage);
    XMLUtils::GetInt(pElement, "clienttimeout", m_sambaclienttimeout, 5, 100);
    XMLUtils::GetBoolean(pElement, "statfiles", m_sambastatfiles);
  }

  pElement = pRootElement->FirstChildElement("httpdirectory");
  if (pElement)
    XMLUtils::GetBoolean(pElement, "statfilesize", m_bHTTPDirectoryStatFilesize);

  XMLUtils::GetString(pRootElement, "cddbaddress", m_cddbAddress);
#ifdef HAS_HAL
  XMLUtils::GetBoolean(pRootElement, "usehalmount", m_useHalMount);
#endif
  XMLUtils::GetBoolean(pRootElement, "nodvdrom", m_noDVDROM);
  XMLUtils::GetBoolean(pRootElement, "usemultipaths", m_useMultipaths);
#ifdef HAS_SDL
  XMLUtils::GetBoolean(pRootElement, "fullscreen", m_startFullScreen);
#endif

  XMLUtils::GetInt(pRootElement, "songinfoduration", m_songInfoDuration, 0, INT_MAX);
  XMLUtils::GetInt(pRootElement, "busydialogdelay", m_busyDialogDelay, 0, 5000);
  XMLUtils::GetInt(pRootElement, "playlistretries", m_playlistRetries, -1, 5000);
  XMLUtils::GetInt(pRootElement, "playlisttimeout", m_playlistTimeout, 0, 5000);


  XMLUtils::GetBoolean(pRootElement,"rootovershoot",m_bUseEvilB);
  XMLUtils::GetBoolean(pRootElement,"glrectanglehack", m_GLRectangleHack);
  XMLUtils::GetInt(pRootElement,"skiploopfilter", m_iSkipLoopFilter, -16, 48);
  XMLUtils::GetFloat(pRootElement, "forcedswaptime", m_ForcedSwapTime, 0.0, 100.0);
  XMLUtils::GetBoolean(pRootElement,"osx_gl_fullscreen", m_osx_GLFullScreen);

  XMLUtils::GetFloat(pRootElement,"sleepbeforeflip", m_sleepBeforeFlip, 0.0f, 1.0f);
  XMLUtils::GetBoolean(pRootElement,"virtualshares", m_bVirtualShares);

  //Tuxbox
  pElement = pRootElement->FirstChildElement("tuxbox");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "audiochannelselection", m_bTuxBoxAudioChannelSelection);
    XMLUtils::GetBoolean(pElement, "submenuselection", m_bTuxBoxSubMenuSelection);
    XMLUtils::GetBoolean(pElement, "pictureicon", m_bTuxBoxPictureIcon);
    XMLUtils::GetBoolean(pElement, "sendallaudiopids", m_bTuxBoxSendAllAPids);
    XMLUtils::GetInt(pElement, "epgrequesttime", m_iTuxBoxEpgRequestTime, 0, 3600);
    XMLUtils::GetInt(pElement, "defaultsubmenu", m_iTuxBoxDefaultSubMenu, 1, 4);
    XMLUtils::GetInt(pElement, "defaultrootmenu", m_iTuxBoxDefaultRootMenu, 0, 4);
    XMLUtils::GetInt(pElement, "zapwaittime", m_iTuxBoxZapWaitTime, 0, 120);
  }
  
  // Myth TV
  pElement = pRootElement->FirstChildElement("myth");
  if (pElement)
  {
    XMLUtils::GetInt(pElement, "movielength", m_iMythMovieLength);
  }

  // EDL commercial break handling
  pElement = pRootElement->FirstChildElement("edl");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "mergeshortcommbreaks", m_bEdlMergeShortCommBreaks);
    XMLUtils::GetInt(pElement, "maxcommbreaklength", m_iEdlMaxCommBreakLength, 0, 10 * 60); // Between 0 and 10 minutes 
    XMLUtils::GetInt(pElement, "mincommbreaklength", m_iEdlMinCommBreakLength, 0, 5 * 60);  // Between 0 and 5 minutes
    XMLUtils::GetInt(pElement, "maxcommbreakgap", m_iEdlMaxCommBreakGap, 0, 5 * 60);        // Between 0 and 5 minutes.
    XMLUtils::GetInt(pElement, "maxstartgap", m_iEdlMaxStartGap, 0, 10 * 60);               // Between 0 and 10 minutes
  }

  // picture exclude regexps
  TiXmlElement* pPictureExcludes = pRootElement->FirstChildElement("pictureexcludes");
  if (pPictureExcludes)
    GetCustomRegexps(pPictureExcludes, m_pictureExcludeFromListingRegExps);

  // picture extensions
  CStdString extraExtensions;
  TiXmlElement* pExts = pRootElement->FirstChildElement("pictureextensions");
  if (pExts)
    GetCustomExtensions(pExts,g_stSettings.m_pictureExtensions);

  // music extensions
  pExts = pRootElement->FirstChildElement("musicextensions");
  if (pExts)
    GetCustomExtensions(pExts,g_stSettings.m_musicExtensions);

  // video extensions
  pExts = pRootElement->FirstChildElement("videoextensions");
  if (pExts)
    GetCustomExtensions(pExts,g_stSettings.m_videoExtensions);

  m_vecTokens.clear();
  CLangInfo::LoadTokens(pRootElement->FirstChild("sorttokens"),m_vecTokens);

  XMLUtils::GetBoolean(pRootElement, "displayremotecodes", m_displayRemoteCodes);

  // TODO: Should cache path be given in terms of our predefined paths??
  //       Are we even going to have predefined paths??
  CSettings::GetPath(pRootElement, "cachepath", m_cachePath);
  CUtil::AddSlashAtEnd(m_cachePath);

  g_LangCodeExpander.LoadUserCodes(pRootElement->FirstChildElement("languagecodes"));

  // stacking regexps
  TiXmlElement* pVideoStacking = pRootElement->FirstChildElement("moviestacking");
  if (pVideoStacking)
    GetCustomRegexps(pVideoStacking, m_videoStackRegExps);

  //tv stacking regexps
  TiXmlElement* pTVStacking = pRootElement->FirstChildElement("tvshowmatching");
  if (pTVStacking)
    GetCustomTVRegexps(pTVStacking, m_tvshowStackRegExps);

  // path substitutions
  TiXmlElement* pPathSubstitution = pRootElement->FirstChildElement("pathsubstitution");
  if (pPathSubstitution)
  {
    m_pathSubstitutions.clear();
    CLog::Log(LOGDEBUG,"Configuring path substitutions");
    TiXmlNode* pSubstitute = pPathSubstitution->FirstChildElement("substitute");
    while (pSubstitute)
    {
      CStdString strFrom, strTo;
      TiXmlNode* pFrom = pSubstitute->FirstChild("from");
      if (pFrom)
        strFrom = pFrom->FirstChild()->Value();
      TiXmlNode* pTo = pSubstitute->FirstChild("to");
      if (pTo)
        strTo = pTo->FirstChild()->Value();

      if (!strFrom.IsEmpty() && !strTo.IsEmpty())
      {
        CLog::Log(LOGDEBUG,"  Registering substition pair:");
        CLog::Log(LOGDEBUG,"    From: [%s]", strFrom.c_str());
        CLog::Log(LOGDEBUG,"    To:   [%s]", strTo.c_str());
        // keep literal commas since we use comma as a seperator
        strFrom.Replace(",",",,");
        strTo.Replace(",",",,");
        m_pathSubstitutions.push_back(strFrom + " , " + strTo);
      }
      else
      {
        // error message about missing tag
        if (strFrom.IsEmpty())
          CLog::Log(LOGERROR,"  Missing <from> tag");
        else
          CLog::Log(LOGERROR,"  Missing <to> tag");
      }

      // get next one
      pSubstitute = pSubstitute->NextSiblingElement("substitute");
    }
  }

  XMLUtils::GetInt(pRootElement, "remotedelay", m_remoteDelay, 1, 20);
  XMLUtils::GetFloat(pRootElement, "controllerdeadzone", m_controllerDeadzone, 0.0f, 1.0f);
  XMLUtils::GetInt(pRootElement, "thumbsize", m_thumbSize, 64, 1024);
  XMLUtils::GetBoolean(pRootElement, "useddsfanart", m_useDDSFanart);

  XMLUtils::GetBoolean(pRootElement, "playlistasfolders", m_playlistAsFolders);
  XMLUtils::GetBoolean(pRootElement, "detectasudf", m_detectAsUdf);

  // music thumbs
  CStdString extraThumbs;
  TiXmlElement* pThumbs = pRootElement->FirstChildElement("musicthumbs");
  if (pThumbs)
    GetCustomExtensions(pThumbs,m_musicThumbs);

  // dvd thumbs
  pThumbs = pRootElement->FirstChildElement("dvdthumbs");
  if (pThumbs)
    GetCustomExtensions(pThumbs,m_dvdThumbs);

  // movie fanarts
  TiXmlElement* pFanart = pRootElement->FirstChildElement("fanart");
  if (pFanart)
    GetCustomExtensions(pFanart,m_fanartImages);

  // music filename->tag filters
  TiXmlElement* filters = pRootElement->FirstChildElement("musicfilenamefilters");
  if (filters)
  {
    TiXmlNode* filter = filters->FirstChild("filter");
    while (filter)
    {
      if (filter->FirstChild())
        m_musicTagsFromFileFilters.push_back(filter->FirstChild()->ValueStr());
      filter = filter->NextSibling("filter");
    }
  }

  TiXmlElement* pHostEntries = pRootElement->FirstChildElement("hosts");
  if (pHostEntries)
  {
    TiXmlElement* element = pHostEntries->FirstChildElement("entry");
    while(element)
    {
      CStdString name  = element->Attribute("name");
      CStdString value;
      if(element->GetText())
        value = element->GetText();

      if(name.length() > 0 && value.length() > 0)
        CDNSNameCache::Add(name, value);
      element = element->NextSiblingElement("entry");
    }
  }

  XMLUtils::GetString(pRootElement, "cputempcommand", m_cpuTempCmd);
  XMLUtils::GetString(pRootElement, "gputempcommand", m_gpuTempCmd);

  XMLUtils::GetBoolean(pRootElement, "alwaysontop", m_alwaysOnTop);

  XMLUtils::GetInt(pRootElement, "bginfoloadermaxthreads", m_bgInfoLoaderMaxThreads);
  m_bgInfoLoaderMaxThreads = std::max(1, m_bgInfoLoaderMaxThreads);

  XMLUtils::GetBoolean(pRootElement, "measurerefreshrate", m_measureRefreshrate);
  XMLUtils::GetBoolean(pRootElement,"wireframe",m_bWireFrameMode);
  XMLUtils::GetBoolean(pRootElement,"countpixels",m_bCountPixels);
  XMLUtils::GetInt(pRootElement, "logfilesize", m_logFileSize);

  XMLUtils::GetBoolean(pRootElement, "forcevideohardwaredecoding", m_bForceVideoHardwareDecoding);
  XMLUtils::GetBoolean(pRootElement, "forceaudiohardwarepassthrough", m_bForceAudioHardwarePassthrough);
  XMLUtils::GetBoolean(pRootElement, "forcewmv3flushing", m_bForceWMV3Flushing);

  XMLUtils::GetBoolean(pRootElement,"allowd3d9ex", m_AllowD3D9Ex);
  XMLUtils::GetBoolean(pRootElement,"forced3d9ex", m_ForceD3D9Ex);
  XMLUtils::GetBoolean(pRootElement,"allowdynamictextures", m_AllowDynamicTextures);
  XMLUtils::GetUInt(pRootElement,"restrictcapsmask", m_RestrictCapsMask);

  // Disk Quota settings
  pElement = pRootElement->FirstChildElement("quota");
  if (pElement)
  {

    uint32_t tmp_size = 0;
    if (XMLUtils::GetUInt(pElement, "httpcachemaxsize", tmp_size))
    {
      m_httpCacheMaxSize = tmp_size * 1024;
    }
    if (XMLUtils::GetUInt(pElement, "appsmaxsize", tmp_size))
    {
      m_appsMaxSize = tmp_size * 1024;
    }
    if (XMLUtils::GetUInt(pElement, "thumbsmaxsize", tmp_size))
    {
      m_thumbsMaxSize = tmp_size * 1024;
    }
  }

  // load in the GUISettings overrides:
  g_guiSettings.LoadXML(pRootElement, true);  // true to hide the settings we read in

  return true;
}

void CAdvancedSettings::Clear()
{
  m_videoCleanStringRegExps.clear();
  m_moviesExcludeFromScanRegExps.clear();
  m_tvshowExcludeFromScanRegExps.clear();
  m_videoExcludeFromListingRegExps.clear();
  m_videoStackRegExps.clear();
  m_audioExcludeFromScanRegExps.clear();
  m_audioExcludeFromListingRegExps.clear();
  m_pictureExcludeFromListingRegExps.clear();
}

void CAdvancedSettings::GetCustomTVRegexps(TiXmlElement *pRootElement, SETTINGS_TVSHOWLIST& settings)
{
  int iAction = 0; // overwrite
  // for backward compatibility
  const char* szAppend = pRootElement->Attribute("append");
  if ((szAppend && stricmp(szAppend, "yes") == 0))
    iAction = 1;
  // action takes precedence if both attributes exist
  const char* szAction = pRootElement->Attribute("action");
  if (szAction)
  {
    iAction = 0; // overwrite
    if (stricmp(szAction, "append") == 0)
      iAction = 1; // append
    else if (stricmp(szAction, "prepend") == 0)
      iAction = 2; // prepend
  }
  if (iAction == 0)
    settings.clear();
  TiXmlNode* pRegExp = pRootElement->FirstChild("regexp");
  int i = 0;
  while (pRegExp)
  {
    if (pRegExp->FirstChild())
    {
      bool bByDate = false;
      if (pRegExp->ToElement())
      {
        CStdString byDate = pRegExp->ToElement()->Attribute("bydate");
        if(byDate && stricmp(byDate, "true") == 0)
        {
          bByDate = true;
        }
      }
      CStdString regExp = pRegExp->FirstChild()->Value();
      regExp.MakeLower();
      if (iAction == 2)
        settings.insert(settings.begin() + i++, 1, TVShowRegexp(bByDate,regExp));
      else
        settings.push_back(TVShowRegexp(bByDate,regExp));
    }
    pRegExp = pRegExp->NextSibling("regexp");
  }
}

void CAdvancedSettings::GetCustomRegexps(TiXmlElement *pRootElement, CStdStringArray& settings)
{
  TiXmlElement *pElement = pRootElement;
  while (pElement)
  {
    int iAction = 0; // overwrite
    // for backward compatibility
    const char* szAppend = pElement->Attribute("append");
    if ((szAppend && stricmp(szAppend, "yes") == 0))
      iAction = 1;
    // action takes precedence if both attributes exist
    const char* szAction = pElement->Attribute("action");
    if (szAction)
    {
      iAction = 0; // overwrite
      if (stricmp(szAction, "append") == 0)
        iAction = 1; // append
      else if (stricmp(szAction, "prepend") == 0)
        iAction = 2; // prepend
    }
    if (iAction == 0)
      settings.clear();
    TiXmlNode* pRegExp = pElement->FirstChild("regexp");
    int i = 0;
    while (pRegExp)
    {
      if (pRegExp->FirstChild())
      {
        CStdString regExp = pRegExp->FirstChild()->Value();
        regExp.MakeLower();
        if (iAction == 2)
          settings.insert(settings.begin() + i++, 1, regExp);
        else
          settings.push_back(regExp);
      }
      pRegExp = pRegExp->NextSibling("regexp");
    }

    pElement = pElement->NextSiblingElement(pRootElement->Value());
  }
}

void CAdvancedSettings::GetCustomExtensions(TiXmlElement *pRootElement, CStdString& extensions)
{
  CStdString extraExtensions;
  CSettings::GetString(pRootElement,"add",extraExtensions,"");
  if (extraExtensions != "")
    extensions += "|" + extraExtensions;
  CSettings::GetString(pRootElement,"remove",extraExtensions,"");
  if (extraExtensions != "")
  {
    CStdStringArray exts;
    StringUtils::SplitString(extraExtensions,"|",exts);
    for (unsigned int i=0;i<exts.size();++i)
    {
      int iPos = extensions.Find(exts[i]);
      if (iPos == -1)
        continue;
      extensions.erase(iPos,exts[i].size()+1);
    }
  }
}

