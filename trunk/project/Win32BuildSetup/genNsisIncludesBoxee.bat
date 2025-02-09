@ECHO OFF
rem XBMC for Windows install script
rem Copyright (C) 2005-2008 Team XBMC
rem http://xbmc.org

rem Script by chadoe
rem This script generates nullsoft installer include files for xbmc's languages, skins, scripts and plugins

rem languages
IF EXIST languages.nsi del languages.nsi > NUL
SETLOCAL ENABLEDELAYEDEXPANSION
SET Counter=1
FOR /F "tokens=*" %%S IN ('dir /B /AD BUILD_WIN32\Boxee\language') DO (
  rem English is already included as default language
  IF "%%S" NEQ "English" (
    ECHO Section "%%S" SecLanguage!Counter! >> languages.nsi
    ECHO SectionIn 1 #section is in installtype Full >> languages.nsi
    ECHO SetOutPath "$INSTDIR\language\%%S" >> languages.nsi
    ECHO File /r "${xbmc_root}\Boxee\language\%%S\*.*" >> languages.nsi
    ECHO SectionEnd >> languages.nsi
    SET /A Counter = !Counter! + 1
  )
)
ENDLOCAL

rem skins
IF EXIST skins.nsi del skins.nsi > NUL
SETLOCAL ENABLEDELAYEDEXPANSION
SET Counter=1
FOR /F "tokens=*" %%S IN ('dir /B /AD BUILD_WIN32\Boxee\Skin') DO (
  rem PM3.HD is already included as default skin
  IF "%%S" NEQ "PM3.HD" (
    ECHO Section "%%S" SecSkin!Counter! >> skins.nsi
    ECHO SectionIn 1 #section is in installtype Full >> skins.nsi
    ECHO SetOutPath "$INSTDIR\skin\%%S" >> skins.nsi
    ECHO File /r "${xbmc_root}\Boxee\Skin\%%S\*.*" >> skins.nsi
    ECHO SectionEnd >> skins.nsi
    SET /A Counter = !Counter! + 1
  )
)
ENDLOCAL

rem scripts
IF EXIST scripts.nsi del scripts.nsi > NUL
IF EXIST BUILD_WIN32\Boxee\scripts (
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET Counter=1
  ECHO SectionGroup "Scripts" SecScripts >> scripts.nsi
  FOR /F "tokens=*" %%S IN ('dir /B /AD BUILD_WIN32\Boxee\scripts') DO (
    ECHO Section "%%S" SecScript!Counter! >> scripts.nsi
    ECHO SectionIn 1 #section is in installtype Full >> scripts.nsi
    ECHO SetOutPath "$INSTDIR\scripts\%%S" >> scripts.nsi
    ECHO File /r "${xbmc_root}\Boxee\scripts\%%S\*.*" >> scripts.nsi
    ECHO SectionEnd >> scripts.nsi
    SET /A Counter = !Counter! + 1
  )
  ECHO Section "Autoexec" SecScript!Counter! >> scripts.nsi
  ECHO SectionIn 1 #section is in installtype Full >> scripts.nsi
  ECHO SetOutPath "$INSTDIR\scripts" >> scripts.nsi
  ECHO File /r "${xbmc_root}\Boxee\scripts\autoexec.py" >> scripts.nsi
  ECHO SectionEnd >> scripts.nsi
  ECHO SectionGroupEnd >> scripts.nsi
  ENDLOCAL
)

rem plugins
IF EXIST plugins.nsi del plugins.nsi > NUL
IF EXIST BUILD_WIN32\Boxee\plugins (
  ECHO SectionGroup "Plugins" SecPlugins >> plugins.nsi
  for %%a IN (Music Pictures Programs Video) DO (
    IF EXIST BUILD_WIN32\Boxee\plugins\%%a (
      SETLOCAL ENABLEDELAYEDEXPANSION
      SET Counter=1
      ECHO SectionGroup "%%a" SecPlugins%%a >> plugins.nsi
      FOR /F "tokens=*" %%S IN ('dir /B /AD BUILD_WIN32\Boxee\plugins\%%a') DO (
        ECHO Section "%%S" SecPlugins%%a!Counter! >> plugins.nsi
        ECHO SectionIn 1 #section is in installtype Full >> plugins.nsi
        ECHO SetOutPath "$INSTDIR\plugins\%%a\%%S" >> plugins.nsi
        ECHO File /r "${xbmc_root}\Boxee\plugins\%%a\%%S\*.*" >> plugins.nsi
        ECHO SectionEnd >> plugins.nsi
        SET /A Counter = !Counter! + 1
      )
      ECHO SectionGroupEnd >> plugins.nsi
      ENDLOCAL
	)
  )
  ECHO SectionGroupEnd >> plugins.nsi
)