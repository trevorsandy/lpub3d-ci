
C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=#"  
 IF 1 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# spec file for package lpub3d"  
 IF 2 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# spec file for package lpub3d

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=#"  
 IF 3 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
#

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# Copyright © 2017 Trevor SANDY"  
 IF 4 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# Copyright © 2017 Trevor SANDY

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# Using RPM Spec file examples by Thomas Baumgart, Peter Bartfai and others"  
 IF 5 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# Using RPM Spec file examples by Thomas Baumgart, Peter Bartfai and others

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# This file and all modifications and additions to the pristine"  
 IF 6 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# This file and all modifications and additions to the pristine

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# package are under the same license as the package itself."  
 IF 7 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# package are under the same license as the package itself.

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=#"  
 IF 8 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
#

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# please send bugfixes or comments to Trevor SANDY <trevor.sandy@gmail.com>"  
 IF 9 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# please send bugfixes or comments to Trevor SANDY <trevor.sandy@gmail.com>

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=#"  
 IF 10 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
#

Version: 2.0.21.175
SET "Line="  
 IF 11 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# set packing platform"  
 IF 12 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# set packing platform

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define serviceprovider %(echo %{vendor})"  
 IF 13 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define serviceprovider %(echo %{vendor})

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if %(if [[ "%{vendor}" == obs://* ]]; then echo 1; else echo 0; fi)"  
 IF 14 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if %(if [[ "%{vendor}" == obs://* ]]; then echo 1; else echo 0; fi)

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define buildservice 1"  
 IF 15 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define buildservice 1

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define packingplatform %(echo openSUSE BuildService)"  
 IF 16 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define packingplatform %(echo openSUSE BuildService)

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%else"  
 IF 17 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%else

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define packingplatform %(source /etc/os-release && if [ "$PRETTY_NAME" != "" ]; then echo $HOSTNAME ["$PRETTY_NAME"]; else echo $HOSTNAME [`uname`]; fi)"  
 IF 18 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define packingplatform %(source /etc/os-release && if [ "$PRETTY_NAME" != "" ]; then echo $HOSTNAME ["$PRETTY_NAME"]; else echo $HOSTNAME [`uname`]; fi)

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 19 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 20 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# set packer"  
 IF 21 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# set packer

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?buildservice}==1"  
 IF 22 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?buildservice}==1

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define distpacker %(echo openSUSE BuildService [abuild])"  
 IF 23 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define distpacker %(echo openSUSE BuildService [abuild])

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define buildserviceflag %{buildservice}"  
 IF 24 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define buildserviceflag %{buildservice}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define targetplatform %{_target}"  
 IF 25 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define targetplatform %{_target}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%else"  
 IF 26 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%else

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: finger"  
 IF 27 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: finger

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define distpacker %(finger -lp `echo "$USER"` | head -n 1 | cut -d: -f 3)"  
 IF 28 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define distpacker %(finger -lp `echo "$USER"` | head -n 1 | cut -d: -f 3)

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define buildserviceflag 0"  
 IF 29 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define buildserviceflag 0

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define targetplatform %{packingplatform}"  
 IF 30 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define targetplatform %{packingplatform}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 31 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

* Mon Oct 30 2017 - trevor.dot.sandy.at.gmail.dot.com 2.0.21.175
SET "Line="  
 IF 32 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# set target platform"  
 IF 33 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# set target platform

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?suse_version}"  
 IF 34 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?suse_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define dist .openSUSE%(echo %{suse_version} | sed 's/0$//')"  
 IF 35 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define dist .openSUSE%(echo %{suse_version} | sed 's/0$//')

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 36 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 37 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?sles_version}"  
 IF 38 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?sles_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define dist .SUSE%(echo %{sles_version} | sed 's/0$//')"  
 IF 39 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define dist .SUSE%(echo %{sles_version} | sed 's/0$//')

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 40 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 41 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora}"  
 IF 42 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define dist fc"  
 IF 43 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define dist fc

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 44 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 45 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?mageia}"  
 IF 46 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?mageia}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define dist .mga%{mgaversion}"  
 IF 47 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define dist .mga%{mgaversion}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define distsuffix .mga%{mgaversion}"  
 IF 48 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define distsuffix .mga%{mgaversion}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 49 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 50 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?scientificlinux_version}"  
 IF 51 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?scientificlinux_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define dist scl"  
 IF 52 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define dist scl

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 53 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 54 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?rhel_version}"  
 IF 55 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?rhel_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define dist rhel"  
 IF 56 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define dist rhel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 57 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 58 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?centos_ver}"  
 IF 59 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?centos_ver}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define centos_version %{centos_ver}00"  
 IF 60 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define centos_version %{centos_ver}00

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define dist cos"  
 IF 61 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define dist cos

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 62 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 63 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# distro group settings"  
 IF 64 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# distro group settings

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?suse_version} || 0%{?sles_version}"  
 IF 65 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?suse_version} || 0%{?sles_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Group: Productivity/Graphics/Viewers"  
 IF 66 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Group: Productivity/Graphics/Viewers

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 67 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 68 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?mageia} || 0%{?rhel_version}"  
 IF 69 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?mageia} || 0%{?rhel_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Group: Graphics"  
 IF 70 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Group: Graphics

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 71 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 72 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?suse_version} || 0%{?sles_version}"  
 IF 73 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?suse_version} || 0%{?sles_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=License: GPL-3.0+"  
 IF 74 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
License: GPL-3.0+

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: fdupes"  
 IF 75 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: fdupes

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 76 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora} || 0%{?centos_version}"  
 IF 77 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora} || 0%{?centos_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Group: Amusements/Graphics"  
 IF 78 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Group: Amusements/Graphics

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 79 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?scientificlinux_version} || 0%{?mageia}"  
 IF 80 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?scientificlinux_version} || 0%{?mageia}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=License: GPLv3+"  
 IF 81 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
License: GPLv3+

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 82 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 83 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# set custom directory paths"  
 IF 84 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# set custom directory paths

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define _3rdexedir /opt/lpub3d/3rdParty"  
 IF 85 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define _3rdexedir /opt/lpub3d/3rdParty

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%define _iconsdir %{_datadir}/icons"  
 IF 86 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%define _iconsdir %{_datadir}/icons

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 87 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# preamble"  
 IF 88 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# preamble

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Summary: An LDraw Building Instruction Editor"  
 IF 89 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Summary: An LDraw Building Instruction Editor

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Name: lpub3d-ci"  
 IF 90 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Name: lpub3d-ci

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Icon: lpub3d.xpm"  
 IF 91 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Icon: lpub3d.xpm

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Version: 2.0.21.175"  
 IF 92 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Version: 2.0.21.175

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Release: %{?dist}"  
 IF 93 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Release: %{?dist}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=URL: https://trevorsandy.github.io/lpub3d"  
 IF 94 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
URL: https://trevorsandy.github.io/lpub3d

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Vendor: Trevor SANDY"  
 IF 95 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Vendor: Trevor SANDY

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRoot: %{_builddir}/%{name}"  
 IF 96 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRoot: %{_builddir}/%{name}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Requires: unzip"  
 IF 97 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Requires: unzip

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: freeglut-devel"  
 IF 98 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: freeglut-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Source0: lpub3d-ci-git.tar.gz"  
 IF 99 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Source0: lpub3d-ci-git.tar.gz

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=Source10: lpub3d-ci-rpmlintrc"  
 IF 100 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
Source10: lpub3d-ci-rpmlintrc

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 101 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# package requirements"  
 IF 102 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# package requirements

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}"  
 IF 103 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: qt5-qtbase-devel"  
 IF 104 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: qt5-qtbase-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora}"  
 IF 105 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: qt5-linguist"  
 IF 106 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: qt5-linguist

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 107 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?buildservice}!=1"  
 IF 108 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?buildservice}!=1

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: git"  
 IF 109 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: git

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 110 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: gcc-c++, libpng-devel, make"  
 IF 111 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: gcc-c++, libpng-devel, make

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 112 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 113 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora} || 0%{?centos_version} || 0%{?scientificlinux_version}"  
 IF 114 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora} || 0%{?centos_version} || 0%{?scientificlinux_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: mesa-libOSMesa-devel"  
 IF 115 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: mesa-libOSMesa-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 116 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 117 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?mageia}"  
 IF 118 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?mageia}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: qtbase5-devel"  
 IF 119 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: qtbase5-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%ifarch x86_64"  
 IF 120 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%ifarch x86_64

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: lib64osmesa-devel"  
 IF 121 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: lib64osmesa-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?buildservice}"  
 IF 122 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?buildservice}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: lib64sane1, lib64proxy-webkit"  
 IF 123 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: lib64sane1, lib64proxy-webkit

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 124 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%else"  
 IF 125 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%else

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: libosmesa-devel"  
 IF 126 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: libosmesa-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?buildservice}"  
 IF 127 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?buildservice}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: libsane1, libproxy-webkit"  
 IF 128 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: libsane1, libproxy-webkit

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 129 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 130 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 131 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 132 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora}"  
 IF 133 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora_version}==22"  
 IF 134 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora_version}==22

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: qca"  
 IF 135 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: qca

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 136 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora_version}==23"  
 IF 137 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora_version}==23

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: qca, gnu-free-sans-fonts"  
 IF 138 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: qca, gnu-free-sans-fonts

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 139 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 140 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 141 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?buildservice}"  
 IF 142 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?buildservice}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora_version}==25"  
 IF 143 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora_version}==25

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: llvm-libs"  
 IF 144 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: llvm-libs

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 145 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 146 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 147 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?rhel_version} || 0%{?centos_version}"  
 IF 148 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?rhel_version} || 0%{?centos_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: libXext-devel"  
 IF 149 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: libXext-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 150 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 151 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?suse_version}"  
 IF 152 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?suse_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: libqt5-qtbase-devel, zlib-devel"  
 IF 153 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: libqt5-qtbase-devel, zlib-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?buildservice}"  
 IF 154 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?buildservice}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: -post-build-checks"  
 IF 155 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: -post-build-checks

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 156 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 157 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 158 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?suse_version} > 1300"  
 IF 159 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?suse_version} > 1300

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=BuildRequires: Mesa-devel"  
 IF 160 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
BuildRequires: Mesa-devel

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 161 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 162 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%description"  
 IF 163 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%description

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= LPub3D is an Open Source WYSIWYG editing application for creating"  
 IF 164 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 LPub3D is an Open Source WYSIWYG editing application for creating

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= LEGO® style digital building instructions. LPub3D is developed and"  
 IF 165 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 LEGO® style digital building instructions. LPub3D is developed and

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= maintained by Trevor SANDY. It uses the LDraw™ parts library, the"  
 IF 166 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 maintained by Trevor SANDY. It uses the LDraw™ parts library, the

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= most comprehensive library of digital Open Source LEGO® bricks"  
 IF 167 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 most comprehensive library of digital Open Source LEGO® bricks

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= available (www.ldraw.org/ ) and reads the LDraw LDR and MPD model"  
 IF 168 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 available (www.ldraw.org/ ) and reads the LDraw LDR and MPD model

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= file formats. LPub3D is available for free under the GNU Public License v3"  
 IF 169 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 file formats. LPub3D is available for free under the GNU Public License v3

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= and runs on Windows, Linux and OSX Operating Systems."  
 IF 170 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 and runs on Windows, Linux and OSX Operating Systems.

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague,"  
 IF 171 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague,

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= LeoCAD© 2015 Leonardo Zide.and additional third party components."  
 IF 172 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 LeoCAD© 2015 Leonardo Zide.and additional third party components.

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= LEGO® is a trademark of the LEGO Group of companies which does not"  
 IF 173 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 LEGO® is a trademark of the LEGO Group of companies which does not

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= sponsor, authorize or endorse this application."  
 IF 174 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 sponsor, authorize or endorse this application.

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line= © 2015-2017 Trevor SANDY"  
 IF 175 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
 © 2015-2017 Trevor SANDY

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 176 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%prep"  
 IF 177 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%prep

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=set +x"  
 IF 178 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
set +x

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Target...................%{_target}""  
 IF 179 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Target...................%{_target}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Target Vendor............%{_target_vendor}""  
 IF 180 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Target Vendor............%{_target_vendor}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Target CPU...............%{_target_cpu}""  
 IF 181 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Target CPU...............%{_target_cpu}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Name.....................%{name}""  
 IF 182 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Name.....................%{name}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Summary..................%{summary}""  
 IF 183 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Summary..................%{summary}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Version..................%{version}""  
 IF 184 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Version..................%{version}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Vendor...................%{vendor}""  
 IF 185 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Vendor...................%{vendor}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Release..................%{release}""  
 IF 186 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Release..................%{release}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Distribution packer......%{distpacker}""  
 IF 187 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Distribution packer......%{distpacker}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Source0..................%{SOURCE0}""  
 IF 188 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Source0..................%{SOURCE0}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Source20.................%{SOURCE10}""  
 IF 189 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Source20.................%{SOURCE10}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Service Provider.........%{serviceprovider}""  
 IF 190 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Service Provider.........%{serviceprovider}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Packing Platform.........%{packingplatform}""  
 IF 191 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Packing Platform.........%{packingplatform}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "OpenBuildService Flag....%{buildservice}""  
 IF 192 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "OpenBuildService Flag....%{buildservice}"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=echo "Build Package............%{name}-%{version}-%{release}-%{_arch}.rpm""  
 IF 193 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
echo "Build Package............%{name}-%{version}-%{release}-%{_arch}.rpm"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=set -x"  
 IF 194 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
set -x

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%setup -q -n %{name}-git"  
 IF 195 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%setup -q -n %{name}-git

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 196 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%build"  
 IF 197 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%build

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=export QT_SELECT=qt5"  
 IF 198 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
export QT_SELECT=qt5

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# for 3rd party apps install"  
 IF 199 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# for 3rd party apps install

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=export LP3D_CREATE_PKG=yes"  
 IF 200 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
export LP3D_CREATE_PKG=yes

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# download ldraw archive libraries"  
 IF 201 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# download ldraw archive libraries

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=set +x"  
 IF 202 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
set +x

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=LDrawLibOffical=../../SOURCES/complete.zip"  
 IF 203 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
LDrawLibOffical=../../SOURCES/complete.zip

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=LDrawLibUnofficial=../../SOURCES/lpub3dldrawunf.zip"  
 IF 204 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
LDrawLibUnofficial=../../SOURCES/lpub3dldrawunf.zip

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=ThirdPartyRepoTarball=../../SOURCES/lpub3d_linux_3rdparty.tar.gz"  
 IF 205 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
ThirdPartyRepoTarball=../../SOURCES/lpub3d_linux_3rdparty.tar.gz

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=ThirdPartyRepo=lpub3d_linux_3rdparty"  
 IF 206 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
ThirdPartyRepo=lpub3d_linux_3rdparty

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=if [ -f ${LDrawLibOffical} ] ; then"  
 IF 207 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
if [ -f ${LDrawLibOffical} ] ; then

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  cp ${LDrawLibOffical} mainApp/extras && echo "complete.zip copied""  
 IF 208 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  cp ${LDrawLibOffical} mainApp/extras && echo "complete.zip copied"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=else"  
 IF 209 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
else

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  echo "complete.zip not found at $PWD!""  
 IF 210 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  echo "complete.zip not found at $PWD!"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=fi"  
 IF 211 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
fi

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=if [ -f ${LDrawLibUnofficial} ] ; then"  
 IF 212 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
if [ -f ${LDrawLibUnofficial} ] ; then

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  cp ${LDrawLibUnofficial} mainApp/extras && echo "lpub3dldrawunf.zip copied""  
 IF 213 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  cp ${LDrawLibUnofficial} mainApp/extras && echo "lpub3dldrawunf.zip copied"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=else"  
 IF 214 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
else

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  echo "lpub3dldrawunf.zip not found at $PWD!""  
 IF 215 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  echo "lpub3dldrawunf.zip not found at $PWD!"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=fi"  
 IF 216 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
fi

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# download lpub3d_linux_3rdparty repository as tar.gz archive"  
 IF 217 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# download lpub3d_linux_3rdparty repository as tar.gz archive

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=if [ -f ${ThirdPartyRepoTarball} ] ; then"  
 IF 218 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
if [ -f ${ThirdPartyRepoTarball} ] ; then

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  mkdir -p ../${ThirdPartyRepo} && tar -xzf ${ThirdPartyRepoTarball} -C ../${ThirdPartyRepo} --strip-components=1"  
 IF 219 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  mkdir -p ../${ThirdPartyRepo} && tar -xzf ${ThirdPartyRepoTarball} -C ../${ThirdPartyRepo} --strip-components=1

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  echo "${ThirdPartyRepo}.tar.gz tarball extracted to ../${ThirdPartyRepo}/""  
 IF 220 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  echo "${ThirdPartyRepo}.tar.gz tarball extracted to ../${ThirdPartyRepo}/"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  rm -f ../${ThirdPartyRepo}.tar.gz && echo "${ThirdPartyRepo}.tar.gz tarball deleted""  
 IF 221 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  rm -f ../${ThirdPartyRepo}.tar.gz && echo "${ThirdPartyRepo}.tar.gz tarball deleted"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=else"  
 IF 222 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
else

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  echo "${ThirdPartyRepo} tarball not found at $PWD!""  
 IF 223 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  echo "${ThirdPartyRepo} tarball not found at $PWD!"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=fi"  
 IF 224 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
fi

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=set -x"  
 IF 225 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
set -x

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=# use Qt5"  
 IF 226 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
# use Qt5

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?fedora}==23"  
 IF 227 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?fedora}==23

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%ifarch x86_64"  
 IF 228 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%ifarch x86_64

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=export Q_CXXFLAGS="$Q_CXXFLAGS -fPIC""  
 IF 229 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
export Q_CXXFLAGS="$Q_CXXFLAGS -fPIC"

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 230 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 231 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=if which qmake-qt5 >/dev/null 2>/dev/null ; then"  
 IF 232 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
if which qmake-qt5 >/dev/null 2>/dev/null ; then

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  qmake-qt5 -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d"  
 IF 233 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  qmake-qt5 -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=else"  
 IF 234 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
else

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=  qmake -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d"  
 IF 235 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
  qmake -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=fi"  
 IF 236 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
fi

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=make clean"  
 IF 237 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
make clean

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=make %{?_smp_mflags}"  
 IF 238 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
make %{?_smp_mflags}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 239 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%install"  
 IF 240 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%install

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=make INSTALL_ROOT=%buildroot install"  
 IF 241 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
make INSTALL_ROOT=%buildroot install

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?suse_version} || 0%{?sles_version}"  
 IF 242 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?suse_version} || 0%{?sles_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%fdupes %{buildroot}/%{_iconsdir}"  
 IF 243 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%fdupes %{buildroot}/%{_iconsdir}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 244 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=export NO_BRP_CHECK_RPATH=true"  
 IF 245 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
export NO_BRP_CHECK_RPATH=true

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 246 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%clean"  
 IF 247 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%clean

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=rm -rf $RPM_BUILD_ROOT"  
 IF 248 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
rm -rf $RPM_BUILD_ROOT

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 249 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%files"  
 IF 250 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%files

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?sles_version} || 0%{?suse_version}"  
 IF 251 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?sles_version} || 0%{?suse_version}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%defattr(-,root,root)"  
 IF 252 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%defattr(-,root,root)

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 253 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%{_bindir}/*"  
 IF 254 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%{_bindir}/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%{_libdir}/*"  
 IF 255 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%{_libdir}/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%{_datadir}/pixmaps/*"  
 IF 256 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%{_datadir}/pixmaps/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%{_datadir}/mime/packages/*"  
 IF 257 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%{_datadir}/mime/packages/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%{_datadir}/applications/*"  
 IF 258 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%{_datadir}/applications/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%{_datadir}/lpub3d"  
 IF 259 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%{_datadir}/lpub3d

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%{_3rdexedir}/*"  
 IF 260 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%{_3rdexedir}/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%dir %{_iconsdir}/hicolor/"  
 IF 261 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%dir %{_iconsdir}/hicolor/

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%dir %{_iconsdir}/hicolor/scalable/"  
 IF 262 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%dir %{_iconsdir}/hicolor/scalable/

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%dir %{_iconsdir}/hicolor/scalable/mimetypes/"  
 IF 263 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%dir %{_iconsdir}/hicolor/scalable/mimetypes/

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%attr(644,-,-) %{_iconsdir}/hicolor/scalable/mimetypes/*"  
 IF 264 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%attr(644,-,-) %{_iconsdir}/hicolor/scalable/mimetypes/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%attr(644,-,-) %doc %{_docdir}/lpub3d"  
 IF 265 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%attr(644,-,-) %doc %{_docdir}/lpub3d

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%attr(644,-,-) %{_mandir}/man1/*"  
 IF 266 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%attr(644,-,-) %{_mandir}/man1/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%attr(755,-,-) %{_3rdexedir}/*"  
 IF 267 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%attr(755,-,-) %{_3rdexedir}/*

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%attr(755,-,-) %{_datadir}/lpub3d/3rdParty/ldglite-1.3/resources/set-ldrawdir.command"  
 IF 268 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%attr(755,-,-) %{_datadir}/lpub3d/3rdParty/ldglite-1.3/resources/set-ldrawdir.command

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%post -p /sbin/ldconfig"  
 IF 269 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%post -p /sbin/ldconfig

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?buildservice}"  
 IF 270 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?buildservice}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=update-mime-database  /usr/share/mime >/dev/null || true"  
 IF 271 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
update-mime-database  /usr/share/mime >/dev/null || true

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=update-desktop-database || true"  
 IF 272 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
update-desktop-database || true

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 273 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 274 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%postun -p /sbin/ldconfig"  
 IF 275 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%postun -p /sbin/ldconfig

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%if 0%{?buildservice}"  
 IF 276 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%if 0%{?buildservice}

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=update-mime-database /usr/share/mime >/dev/null || true"  
 IF 277 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
update-mime-database /usr/share/mime >/dev/null || true

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=update-desktop-database || true"  
 IF 278 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
update-desktop-database || true

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=%endif"  
 IF 279 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
%endif

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line="  
 IF 280 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 


C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=* Mon Oct 30 2017 - trevor.dot.sandy.at.gmail.dot.com 2.0.21.175"  
 IF 281 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
* Mon Oct 30 2017 - trevor.dot.sandy.at.gmail.dot.com 2.0.21.175

C:\Users\Trevor\Projects\lpub3d-ci\builds>(
SET "Line=- LPub3D Linux package (rpm) release"  
 IF 282 EQU 1 SET "Line=LPub3D 2.0.21.107.174 (30 10 2017 04:15:48)"  
 SETLOCAL ENABLEDELAYEDEXPANSION  
 ECHO(!Line!  
 ENDLOCAL
) 
- LPub3D Linux package (rpm) release
