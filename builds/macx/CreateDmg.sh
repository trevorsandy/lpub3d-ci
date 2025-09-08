#!/bin/bash
# Trevor SANDY
# Last Update: September 05, 2025
# Build and package LPub3D for macOS
# To run:
# $ chmod 755 CreateDmg.sh
# $ ./CreateDmg.sh

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  ME="${ME} for (${LP3D_ARCH})"
  if [ "$BUILD_OPT" = "verify" ]; then
    echo "$ME Verification Finished!"
  elif [ "$BUILD_OPT" = "compile" ]; then
    echo "LPub3D Compile Finished!"
  elif [ "$BUILD_OPT" = "renderers" ]; then
    echo "LPub3D Renderers Build Finished!"
  else
    echo "$ME Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# Fake realpath
realpath() {
  OURPWD=$PWD
  cd "$(dirname "$1")"
  LINK=$(readlink "$(basename "$1")")
  while [ "$LINK" ]; do
    cd "$(dirname "$LINK")"
    LINK=$(readlink "$(basename "$1")")
  done
  REALPATH_="$PWD/$(basename "$1")"
  cd "$OURPWD"
  echo "$REALPATH_"
}

CWD=`pwd`

# Format name - SOURCED if $1 is empty
ME="CreateDmg"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced

# automatic logging
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
if [ "${WRITE_LOG}" = "true" ]; then
    f="${LP3D_LOG_PATH}/$ME"
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
    exec > >(tee -a ${LOG})
    exec 2> >(tee -a ${LOG} >&2)
fi

echo "Start $ME execution at $CWD..."

# Change these when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d-ci}"
LP3D_ARCH="${LP3D_ARCH:-$(uname -m)}"
LP3D_CPU_CORES="${LP3D_CPU_CORES:-$(nproc)}"
LP3D_GITHUB_URL="https://github.com/trevorsandy"
BUILD_BRANCH=${BUILD_BRANCH:-master}
CMD_CNT=0

echo && echo "   LPUB3D BUILD ARCH......[${LP3D_ARCH}]"
echo "   LPUB3D SOURCE DIR......[$(realpath .)]"

if [ "$BUILD_OPT" = "verify" ]; then
  echo "   BUILD OPTION...........[verify only]"
elif [ "$BUILD_OPT" = "compile" ]; then
  echo "   BUILD OPTION...........[comple only]"
elif [ "$BUILD_OPT" = "renderers" ]; then
  echo "   BUILD OPTION...........[renderers only]"
else
  echo "   BUILD OPTION...........[build package]"
fi
echo "   CPU CORES..............[${LP3D_CPU_CORES}]"
echo "   LOG FILE...............[$([ -n "${LOG}" ] && echo ${LOG} || echo "not writing log")]" && echo
echo "   PRESERVE BUILD REPO....$(if test "${PRESERVE}" = "true"; then echo YES; else echo NO; fi)"

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

l=Log

# when running with Installer Qt, use this block...
if [ "${CI}" != "true"  ]; then
  # use this instance of Qt if exist - this entry is my dev machine, change for your environment accordingly
  if [ -d ~/Qt/IDE/6.9.2/clang_64 ]; then
    export PATH=~/Qt/IDE/6.9.2/clang_64:~/Qt/IDE/6.9.2/clang_64/bin:$PATH
  else
    echo "PATH not udpated with Qt location, could not find ${HOME}/Qt/IDE/6.9.2/clang_64"
    exit 1
  fi
  echo
  echo "LPub3D will uninstall all versions of Boost ignoring dependencies and install 1.60."
  echo "You can stop here if you do not want to uninstall your current version of Boost"
  echo "or if you prefer to personally configure the availability of Boost 1.60."
  read -n 1 -p "Enter c to continue Boost uninstall: " getoption
  if [ "$getoption" = "c" ] || [ "$getoption" = "C" ]; then
     echo  "  You selected to uninstall any current versions of Boost."
  else
     echo  "  You selected to stop the installation."
     exit 0
  fi
  echo
  echo "Enter d to download LPub3D source or any key to"
  echo "skip download and use existing source if available."
  read -n 1 -p "Do you want to continue with this option? : " getsource
else
  # Use this block for Homebrew Qt (script called from clone directory - e.g. lpub3d)
  # Set no-prompt getsource flag: 'c' = copy flag, 'd' = download flag
  getsource=c
  # move outside clone directory (lpub3d)/
  cd ../
fi

echo "$((CMD_CNT+=1))- create DMG build working directory $(realpath dmgbuild/)"
if [ ! -d dmgbuild ]
then
  mkdir dmgbuild
fi

cd dmgbuild

if [ "$getsource" = "d" ] || [ "$getsource" = "D" ]
then
  echo "$((CMD_CNT+=1))- you selected download LPub3D source."
  if [ -d ${LPUB3D} ]; then
    echo "$((CMD_CNT+=1))- remove old ${LPUB3D} from dmgbuild/"
    rm -rf ${LPUB3D}
  fi
  echo -n "$((CMD_CNT+=1))- cloning ${LPUB3D}/ to $(realpath dmgbuild/)..."
  (git clone ${LP3D_GITHUB_URL}/${LPUB3D}.git) >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
elif [ "$getsource" = "c" ] || [ "$getsource" = "C" ] || [ ! -d ${LPUB3D} ]
then
  echo "$((CMD_CNT+=1))- copying ${LPUB3D}/ to $(realpath dmgbuild/)"
  if [ ! -d ../${LPUB3D} ]; then
    echo "$((CMD_CNT+=1))- NOTICE - Could not find folder $(realpath ../${LPUB3D}/)"
    if [ -d ${LPUB3D} ]; then
      echo "$((CMD_CNT+=1))- remove old ${LPUB3D} from dmgbuild/"
      rm -rf ${LPUB3D}
    fi
    echo -n "$((CMD_CNT+=1))- cloning ${LPUB3D} ${BUILD_BRANCH} branch into $(realpath dmgbuild/)..."
    (git clone -b ${BUILD_BRANCH} ${LP3D_GITHUB_URL}/${LPUB3D}.git) >$l.out 2>&1 && rm $l.out
    [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
  else
    echo -n "2. copy ${LPUB3D} source to dmgbuild/..."
    (cp -rf ../${LPUB3D}/ ./${LPUB3D}/) >$l.out 2>&1 && rm $l.out
    [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
  fi
else
  echo "$((CMD_CNT+=1))- ${LPUB3D}/ exist. skipping download"
fi

if [ -z "$LDRAWDIR" ]; then
  LDRAWDIR=${HOME}/Library/LDraw
fi

# set pwd before entering lpub3d root directory
export WD=$PWD
export OBS=false
export LPUB3D=${LPUB3D}

echo "$((CMD_CNT+=1))- source update_config_files.sh" && echo

_PRO_FILE_PWD_=${WD}/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh
SOURCE_DIR=${LPUB3D}-${LP3D_VERSION}

case ${LP3D_ARCH} in
  "x86_64"|"aarch64"|"arm64")
    release="64bit_release" ;;
  *)
    release="32bit_release" ;;
esac

echo "$((CMD_CNT+=1))- execute CreateRenderers from $(realpath ${LPUB3D}/)"

cd ${LPUB3D}

# Build LPub3D renderers - LDGLite, LDView, POV-Ray
chmod +x builds/utilities/CreateRenderers.sh && \
env \
WD=${WD} \
OBS=${OBS:-false} \
LPUB3D=${LPUB3D} \
GITHUB=${GITHUB} \
DOCKER=${DOCKER} \
LDRAWDIR=${LDRAWDIR} \
LP3D_LOG_PATH=${LP3D_LOG_PATH} \
LP3D_CPU_CORES=${LP3D_CPU_CORES} \
LP3D_NO_CLEANUP=${LP3D_NO_CLEANUP:-true} \
LP3D_3RD_DIST_DIR=${LP3D_3RD_DIST_DIR} \
./builds/utilities/CreateRenderers.sh

DIST_DIR="$(cd ../ && echo "$PWD/lpub3d_macos_3rdparty")"

# POVRay configure uses aarch64 architecture value on arm64 Mac build
[ "${LP3D_ARCH}" = "arm64" ] && LP3D_RAY_ARCH="aarch64" || LP3D_RAY_ARCH=${LP3D_ARCH}

# Check if renderers exist or were successfully built
LDGLITE_PATH="${DIST_DIR}/LDGLite-1.3/bin/${LP3D_ARCH}"
LDVIEW_PATH="${DIST_DIR}/LDView-4.6/bin/${LP3D_ARCH}"
POVRAY_PATH="${DIST_DIR}/lpub3d_trace_cui-3.8/bin/${LP3D_RAY_ARCH}"
if [ ! -f "${LDGLITE_PATH}/LDGLite" ]; then
  echo "-WARNING - LDGLite not found at ${LDGLITE_PATH}/"
fi
if [ ! -f "${LDVIEW_PATH}/LDView" ]; then
  echo "-WARNING - LDView not found at ${LDVIEW_PATH}/"
fi
if [ ! -f "${POVRAY_PATH}/lpub3d_trace_cui" ]; then
  echo "-WARNING - POVRay not found at ${POVRAY_PATH}/"
fi

# Stop here if we are only building renderers
if [ "$BUILD_OPT" = "renderers" ]; then
  exit 0
fi

# Copy LDraw archive libraries to mainApp/extras
EXTRAS_DIR=$(realpath mainApp/extras)
LP3D_LIBS_BASE=${LP3D_GITHUB_URL}/lpub3d_libs/releases/download/v1.0.1
LDRAW_LIB_FILES=(complete.zip lpub3dldrawunf.zip tenteparts.zip vexiqparts.zip)
for libFile in "${LDRAW_LIB_FILES[@]}"; do
  if [ ! -f "${EXTRAS_DIR}/${libFile}" ]; then
    if [ -f "${DIST_DIR}/${libFile}" ]; then
      echo -n "$((CMD_CNT+=1))- copying ${libFile} from ${DIST_DIR}/ to ${EXTRAS_DIR}/..."
      (cp -f "${DIST_DIR}/${libFile}" "${EXTRAS_DIR}/${libFile}") >$l.out 2>&1 && rm $l.out
      [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
    else
      echo -n "$((CMD_CNT+=1))- downloading ${libFile} into ${EXTRAS_DIR}/..."
      (curl $curlopts ${LP3D_LIBS_BASE}/${libFile} -o ${libFile}) >$l.out 2>&1 && rm $l.out
      [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
    fi
  else
    echo "   - ${libFile} exist. skipping download"
  fi
done

echo && echo "$((CMD_CNT+=1))- configure and build source from $(realpath .)"
#qmake LPub3D.pro -spec macx-clang CONFIG+=x86_64 /usr/bin/make qmake_all
echo && qmake -v && echo
qmake CONFIG+=x86_64 CONFIG+=release CONFIG+=sdk_no_version_check CONFIG+=build_check CONFIG-=debug_and_release CONFIG+=dmg
/usr/bin/make -j${LP3D_CPU_CORES} || exit 1

# Check if build is OK or stop and return error.
if [ ! -f "mainApp/$release/LPub3D.app/Contents/MacOS/LPub3D" ]; then
  echo "-ERROR - build executable at $(realpath mainApp/$release/LPub3D.app/Contents/MacOS/LPub3D) not found."
  exit 1
else
  # run otool -L on LPub3D.app
  LPUB3D_OTOOL="$(realpath mainApp/$release/LPub3D.app/Contents/MacOS/LPub3D)"
  echo && echo -n "$((CMD_CNT+=1))- otool -L check ${LPUB3D_OTOOL}..."
  (otool -L ${LPUB3D_OTOOL} 2>/dev/null) >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
  # Stop here if we are only compiling
  if [ "$BUILD_OPT" = "compile" ]; then
    exit 0
  fi
fi

# create dmg environment - begin #
#
cd builds/macx

echo "$((CMD_CNT+=1))- generate README file"
cat <<EOF >README
Thank you for installing LPub3D v${LP3D_APP_VERSION} for macOS.

Drag the LPub3D Application icon to the Applications folder.

After installation, remove the mounted LPub3D disk image by dragging it to the Trash.

Required LPub3D libraries for macOS distribution.
Library versions for LPub3D built from source may differ.
========================
LDView:

- XQuartz version 11.0 (2.7.11) or above (needed for OSMesa)
  https://www.xquartz.org

- LibPNG version 1.6.37 or above
  http://www.libpng.org

- GL2PS version 1.4.0 or above
  http://geuz.org/gl2ps

- LibJPEG version 9c or above
  http://www.ijg.org

- MiniZIP version 1.2.11 or above
  http://www.winimage.com/zLibDll/minizip.html

POVRay:

- XQuartz version 11.0 (2.7.11)  or above (needed for X11)
  https://www.xquartz.org

- LibTIFF version 4.0.10 or above
  http://www.libtiff.org

- OpenEXR greater than version 2.3.0 or above
  http://www.openexr.com

- SDL2 version 2.0.10 or above (for display preview)
  http://www.libsdl.org

Homebrew
======================================
LPub3D and its renderers are compiled for both the Apple silicon arm64 and Intel x86_64 processors.

Running LPub3D x86_64 on an Apple silicon PC will require Rosetta which, if not already
installed on your PC will, you will be prompted to install it on your attempt to run LPub3D.
If you wish to install Rosetta from the command line, the command is:

- \$ /usr/sbin/softwareupdate --install-rosetta agree-to-license (root permission required)

Install brew (if not already installed)
======================================
For an Apple Intel processor:
- \$ /usr/bin/ruby -e "\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

For an Apple silicon arm64 processor:
- \$ /bin/bash -c "\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

For an x86_64 brew installation on an Apple silicon arm64 processor:
- \$ arch -x86_64 zsh
- \$ cd /usr/local && mkdir homebrew
- \$ curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew

Install libraries
=================
For an Apple silicon or Apple Intel processor:
- \$ brew update
- \$ brew reinstall libpng gl2ps libjpeg minizip openexr sdl2 libtiff
- \$ brew install --cask xquartz

For an x86_64 brew installation on an Apple silicon arm64 processor:
- \$ arch -x86_64 /usr/local/homebrew/bin/brew reinstall libpng gl2ps libjpeg minizip openexr sdl2 libtiff
- \$ arch -x86_64 /usr/local/homebrew/bin/brew install --cask xquartz

Optional - Check installed library (e.g. libpng)
============================================
- \$ otool -L \$(brew list libpng | grep dylib\$)

Object Tool (otool) output:
  /usr/local/Cellar/libpng/1.6.35/lib/libpng.dylib:
  /usr/local/opt/libpng/lib/libpng16.16.dylib (compatibility version 52.0.0, current version 52.0.0)
  /usr/lib/libz.1.dylib (compatibility version 1.0.0, current version 1.2.11)
  /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)

LPub3D Library Check Note: On startup LPub3D will test for the /opt/homebrew/bin/brew binary.
If found, the library check will use the Apple silicon path prefix:
- HomebrewLibPathPrefix: /opt/homebrew/opt.
...otherwise, it will use the Apple intel path prefix.
- HomebrewLibPathPrefix: /usr/local/opt.

Additionally, LPub3D will check Homebrew x86_64 libraries using PATH entries:
- LibraryCheckPathInsert: PATH=/usr/local/Homebrew/bin:/opt/local/bin:/usr/local/bin
...and Homebrew arm64 libraries using PATH entries:
- LibraryCheckPathInsert: PATH=/opt/homebrew/bin:/opt/homebrew/sbin
Of course, the assumption is you are running the appropriate macOS distribution for your processor.

If you choose to run an Intel distribution of LPub3D on an Apple silicon PC, running Rosetta
etc..., you are advised to configure the LPub3D plist with the appropriate Intel Homebrew path.
Or, if you choose to place your Homebrew binaries and libraries in alternate locations,
You can configure your personalized paths in the LPub3D plist at:
- \$HOME/Library/Preferences/com.lpub3d-software.LPub3D.plist.

The Homebrew plist keys are:
- HomebrewLibPathPrefix - the path prefix LPub3D will use to locate the Homebrew libraries.
- LibraryCheckPathInsert - the PATH entries needed to help brew run the info command

Cheers,
EOF

echo -n "$((CMD_CNT+=1))- copy README to LPub3D.app/Contents/Resources/README_macOS.txt..."
(cp -f README ../../mainApp/$release/LPub3D.app/Contents/Resources/README_macOS.txt) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

echo -n "$((CMD_CNT+=1))- copy ${LPUB3D} bundle components to $(realpath .)..."
(cp -rf ../../mainApp/$release/LPub3D.app .) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

echo -n "$((CMD_CNT+=1))- bundle LPub3D.app with Qt framework and plugins..."
(macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

echo -n "$((CMD_CNT+=1))- replace LPub3D.app bundle signature..."
(/usr/bin/codesign --force --deep --sign - LPub3D.app) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

echo -n "$((CMD_CNT+=1))- verify LPub3D.app bundle signature..."
(/usr/bin/codesign --verify --deep --verbose LPub3D.app) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

# build checks
LPUB3D_EXE=LPub3D.app/Contents/MacOS/LPub3D
if [ -n "$LP3D_SKIP_BUILD_CHECK" ]; then
  echo "$((CMD_CNT+=1))- skipping ${LPUB3D_EXE} build check."
else
  echo "$((CMD_CNT+=1))- build checks"
  # Check if exe exist - here we use the executable name
  if [ -f "${LPUB3D_EXE}" ]; then
    echo "$((CMD_CNT+=1))- Build package: $PWD/${LPUB3D_EXE}"
    # Check commands
    SOURCE_DIR=../..
    echo "$((CMD_CNT+=1))- build check SOURCE_DIR is $(realpath ${SOURCE_DIR})"
    source ${SOURCE_DIR}/builds/check/build_checks.sh
    # Stop here if we are only verifying
    if [ "$BUILD_OPT" = "verify" ]; then
      exit 0
    fi
  else
    echo "-ERROR - build-check failed. $(realpath ${LPUB3D_EXE}) not found."
  fi
fi

echo "$((CMD_CNT+=1))- setup dmg source dir $(realpath DMGSRC/)"
if [ -d DMGSRC ]
then
  rm -f -R DMGSRC
fi
mkdir DMGSRC

echo -n "$((CMD_CNT+=1))- move LPub3D.app to $(realpath DMGSRC/)..."
(mv -f LPub3D.app DMGSRC/LPub3D.app) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

echo "$((CMD_CNT+=1))- setup dmg output directory $(realpath ../../../DMGS/)"
DMGDIR=../../../DMGS
if [ -d ${DMGDIR} ]
then
  rm -f -R ${DMGDIR}
fi
echo -n "$((CMD_CNT+=1))- create dmg output directory $(realpath $DMGDIR)..."
(mkdir -p ${DMGDIR}) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

# pos: builds/macx
echo "$((CMD_CNT+=1))- copy ${LPUB3D} package assets to to $(realpath .)"
PACKAGE_ASSETS=(setup.icns lpub3dbkg.png COPYING_BRIEF)
for asset in "${PACKAGE_ASSETS[@]}"; do
  if [ "${asset}" = "COPYING_BRIEF" ]; then
    COPY_CMD="../../mainApp/docs/${asset} .COPYING"
  else
    COPY_CMD="../utilities/icons/${asset} ."
  fi
  echo -n "   - copying ${asset}..."
  (cp -f ${COPY_CMD}) >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
done

echo "$((CMD_CNT+=1))- set create-dmg build scrpt permissions"
chmod +x ../utilities/dmg-utils/create-dmg

echo "$((CMD_CNT+=1))- generate make dmg script"
LP3D_DMG="LPub3D-${LP3D_APP_VERSION_LONG}-${LP3D_ARCH}-macos.dmg"
cat <<EOF >makedmg
#!/bin/bash
../utilities/dmg-utils/create-dmg \\
--volname "LPub3D-Installer" \\
--volicon "setup.icns" \\
--background "lpub3dbkg.png" \\
--icon-size 90 \\
--text-size 10 \\
--window-pos 200 120 \\
--window-size 640 480 \\
--icon LPub3D.app 192 344 \\
--hide-extension LPub3D.app \\
--add-file Readme README 512 128 \\
--app-drop-link 448 344 \\
--eula .COPYING \\
"${DMGDIR}/${LP3D_DMG}" \\
DMGSRC/
EOF

echo "$((CMD_CNT+=1))- create LPub3D dmg package in $(realpath $DMGDIR/)"
[[ -f LPub3D-Installer.dmg ]] && rm LPub3D-Installer.dmg
if [ -d DMGSRC/LPub3D.app ]; then
   chmod +x makedmg && ./makedmg
else
  echo "   - Could not find LPub3D.app at $(realpath DMGSRC/)"
  echo "   - $ME Failed."
  exit 1
fi

if [ -f "${DMGDIR}/${LP3D_DMG}" ]; then
echo "      Distribution package.: ${LP3D_DMG}"
  echo "      Package path.........: $PWD/${LP3D_DMG}"
  echo "   - cleanup"
  rm -f -R DMGSRC
  rm -f lpub3d.icns lpub3dbkg.png README .COPYING makedmg
else
  echo "   - ${DMGDIR}/${LP3D_DMG} was not found."
  echo "   - $ME Failed."
fi

exit 0
