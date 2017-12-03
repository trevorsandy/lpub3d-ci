#!/bin/bash
#
# Build all LPub3D 3rd-party renderers
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: December 03, 2017
#  Copyright (c) 2017 by Trevor SANDY
#

# sample commands [call from root build directory - e.g. lpub3d]:
# -export WD=$PWD; export DOCKER=true; chmod +x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# -export OBS=false; source ${SOURCE_DIR}/builds/utilities/CreateRenderers.sh

# NOTE: OBS flag is 'ON' by default, be sure to set this flag in your calling command accordingly
# NOTE: elevated access required for dnf builddeps, execute with sudo if running noninteractive

# Capture elapsed time - reset BASH time counter
SECONDS=0

# Grab te script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# Grab the calling dir
CallDir=$PWD

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# Set sourced flag
if [ "${ME}" = "CreateRenderers.sh" ]; then
  SOURCED="false"
else
  SOURCED="true"
fi

# Get platform
OS_NAME=`uname`
if [ "$OS_NAME" = "Darwin" ]; then
  platform=$(echo `sw_vers -productName` `sw_vers -productVersion`)
else
  platform=$(. /etc/os-release && if test "$PRETTY_NAME" != ""; then echo "$PRETTY_NAME"; else echo `uname`; fi)
fi

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
	mkdir -p $1 && chown user:user $1
	tar -mxzf $1.tar.gz -C $1 --strip-components=1
	if [ -d $1/$2 ]; then
	  Info "Archive $1.tar.gz successfully extracted."
	  rm -rf $1.tar.gz && Info "Cleanup archive $1.tar.gz."
	  cd $1 && Info ""
	else
	  Info "ERROR - $1.tar.gz did not extract properly."
	fi
}

# args $1 = <build folder>
InstallDependencies() {
  Info "Installing library dependencies for $1..."
  platform_=$(. /etc/os-release && echo $ID)
  # if [ "$OBS" = "true" ]; then
  #   useSudo=""
  #   Info "Using sudo..........[No]"
  # else
  useSudo="sudo"
  Info "Using sudo..........[Yes]"
  # fi
  if [ "$OS_NAME" = "Linux" ]; then
    Info "Platform............[${platform_}]"
    case ${platform_} in
    *fedora*)
      # Initialize install mesa
      updateMesa=0
      mesaUpdated=0
      case $1 in
      ldglite)
        specFile="$PWD/obs/ldglite.spec"
        updateMesa=1
        ;;
      ldview)
        cp -f QT/LDView.spec QT/ldview-qt5.spec
        sed 's/define qt5 0/define qt5 1/' -i QT/ldview-qt5.spec
        specFile="$PWD/QT/ldview-qt5.spec"
        updateMesa=1
        ;;
      povray)
        specFile="$PWD/unix/obs/povray.spec"
       ;;
     esac;
      Info "Spec File...........[${specFile}]"
      if [[ ${updateMesa} = 1 && ! ${mesaUpdated} = 1 ]]; then
        Info "Update OSMesa.......[Yes]"
        Info ""
        $useSudo dnf builddep -y mesa
        $useSudo dnf builddep -y "$CallDir/builds/utilities/mesa/glu.spec"
        "$CallDir/builds/utilities/mesa/buildosmesa.sh"
        mesaUpdated=1
      fi
      Info ""
      $useSudo dnf builddep -y $specFile
      ;;
    *arch*)
      case $1 in
      ldglite)
        pkgbuildFile="$PWD/obs/PKGBUILD"
        ;;
      ldview)
        pkgbuildFile="$PWD/QT/OBS/PKGBUILD"
        ;;
      povray)
        pkgbuildFile="$PWD/unix/obs/PKGBUILD"
        ;;
      esac;
      pkgbuildDeps="$(echo `grep -wr 'depends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'` \
                           `grep -wr 'makedepends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'`)"
      Info "PKGBUILD File.......[${pkgbuildFile}]"
      Info "Dependencies List...[${pkgbuildDeps}]"
      Info ""
      $useSudo pacman -Syy --noconfirm
      $useSudo pacman -Syu --noconfirm
      $useSudo pacman -S --noconfirm --needed $pkgbuildDeps
      ;;
    *ubuntu*)
      case $1 in
      ldglite)
        controlFile="$PWD/obs/debian/control"
        extraFiles=""
        ;;
      ldview)
        controlFile="$PWD/QT/debian/control"
        extraFiles="libtinyxml-dev libgl2ps-dev"
        ;;
      povray)
        controlFile="$PWD/unix/obs/debian/control"
        extraFiles=""
        ;;
      esac;
      controlDeps=`grep Build-Depends $controlFile | cut -d: -f2| sed 's/(.*)//g' | tr -d ,`
      Info "Control File........[${controlFile}]"
      Info "Dependencies List...[${controlDeps} $extraFiles]"
      Info ""
      $useSudo apt-get install -y $controlDeps $extraFiles
      ;;
    esac;
  elif [ "$OS_NAME" = "Darwin" ]; then
    # brew bottles here...
    Info "brew bottles..."
    brew install qt5 openexr sdl2 tinyxml gl2ps libjpeg minizip
    brew link --force qt5
  fi
}

buildLdglite() {
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="CONFIG+=debug"
  else
    BUILD_CONFIG="CONFIG+=release"
  fi
  ${QMAKE_EXE} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG} CONFIG+=BUILD_CHECK
  make
  make install > /dev/tty
}

buildLdview() {
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="CONFIG+=debug"
  else
    BUILD_CONFIG="CONFIG+=release"
  fi
  ${QMAKE_EXE} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG} CONFIG+=BUILD_CUI_ONLY CONFIG+=USE_SYSTEM_LIBS CONFIG+=BUILD_CHECK
  make
  make install > /dev/tty
}

buildPovray() {
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="--enable-debug"
  else
    BUILD_CONFIG=""
  fi
  cd unix && chmod +x prebuild3rdparty.sh && ./prebuild3rdparty.sh
  cd ../
  ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." --prefix="${DIST_PKG_DIR}" LPUB3D_3RD_PARTY="yes" --with-libsdl2 --enable-watch-cursor ${BUILD_CONFIG}
  make check
  make install > /dev/tty
}

# Logging
Info ""
if [ "${SOURCED}" = "true" ]; then
  Info "Start CreateRenderers execution at $PWD..."
else
  Info "Start $ME execution at $PWD..."
  if [ ! "$OBS" = "true" ]; then
    # logging stuff - increment log file name
    f="$PWD/$ME"
    ext=".log"
    if [[ -e "$f$ext" ]] ; then
      i=1
      f="${f%.*}";
      while [[ -e "${f}_${i}${ext}" ]]; do
        let i++
      done
      f="${f}_${i}${ext}"
    else
      f="${f}${ext}"
    fi
    # output log file
    LOG="$f"
    # exec > >(tee -a ${LOG} )
    # exec 2> >(tee -a ${LOG} >&2)
  fi
fi

Info ""
Info "Building............[LPub3D 3rd Party Renderers]"

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
  parent_dir=${PWD##*/}
  if  [ "$parent_dir" = "utilities" ]; then
    chkdir="$(readlink - ../../../)"
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
  Info "WARNING - OBS environment variable was not specified. Setting to true."
fi

# Platform ID
if [ "${DOCKER}" = "true" ]; then
	Info "Platform............[Docker - ${platform}]"
elif [ "${TRAVIS}" = "true" ]; then
  Info "Platform............[Travis CI - ${platform}]"
elif [ "${OBS}" = "true" ]; then
  Info "Platform............[Open Build System - ${platform}]"
else
	Info "Platform............[${platform}]"
fi
Info "Working directory...[$WD]"

# Distribution directory
DIST_DIR=lpub3d_linux_3rdparty
DIST_PKG_DIR=${WD}/${DIST_DIR}
if [ ! -d ${DIST_PKG_DIR} ]; then
  mkdir -p ${DIST_PKG_DIR} && Info "Dist Directory......[${DIST_PKG_DIR}]"
fi

# Change to Working directory
cd ${WD}

# LDRaw Library - for testing LDView and LDGLite
export LDRAWDIR=${HOME}/ldraw
if [ ! -d ${LDRAWDIR}/parts ]; then
  if [ "$OS_NAME" = "Darwin" ]; then
	  LDRAWDIR_ROOT=${HOME}/Library
  else
	  LDRAWDIR_ROOT=${HOME}
  fi
  Info "LDraw library not found at ${LDRAWDIR}. Checking for library archive..."
  if [ ! -f complete.zip ]; then
	  Info "LDraw library archive complete.zip not found. Downloading library..."
	  curl $curlopts -O http://www.ldraw.org/library/updates/complete.zip
  fi
  Info "Extracting LDraw library into ${LDRAWDIR}..."
  unzip -d ${LDRAWDIR_ROOT} -q complete.zip;
  if [ -d ${LDRAWDIR} ]; then
	  Info "LDraw library extracted. LDRAWDIR defined." && if [ ! "$OS_NAME" = "Darwin" ]; then Info ""; fi
  fi
fi
if [ "$OS_NAME" = "Darwin" ]; then
  Info "set LDRAWDIR in environment.plist..."
  chmod +x builds/utilities/set-ldrawdir.command && ./builds/utilities/set-ldrawdir.command
  grep -A1 -e 'LDRAWDIR' ~/.MacOSX/environment.plist
  Info "LDRAWDIR......${LDRAWDIR}"
  Info "set LDRAWDIR Completed." && Info ""
fi

# Qt setup
export QT_SELECT=qt5
if [ -x /usr/bin/qmake ] ; then
  QMAKE_EXE=qmake
elif [ -x /usr/bin/qmake-qt5 ] ; then
  QMAKE_EXE=qmake-qt5
fi
Info ""
${QMAKE_EXE} -v
QMAKE_EXE="${QMAKE_EXE} -makefile"

# Main loop
for buildDir in ldglite ldview povray; do
  case ${buildDir} in
  ldglite)
  	curlCommand="https://github.com/trevorsandy/ldglite/archive/master.tar.gz"
  	buildCommand="buildLdglite"
  	validSubDir="mui"
    buildConfig="release"
	  ;;
  ldview)
  	curlCommand="https://github.com/trevorsandy/ldview/archive/qmake-build.tar.gz"
  	buildCommand="buildLdview"
  	validSubDir="OSMesa"
    buildConfig="release"
	  ;;
  povray)
  	curlCommand="https://github.com/trevorsandy/povray/archive/lpub3d/raytracer-cui.tar.gz"
  	buildCommand="buildPovray"
  	validSubDir="unix"
    buildConfig="release"
	  ;;
  esac
  if [ "$OBS" = "true" ]; then
    if [ -f ${buildDir}.tar.gz ]; then
      ExtractArchive ${buildDir} ${validSubDir}
    else
      Info "ERROR - Unable to find ${buildDir}.tar.gz at $PWD"
    fi
  elif [ ! -d ${buildDir}/${validSubDir} ]; then
    Info ""
    Info "`echo ${buildDir} | awk '{print toupper($0)}'` folder does not exist. Checking for tarball archive..."
    if [ ! -f ${buildDir}.tar.gz ]; then
      Info "`echo ${buildDir} | awk '{print toupper($0)}'` tarball ${buildDir}.tar.gz does not exist. Downloading..."
      curl $curlopts ${buildDir}.tar.gz -o ${curlCommand}
    fi
    ExtractArchive ${buildDir} ${validSubDir}
  else
    cd ${buildDir}
  fi
  Info "Install ${buildDir} build dependencies..."
  InstallDependencies ${buildDir} >> ${WD}/${ME}_${buildDir}.log 2>&1
  Info "${buildDir} dependencies installed - see ${WD}/${ME}_${buildDir}.log for details"
  Info "" && Info "----------------------------------------------------"
  Info "Build ${buildDir}..."
  ${buildCommand} ${buildConfig} >> ${WD}/${ME}_${buildDir}.log 2>&1
  Info "${buildDir} build completed - see ${WD}/${ME}_${buildDir}.log for details"
  cd ${WD}
done

# Elapsed execution time
ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
Info ""
Info "$ME Finished!"
Info "$ELAPSED"
