@ECHO OFF &SETLOCAL

Title Setup a Local LPub3D GitHub actions build environment

rem This script allows you to locally simulate the GitHub actions that build LPub3D for Windows.
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: October 08, 2025
rem  Copyright (C) 2017 - 2025 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

rem To Run:
rem LPub3D
::CD C:\Users\Trevor\Projects\Working\CI_Build_Local && SET "APP_INSTANCE=lpub3d-ci"
::SET APP_INSTANCE_PATH=%CD%
::SET BUILD_DIR=%CD%\%APP_INSTANCE%
::SET LOG_DIR=%APP_INSTANCE_PATH%
::SET LP3D_NO_CHECK=1
::SET LP3D_BUILD_ARCH=-all_amd
::CALL %BUILD_DIR%\builds\windows\GitHub_Run_Local.cmd
REM ///////////////////////////////////////////////////////////////////
rem LPub3D build options
rem LP3D_BUILD_ARCH Valid flags: arm64, x86, x86_64 and -all_amd
::IF EXIST %APP_INSTANCE% (RMDIR /S /Q %APP_INSTANCE%)
::FOR /f "tokens=1-4 delims=/ " %a IN ('date /t') DO (SET LP3D_DATE=%c-%a-%b)
::IF EXIST %APP_INSTANCE% (REN %APP_INSTANCE% %APP_INSTANCE%_%LP3D_DATE%)
::XCOPY /Q /S /I /E /V /Y /H ..\..\%APP_INSTANCE% %APP_INSTANCE%
REM ///////////////////////////////////////////////////////////////////

rem LDGLite
::CD C:\Users\Trevor\Projects\Working\CI_Build_Local && SET "APP_INSTANCE=ldglite" && SET "APP_VER=1.3"
::SET LP3D_BUILD_ARCH=arm64
::SET LP3D_APP_ARCH=lpub3d_windows_3rdparty\%APP_INSTANCE%-%APP_VER%\bin\%LP3D_BUILD_ARCH%
::IF EXIST %LP3D_APP_ARCH% (RMDIR /S /Q %LP3D_APP_ARCH%)
::SET APP_INSTANCE_PATH=%CD%
::SET BUILD_DIR=%CD%\%APP_INSTANCE%
::SET LOG_DIR=%APP_INSTANCE_PATH%
::CALL %CD%\lpub3d-ci\builds\windows\GitHub_Run_Local.cmd

rem POV-Ray
::CD C:\Users\Trevor\Projects\Working\CI_Build_Local && SET "APP_INSTANCE=povray" && SET "APP_VER=3.8"
::SET LP3D_BUILD_ARCH=x86
::SET LP3D_APP_ARCH=lpub3d_windows_3rdparty\lpub3d_trace_cui-%APP_VER%\bin\%LP3D_BUILD_ARCH%
::IF EXIST %LP3D_APP_ARCH% (RMDIR /S /Q %LP3D_APP_ARCH%)
::SET APP_INSTANCE_PATH=%CD%
::SET BUILD_DIR=%CD%\%APP_INSTANCE%
::SET LOG_DIR=%APP_INSTANCE_PATH%
::CALL %CD%\lpub3d-ci\builds\windows\GitHub_Run_Local.cmd

rem LDView
::CD C:\Users\Trevor\Projects\Working\CI_Build_Local && SET "APP_INSTANCE=ldview" && SET "APP_VER=4.6"
::SET LP3D_BUILD_ARCH=arm64
::SET LP3D_APP_ARCH=lpub3d_windows_3rdparty\%APP_INSTANCE%-%APP_VER%\bin\%LP3D_BUILD_ARCH%
::IF EXIST %LP3D_APP_ARCH% (RMDIR /S /Q %LP3D_APP_ARCH%)
::SET APP_INSTANCE_PATH=%CD%
::SET BUILD_DIR=%CD%\%APP_INSTANCE%
::SET LOG_DIR=%APP_INSTANCE_PATH%
::CALL %CD%\lpub3d-ci\builds\windows\GitHub_Run_Local.cmd

REM CLS
rem set repository name
IF "%APP_INSTANCE%" == "" SET APP_INSTANCE=lpub3d-ci
ECHO.
ECHO ======================================================
ECHO   -Start %~nx0 running "%APP_INSTANCE%"
ECHO ------------------------------------------------------
ECHO.
rem ensure we are running from outside of the lpub3d repository
IF NOT EXIST "%CD%\%APP_INSTANCE%" (
  ECHO - ERROR: Invalid script run path: %~nx0 must be called from outside the %APP_INSTANCE% repository folder.
  GOTO :END
)
rem Setup paths
IF "%APP_INSTANCE_PATH%" == "" SET APP_INSTANCE_PATH=%CD%
ECHO - APP_INSTANCE_PATH=%APP_INSTANCE_PATH%
CD %APP_INSTANCE_PATH%
rem Set GitHub vars
SET GITHUB=True
SET RUNNER_OS=Windows
SET GITHUB_CONFIG=release
SET GITHUB_JOB=Local CI Build
SET GITHUB_WORKSPACE=%CD%\%APP_INSTANCE%
SET GITHUB_REPOSITORY=trevorsandy\%APP_INSTANCE%
rem Set your desired commit message
FOR /F "tokens=1-4 delims=/ " %%a IN ('date /t') DO (SET LP3D_DATE=%%c.%%a.%%b)
SET LP3D_COMMIT_MSG=LPub3D %LP3D_DATE%
SET LP3D_BUILD_BASE=%CD%
SET LP3D_3RD_PARTY=third_party
SET LP3D_3RD_PARTY_PATH=%LP3D_BUILD_BASE%\%LP3D_3RD_PARTY%
SET LP3D_BUILDPKG_PATH=builds\windows\%GITHUB_CONFIG%
rem Set your Visual Studio year
SET LP3D_VSVERSION=2022
rem https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/
rem https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/index-legacy
rem https://learn.microsoft.com/en-us/cpp/overview/compiler-versions
rem https://en.wikipedia.org/wiki/Microsoft_Visual_C++
rem Visual C++ 2012 -vcvars_ver=11.0 Toolset v110 VSVersion 11.0    _MSC_VER 1700
rem Visual C++ 2013 -vcvars_ver=12.0 Toolset v120 VSVersion 12.0    _MSC_VER 1800
rem Visual C++ 2015 -vcvars_ver=14.0 Toolset v140 VSVersion 14.0    _MSC_VER 1900
rem Visual C++ 2017 -vcvars_ver=14.1 Toolset v141 VSVersion 15.9    _MSC_VER 1916
rem Visual C++ 2019 -vcvars_ver=14.2 Toolset v142 VSVersion 16.11.3 _MSC_VER 1929
rem Visual C++ 2022 -vcvars_ver=14.4 Toolset v143 VSVersion 17.14.0 _MSC_VER 1944
rem 32bit AMD VC settings
SET LP3D_MSC32_VER=1944
SET LP3D_VC32SDKVER=10.0
SET LP3D_VC32TOOLSET=v141
SET LP3D_VC32VARSALL_VER=-vcvars_ver=14.1
rem 64bit AMD VC settings
SET LP3D_MSC64_VER=1944
SET LP3D_VC64SDKVER=10.0
SET LP3D_VC64TOOLSET=v143
SET LP3D_VC64VARSALL_VER=-vcvars_ver=14.4
rem 64bit ARM VC settings
SET LP3D_MSCARM64_VER=1944
SET LP3D_VCARM64SDKVER=10.0
SET LP3D_VCARM64TOOLSET=v143
SET LP3D_VCARM64VARSALL_VER=-vcvars_ver=14.4
rem Set your Qt Version and installed QtMSVC platforms paths
SET LP3D_QT64VCVERSION=2022
SET LP3D_QT64VERSION=6.10.0
SET LP3D_QT64_MSVC=C:\Qt\IDE\%LP3D_QT64VERSION%\msvc%LP3D_QT64VCVERSION%_64\bin
SET LP3D_QT32VERSION=5.15.2
SET LP3D_QT32VCVERSION=2019
SET LP3D_QT32_MSVC=C:\Qt\IDE\%LP3D_QT32VERSION%\msvc%LP3D_QT32VCVERSION%\bin
rem Set Create executable package flag based on LP3D_BUILD_ARCH
IF "%LP3D_BUILD_ARCH%" EQU "" SET LP3D_BUILD_ARCH=-all_amd
IF "%LP3D_BUILD_ARCH%" EQU "arm64" (
  SET LP3D_QT64_MSVC=C:\Qt\IDE\%LP3D_QT64VERSION%\msvc%LP3D_QT64VCVERSION%_arm64\bin
)
rem Renderer Vars
SET LP3D_DIST_DIR_PATH=%APP_INSTANCE_PATH%\lpub3d_windows_3rdparty
SET LP3D_LDRAW_DIR_PATH=%USERPROFILE%\ldraw
rem Setup distribution folders
IF NOT EXIST "%LP3D_3RD_PARTY%" ( MKLINK /d %LP3D_3RD_PARTY% %LP3D_BUILD_BASE%\lpub3d_windows_3rdparty 2>&1)
IF NOT EXIST "%LP3D_3RD_PARTY_PATH%\windows" ( MKLINK /d "%LP3D_3RD_PARTY%\windows" %LP3D_BUILD_BASE%\lpub3d_windows_3rdparty 2>&1 )
IF NOT EXIST "%LP3D_3RD_PARTY_PATH%\ldraw" ( MKLINK /d "%LP3D_3RD_PARTY%\ldraw" %USERPROFILE%\LDraw 2>&1 )
rem Set and display build log paths
SET BUILD_DIR=%APP_INSTANCE_PATH%\%APP_INSTANCE%
ECHO - BUILD_DIR %BUILD_DIR%
SET LOG_DIR=%APP_INSTANCE_PATH%
SET BUILD_LOG=%LOG_DIR%\%APP_INSTANCE%_%LP3D_BUILD_ARCH%_build_check_log.txt
ECHO - BUILD_LOG %BUILD_LOG%
rem Set your repository version and tag details
PUSHD %GITHUB_WORKSPACE%
FOR /F "usebackq delims==" %%G IN (`git describe --abbrev^=0 --tags`) DO (SET LP3D_LOCAL_TAG=%%G)
FOR /F "usebackq delims==" %%G IN (`git describe`) DO (SET GITHUB_REF_NAME=%%G)
FOR /F "usebackq delims==" %%G IN (`git rev-parse HEAD`) DO (SET GITHUB_SHA=%%G)
POPD
SET GITHUB_REF=refs/tags/%GITHUB_REF_NAME%
rem Set LP3D_INSTALL_PKG_ONLY=1 to skip package build and perform install for packaging only - FIX NEEDED
SET LP3D_INSTALL_PKG_ONLY=0
rem Set LP3D_LOCAL_CI_BUILD=1 so windows-build.bat does not check stale commit
SET LP3D_LOCAL_CI_BUILD=1
CD %APP_INSTANCE_PATH%
rem Add Application environment variables here...
SET LDVIEW_DEV=Do not use
rem ----------------------------------------
IF [%QT_BUILD%]==[] SET QT_BUILD=0
IF "%APP_INSTANCE%" == "ldglite" (
  IF %QT_BUILD%==1 (
    ECHO - BUILDING %APP_INSTANCE% %LP3D_BUILD_ARCH% QT_BUILD...
    CD %BUILD_DIR% && CALL build_qt.cmd %LP3D_BUILD_ARCH% -ins -chk -minlog > "%BUILD_LOG%" 2>&1
  ) ELSE (
    ECHO - BUILDING %APP_INSTANCE% %LP3D_BUILD_ARCH% VS_BUILD...
    CD %BUILD_DIR% && CALL build.cmd %LP3D_BUILD_ARCH% -ins -chk -minlog > "%BUILD_LOG%" 2>&1
  )
)
IF "%APP_INSTANCE%" == "ldview" (
  ECHO - BUILDING %APP_INSTANCE% %LP3D_BUILD_ARCH%...
  CD %BUILD_DIR% && CALL build.cmd %LP3D_BUILD_ARCH% -ins_libs -chk -minlog | %APP_INSTANCE_PATH%\win_tee.cmd "%BUILD_LOG%"
)
IF "%APP_INSTANCE%" == "povray" (
  ECHO - BUILDING %APP_INSTANCE% %LP3D_BUILD_ARCH%...
  CD %BUILD_DIR%\windows\vs2015 && CALL autobuild.cmd %LP3D_BUILD_ARCH% -allins -chk -minlog > "%BUILD_LOG%" 2>&1
)
IF "%APP_INSTANCE%" == "lpub3d-ci" (
  ECHO - BUILDING %APP_INSTANCE% %LP3D_BUILD_ARCH%...
  CD %BUILD_DIR% && CALL builds\utilities\ci\github\windows-build.bat | builds\utilities\win_tee.cmd "%BUILD_LOG%"
)
:END
ECHO.
ECHO - %~nx0 finished with return code %ERRORLEVEL%.
ENDLOCAL
EXIT /b
