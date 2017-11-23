#!/bin/bash
#
# Build all LPub3D 3rd-party renderers
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: November 18, 2017
#  Copyright (c) 2017 by Trevor SANDY
#

# sample commands [call from <path>]:
# -chmod +x builds/utilities/CreateRenderers.sh && env DOCKER=true OBS=false WD=$PWD ./builds/utilities/CreateRenderers.sh
# -env OBS=false source ${SOURCE_DIR}/builds/utilities/CreateRenderers.sh

# IMPORTANT: OBS flag is 'ON' by default, be sure to set this flag in your calling command accordingly

# Grab te script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# Capture elapsed time - reset BASH time counter
SECONDS=0

# Set sourced flag
if [ "${ME}" = "CreateRenderers.sh" ]; then
  SOURCED="false"
  WD=$PWD
else
  SOURCED="true"
fi

# Initialize OBS if not in command line input
if [ "${OBS}" = "" ]; then
  OBS=true
fi

Info () {
    if [ "${SOURCED}" = "true" ]
    then
        echo "   renderers: ${*}" >&2
    else
        echo "-${*}" >&2
    fi
}

# Functions
ExtractArchive() {
	# args: $1 = <folder>, $2 = <valid subfolder>    
	Info "Extracting $1.tar.gz..."
	mkdir -p $1 && chown user:user $1
	tar -mxzf $1.tar.gz -C $1 --strip-components=1
	if [ -d $1/$2 ]; then
	  Info "Archive $1.tar.gz successfully extracted."
	  rm -rf $1.tar.gz && Info "Cleanup archive $1.tar.gz."
	  cd $1 && echo
	else
	  Info "ERROR - $1.tar.gz did not extract properly."
	fi
}

buildLdglite() {
  ${QMAKE_EXE} CONFIG+=3RD_PARTY_INSTALL=../${DIST_DIR} CONFIG+=release CONFIG+=BUILD_CHECK
  make && make install
}

buildLdview() {
  ${QMAKE_EXE} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} CONFIG+=release CONFIG+=BUILD_CUI_ONLY CONFIG+=USE_SYSTEM_LIBS CONFIG+=BUILD_CHECK
  make && make install
}

buildPovray() {
  cd unix && chmod +x prebuild3rdparty.sh && ./prebuild3rdparty.sh
  cd ../
  ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." --prefix="${DIST_PKG_DIR}" LPUB3D_3RD_PARTY="yes" --with-libsdl2 --enable-watch-cursor
  make check
  make install
}

# Logging
if [ "${SOURCED}" = "true" ]; then
  Info "Start CreateRenderers execution at $PWD..."
else
  Info "Start $ME execution at $PWD..."
  if [ ! "$OBS" = "true" ]; then
    # logging stuff
    # increment log file name
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
    exec > >(tee -a ${LOG} )
    exec 2> >(tee -a ${LOG} >&2)
  fi
fi

echo
Info "Building............[LPub3D 3rd Party Renderers]"

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
  echo
  Info "Requird Working Directory 'WD' environment varialbe not specified - the script will exit."
  exit 1
fi

# Platform ID
OS_NAME=`uname`
if [ "OS_NAME" = "Darwin" ]; then
	platform=$(echo `sw_vers -productName` `sw_vers -productVersion`)
else
	platform=$(. /etc/os-release && if test "$PRETTY_NAME" != ""; then echo "$PRETTY_NAME"; else echo `uname`; fi)
fi
if [ "${DOCKER}" = "true" ]; then
	Info "Platform............[Docker - ${platform}]"
elif [ "$TRAVIS" = "true" ]; then
  Info "Platform............[Travis CI - ${platform}]"
elif [ "$OBS" = "true" ]; then
  Info "Platform............[Open Build System - ${platform}]"
else
	Info "Platform............[${platform}]"
fi

# Distribution directory
DIST_DIR=lpub3d_linux_3rdparty
DIST_PKG_DIR=${WD}/${DIST_DIR}
if [ ! -d ${DIST_PKG_DIR} ]; then
  mkdir -p ${DIST_PKG_DIR} && Info "${DIST_PKG_DIR} created."
fi

# Working directory
cd ${WD}
Info "Working directory...[$WD]";

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
	  curl -s -O http://www.ldraw.org/library/updates/complete.zip;
  fi
  Info "Extracting LDraw library into ${LDRAWDIR}..."
  unzip -d ${LDRAWDIR_ROOT} -q complete.zip;
  if [ -d ${LDRAWDIR} ]; then
	  Info "LDraw library extracted. LDRAWDIR defined."
  fi
fi
if [ "$OS_NAME" = "Darwin" ]; then
  Info "set LDRAWDIR in environment.plist..."
  chmod +x builds/utilities/set-ldrawdir.command && ./builds/utilities/set-ldrawdir.command
  grep -A1 -e 'LDRAWDIR' ~/.MacOSX/environment.plist
  Info "LDRAWDIR......${LDRAWDIR}"
  Info "set LDRAWDIR Completed."
fi
echo

# Qt setup
if [ "$OBS" = "true" ]; then
  export QT_SELECT=qt5
  if [ -x /usr/bin/qmake ] ; then
    QMAKE_EXE=qmake
  elif [ -x /usr/bin/qmake-qt5 ] ; then
    QMAKE_EXE=qmake-qt5
  fi
else
  QMAKE_EXE=qmake
fi
${QMAKE_EXE} -v
QMAKE_EXE="${QMAKE_EXE} -makefile"

# Main loop
for buildDir in ldglite ldview povray; do
  case ${buildDir} in
  ldglite)
	curlCommand="https://github.com/trevorsandy/ldglite/archive/master.tar.gz"
	buildCommand="buildLdglite"
	validSubDir="mui"
	;;
  ldview)
	curlCommand="https://github.com/trevorsandy/ldview/archive/qmake-build.tar.gz"
	buildCommand="buildLdview"
	validSubDir="OSMesa"
	;;
  povray)
	curlCommand="https://github.com/trevorsandy/povray/archive/lpub3d/raytracer-cui.tar.gz"
	buildCommand="buildPovray"
	validSubDir="unix"
	;;
  esac
  if [ "$OBS" = "true" ]; then
    if [ -f ${buildDir}.tar.gz ]; then
      ExtractArchive ${buildDir} ${validSubDir}
    else
      Info "ERROR - Unable to find ${buildDir}.tar.gz at $PWD"
    fi
  elif [ ! -d ${buildDir}/${validSubDir} ]; then
    echo
    Info "LDVIEW directory does not exist. Checking for tarball archive..."
    if [ -f ${buildDir}.tar.gz ]; then
      ExtractArchive ${buildDir} ${validSubDir}
    else
      Info "`echo ${buildDir} | awk '{print toupper($0)}'` \
	  tarball ${buildDir}.tar.gz does not exist. Downloading..."
	  
      curl -sL -o ${buildDir}.tar.gz ${curlCommand}
      ExtractArchive ${buildDir} ${validSubDir}
    fi
  else
    cd ${buildDir}
  fi
  echo && echo "----------------------------------------------------"
  ${buildCommand} && cd ${WD}
done

# Elapsed execution time
ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
echo
Info "Finished."
Info "$ELAPSED"
