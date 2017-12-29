#!/bin/bash
#
# Build all LPub3D 3rd-party renderers
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: December 29, 2017
#  Copyright (c) 2017 by Trevor SANDY
#

# sample commands [call from root build directory - e.g. lpub3d]:
# -export WD=$PWD; export DOCKER=true; chmod +x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# -export OBS=false; source ${SOURCE_DIR}/builds/utilities/CreateRenderers.sh

# NOTE: OBS flag is 'ON' by default, be sure to set this flag in your calling command accordingly
# NOTE: elevated access required for dnf builddeps, execute with sudo if running noninteractive

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

# Functions
Info () {
    if [ "${SOURCED}" = "true" ]
    then
        echo "   renderers: ${*}" >&2
    else
        echo "-${*}" >&2
    fi
}

ExtractArchive() {
  # args: $1 = <build folder>, $2 = <valid subfolder>
  Info "Extracting $1.tar.gz..."
  mkdir -p $1 && tar -mxzf $1.tar.gz -C $1 --strip-components=1
  if [ -d $1/$2 ]; then
    Info "Archive $1.tar.gz successfully extracted."
    rm -rf $1.tar.gz && Info "Cleanup archive $1.tar.gz."
    cd $1
  else
    Info "ERROR - $1.tar.gz did not extract properly."
  fi
}

BuildMesaLibs() {
  mesaSpecDir="$CallDir/builds/utilities/mesa"
  mesaBuildDeps="TBD"
  if [ ! "${OBS}" = "true" ]; then
    mesaDepsLog=${LOG_PATH}/${ME}_${host}_mesadeps_${1}.log
    mesaBuildLog=${LOG_PATH}/${ME}_${host}_mesabuild_${1}.log
  fi
  if [ -z "$2" ]; then
    useSudo=
  else
    useSudo=$2
  fi

  Info "Update OSMesa.......[Yes]"
  if [ ! "${OBS}" = "true" ]; then
    Info "OSMesa Dependencies.[${mesaBuildDeps}]"
    Info
    $useSudo dnf builddep -y mesa > $mesaDepsLog 2>&1
    Info "${1} Mesa dependencies installed." && DisplayLogTail $mesaDepsLog 10
    $useSudo dnf builddep -y "${mesaSpecDir}/glu.spec" >> $mesaDepsLog 2>&1
    Info "${1} GLU dependencies installed." && DisplayLogTail $mesaDepsLog 5
  fi
  Info && Info "Build OSMesa and GLU static libraries..."
  chmod +x "${mesaSpecDir}/build_osmesa.sh"
  if [ "${OBS}" = "true" ]; then
    Info "Using sudo..........[No]"
    "${mesaSpecDir}/build_osmesa.sh" &
  else
    "${mesaSpecDir}/build_osmesa.sh" > $mesaBuildLog 2>&1 &
  fi
  TreatLongProcess $! 60 "OSMesa and GLU build"

  if [[ -f "$WD/${DIST_DIR}/mesa/lib/libOSMesa32.a" && \
        -f "$WD/${DIST_DIR}/mesa/lib/libGLU.a" ]]; then
    if [ ! "${OBS}" = "true" ]; then
      Info &&  Info "OSMesa and GLU build check..."
      DisplayCheckStatus "$mesaBuildLog" "Libraries have been installed in:" "1" "16"
      DisplayLogTail $mesaBuildLog 10
    fi
    OSMesaBuilt=1
  else
    if [ ! -f "$WD/${DIST_DIR}/mesa/lib/libOSMesa32.a" ]; then
      Info && Info "ERROR - libOSMesa32 not found. Binary was not successfully built"
    fi
    if [ ! -f "$WD/${DIST_DIR}/mesa/lib/libGLU.a" ]; then
      Info && Info "ERROR - libGLU not found. Binary was not successfully built"
    fi
    if [ ! "${OBS}" = "true" ]; then
      Info "------------------Build Log-------------------------"
      cat $mesaBuildLog
    fi
  fi
  Info && Info "${1} library OSMesa build finished."
}

# args: $1 = <log file>, $2 = <position>
DisplayLogTail() {
  if [[ -f "$1" && -s "$1" ]]; then
    logFile="$1"
    if [ "$2" = "" ]; then
      # default to 5 lines from the bottom of the file if not specified
      startPosition=-5
    elif [[ "${2:0:1}" != "-" || "${2:0:1}" != "+" ]]; then
      # default to the bottom of the file if not specified
      startPosition=-$2
    else
      startPosition=$2
    fi
    Info "Log file tail..."
    Info "$1 last $2 lines:"
    tail $startPositin $logFile
  else
    Info "ERROR (log tail) - $1 not found or not valid!"
  fi
}

# args: $1 = <log file>, $2 = <search String>, $3 = <lines Before>, $4 = <lines After>
DisplayCheckStatus() {
  declare -i i; i=0
  for arg in "$@"; do
    i=i+1
    if test $i -eq 1; then s_buildLog="$arg"; fi
    if test $i -eq 2; then s_checkString="$arg"; fi
    if test $i -eq 3; then s_linesBefore="$arg"; fi
    if test $i -eq 4; then s_linesAfter="$arg"; fi
  done
  if [[ -f "$s_buildLog" && -s "$s_buildLog" ]]; then
    if test -z "$s_checkString"; then Info "ERROR - check string not specified."; return 1; fi
    if test -z "$s_linesBefore"; then s_linesBefore=2; Info "INFO - display 2 lines before"; fi
    if test -z "$s_linesAfter"; then s_linesAfter=10; Info "INFO - display 10 lines after"; fi
    Info "Checking for ${s_checkString} in ${s_buildLog}..."
    grep -B${s_linesBefore} -A${s_linesAfter} "${s_checkString}" $s_buildLog
  else
    Info "ERROR - Check display [$s_buildLog] not found or is not valid!"
  fi
}

# args: 1 = <start> (seconds mark)
ElapsedTime() {
  if test -z "$1"; then return 0; fi
  TIME_ELAPSED="$(((SECONDS - $1) % 60))sec"
  TIME_MINUTES=$((((SECONDS - $1) / 60) % 60))
  TIME_HOURS=$(((SECONDS - $1) / 3600))
  if [ "$TIME_MINUTES" -gt 0 ]; then
    TIME_ELAPSED="${TIME_MINUTES}mins $TIME_ELAPSED"
  fi
  if [ "$TIME_HOURS" -gt 0 ]; then
    TIME_ELAPSED="${TIME_HOURS}hrs $TIME_ELAPSED"
  fi
  echo "$TIME_ELAPSED"
}

# args: 1 = <pid>, 2 = <message interval>, [3 = <pretty label>]
TreatLongProcess() {
  declare -i i; i=0
  for arg in "$@"; do
    i=i+1
    if test $i -eq 1; then s_pid="$arg"; fi    # pid
    if test $i -eq 2; then s_msgint="$arg"; fi # message interval
    if test $i -eq 3; then s_plabel="$arg"; fi # pretty label
  done

  # initialize the duration counter
  s_start=$SECONDS

  # Validate the optional pretty label
  if test -z "$s_plabel"; then s_plabel="Create renderer"; fi

  # Spawn a process that coninually reports the command is running
  while Info "$(date): $s_plabel process $s_pid is running since `ElapsedTime $s_start`..."; \
  do sleep $s_msgint; done &
  s_nark=$!

  # Set a trap to kill the messenger when the process finishes
  trap 'kill $s_nark 2>/dev/null && wait $s_nark 2>/dev/null' RETURN

  # Wait for the process to finish and display exit code
  if wait $s_pid; then
    Info "$(date): $s_plabel process finished (returned $?)"
  else
    Info "$(date): $s_plabel process FAILED! (returned $?)"
  fi
}

# args: 1 = <build folder>
InstallDependencies() {
  if [ "$OS_NAME" = "Linux" ]; then
    Info &&  Info "Install $1 build dependencies..."
    platform_=$(. /etc/os-release && echo $ID)
    useSudo="sudo"
    Info "Using sudo..........[Yes]"
    case ${platform_} in
    fedora|redhat|suse|mageia|arch|ubuntu|debian)
      true
      ;;
    *)
      Info "ERROR - Unable to process this target platform: [$platform_]."
      ;;
    esac
    depsLog=${LOG_PATH}/${ME}_${host}_deps_${1}.log
    Info "Platform............[${platform_}]"
    case ${platform_} in
    fedora|redhat|suse|mageia)
      # Initialize install mesa
      case $1 in
      ldglite)
        specFile="$PWD/obs/ldglite.spec"
        build_osmesa=1
        ;;
      ldview)
        cp -f QT/LDView.spec QT/ldview-lp3d-qt5.spec
        specFile="$PWD/QT/ldview-lp3d-qt5.spec"
        sed -e 's/define qt5 0/define qt5 1/g' -e 's/kdebase-devel/make/g' -e 's/, kdelibs-devel//g' -i $specFile
        build_osmesa=1
        ;;
      povray)
        specFile="$PWD/unix/obs/povray.spec"
       ;;
      esac;
      rpmbuildDeps="TBD"
      Info "Spec File...........[${specFile}]"
      Info "Dependencies List...[${rpmbuildDeps}]"
      if [[ "${build_osmesa}" = 1 && ! "${OSMesaBuilt}" = 1 ]]; then
        BuildMesaLibs $1 $useSudo
      fi
      Info
      $useSudo dnf builddep -y $specFile > $depsLog 2>&1
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
      ;;
    arch)
      case $1 in
      ldglite)
        pkgbuildFile="$PWD/obs/PKGBUILD"
        ;;
      ldview)
        pkgbuildFile="$PWD/QT/OBS/PKGBUILD"
        sed "s/'kdelibs'/'tinyxml' 'gl2ps'/g" -i $pkgbuildFile
        if [ ! -d /usr/share/mime ]; then
          $useSudo mkdir /usr/share/mime
        fi
        ;;
      povray)
        pkgbuildFile="$PWD/unix/obs/PKGBUILD"
        ;;
      esac;
      pkgbuildDeps="$(echo `grep -wr 'depends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'` \
                           `grep -wr 'makedepends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'`)"
      Info "PKGBUILD File.......[${pkgbuildFile}]"
      Info "Dependencies List...[${pkgbuildDeps}]"
      Info
      $useSudo pacman -Syy --noconfirm --needed > $depsLog 2>&1
      $useSudo pacman -Syu --noconfirm --needed >> $depsLog 2>&1
      $useSudo pacman -S --noconfirm --needed $pkgbuildDeps >> $depsLog 2>&1
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
      ;;
    ubuntu|debian)
      case $1 in
      ldglite)
        controlFile="$PWD/obs/debian/control"
        sed '/^Build-Depends:/ s/$/ libosmesa6-dev/' -i $controlFile
        ;;
      ldview)
        controlFile="$PWD/QT/debian/control"
        sed -e '/#Qt4.x/d' -e '/libqt4-dev/d' -e 's/#Build-Depends/Build-Depends/g' -e 's/kdelibs5-dev//g' \
            -e '/^Build-Depends:/ s/$/ qt5-qmake libqt5opengl5-dev libosmesa6-dev libtinyxml-dev libgl2ps-dev/' -i $controlFile
        ;;
      povray)
        controlFile="$PWD/unix/obs/debian/control"
        ;;
      esac;
      controlDeps=`grep Build-Depends $controlFile | cut -d: -f2| sed 's/(.*)//g' | tr -d ,`
      Info "Control File........[${controlFile}]"
      Info "Dependencies List...[${controlDeps}]"
      Info
      $useSudo apt-get update -qq > $depsLog 2>&1
      $useSudo apt-get install -y $controlDeps >> $depsLog 2>&1
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
      ;;
      *)
      Info "ERROR - Unknown platform [$platform_]"
      ;;
    esac;
  else
    Info "ERROR - Platform is undefined or invalid [$OS_NAME] - Cannot continue."
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildLDGLite() {
  BUILD_CONFIG="CONFIG+=BUILD_CHECK CONFIG-=debug_and_release"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  if [ "${build_osmesa}" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  ${QMAKE_EXEC} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make
    make install
  else
    make > $2 2>&1
    make install >> $2 2>&1
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildLDView() {
  BUILD_CONFIG="CONFIG+=BUILD_CUI_ONLY CONFIG+=USE_SYSTEM_LIBS CONFIG+=BUILD_CHECK CONFIG-=debug_and_release"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  if [ "$build_osmesa" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  if [ "$build_tinyxml" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=OBS_TINYXML"
  fi
  if [ "$build_gl2ps" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=OBS_GL2PS"
  fi
  ${QMAKE_EXEC} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make
    make install
  else
    make > $2 2>&1 &
    TreatLongProcess "$!" "60" "LDView make"
    make install >> $2 2>&1
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildPOVRay() {
  BUILD_CONFIG="--prefix=${DIST_PKG_DIR} LPUB3D_3RD_PARTY=yes --enable-watch-cursor"
  if [ "$build_sdl2" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG --with-libsdl2=from-src"
  else
    BUILD_CONFIG="$BUILD_CONFIG --with-libsdl2"
  fi
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG --enable-debug"
  fi
  export POV_IGNORE_SYSCONF_MSG="yes"
  cd unix && chmod +x prebuild3rdparty.sh && ./prebuild3rdparty.sh && cd ../
  ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make
    make install
    make check
  else
    make > $2 2>&1 &
    TreatLongProcess "$!" "60" "POV-Ray make"
    make check >> $2 2>&1
    make install >> $2 2>&1
  fi
}

# Grab te script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# Start message and set sourced flag
if [ "${ME}" = "CreateRenderers.sh" ]; then
  SOURCED="false"
  Info "Start $ME execution at $PWD..."
else
  SOURCED="true"
  Info "Start CreateRenderers execution at $PWD..."
fi

# Grab the calling dir
CallDir=$PWD

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# Get platform
OS_NAME=$(uname)
if [ "$OS_NAME" = "Darwin" ]; then
  platform=$(echo `sw_vers -productName` `sw_vers -productVersion`)
else
  platform=$(. /etc/os-release && if test "$PRETTY_NAME" != ""; then echo "$PRETTY_NAME"; else echo `uname`; fi)
fi

Info && Info "Building............[LPub3D 3rd Party Renderers]"

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
  parent_dir=${PWD##*/}
  if  [ "$parent_dir" = "utilities" ]; then
    if [ "$OS_NAME" = "Darwin" ]; then
      chkdir="$PWD/../../../"
    else
      chkdir="$(realpath ../../../)"
    fi
    if [ -d "$chkdir" ]; then
      WD=$chkdir
    fi
  else
    WD=$PWD
  fi
  Info "WARNING - 'WD' environment varialbe not specified. Using $WD"
fi

# Initialize OBS if not in command line input
if [[ "${OBS}" = "" && "${DOCKER}" = "" &&  "${TRAVIS}" = "" ]]; then
  OBS=true
fi

# Platform ID
if [ "${DOCKER}" = "true" ]; then
  Info "Platform............[Docker - ${platform}]"
elif [ "${TRAVIS}" = "true" ]; then
  Info "Platform............[Travis CI - ${platform}]"
elif [ "${OBS}" = "true" ]; then
  if [ "$platform" = "" ]; then
    platform=${OS_NAME}
  fi
  Info "Platform............[Open Build System - ${platform}]"
else
  Info "Platform............[${platform}]"
fi
Info "Working directory...[$WD]"

# Distribution directory
if [ "$OS_NAME" = "Darwin" ]; then
  DIST_DIR=lpub3d_macos_3rdparty
else
  DIST_DIR=lpub3d_linux_3rdparty
fi
DIST_PKG_DIR=${WD}/${DIST_DIR}
if [ ! -d "${DIST_PKG_DIR}" ]; then
  mkdir -p ${DIST_PKG_DIR}
fi
Info "Dist Directory......[${DIST_PKG_DIR}]"

# get host name
if [ "${OBS}" = "true" ]; then
  host=obs
else
  if [ ! "$OS_NAME" = "Darwin" ]; then
    platform_=$(. /etc/os-release && echo $ID)
    if [ -n "$platform_" ]; then
      host=$platform_
    else
      host=undefined
    fi
  else
    host=macos
  fi
fi

# Change to Working directory
cd ${WD}

# Setup LDraw Library - for testing LDView and LDGLite
if [ "$OS_NAME" = "Darwin" ]; then
  LDRAWDIR_ROOT=${HOME}/Library
else
  LDRAWDIR_ROOT=${HOME}
fi
export LDRAWDIR=${LDRAWDIR_ROOT}/ldraw
if [ ! -d "${LDRAWDIR}/parts" ]; then
  Info && Info "LDraw library not found at ${LDRAWDIR}. Checking for complete.zip archive..."
  if [ ! -f complete.zip ]; then
    Info "Library archive complete.zip not found at $PWD. Downloading archive..."
    curl -s -O http://www.ldraw.org/library/updates/complete.zip;
  fi
  Info "Extracting LDraw library into ${LDRAWDIR}..."
  unzip -od ${LDRAWDIR_ROOT} -q complete.zip;
  if [ -d "${LDRAWDIR}/parts" ]; then
    Info "LDraw library extracted. LDRAWDIR defined."
  fi
elif [ ! "$OS_NAME" = "Darwin" ]; then
  Info "LDraw library.......[${LDRAWDIR}]"
fi
# Additional LDraw configuration for MacOS
if [ "$OS_NAME" = "Darwin" ]; then
  Info "LDraw library.......[${LDRAWDIR}]"
  Info && Info "set LDRAWDIR in environment.plist..."
  chmod +x ${LPUB3D}/builds/utilities/set-ldrawdir.command && ./${LPUB3D}/builds/utilities/set-ldrawdir.command
  grep -A1 -e 'LDRAWDIR' ~/.MacOSX/environment.plist
  Info "set LDRAWDIR Completed."

  # Qt setup - MacOS
  QMAKE_EXEC=qmake
else
  # Qt setup - Linux
  export QT_SELECT=qt5
  if [ -x /usr/bin/qmake ] ; then
    QMAKE_EXEC=qmake
  elif [ -x /usr/bin/qmake-qt5 ] ; then
    QMAKE_EXEC=qmake-qt5
  fi
fi

# get Qt version
Info && ${QMAKE_EXEC} -v
QMAKE_EXEC="${QMAKE_EXEC} -makefile"

# set log output path
LOG_PATH=${WD}

# initialize mesa build flag
OSMesaBuilt=0

# define build architecture and cached renderer paths
VER_LDGLITE=ldglite-1.3
VER_LDVIEW=ldview-4.3
VER_POVRAY=lpub3d_trace_cui-3.8
distArch=$(uname -m)
if [ "$distArch" = x86_64 ]; then
  buildArch="64bit_release";
  LP3D_LDGLITE=${DIST_PKG_DIR}/${VER_LDGLITE}/bin/${distArch}/ldglite
  LP3D_LDVIEW=${DIST_PKG_DIR}/${VER_LDVIEW}/bin/${distArch}/ldview
  LP3D_POVRAY=${DIST_PKG_DIR}/${VER_POVRAY}/bin/${distArch}/lpub3d_trace_cui
else
  buildArch="32bit_release";
  LP3D_LDGLITE=${DIST_PKG_DIR}/${VER_LDGLITE}/bin/i386/ldglite
  LP3D_LDVIEW=${DIST_PKG_DIR}/${VER_LDVIEW}/bin/i386/ldview
  LP3D_POVRAY=${DIST_PKG_DIR}/${VER_POVRAY}/bin/i386/lpub3d_trace_cui
fi

#echo && echo "================================================"
#echo "DEBUG - DISTRIBUTION FILES:" && find $DIST_PKG_DIR -type f;
#echo "================================================" && echo

# install build dependencies for MacOS
if [ "$OS_NAME" = "Darwin" ]; then
  Info &&  Info "Install $OS_NAME renderer build dependencies..."
  Info "Platform............[macos]"
  Info "Using sudo..........[No]"
  for buildDir in ldview povray; do
    artefactPath="LP3D_$(echo ${buildDir} | awk '{print toupper($0)}')"  
    if [ ! -f "${!artefactPath}" ]; then
      case ${buildDir} in
      ldview)
        brewDeps="tinyxml gl2ps libjpeg minizip"
        ;;
      povray)
        brewDeps="$brewDeps openexr sdl2 libtiff boost autoconf automake pkg-config"
        ;;
      esac
    fi
  done
  if [ -n $brewDeps ]; then
    Info "Dependencies List...[X11 ${brewDeps}]"
    Info "Checking for X11 (xquartz) at /usr/X11..."
    if [[ -d /usr/X11/lib && /usr/X11/include ]]; then
      Info "Good to go - X11 found."
    else
      Info "ERROR - Sorry to say friend, I cannot go on - X11 not found."
      if [ "${TRAVIS}" != "true" ]; then
        Info "  You can install xquartz using homebrew:"
        Info "  \$ brew cask list"
        Info "  \$ brew cask install xquartz"
        Info "  Note: elevated access password will be required."
      fi
      # Elapsed execution time
      FinishElapsedTime
      exit 1
    fi
    depsLog=${LOG_PATH}/${ME}_${host}_deps_$OS_NAME.log
    brew update > $depsLog 2>&1
    brew install $brewDeps >> $depsLog 2>&1
    Info "$OS_NAME dependencies installed." && DisplayLogTail $depsLog 10
  else
    Info "Renderer artefacts exist, nothing to build. Install dependencies skippes"
  fi
fi

# Main loop
for buildDir in ldglite ldview povray; do
  buildDirUpper="$(echo ${buildDir} | awk '{print toupper($0)}')"
  artefactVer="VER_${buildDirUpper}"
  artefactPath="LP3D_${buildDirUpper}"
  buildLog=${LOG_PATH}/${ME}_${host}_build_${buildDir}.log
  linesBefore=1
  case ${buildDir} in
  ldglite)
    curlCommand="https://github.com/trevorsandy/ldglite/archive/master.tar.gz"
    checkString="LDGLite Output"
    linesAfter="2"
    buildCommand="BuildLDGLite"
    validSubDir="app"
    validExe="${validSubDir}/${buildArch}/ldglite"
    buildType="release"
    ;;
  ldview)
    curlCommand="https://github.com/trevorsandy/ldview/archive/qmake-build.tar.gz"
    checkString="LDView Image Output"
    linesAfter="9"
    buildCommand="BuildLDView"
    validSubDir="OSMesa"
    validExe="${validSubDir}/${buildArch}/ldview"
    buildType="release"
    ;;
  povray)
    curlCommand="https://github.com/trevorsandy/povray/archive/lpub3d/raytracer-cui.tar.gz"
    checkString="Render Statistics"
    linesAfter="42"
    buildCommand="BuildPOVRay"
    validSubDir="unix"
    validExe="${validSubDir}/lpub3d_trace_cui"
    buildType="release"
    ;;
  esac

  # OBS build routine...
  if [ "$OBS" = "true" ]; then
    if [ -f "${buildDir}.tar.gz" ]; then
      ExtractArchive ${buildDir} ${validSubDir}
    else
      Info && Info "ERROR - Unable to find ${buildDir}.tar.gz at $PWD"
    fi
    platform_=$(. /etc/os-release && echo $ID)
    case ${platform_} in
    arch|ubuntu|debian)
      Info "Processing OBS platform_ ${platform_}..."
      if [ "$platform_" = "arch" ]; then
        build_tinyxml=1
      fi
      true
      ;;
    *)
      if [ "$TARGET_VENDOR" != "" ]; then
        platform_=$TARGET_VENDOR
      else
        if [ -z "$platform_" ]; then
          platform_=undefined
        fi
        Info "WARNING - Open Build Service did not provide a target platform."
      fi
      if [[ "${build_osmesa}" = 1 && ! "${OSMesaBuilt}" = 1 ]]; then
        Info "Processing OBS platform_ ${platform_}..."
        BuildMesaLibs
      fi
      ;;
    esac
  fi

  # CI/Local build routine...
  # Check if build folder exist and donwload if not
  if [ ! -d "${buildDir}/${validSubDir}" ]; then
    Info && Info "$(echo ${buildDir} | awk '{print toupper($0)}') build folder does not exist. Checking for tarball archive..."
    if [ ! -f ${buildDir}.tar.gz ]; then
      Info "$(echo ${buildDir} | awk '{print toupper($0)}') tarball ${buildDir}.tar.gz does not exist. Downloading..."
      curl $curlopts ${curlCommand} -o ${buildDir}.tar.gz
    fi
    ExtractArchive ${buildDir} ${validSubDir}
  else
    cd ${buildDir}
  fi
  Info && Info "Install ${!artefactVer} dependencies..."
  Info "----------------------------------------------------"
  # Install build dependencies
  if [[ ! "$OS_NAME" = "Darwin" && ! "$OBS" = "true" ]]; then
    InstallDependencies ${buildDir}
    sleep .5
  fi
  # Perform build
  Info && Info "Build ${!artefactVer}..."
  Info "----------------------------------------------------"
  if [ ! -f "${!artefactPath}" ]; then
    ${buildCommand} ${buildType} ${buildLog}
    if [ ! "${OBS}" = "true" ]; then
      if [ -f "${validExe}" ]; then
        Info && Info "Build check - ${buildDir}..."
        DisplayCheckStatus "${buildLog}" "${checkString}" "${linesBefore}" "${linesAfter}"
        Info
        DisplayLogTail ${buildLog} 10
      else
        Info && Info "ERROR - ${validExe} not found. Binary was not successfully built"
        Info "------------------Build Log-------------------------"
        cat ${buildLog}
      fi
    fi
    Info && Info "Build ${buildDir} finished."
  else
    Info "Renderer artefacts for ${!artefactVer} exists - build skipped."
  fi
  cd ${WD}
done

# Elapsed execution time
FinishElapsedTime
