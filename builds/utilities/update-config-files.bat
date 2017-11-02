@ECHO OFF &SETLOCAL
Title Update LPub3D files with build version number
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: November 01, 2017
rem  Copyright (c) 2015 - 2017 by Trevor Sandy
rem --

SET LP3D_ME=%~nx0

rem Change these when you change the LPub3D root directory (e.g. if using a different root folder when testing)
SET OLD_LPUB3D=lpub3d-ci

CALL :FIXUP_PWD %1

IF [%LP3D_BUILDS_DIR%] == [] (
  ECHO Error: Did not receive required argument _PRO_FILE_PWD_
  ECHO %LP3D_ME% terminated!
  ECHO.
  EXIT /b 0
)

REM LP3D_BUILDS_DIR = C:\Users\Trevor\Projects\lpub3d-ci\builds
SET LP3D_BUILDS_DIR=%LP3D_BUILDS_DIR:"=%
SET LP3D_CALL_DIR=%CD%
SET LP3D_OBS_DIR=%LP3D_BUILDS_DIR%\linux\obs
SET LP3D_VER_INFO_FILE=%LP3D_BUILDS_DIR%\utilities\version.info
SET LP3D_AV_VER_INFO_DIR=%LP3D_BUILDS_DIR%\windows\release
SET LP3D_AV_VER_INFO_FILE=%LP3D_AV_VER_INFO_DIR%\version.info

ECHO  Start %LP3D_ME% execution at %CD%...
IF [%3] EQU [] (
  ECHO  1. capture version info using git queries...
  CALL :GET_GIT_VERSION
) ELSE (
  ECHO  1. capture version info using version arguments...
  SET LP3D_VER_MAJOR=%2
  SET LP3D_VER_MINOR=%3
  SET LP3D_VER_PATCH=%4
  SET LP3D_VER_REVISION=%5
  SET LP3D_VER_BUILD=%6
  SET LP3D_VER_SHA_HASH=%7
)

SET LP3D_VERSION=unknown
SET LP3D_WEEK_DAY=unknown
SET LP3D_MONTH_OF_YEAR=unknown
SET LP3D_AVAILABLE_VERSIONS=unknown

CALL :GET_DATE_AND_TIME

SET LP3D_TIME=%LP3D_HOUR%:%LP3D_MIN%:%LP3D_SEC%
SET LP3D_BUILD_DATE=%LP3D_YEAR%%LP3D_MONTH%%LP3D_DAY%
SET LP3D_BUILD_DATE_TIME=%LP3D_DAY% %LP3D_MONTH% %LP3D_YEAR% %LP3D_TIME%
SET LP3D_CHANGE_DATE_LONG=%LP3D_WEEK_DAY%, %LP3D_DAY% %LP3D_MONTH_OF_YEAR% %LP3D_YEAR% %LP3D_TIME% +0100
SET LP3D_CHANGE_DATE=%LP3D_WEEK_DAY% %LP3D_MONTH_OF_YEAR% %LP3D_DAY% %LP3D_YEAR%
SET LP3D_VERSION=%LP3D_VER_MAJOR%.%LP3D_VER_MINOR%.%LP3D_VER_PATCH%
SET LP3D_APP_VERSION=%LP3D_VERSION%.%LP3D_VER_BUILD%
SET LP3D_APP_VER_SUFFIX=%LP3D_VER_MAJOR%%LP3D_VER_MINOR%
SET LP3D_APP_VERSION_LONG=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD%_%LP3D_BUILD_DATE%
SET LP3D_BUILD_VERSION=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD% (%LP3D_BUILD_DATE_TIME%)

CALL :GET_AVAILABLE_VERSIONS %*

rem AppVeyor 64bit Qt MinGW build has git.exe/cygwin conflict returning no .git directory found so generate version.info file
IF EXIST "%LP3D_VER_INFO_FILE%" DEL /Q "%LP3D_VER_INFO_FILE%"
REM ECHO "   writing version info to builds/utilities/version.info"
SET LP3D_VERSION_INFO=%LP3D_VER_MAJOR% %LP3D_VER_MINOR% %LP3D_VER_PATCH% %LP3D_VER_REVISION% %LP3D_VER_BUILD% %LP3D_VER_SHA_HASH% %LP3D_BUILD_DATE_TIME% %LP3D_AVAILABLE_VERSIONS%
ECHO %LP3D_VERSION_INFO% > %LP3D_VER_INFO_FILE%
IF EXIST "%LP3D_VER_INFO_FILE%" (ECHO   FILE version.info..............[written to .\builds\utilities\version.info]) ELSE (ECHO   FILE version.info..............[Error, file not found])

IF "%APPVEYOR%" == "True" (
  ECHO   LP3D_VERSION_INFO............[%LP3D_VERSION_INFO%]
  ECHO   LPUB3D_DIR...................[%LPUB3D%]
  ECHO   %LP3D_ME% execution finished.
  GOTO :END
)

ECHO   LPUB3D_DIR.....................[%LPUB3D%]
ECHO   LP3D_BUILDS_DIR................[%LP3D_BUILDS_DIR%]
ECHO   LP3D_CALL_DIR..................[%LP3D_CALL_DIR%]

ECHO   LP3D_VERSION_INFO..............[%LP3D_VERSION_INFO%]
ECHO   LP3D_VER_MAJOR.................[%LP3D_VER_MAJOR%]
ECHO   LP3D_VER_MINOR.................[%LP3D_VER_MINOR%]
ECHO   LP3D_VER_PATCH.................[%LP3D_VER_PATCH%]
ECHO   LP3D_VER_REVISION..............[%LP3D_VER_REVISION%]
ECHO   LP3D_VER_BUILD.................[%LP3D_VER_BUILD%]
ECHO   LP3D_VER_SHA_HASH..............[%LP3D_VER_SHA_HASH%]
ECHO   LP3D_APP_VER_SUFFIX............[%LP3D_APP_VER_SUFFIX%]
ECHO   LP3D_BUILD_VERSION.............[%LP3D_BUILD_VERSION%]
ECHO   LP3D_BUILD_DATE_TIME...........[%LP3D_BUILD_DATE_TIME%]
ECHO   LP3D_CHANGE_DATE_LONG..........[%LP3D_CHANGE_DATE_LONG%]

ECHO   LP3D_VERSION...................[%LP3D_VERSION%]
ECHO   LP3D_APP_VERSION...............[%LP3D_APP_VERSION%]
ECHO   LP3D_APP_VERSION_LONG..........[%LP3D_APP_VERSION_LONG%]

ECHO   LP3D_SOURCE_DIR................[%LPUB3D%-%LP3D_APP_VERSION%]

ECHO   LP3D_AVAILABLE_VERSIONS........[%LP3D_AVAILABLE_VERSIONS%]

ECHO.
ECHO 2. set root directory name for linux config files...
SET LP3D_DEB_DSC_FILE=%LP3D_OBS_DIR%\debian\%OLD_LPUB3D%.dsc
SET LP3D_DEB_LINT_FILE=%LP3D_OBS_DIR%\debian\%OLD_LPUB3D%.lintian-overrides
SET LP3D_OBS_SPEC_FILE=%LP3D_OBS_DIR%\%OLD_LPUB3D%.spec
SET LP3D_RPM_LINT_FILE=%LP3D_OBS_DIR%\%OLD_LPUB3D%-rpmlintrc
IF "%LPUB3D%" EQU "%OLD_LPUB3D%" (
  ECHO     nothing to do, skipping
) ELSE (
   IF EXIST %LP3D_DEB_DSC_FILE% (
    MOVE /y "%LP3D_DEB_DSC_FILE%" "%LP3D_OBS_DIR%\debian\%LPUB3D%.dsc" | findstr /i /v /r /c:"moved\>"
   )
   IF EXIST %LP3D_DEB_LINT_FILE% (
    MOVE /y "%LP3D_DEB_LINT_FILE%" "%LP3D_OBS_DIR%\debian\%LPUB3D%.lintian-overrides" | findstr /i /v /r /c:"moved\>"
   )
   IF EXIST %LP3D_OBS_SPEC_FILE% (
    MOVE /y "%LP3D_OBS_SPEC_FILE%" "%LP3D_OBS_DIR%\%LPUB3D%.spec" | findstr /i /v /r /c:"moved\>"
   )
   IF EXIST %LP3D_RPM_LINT_FILE% (
    MOVE /y "%LP3D_RPM_LINT_FILE%" "%LP3D_OBS_DIR%\%LPUB3D%-rpmlintrc" | findstr /i /v /r /c:"moved\>"
   )
   FOR %%i IN (
      %LP3D_OBS_DIR%\debian\source\include-binaries
      %LP3D_OBS_DIR%\debian\changelog
      %LP3D_OBS_DIR%\debian\control
      %LP3D_OBS_DIR%\debian\copyright
      %LP3D_OBS_DIR%\debian\%LPUB3D%.dsc
      %LP3D_OBS_DIR%\debian\%LPUB3D%.lintian-overrides
      %LP3D_OBS_DIR%\_service
      %LP3D_OBS_DIR%\PKGBUILD
      %LP3D_OBS_DIR%\%LPUB3D%-rpmlintrc
      %LP3D_BUILDS_DIR%\linux\docker-compose\docker-compose-linux.yml
      %LP3D_BUILDS_DIR%\utilities\docker\Dockerfile-archlinux_2017.10.01
      %LP3D_BUILDS_DIR%\utilities\docker\Dockerfile-fedora_25
      %LP3D_BUILDS_DIR%\utilities\docker\Dockerfile-ubuntu_xenial
      %LP3D_BUILDS_DIR%\..\appveyor.yml
      %LP3D_BUILDS_DIR%\..\travis.yml
    ) DO (
      CALL :FIND_REPLACE %OLD_LPUB3D% %LPUB3D% %%i
  )
  ECHO.
)

ECHO  3. update desktop configuration - add version suffix
SET LP3D_FILE="%LP3D_MAIN_APP%\lpub3d.desktop"
SET /a LineToReplace=10
SET "Replacement=Exec=lpub3d%LP3D_APP_VER_SUFFIX% %%f"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

ECHO  4. update man page - add version suffix
SET LP3D_FILE="%LP3D_MAIN_APP%\docs\lpub3d%LP3D_APP_VER_SUFFIX%.1"
SET /a LineToReplace=61
SET "Replacement=     /usr/bin/lpub3d%LP3D_APP_VER_SUFFIX%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

ECHO  5. update PKGBUILD - add app version
SET LP3D_FILE="%LP3D_OBS_DIR%\PKGBUILD"
SET /a LineToReplace=3
SET "Replacement=Exec=lpub3d%LP3D_APP_VER_SUFFIX% %%f"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

ECHO  6. create changelog - add app version and change date
SET LP3D_FILE="%LP3D_OBS_DIR%\debian\changelog"
IF EXIST %LP3D_FILE% DEL /Q %LP3D_FILE%
SET createChangeLog=%LP3D_FILE% ECHO
:GENERATE debian change log
>%createChangeLog% lpub3d ^(%LP3D_APP_VERSION%^) xenial; urgency=medium
>>%createChangeLog%.
>>%createChangeLog%   * LPub3D version %LP3D_APP_VERSION_LONG% for Linux
>>%createChangeLog%.
>>%createChangeLog%  -- Trevor SANDY ^<trevor.sandy@gmail.com^>  %LP3D_CHANGE_DATE_LONG%

ECHO  7. update lpub3d.dsc - add app version
SET LP3D_FILE="%LP3D_OBS_DIR%\debian\%LPUB3D%.dsc"
SET /a LineToReplace=5
SET "Replacement=Version: %LP3D_APP_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

ECHO  8. update README.txt - add build version
SET LP3D_FILE="%LP3D_MAIN_APP%\docs\README.txt"
SET /a LineToReplace=1
SET "Replacement=LPub3D %LP3D_BUILD_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

ECHO  9. update lpub3d.spec - add app version and change date
SET LP3D_FILE="%LP3D_OBS_DIR%\%LPUB3D%.spec"
SET LinesToReplace=92 281
FOR /F "tokens=1" %%i IN ("%LinesToReplace%") DO SET "FirstLine=%%i"
FOR /F "tokens=2" %%i IN ("%LinesToReplace%") DO SET "SecondLine=%%i"
FOR %%i IN ("%LinesToReplace%") DO (
IF %%i EQU %FirstLine% SET "Replacement=%LP3D_APP_VERSION%"
IF %%i EQU %SecondLine% SET "Replacement=* %LP3D_CHANGE_DATE% - trevor.dot.sandy.at.gmail.dot.com %LP3D_APP_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new")
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

%LP3D_ME% execution finished.
GOTO :END

:GET_DATE_AND_TIME
SET LP3D_DAY=unknown
SET LP3D_MONTH=unknown
SET LP3D_YEAR=unknown
SET LP3D_HOUR=unknown
SET LP3D_MIN=unknown
SET LP3D_SEC=unknown
FOR /F "skip=1 delims=" %%F IN ('
    wmic PATH Win32_LocalTime GET Day^,DayOfWeek^,Hour^,Minute^,Month^,Second^,Year /FORMAT:TABLE
') DO (
    FOR /F "tokens=1-7" %%L IN ("%%F") DO (
        SET _Day=0%%L
        SET _WeekDay=%%M
        SET _Hour=0%%N
        SET _Min=0%%O
        SET _Month=0%%P
        SET _Sec=0%%Q
        SET _Year=%%R
    )
)
SET LP3D_DAY=%_Day:~-2%
SET LP3D_MONTH=%_Month:~-2%
SET LP3D_YEAR=%_Year%
SET LP3D_HOUR=%_Hour:~-2%
SET LP3D_MIN=%_Min:~-2%
SET LP3D_SEC=%_Sec:~-2%

IF %_WeekDay% == 0 SET LP3D_WEEK_DAY=Sun
IF %_WeekDay% == 1 SET LP3D_WEEK_DAY=Mon
IF %_WeekDay% == 2 SET LP3D_WEEK_DAY=Tue
IF %_WeekDay% == 3 SET LP3D_WEEK_DAY=Wed
IF %_WeekDay% == 4 SET LP3D_WEEK_DAY=Thu
IF %_WeekDay% == 5 SET LP3D_WEEK_DAY=Fri
IF %_WeekDay% == 6 SET LP3D_WEEK_DAY=Sat

IF %LP3D_MONTH% == 01 SET LP3D_MONTH_OF_YEAR=Jan
IF %LP3D_MONTH% == 02 SET LP3D_MONTH_OF_YEAR=Feb
IF %LP3D_MONTH% == 03 SET LP3D_MONTH_OF_YEAR=Mar
IF %LP3D_MONTH% == 04 SET LP3D_MONTH_OF_YEAR=Apr
IF %LP3D_MONTH% == 05 SET LP3D_MONTH_OF_YEAR=May
IF %LP3D_MONTH% == 06 SET LP3D_MONTH_OF_YEAR=Jun
IF %LP3D_MONTH% == 07 SET LP3D_MONTH_OF_YEAR=Jul
IF %LP3D_MONTH% == 08 SET LP3D_MONTH_OF_YEAR=Aug
IF %LP3D_MONTH% == 09 SET LP3D_MONTH_OF_YEAR=Sep
IF %LP3D_MONTH% == 10 SET LP3D_MONTH_OF_YEAR=Oct
IF %LP3D_MONTH% == 11 SET LP3D_MONTH_OF_YEAR=Nov
IF %LP3D_MONTH% == 12 SET LP3D_MONTH_OF_YEAR=Dec
EXIT /b

:GET_GIT_VERSION
CD /D "%LP3D_BUILDS_DIR%\.."
REM Extract Revision Number
FOR /F "usebackq delims==" %%G IN (`git describe --tags --long`) DO SET lp3d_git_ver_tag_long=%%G
SET "tag_input=%lp3d_git_ver_tag_long%"

REM Remove revision prefix ending in "-"
SET "tag_val_1=%tag_input:*-=%"
IF "%tag_val_1%"=="%tag_input%" ECHO revision prefix ending in "-" not found
FOR /F "delims=\" %%a IN ("%tag_val_1%") DO SET "tag_val_1=%%~a"

REM remove revision suffix starting with "-"
SET "lp3d_revision_=%tag_val_1%"
FOR /F "tokens=1 delims=-" %%a IN ("%lp3d_revision_%") DO SET LP3D_VER_REVISION=%%~a

REM Extract commit count (build)
FOR /F "usebackq delims==" %%G IN (`git rev-list HEAD --count`) DO SET LP3D_VER_BUILD=%%G

REM Extract short sha hash
FOR /F "usebackq delims==" %%G IN (`git rev-parse --short HEAD`) DO SET LP3D_VER_SHA_HASH=%%G

REM Extract version
FOR /F "usebackq delims==" %%G IN (`git describe --tags --abbrev^=0`) DO SET lp3d_git_ver_tag_short=%%G

REM Remove version prefix 'v'
SET "lp3d_version_=%lp3d_git_ver_tag_short:v=%"

REM Replace version '.' with ' '
SET "lp3d_version_=%lp3d_version_:.= %"

REM Parse version
FOR /F "tokens=1" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_MAJOR=%%i
FOR /F "tokens=2" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_MINOR=%%i
FOR /F "tokens=3" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_PATCH=%%i
EXIT /b

:GET_AVAILABLE_VERSIONS
SET LP3D_PAST_RELEASES=1.3.5,1.2.3,1.0.0
SET LP3D_AVAILABLE_VERSIONS=%LP3D_VERSION%,%LP3D_PAST_RELEASES%
SET LP3D_PREVIOUS_TAG=unknown
IF [%8] == [] (
  FOR /F "usebackq" %%G IN (`git describe --abbrev^=0 %lp3d_git_ver_tag_short%^^^^ 2^> nul`) DO (
    IF [%%G] NEQ [] SET LP3D_PREVIOUS_TAG=%%G
  )
) ELSE (
  FOR /F "tokens=8*" %%G IN ("%*") DO SET LP3D_PREVIOUS_VERSION=%%G
)
IF "%LP3D_PREVIOUS_TAG%" NEQ "unknown" (
  FOR /F "usebackq" %%G IN (`git describe --abbrev^=0 %LP3D_PREVIOUS_TAG%^^^^ 2^> nul`) DO (
    IF [%%G] NEQ [] SET LP3D_PREVIOUS_VERSIONS=%LP3D_PREVIOUS_TAG%,%%G
  )
)
IF "%LP3D_PREVIOUS_VERSIONS%" NEQ "" (
  rem ECHO   PREVIOUS VERSIONS   = %LP3D_PREVIOUS_VERSIONS%
  SET LP3D_AVAILABLE_VERSIONS=%LP3D_VERSION%,%LP3D_PREVIOUS_VERSIONS:v=%,%LP3D_PAST_RELEASES%
)
CD /D "%LP3D_BUILDS_DIR%"
EXIT /b

:FIXUP_PWD
SET TEMP=%CD%
IF [%1] NEQ [] CD /D "%1\..\builds"
SET LP3D_MAIN_APP=%1
SET LP3D_BUILDS_DIR=%CD%
CD "%1\.."
FOR %%* IN (%CD%) DO SET LPUB3D=%%~nx*
CD %TEMP%
EXIT /b

:FIND_REPLACE <findstr> <replstr> <file>
SET tmp="%temp%\tmp.txt"
IF NOT EXIST %temp%\_.vbs CALL :MAKE_REPLACE
FOR /F "tokens=*" %%a IN ('DIR "%3" /s /b /a-d /on') do (
  FOR /F "usebackq" %%b IN (`Findstr /mic:"%~1" "%%a"`) do (
    ECHO(&ECHO     Replacing "%~1" with "%~2" in file %%~nxa
    <%%a cscript //nologo %temp%\_.vbs "%~1" "%~2">%tmp%
    IF EXIST %tmp% MOVE /Y %tmp% "%%~dpnxa">nul
  )
)
DEL %temp%\_.vbs
EXIT /b

:MAKE_REPLACE
>%temp%\_.vbs echo with Wscript
>>%temp%\_.vbs echo set args=.arguments
>>%temp%\_.vbs echo .StdOut.Write _
>>%temp%\_.vbs echo Replace(.StdIn.ReadAll,args(0),args(1),1,-1,1)
>>%temp%\_.vbs echo end with
EXIT /b

:END
ENDLOCAL
EXIT /b 0
