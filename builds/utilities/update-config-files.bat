@ECHO OFF &SETLOCAL
Title Update LPub3D files with build version number
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: September 10, 2025
rem  Copyright (C) 2015 - 2025 by Trevor SANDY
rem --
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
rem
rem To Run:
rem CD <LPub3D root>
rem SET _PRO_FILE_PWD_=<LPub3D absolute path>\mainApp
rem CALL builds/utilities/update-config-files.bat %_PRO_FILE_PWD_% [ParseVersionInfoFile|NoFileUpdates|<version string>]
rem Options:
rem - ParseVersionInfoFile: Use the version info file to determine the version attributes - used by Conda-build
rem - NoFileUpdates: Do not perfor configuration file updates - used by RunBulildCheck.bat
rem - <version string>: Space delimited version attributes - Major Minor Patch Revision Build ShaHash

SET LP3D_ME=%~nx0

CALL :UCF_FIXUP_PWD %1

IF [%LP3D_BUILDS_DIR%] == [] (
  ECHO Error: Did not receive required argument _PRO_FILE_PWD_
  ECHO %LP3D_ME% terminated!
  ECHO.
  GOTO :UCF_FATAL_ERROR
)

SET /a NUM_ARGS=0
FOR %%a IN (%*) DO SET /a NUM_ARGS+=1

rem Line number to replace
SET LINE_README_TXT=1
SET LINE_README_MD_VER=67
SET LINE_RELEASE_NOTES_HTM=83

SET LP3D_GIT_DEPTH=150000
SET LP3D_PAST_RELEASES=2.3.6,2.0.20
SET LP3D_BUILDS_DIR=%LP3D_BUILDS_DIR:"=%
SET LP3D_VER_INFO_FILE=%LP3D_BUILDS_DIR%\utilities\version.info
SET LP3D_CALL_DIR=%CD%

ECHO  Start %LP3D_ME% execution at %CD%...
IF [%3] EQU [] (
  IF [%2] EQU [ParseVersionInfoFile] (
    ECHO  capture version info using version.info file...
    CALL :UCF_PARSE_VERSION_INFO_FILE
  ) ELSE (
    IF [%2] NEQ [QuietConfiguration] (
      ECHO  capture version info using git queries...
	)
    CALL :UCF_GET_GIT_VERSION
  )
  IF ERRORLEVEL 1 (
    GOTO :UCF_FATAL_ERROR
  )
) ELSE (
  IF %NUM_ARGS% GTR 6 (
    ECHO  capture version info using version arguments...
    SET LP3D_VER_MAJOR=%2
    SET LP3D_VER_MINOR=%3
    SET LP3D_VER_PATCH=%4
    SET LP3D_VER_REVISION=%5
    SET LP3D_VER_BUILD=%6
    SET LP3D_VER_SHA_HASH=%7
    IF [%8] NEQ [] (SET LP3D_VER_SUFFIX=%8)
  )
)

SET LP3D_VERSION=unknown
SET LP3D_WEEK_DAY=unknown
SET LP3D_MONTH_OF_YEAR=unknown
SET LP3D_AVAILABLE_VERSIONS=unknown

CALL :UCF_GET_DATE_AND_TIME

SET LP3D_TIME=%LP3D_HOUR%:%LP3D_MIN%:%LP3D_SEC%
SET LP3D_BUILD_DATE=%LP3D_YEAR%%LP3D_MONTH%%LP3D_DAY%
SET LP3D_LAST_EDIT=%LP3D_DAY%-%LP3D_MONTH%-%LP3D_YEAR%
SET LP3D_BUILD_DATE_TIME=%LP3D_DAY% %LP3D_MONTH% %LP3D_YEAR% %LP3D_TIME%
SET LP3D_CHANGE_DATE_LONG=%LP3D_WEEK_DAY%, %LP3D_DAY% %LP3D_MONTH_OF_YEAR% %LP3D_YEAR% %LP3D_TIME% +0100
SET LP3D_CHANGE_DATE=%LP3D_WEEK_DAY% %LP3D_MONTH_OF_YEAR% %LP3D_DAY% %LP3D_YEAR%
SET LP3D_VERSION=%LP3D_VER_MAJOR%.%LP3D_VER_MINOR%.%LP3D_VER_PATCH%
SET LP3D_APP_VERSION=%LP3D_VERSION%.%LP3D_VER_BUILD%
SET LP3D_APP_VERSION_TAG=v%LP3D_VERSION%
SET LP3D_APP_VER_SUFFIX=%LP3D_VER_MAJOR%%LP3D_VER_MINOR%
SET LP3D_APP_VERSION_LONG=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD%_%LP3D_BUILD_DATE%
SET LP3D_BUILD_VERSION=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD% ^(%LP3D_BUILD_DATE_TIME%^)
SET LP3D_AVAILABLE_VERSIONS=%LP3D_VERSION%,%LP3D_PAST_RELEASES%
SET LP3D_VERSION_INFO=%LP3D_VER_MAJOR% %LP3D_VER_MINOR% %LP3D_VER_PATCH% %LP3D_VER_REVISION% %LP3D_VER_BUILD% %LP3D_VER_SHA_HASH%
IF [%LP3D_VER_SUFFIX%] NEQ [] (
  SET LP3D_VERSION_INFO=%LP3D_VERSION_INFO% %LP3D_VER_SUFFIX%
  SET LP3D_APP_VERSION_TAG=v%LP3D_VERSION%_%LP3D_VER_SUFFIX%
)
IF "%LP3D_CONDA_BUILD%" EQU "True" (
  SET "BUILD_WORKER_REF=refs/tags/v%LP3D_VERSION%"
  SET "LP3D_COMMIT_MSG=LPub3D conda build %LP3D_DAY%.%LP3D_MONTH%.%LP3D_YEAR%"
)

CD /D "%LP3D_BUILDS_DIR%"

IF [%2] NEQ [NoFileUpdates] (
  IF [%2] NEQ [QuietConfiguration] CALL :UCF_UPDATE_CONFIG_FILES
)

IF "%LP3D_BUILD_TYPE%" EQU "continuous" (
  ECHO   LP3D_BUILD_TYPE................[Continuous]
) ELSE (
  ECHO   LP3D_BUILD_TYPE................[Release]
)
IF [%2] EQU [QuietConfiguration] GOTO :UCF_WRITE_VERSION_INFO
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
IF [%LP3D_VER_SUFFIX%] NEQ [] (
  ECHO   LP3D_VER_SUFFIX................[%LP3D_VER_SUFFIX%]
)
ECHO   LP3D_APP_VER_SUFFIX............[%LP3D_APP_VER_SUFFIX%]
ECHO   LP3D_BUILD_VERSION.............[%LP3D_BUILD_VERSION%]
ECHO   LP3D_BUILD_DATE_TIME...........[%LP3D_BUILD_DATE_TIME%]
ECHO   LP3D_CHANGE_DATE_LONG..........[%LP3D_CHANGE_DATE_LONG%]
ECHO   LP3D_LAST_EDIT.................[%LP3D_LAST_EDIT%]

ECHO   LP3D_VERSION...................[%LP3D_VERSION%]
ECHO   LP3D_APP_VERSION...............[%LP3D_APP_VERSION%]
ECHO   LP3D_APP_VERSION_LONG..........[%LP3D_APP_VERSION_LONG%]
REM ECHO   LP3D_APP_VERSION_TAG...........[%LP3D_APP_VERSION_TAG%]

ECHO   LP3D_SOURCE_DIR................[%LPUB3D%-%LP3D_APP_VERSION%]
ECHO   LP3D_AVAILABLE_VERSIONS........[%LP3D_AVAILABLE_VERSIONS%]

:UCF_WRITE_VERSION_INFO
IF [%2] NEQ [ParseVersionInfoFile] (
  IF EXIST "%LP3D_VER_INFO_FILE%" DEL /Q "%LP3D_VER_INFO_FILE%"
  ECHO %LP3D_VERSION_INFO% > %LP3D_VER_INFO_FILE%
  IF EXIST "%LP3D_VER_INFO_FILE%" (
    IF [%2] NEQ [QuietConfiguration] (
      ECHO   FILE version.info..............[written to builds\utilities\version.info]
    )
  ) ELSE (
    ECHO   FILE version.info..............[ERROR - file %LP3D_VER_INFO_FILE% not found]
  )
)
GOTO :UCF_END

:UCF_UPDATE_CONFIG_FILES
SET LP3D_FILE="%LP3D_MAIN_APP%\docs\RELEASE_NOTES.html"
ECHO  update RELEASE_NOTES.html build version [%LP3D_FILE%]
SET /a LineToReplace=%LINE_RELEASE_NOTES_HTM%

SET "Replacement=  ^<li^>^<code^>^<h4^>LPub3D %LP3D_BUILD_VERSION%^</h4^>^</code>^</li>"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a EQU %LineToReplace% SET "Line=%Replacement:^=%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE% | findstr /i /v /r /c:"moved\>"

SET LP3D_FILE="%LP3D_MAIN_APP%\docs\README.txt"
ECHO  update README.txt build version [%LP3D_FILE%]
SET /a LineToReplace=%LINE_README_TXT%
SET "Replacement=LPub3D %LP3D_BUILD_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a EQU %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE% | findstr /i /v /r /c:"moved\>"

SET LP3D_FILE="%LP3D_MAIN_APP%\..\README.md"
ECHO  update README.md version        [%LP3D_FILE%]
SET /a LineToReplace=%LINE_README_MD_VER%
SET "Replacement=[gh-maintained-url]: https://github.com/trevorsandy/lpub3d-ci/projects/1 "Last edited %LP3D_LAST_EDIT%""
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a EQU %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE% | findstr /i /v /r /c:"moved\>"
EXIT /b

:UCF_FIXUP_PWD
SET TEMP=%CD%
IF [%1] NEQ [] CD /D "%1\..\builds"
SET LP3D_MAIN_APP=%1
SET LP3D_BUILDS_DIR=%CD%
CD "%1\.."
FOR %%* IN (%CD%) DO SET LPUB3D=%%~nx*
CD %TEMP%
EXIT /b

:UCF_PARSE_VERSION_INFO_FILE
SET /p LP3D_VERSION_INFO=<%LP3D_VER_INFO_FILE%
FOR /f "tokens=1-6 delims= " %%I IN ("%LP3D_VERSION_INFO%") DO (
  SET LP3D_VER_MAJOR=%%I
  SET LP3D_VER_MINOR=%%J
  SET LP3D_VER_PATCH=%%K
  SET LP3D_VER_REVISION=%%L
  SET LP3D_VER_BUILD=%%M
  SET LP3D_VER_SHA_HASH=%%N
)
EXIT /b

:UCF_GET_GIT_VERSION
IF "%LP3D_CONDA_BUILD%" EQU "True" (
  ECHO  ERROR: Conda-build source does not include .git folder.
  EXIT /b
)

CD /D "%LP3D_BUILDS_DIR%\.."

REM Test for .git folder
IF "%APPVEYOR%" NEQ "True" (
  IF "%APPVEYOR%" NEQ "True" (
    IF NOT EXIST ".git" (
      ECHO  ERROR: .git folder not found.
      EXIT /b 1
    )
  )
)

REM Get build type
FOR /F "usebackq delims==" %%G IN (`git describe --tags --abbrev^=0 2^> nul`) DO SET LP3D_BUILD_TYPE=%%G
rem ECHO  DEBUG LP3D_BUILD_TYPE IS %LP3D_BUILD_TYPE%

REM Update Github refs and tags
REM IF "%GITHUB%" EQU "True" (
REM   git fetch -qfup --depth=%LP3D_GIT_DEPTH% origin +%GITHUB_REF% +refs/tags/*:refs/tags/*
REM   git checkout -qf %GITHUB_SHA%
REM )

REM Update Appveyor refs and tags
IF "%APPVEYOR%" EQU "True" (
  git fetch -qfup --depth=%LP3D_GIT_DEPTH% origin +%APPVEYOR_REPO_BRANCH% +refs/tags/*:refs/tags/*
  git checkout -qf %APPVEYOR_REPO_COMMIT%
)

REM Get long tag - ignore continuous
FOR /F "usebackq delims==" %%G IN (`git describe --tags --match v* --long 2^> nul`) DO SET lp3d_git_ver_tag_long=%%G
SET "tag_input=%lp3d_git_ver_tag_long%"
rem ECHO  DEBUG LP3D_GIT_VER_TAG_LONG IS %tag_input%

REM Get Revision - remove everything before and including "-" from long tag
SET "tag_val_1=%tag_input:*-=%"
IF "%tag_val_1%"=="%tag_input%" ECHO ERROR - revision prefix ending in "-" not found in %tag_input%
FOR /F "delims=\" %%a IN ("%tag_val_1%") DO SET "tag_val_1=%%~a"

REM Get Revision - remove everything after and including "-" from long tag
SET "lp3d_revision_=%tag_val_1%"
FOR /F "tokens=1 delims=-" %%a IN ("%lp3d_revision_%") DO SET LP3D_VER_REVISION=%%~a
rem ECHO  DEBUG LP3D_VER_REVISION IS %LP3D_VER_REVISION%

REM Extract commit count ^(build^)
FOR /F "usebackq delims==" %%G IN (`git rev-list HEAD --count 2^> nul`) DO SET LP3D_VER_BUILD=%%G
rem ECHO  DEBUG LP3D_VER_BUILD IS %LP3D_VER_BUILD%

REM Extract short sha hash
FOR /F "usebackq delims==" %%G IN (`git rev-parse --short HEAD 2^> nul`) DO SET LP3D_VER_SHA_HASH=%%G
rem ECHO  DEBUG LP3D_VER_SHA_HASH IS %LP3D_VER_SHA_HASH%

REM Get short tag - ignore continuous
FOR /F "usebackq delims==" %%G IN (`git describe --tags --match v* --abbrev^=0 2^> nul`) DO SET lp3d_git_ver_tag_short=%%G
rem ECHO  DEBUG LP3D_GIT_VER_TAG_SHORT IS %lp3d_git_ver_tag_short%

REM Remove version prefix 'v'
SET "lp3d_version_=%lp3d_git_ver_tag_short:v=%"

REM Capture version suffix - everything after "_" if it exist
FOR /F "tokens=2 delims=_" %%a IN ("%lp3d_version_%") DO SET LP3D_VER_SUFFIX=%%~a

REM Remove version suffix - everything after and including "_" if it exist
IF [%LP3D_VER_SUFFIX%] NEQ [] (
  FOR /F "tokens=1 delims=_" %%a IN ("%lp3d_version_%") DO SET lp3d_version_=%%~a
)

REM Replace version '.' with ' '
SET "lp3d_version_=%lp3d_version_:.= %"

REM Parse version
FOR /F "tokens=1" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_MAJOR=%%i
FOR /F "tokens=2" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_MINOR=%%i
FOR /F "tokens=3" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_PATCH=%%i
EXIT /b

:UCF_GET_DATE_AND_TIME
SET LP3D_DAY=unknown
SET LP3D_MONTH=unknown
SET LP3D_YEAR=unknown
SET LP3D_HOUR=unknown
SET LP3D_MIN=unknown
SET LP3D_SEC=unknown
FOR /F "delims=" %%F IN ('
	%WINDIR%\System32\WindowsPowerShell\v1.0\powershell -c "$Time = Get-WmiObject Win32_LocalTime; '{0} {1} {2} {3} {4} {5} {6}' -f $Time.Day, $Time.DayOfWeek, $Time.Hour, $Time.Minute, $Time.Month, $Time.Second, $Time.Year"
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

:UCF_FIND_REPLACE <findstr> <replstr> <file>
SET tmp="%temp%\tmp.txt"
IF NOT EXIST %temp%\_.vbs CALL :UCF_MAKE_REPLACE
FOR /F "tokens=*" %%a IN ('DIR "%3" /s /b /a-d /on') do (
  FOR /F "usebackq" %%b IN (`Findstr /mic:"%~1" "%%a"`) do (
    ECHO(&ECHO     Replacing "%~1" with "%~2" in file %%~nxa
    <%%a cscript //nologo %temp%\_.vbs "%~1" "%~2">%tmp%
    IF EXIST %tmp% MOVE /Y %tmp% "%%~dpnxa">nul
  )
)
DEL %temp%\_.vbs
EXIT /b

:UCF_MAKE_REPLACE
>%temp%\_.vbs echo with Wscript
>>%temp%\_.vbs echo set args=.arguments
>>%temp%\_.vbs echo .StdOut.Write _
>>%temp%\_.vbs echo Replace(.StdIn.ReadAll,args(0),args(1),1,-1,1)
>>%temp%\_.vbs echo end with
EXIT /b

:UCF_FATAL_ERROR
ECHO  %LP3D_ME% execution failed.
EXIT /b 1

:UCF_END
ECHO  %LP3D_ME% execution finished.
ENDLOCAL & (
  SET LP3D_SOURCE_DIR=%LP3D_SOURCE_DIR%
  SET LP3D_CALL_DIR=%LP3D_CALL_DIR%

  SET LP3D_DAY=%LP3D_DAY%
  SET LP3D_MONTH=%LP3D_MONTH%
  SET LP3D_YEAR=%LP3D_YEAR%
  SET LP3D_HOUR=%LP3D_HOUR%
  SET LP3D_MIN=%LP3D_MIN%
  SET LP3D_SEC=%LP3D_SEC%

  SET LP3D_TIME=%LP3D_TIME%
  SET LP3D_WEEK_DAY=%LP3D_WEEK_DAY%
  SET LP3D_MONTH_OF_YEAR=%LP3D_MONTH_OF_YEAR%

  SET LP3D_BUILD_TYPE=%LP3D_BUILD_TYPE%
  SET LP3D_VER_MAJOR=%LP3D_VER_MAJOR%
  SET LP3D_VER_MINOR=%LP3D_VER_MINOR%
  SET LP3D_VER_PATCH=%LP3D_VER_PATCH%
  SET LP3D_VER_REVISION=%LP3D_VER_REVISION%
  SET LP3D_VER_BUILD=%LP3D_VER_BUILD%
  SET LP3D_VER_SHA_HASH=%LP3D_VER_SHA_HASH%
  IF [%LP3D_VER_SUFFIX%] NEQ [] (
    SET LP3D_VER_SUFFIX=%LP3D_VER_SUFFIX%
  )
  SET LP3D_VERSION=%LP3D_VERSION%
  SET LP3D_APP_VERSION=%LP3D_APP_VERSION%
  SET LP3D_APP_VERSION_TAG=%LP3D_APP_VERSION_TAG%
  SET LP3D_APP_VER_SUFFIX=%LP3D_APP_VER_SUFFIX%
  SET LP3D_APP_VERSION_LONG=%LP3D_APP_VERSION_LONG%
  SET LP3D_VERSION_INFO=%LP3D_VERSION_INFO%
  SET LP3D_BUILD_DATE_TIME=%LP3D_BUILD_DATE_TIME%
  SET LP3D_CHANGE_DATE_LONG=%LP3D_CHANGE_DATE_LONG%
  SET LP3D_AVAILABLE_VERSIONS=%LP3D_AVAILABLE_VERSIONS%
  SET LP3D_BUILD_VERSION=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD% ^(%LP3D_BUILD_DATE_TIME%^)
  IF "%LP3D_CONDA_BUILD%" EQU "True" (
    SET BUILD_WORKER_REF=%BUILD_WORKER_REF%
    SET LP3D_COMMIT_MSG=%LP3D_COMMIT_MSG%
  )
)
EXIT /b 0
