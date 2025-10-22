#!/bin/bash
# Trevor SANDY
# Last Update October 22, 2025
# Copyright (C) 2022 - 2025 by Trevor SANDY
#
# This script is run from a Docker container call
# See builds/utilities/ci/github/linux-build.sh
#
# Note the container must mount the following volumes
#   -v <3rd-party apps path>:/out
#   -v <         ldraw path>:/dist
#   -v <        output path>:/ldraw
#
# To Run:
# /bin/bash -xc "chmod a+x builds/linux/CreateLinux.sh && builds/linux/CreateLinux.sh"

set -o functrace

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: (($SECONDS / 3600))hrs ((($SECONDS / 60) % 60))min (($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  if [ "${LP3D_APPIMAGE}" = "true" ]; then
    ME="${ME} for (${LP3D_BASE}-${LP3D_ARCH}-appimage)"
  else
    ME="${ME} for (${LP3D_BASE}-${LP3D_ARCH})"
  fi
  if [ "$BUILD_OPT" = "verify" ]; then
    echo "${ME} Verification Finished!"
  else
    echo "${ME} Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

SaveAppImageSetupProgress() {
  # backup AppDir build artifacts in case of failure
  if [[ "${CI}" != "true" && $? = 0 ]]; then
    mkdir -p ${LP3D_DIST_DIR_PATH}/AppDir/tools
    [ -f "${AppDirBuildPath}/linuxdeployqt" ] && \
    cp -af ${AppDirBuildPath}/linuxdeployqt ${LP3D_DIST_DIR_PATH}/AppDir/tools/ || :
    if [ -n "${LP3D_AI_BUILD_TOOLS}" ]; then
      [ -d "${WD}/bin" ] && cp -af bin/ ${LP3D_DIST_DIR_PATH}/AppDir/tools/ || :
      [ -d "${WD}/share" ] && cp -af share/ ${LP3D_DIST_DIR_PATH}/AppDir/tools/ || :
      [ -f "${WD}/patchelf-0.9.tar.bz2" ] && \
      cp -af patchelf-0.9.tar.bz2 ${LP3D_DIST_DIR_PATH}/AppDir/tools/ || :
    fi
  fi
}

Info () {
  f="${0##*/}"; f="${f%.*}"
  echo "-${f}: ${*}" >&2
}

Error () {
  Info "ERROR - $*" >&2
}

# Format name and set WRITE_LOG - SOURCED if $1 is empty
ME="CreateLinux"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced

# automatic logging
[ -d "/out" ] && LP3D_LOG_PATH=/out
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
if [ "${WRITE_LOG}" = "true" ]; then
  f="${0##*/}"; f="${f%.*}"; [ -n "${LP3D_ARCH}" ] && f="${f}-${LP3D_ARCH}" || f="${f}-amd64"
  [ "${LP3D_APPIMAGE}" = "true" ] && f="${f}-appimage" || :
  f="${LP3D_LOG_PATH}/${f}"
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

export WRITE_LOG
export LP3D_LOG_PATH

Info "Start $ME execution from $PWD..."

# build in a temporary directory, use RAM disk if possible
if [ -d /dev/shm ] && mount | grep /dev/shm | grep -v -q noexec; then
  TEMP_BASE=/dev/shm
elif [ -d /docker-ramdisk ]; then
  TEMP_BASE=/docker-ramdisk
else
  TEMP_BASE=/tmp
fi

BUILD_DIR="$(mktemp -d -p "${TEMP_BASE}" build-XXXXXX)"

export BUILD_DIR=$BUILD_DIR

finish () {
  if [ -d "${BUILD_DIR}" ]; then
    rm -rf "${BUILD_DIR}"
  fi
  FinishElapsedTime
}

trap finish EXIT

# Move to build directory
cd ${BUILD_DIR} || exit 1

export LP3D_DIST_DIR_PATH=${LP3D_DIST_DIR_PATH:-/dist/${LP3D_BASE}_${LP3D_ARCH}}
export LP3D_3RD_DIST_DIR=${LP3D_3RD_DIST_DIR:-lpub3d_linux_3rdparty}
export LPUB3D=${LPUB3D:-lpub3d-ci}
export LDRAWDIR_ROOT=~
export LDRAWDIR=~/ldraw
export WD=$PWD
export CI=${CI:-true}
export OBS=${OBS:-false}
export GITHUB=${GITHUB:-true}
export GITHUB_REF=${GITHUB_REF:-}
export GITHUB_SHA=${GITHUB_SHA:-}
export DOCKER=${DOCKER:-true}
export LP3D_NO_DEPS=${LP3D_NO_DEPS:-true}
export LP3D_LOG_PATH=${LP3D_LOG_PATH:-/out}
export LP3D_NO_CLEANUP=${LP3D_NO_CLEANUP:-true}
LP3D_GITHUB_URL="https://github.com/trevorsandy"

[ -n "${BUILD_OPT}" ] && Info "BUILD OPTION.......${BUILD_OPT}" || :
[ -n "${LPUB3D}" ] && Info "SOURCE DIR.........${LPUB3D}" || :
[ -n "${BUILD_DIR}" ] && Info "BUILD DIR..........${BUILD_DIR}" || :
if [ -n "${LP3D_BASE}" ]; then
  Info "BUILD BASE.........${LP3D_BASE}"
  if [ "${LP3D_BASE}" = "fedora" ]; then
    LP3D_SPEC_VERSION="$(grep Version: /${LPUB3D}.spec | cut -d: -f2| sed 's/(.*)//g' | tr -d ' ')"
    Info "RPM SPEC VERSION...${LP3D_SPEC_VERSION}"
  fi
fi
[ -n "${LP3D_ARCH}" ] && Info "BUILD ARCH.........${LP3D_ARCH}" || :
[ -n "${CI}" ] && Info "CI.................${CI}" || :
[ -n "${GITHUB}" ] && Info "GITHUB.............${GITHUB}" || :
[ -n "${GITHUB_REF}" ] && Info "GITHUB_REF.........${GITHUB_REF}" || :
[ -n "${GITHUB_SHA}" ] && Info "GITHUB_SHA.........${GITHUB_SHA}" || :
[ -n "${LP3D_PUBLISH_RENDERERS}" ] && Info "PUBLISH RENDERERS..${LP3D_PUBLISH_RENDERERS}" || :
[ -n "${LP3D_APPIMAGE}" ] && Info "APPIMAGE...........${LP3D_APPIMAGE}" || :
if [ "${LP3D_APPIMAGE}" = "true" ]; then
  Info "BUILD AI ARCH......${LP3D_AI_ARCH}"
  Info "BUILD AI TOOLS.....$([ -n "${LP3D_AI_BUILD_TOOLS}" ] && echo "true" || echo "false")"
  Info "PATCH MAGIC_BYTES..$([ -n "${LP3D_AI_MAGIC_BYTES}" ] && echo "true" || echo "false")"
  Info "EXTRACT AI PAYLOAD.$([ -n "${LP3D_AI_EXTRACT_PAYLOAD}" ] && echo "true" || echo "false")"
else
  Info "PRE-PACKAGE CHECK..$([ -n "${LP3D_PRE_PACKAGE_CHECK}" ] && echo "true" || echo "false")"
fi

# LDraw library archives
ldrawLibFiles=(complete.zip lpub3dldrawunf.zip tenteparts.zip vexiqparts.zip)

# Download LDraw library archive files if not available
l=Log
Info && Info "Checking LDraw archive libraries..."
LP3D_LIBS_BASE="${LP3D_GITHUB_URL}/lpub3d_libs/releases/download/v1.0.1"
[[ ! -L "/dist" && ! -d "/dist" ]] && mkdir -p "/dist" || :
for libFile in "${ldrawLibFiles[@]}"; do
  if [ ! -f "/dist/${libFile}" ]; then
    echo -n "downloading ${libFile} into ${BUILD_DIR}..."
    (wget ${LP3D_LIBS_BASE}/${libFile} -O /dist/${libFile}) >$l.out 2>&1 && rm $l.out
    [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
  fi
done

# If not AppImage build, source package build script and exit
if [[ "${LP3D_APPIMAGE}" != "true" ]]; then
  # copy files from user directory to build directory
  rsync -avr --exclude='ldraw' --exclude='.config' ~/ .
  # set paths and package script
  case ${LP3D_BASE} in
    "ubuntu")
      pkgblddir="debbuild"
      pkgsrcdir="${pkgblddir}/SOURCES"
      pkgscript="CreateDeb.sh"
      ;;
    "fedora")
      pkgblddir="rpmbuild/BUILD/${LPUB3D}-${LP3D_SPEC_VERSION}-build"
      pkgsrcdir="rpmbuild/SOURCES"
      pkgscript="CreateRpm.sh"
      ;;
    "archlinux")
      pkgblddir="pkgbuild/src"
      pkgsrcdir="pkgbuild"
      pkgscript="CreatePkg.sh"
      ;;
    *)
      Error "Invalid build base specified"
      exit 2
      ;;
  esac

  # Copy LDraw libraries to package source path
  for libFile in "${ldrawLibFiles[@]}"; do
    [ ! -f "${pkgsrcdir}/${libFile}" ] && \
    (cd ${pkgsrcdir} && cp -af /dist/${libFile} .) || \
    Info "${pkgsrcdir}/${libFile} exists. Nothing to do."
  done

  # Link lpub3d_linux_3rdparty to package build source path
  if [ "${LP3D_BASE}" != "fedora" ]; then
    LP3D_TD=${WD}/${pkgblddir}/${LP3D_3RD_DIST_DIR}
    if [ ! -d "${LP3D_TD}" ]; then
      (cd ${WD}/${pkgblddir} && ln -sf "${LP3D_DIST_DIR_PATH}" "${LP3D_3RD_DIST_DIR}")
      if [ -d "${LP3D_TD}" ]; then
        Info "${LP3D_DIST_DIR_PATH} linked to ${LP3D_TD}"
      else
        Error "${LP3D_DIST_DIR_PATH} failed to link to ${LP3D_TD}"
      fi
    else
      Info "Using cached 3rd Party repository ${LP3D_TD}"
    fi
  fi

  # Source package script
  source ${pkgscript}

  # Done so exit.
  exit 0
fi

# ............Compile for AppImage Build...................#

# Copy or download source
if [ "${TRAVIS}" != "true" ]; then
   if [ -d "/in" ]; then
     Info "Copy source to ${BUILD_DIR}/..."
     cp -rf /in/. .
   else
     Info "Download source to ${BUILD_DIR}/..."
     git clone ${LP3D_GITHUB_URL}/${LPUB3D}.git
     mv -f ./${LPUB3D}/.[!.]* . && mv -f ./${LPUB3D}/* . && rm -rf ./${LPUB3D}
   fi
else
   Info "Copy source to ${BUILD_DIR}/..."
   cp -rf "../../${LPUB3D}" .
fi

# For Docker build, check if there is a tag after the last commit
if [ -d "${WD}/.git" ]; then
   # Setup git command
   LP3D_GIT="git --git-dir ${WD}/.git --work-tree ${WD}"
fi
if [[ -n "${LP3D_GIT}" && "$DOCKER" = "true" ]]; then
  # Pull latest
  [ "${CI}" = "true" ] && ${LP3D_GIT} pull >/dev/null 2>&1 || :
  #1. Get the latest version tag - check across all branches
  BUILD_TAG="$(${LP3D_GIT} describe --tags --match v* "$(${LP3D_GIT} rev-list --tags --max-count=1)" 2> /dev/null)"
  if [ -n "${BUILD_TAG}" ]; then
    #2. Get the tag datetime
    BUILD_TAG_TIME=$(${LP3D_GIT} log -1 --format=%ai $BUILD_TAG 2> /dev/null)
    #3. Get the latest commit datetime from the build branch
    GIT_COMMIT_TIME=$(${LP3D_GIT} log -1 --format=%ai 2> /dev/null)
    #4. If tag is newer than commit, check out the tag
    if [[ "$(date -d "${GIT_COMMIT_TIME}" +%s)" -lt "$(date -d "${BUILD_TAG_TIME}" +%s)" ]]; then
      Info "2a. checking out build tag ${BUILD_TAG}..."
      ${LP3D_GIT} checkout -qf ${BUILD_TAG}
    fi
  fi
fi

# Source update_config_files.sh"
Info "Source update_config_files.sh..."
export _PRO_FILE_PWD_=${WD}/mainApp
set +x && source builds/utilities/update-config-files.sh && set -x

# Link lpub3d_linux_3rdparty to package build source path
LP3D_TD=${WD}/${LP3D_3RD_DIST_DIR}
if [ ! -d "${LP3D_TD}" ]; then
  (ln -sf "${LP3D_DIST_DIR_PATH}" "${LP3D_3RD_DIST_DIR}")
  if [ -d "${LP3D_TD}" ]; then
    Info "${LP3D_DIST_DIR_PATH} linked to ${LP3D_TD}"
  else
    Error "${LP3D_DIST_DIR_PATH} failed to link to ${LP3D_TD}"
    exit 4
  fi
else
  Info "Using cached 3rd Party repository ${LP3D_TD}"
fi

# Link and copy locally LDraw library archive files
Info "Link archive libraries to ${LP3D_DIST_DIR_PATH}..."
(cd ${LP3D_DIST_DIR_PATH} && \
ln -sf /dist/lpub3dldrawunf.zip lpub3dldrawunf.zip && \
ln -sf /dist/complete.zip complete.zip && \
ln -sf /dist/tenteparts.zip tenteparts.zip && \
ln -sf /dist/vexiqparts.zip vexiqparts.zip) && \
Info "Copy archive libraries to ${BUILD_DIR}..." && \
cp -af /dist/*.zip .

# List global and local 'LP3D_*' environment variables - use 'env' for 'exported' variables
set +x && \
Info && Info "LP3D* environment variables:" && compgen -v | grep LP3D_ | while read line; do Info $line=${!line}; done && \
set -x

# Build LPub3D renderers - LDGLite, LDView, POV-Ray
chmod a+x builds/utilities/CreateRenderers.sh && \
env \
WD=${WD} \
OBS=${OBS} \
LPUB3D=${LPUB3D} \
GITHUB=${GITHUB} \
DOCKER=${DOCKER} \
LDRAWDIR=${LDRAWDIR} \
LP3D_NO_DEPS=${LP3D_NO_DEPS} \
LP3D_LOG_PATH=${LP3D_LOG_PATH} \
LP3D_CPU_CORES=${LP3D_CPU_CORES} \
LP3D_NO_CLEANUP=${LP3D_NO_CLEANUP} \
LP3D_3RD_DIST_DIR=${LP3D_3RD_DIST_DIR} \
LP3D_PUBLISH_RENDERERS=${LP3D_PUBLISH_RENDERERS} \
./builds/utilities/CreateRenderers.sh

# Set application build path
[ ! -d AppDir ] && mkdir -p AppDir || :
AppDirBuildPath=$(readlink -f AppDir/)
export AppDirBuildPath=$AppDirBuildPath

AppDirDistPath="${LP3D_DIST_DIR_PATH}/AppDir"

# Build LPub3D
if [[ ! -d "${AppDirDistPath}/usr" || -z "$(ls -A ${AppDirDistPath}/usr)" ]]; then
  if [[ "${LP3D_APPIMAGE}" == "false" ]]; then
    case ${LP3D_BASE} in
      "ubuntu")
        distropkg=deb ;;
      "fedora")
        distropkg=rpm ;;
      "archlinux")
        distropkg=pkg ;;
    esac
  else
    distropkg=api
  fi

  # qmake setup
  export QT_SELECT=$(which qmake6 && echo qt6 || echo qt5)
  if which qmake6 >/dev/null 2>&1; then
    QMAKE_EXEC=qmake6
    ln -s `which qmake6` ./qmake; \
    export PATH=`pwd`:${PATH}; \
    qmake -v;
  elif which qmake-qt5 >/dev/null 2>&1; then
    QMAKE_EXEC=qmake-qt5
  elif which qmake >/dev/null 2>&1; then
    QMAKE_EXEC=qmake
  else
    Info "QMake was not found - exiting..." && exit 1
  fi
  ${QMAKE_EXEC} -v

  # build command
  ${QMAKE_EXEC} -nocache CONFIG+=release CONFIG-=debug_and_release CONFIG+=${distropkg} LPub3D.pro
  make || exit 1
  make INSTALL_ROOT=${AppDirBuildPath} install || exit 1

  # copy AppRun to AppDir
  if [ "${LP3D_APPIMAGE}" = "true" ]; then
    Info "Copy personalized AppRun script to AppDir"
    cp -af builds/linux/obs/alldeps/AppRun ${AppDirBuildPath}
    chmod a+x ${AppDirBuildPath}/AppRun
  fi

  # backup build artifacts in case of failure
  if [[ "${CI}" != "true" && $? = 0 ]]; then
    cp -ar ${AppDirBuildPath} ${LP3D_DIST_DIR_PATH}/
  fi
else
  Info "LPub3D build artifacts exists - build skipped."
  [ -d "${AppDirDistPath}/usr" ] && \
  cp -ar ${AppDirDistPath}/usr ${AppDirBuildPath}/ || :
  [ -d "${AppDirDistPath}/opt" ] && \
  cp -ar ${AppDirDistPath}/opt ${AppDirBuildPath}/ || :
  if [ -d "${AppDirDistPath}/tools" ]; then
    [ -f "${AppDirDistPath}/tools/linuxdeployqt" ] && \
    cp -af ${AppDirDistPath}/tools/linuxdeployqt ${AppDirBuildPath}/ || :
    if [ -n "${LP3D_AI_BUILD_TOOLS}" ]; then
      [ -d "${AppDirDistPath}/tools/bin" ] && \
      cp -ar ${AppDirDistPath}/tools/bin ${WD}/ || :
      [ -d "${AppDirDistPath}/tools/share" ] && \
      cp -ar ${AppDirDistPath}/tools/share ${WD}/ || :
      [ -f "${AppDirDistPath}/tools/patchelf-0.9.tar.bz2" ] && \
      cp -af ${AppDirDistPath}/tools/patchelf-0.9.tar.bz2 ${WD}/ || :
    fi
  fi
fi

# Copy LDraw archive libraries to share/lpub3d folder
AppDirLPub3DPath=${AppDirBuildPath}/usr/share/lpub3d
if [ -d "${AppDirLPub3DPath}" ]; then
  Info "Copy LDraw libraries to ${AppDirLPub3DPath}"
  for libFile in "${ldrawLibFiles[@]}"; do
    if [ ! -f "${AppDirLPub3DPath}/${libFile}" ]; then
      echo -n " - Copyinging ${libFile}..."
      ( cp -ar ${BUILD_DIR}/${libFile}  ${AppDirLPub3DPath} ) >$l.out 2>&1 && rm -f $l.out
      [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
    fi
  done
else
  Error "Path ${AppDirLPub3DPath} was not found."
fi

# AppImage pre-package build check
if [[ -n "${LP3D_PRE_PACKAGE_CHECK}" ]]; then
  Info "Build check LPub3D bundle..."
  export LP3D_BUILD_OS=
  export SOURCE_DIR=${WD}
  export LP3D_CHECK_LDD="1"
  export LP3D_CHECK_STATUS="--version --app-paths"
  export LPUB3D_EXE="${AppDirBuildPath}/usr/bin/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
  chmod a+x builds/check/build_checks.sh && ./builds/check/build_checks.sh
fi

# ..........AppImage Build..................#

# Setup AppImage tools - linuxdeployqt, lconvert
p=LinuxDeployQt
Info && Info "Installing AppImage tools..."
if [[ -z "${LP3D_AI_BUILD_TOOLS}" ]]; then
  cd "${AppDirBuildPath}" || exit 1
  CommandArg=-version
  if [ ! -e linuxdeployqt ]; then
    Info "Insalling $p for ${LP3D_AI_ARCH}..."
    deployqtAppImage="linuxdeployqt-continuous-${LP3D_AI_ARCH}.AppImage"
    deployqtDownloadURL="https://github.com/probonopd/linuxdeployqt/releases/download/continuous"
    ( wget -c -nv "${deployqtDownloadURL}/${deployqtAppImage}" -O linuxdeployqt ) >$p.out 2>&1 && rm -f $p.out
    if [[ -f $p.out || ! -f linuxdeployqt ]]; then
      Error Download $p FAILED
      tail -80 $p.out
      exit 5
    fi
  fi
  ( chmod a+x linuxdeployqt && ./linuxdeployqt ${CommandArg} ) >$p.out 2>&1 && rm -f $p.out
  if [ ! -f $p.out ]; then
    Info "Install linuxdeployqt completed"
  else
    Error Install $p FAILED
    tail -80 $p.out
    exit 5
  fi
  SaveAppImageSetupProgress
else
  cd "${WD}" || exit 1
  [ ! -d bin ] && mkdir bin || :
  export PATH="${WD}/bin":"${PATH}"
  CommandArg=-version
  # LinuxDeployQt
  if [ ! -e ${AppDirBuildPath}/linuxdeployqt ]; then
    [ -d "$p" ] && rm -rf $p || :
    Info "Building $p for ${LP3D_ARCH} at ${PWD}..."
    git clone https://github.com/probonopd/linuxdeployqt.git $p
    ( cd $p && qmake && make && cp -a ./bin/* ${AppDirBuildPath}/ ) >$p.out 2>&1 && rm -f $p.out
    if [[ ! -f $p.out && -f ${AppDirBuildPath}/linuxdeployqt ]]; then
      Info "linuxdeployqt copied to ${AppDirBuildPath}"
    else
      Error $p FAILED
      tail -80 $p.out
      exit 5
    fi
    SaveAppImageSetupProgress
  fi
  ( cd ${AppDirBuildPath} && chmod a+x linuxdeployqt && ./linuxdeployqt ${CommandArg} ) >$p.out 2>&1 && rm -f $p.out
  if [ ! -f $p.out ]; then
    Info Build $p completed
  else
    Error Build $p FAILED
    tail -80 $p.out
    exit 5
  fi

  # PatchELF
  p=patchelf-0.9
  if [ ! -e bin/patchelf ]; then
    [ -d "$p" ] && rm -rf $p || :
    Info "Building $p for ${LP3D_ARCH}......"
    [ ! -f $p.tar.bz2 ] && \
    wget https://nixos.org/releases/patchelf/patchelf-0.9/$p.tar.bz2 || :
    tar xf $p.tar.bz2
    ( cd $p && ./configure --prefix=${WD} --exec_prefix=${WD} && make && make install ) >$p.out 2>&1 && mv $p.out $p.ok
    if [ ! -f $p.out ]; then
      cat $p.ok
      Info Build $p completed
    else
      Error Build $p FAILED
      tail -80 $p.out
      exit 5
    fi
    SaveAppImageSetupProgress
  else
    Info $p exists. Nothing to do.
  fi

  # AppImageTool
  p=appimagetool
  CommandArg=--appimage-version
  if [ ! -e bin/appimagetool ]; then
    Info "Setting up $p for ${LP3D_ARCH} at ${PWD}..."
    if [ -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$p ]; then
      cp -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$p bin/ && chmod a+x bin/$p
      ( ./bin/$p ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
      formaterror="Exec format error"
      if [ "$(grep -F "${formaterror}" $p.out 2>/dev/null)" ]; then
        echo Format error, command $p ${CommandArg} FAILED
        tail -80 $p.out
        if [[ -n "${LP3D_AI_MAGIC_BYTES}" ]]; then
          mb="41 49 02 00"
          hd="$(hexdump -Cv bin/$p | head -n 1 | (grep -oE '41 49 02 00'))"
          if [ "${mb}" = "${hd}" ]; then
            echo "$p magic bytes: ${hd}"
            echo "Patching out $p magic bytes..."
            dd if=/dev/zero of="bin/$p" bs=1 count=3 seek=8 conv=notrunc
            if [ -z "$(hexdump -Cv bin/$p | head -n 1 | (grep -oE '41 49 02 00'))" ]; then
              echo "$p magic bytes patched $(hexdump -Cv bin/$p | head -n 1)"
              ( ./bin/$p ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
              if [ -f $p.ok ]; then
                unset runnablecheck
                cat $p.ok >> ${LP3D_LOG_PATH}/$p.ok && rm $p.ok
                echo $p is runnable
              else
                echo Patched magic bytes command $p ${CommandArg} FAILED
                tail -80 $p.out
              fi
            else
              echo Patch $p magic bytes FAILED
              hexdump -Cv bin/$p | head -n 3
              exit 5
            fi
          else
            echo "Magic bytes 'AI' not found in $p"
            hexdump -Cv bin/$p | head -n 3
          fi
        fi
      elif [ -f $p.ok ]; then
        cat $p.ok >> ${LP3D_LOG_PATH}/$p.ok && rm $p.ok
        echo $p is runnable
      else
        Command $p ${CommandArg} FAILED
        tail -80 $p.out
        exit 5
      fi
      [ -f "${LP3D_LOG_PATH}/$p.ok" ] && mv ${LP3D_LOG_PATH}/$p.ok ${LP3D_LOG_PATH}/$p.ok.log || :
      ( [ ! -d AitDir ] && mkdir AitDir || : ; cd AitDir && cp ../bin/$p . && \
      ./$p --appimage-extract ) >$p.out 2>&1 && mv $p.out $p.ok
        if [ -f $p.ok ]; then
           Info Extract $p succeeded
           t=appimagetool; s=mksquashfs; z=zsyncmake; a=AppRun; r=runtime
           [[ ! -f bin/$t && -f AitDir/squashfs-root/usr/bin/$t ]] && \
           cp -f AitDir/squashfs-root/usr/bin/$t bin/ || :
           [[ ! -f bin/$s && -f AitDir/squashfs-root/usr/bin/$s ]] && \
           cp -f AitDir/squashfs-root/usr/bin/$s bin/ || :
           [[ ! -f bin/$z && -f AitDir/squashfs-root/usr/bin/$z ]] && \
           cp -f AitDir/squashfs-root/usr/bin/$z bin/ || :
           [[ ! -f bin/$a && -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$a ]] && \
           cp -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$a bin/ || :
           [[ ! -f bin/$r && -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$r ]] && \
           cp -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$r bin/ || :
        else
          Error Extract $p FAILED
          tail -80 $p.out
          exit 5
        fi
      Info Setup $p completed
    else
      Error Setup $p FAILED - file was not found.
      exit 5
    fi
    SaveAppImageSetupProgress
  else
    Info $p exists. Nothing to do.
  fi

  # lcConvert
  p=lconvert
  if [ ! -L ${AppDirBuildPath}/$p ]; then
    lcv="$(which $p)"
    Info "Linking to $p to $lcv..."
    if [ -n "$lcv" ]; then
      ( cd ${AppDirBuildPath} && ln -sf $lcv $p \
      ) >$p.out 2>&1 && rm -f $p.out
      if [ ! -f $p.out ]; then
        Info  "${AppDirBuildPath}/$p linked to $lcv" || :
        Info Link $p completed
      else
        Error Link $p FAILED
        tail -80 $p.out
        exit 5
      fi
      SaveAppImageSetupProgress
    else
      Error $p was not found
      exit 5
    fi
  else
    Info $p exists. Nothing to do.
  fi
fi

# Build AppImage
cd "${AppDirBuildPath}" || exit 1
Info && Info "Building AppImage from AppDirBuildPath: ${AppDirBuildPath}..."
renderers=$(find ${AppDirBuildPath}/usr/bin/lpub3d/3rdParty -type f);
for r in $renderers; do executables="$executables -executable=$r"; done;
Info "Renderer executables: $executables"
unset QTDIR; unset QT_PLUGIN_PATH # no longer needed, superceded by AppRun
export VERSION="$LP3D_VERSION"    # used to construct the file name
./linuxdeployqt ./usr/share/applications/*.desktop $executables -bundle-non-qt-libs -verbose=2
if [[ -z "${LP3D_AI_BUILD_TOOLS}" ]]; then
  ./linuxdeployqt ./usr/share/applications/*.desktop -appimage -verbose=2
  AppImage=$(ls LPub3D*.AppImage)  # name with full path
  if [ ! -f "$AppImage" ]; then
    Error Build AppImage FAILED
    exit 9
  fi  
else
  # lpub3d.desktop
  [ -f "./usr/share/applications/lpub3d.desktop" ] && \
  cp -f ./usr/share/applications/lpub3d.desktop . || \
  Error "./usr/share/applications/lpub3d.desktop was not found"

  # lpub3d.png and .DirIcon
  [ -f "./usr/share/icons/hicolor/128x128/apps/lpub3d.png" ] && \
  cp -f ./usr/share/icons/hicolor/128x128/apps/lpub3d.png . && \
  ln -sf ./usr/share/icons/hicolor/128x128/apps/lpub3d.png .DirIcon || \
  Error "./usr/share/icons/hicolor/128x128/apps/lpub3d.png was not found"

  # libffi
  if [ ! -f "./usr/lib/libffi.so.6" ]; then
    [ ! -d "./usr/lib/" ] && mkdir -p ./usr/lib || :
    # libffi is a runtime dynamic dependency
    # see this thread for more information on the topic:
    # https://mail.gnome.org/archives/gtk-devel-list/2012-July/msg00062.html
    if [[ "${LP3D_ARCH}" == "amd64" || "${LP3D_ARCH}" == "x86_64" ]]; then
      cp "$(ldconfig -p | grep libffi.so.6 | grep x86-64 | cut -d'>' -f2 | tr -d ' ')" ./usr/lib/
    elif [[ "${LP3D_ARCH}" == "amd32" || "${LP3D_ARCH}" == "i686" ]]; then
      cp "$(ldconfig -p | grep libffi.so.6 | head -n1 | cut -d'>' -f2 | tr -d ' ')" ./usr/lib/
    elif [[ "${LP3D_ARCH}" == "arm32" || "${LP3D_ARCH}" == "armhf" ]]; then
      cp "$(ldconfig -p | grep libffi.so.6 | grep arm | grep hf | cut -d'>' -f2 | tr -d ' ')" ./usr/lib/
    elif [[ "${LP3D_ARCH}" == "arm64" || "${LP3D_ARCH}" == "aarch64" ]]; then
      cp "$(ldconfig -p | grep libffi.so.6 | grep aarch64 | cut -d'>' -f2 | tr -d ' ')" ./usr/lib/
    else
      Error "WARNING: unknown architecture, not bundling libffi"
    fi
  fi

  # set path to working directory
  cd "${WD}" || exit 1

  # AppRun
  a=AppRun
  if [ -f "${AppDirBuildPath}/$a" ]; then
    Info "Using personalized AppRun script"
  elif [ -f "bin/$a" ]; then
   ( cp -f bin/$a ${AppDirBuildPath}/usr/bin/ && \
     cd ${AppDirBuildPath} && ln -sf ./usr/bin/$a $a \
   ) >$a.out 2>&1 && rm -f $a.out
   if [ -f $a.out ]; then
     Error Copy and link $p FAILED
     tail -80 $a.out
   fi
  else
    Error "File ./bin/$a was not found"
  fi

  # zsyncmake - see note above
  a=zsyncmake
  if [ -f "bin/$a" ]; then
   ( cp -f bin/$a ${AppDirBuildPath}/usr/bin/ \
   ) >$a.out 2>&1 && rm -f $a.out
   if [ -f $a.out ]; then
     Error Copy $p FAILED
     tail -80 $a.out
   fi
  else
   Error "File ./bin/$a was not found"
  fi

  # set path to AppDir
  cd ${AppDirBuildPath} || exit 1

  # make AppImage
  AppImage=LPub3D-${LP3D_VERSION}-$(uname -m).AppImage
  if [ -f "${WD}/bin/mksquashfs" ]; then
    p=LPub3D.AppImage
    ( chmod a+x ${WD}/bin/mksquashfs && \
     ${WD}/bin/mksquashfs ${WD}/AppDir squashfs-root -root-owned -noappend \
    ) >$p.out 2>&1 && rm -f $p.out
    if [ ! -f $p.out ]; then
      AppImage=${PWD}/${AppImage}
      ( cat ${WD}/bin/runtime >> ${AppImage} ) >$p.out 2>&1 && rm -f $p.out
      [ -f $p.out ] && Error cat runtime FAILED && tail -80 $p.out && exit 5 || :
      ( cat squashfs-root >> ${AppImage} ) >$p.out 2>&1 && rm -f $p.out
      [ -f $p.out ] && Error cat squashfs-root FAILED && tail -80 $p.out && exit 5 || :
    else
      Error Run mksquashfs to make $p FAILED
      tail -80 $p.out
      exit 5
    fi
  else
    Error "${WD}/bin/mksquashfs was not found"
    exit 7
  fi
fi

#Info && Info "AppImage Dynamic Library Dependencies:" && \
#find ./ -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq && Info
declare -r t=Trace
Info && Info "Confirm AppImage..."
if [[ -f "${AppImage}" ]]; then
  CommandArg=--appimage-version
  chmod a+x ${AppImage}
  if [[ -n "${LP3D_AI_MAGIC_BYTES}" ]]; then
    Info "Patch out AppImage magic bytes"
    p=AppImagePatch
    AppImageMagicBytes="$(hexdump -Cv ${AppImage} | head -n 1 | grep '41 49 02 00')"
    if [[ -n "${AppImageMagicBytes}" ]]; then
      formaterror="Exec format error"
      ( ${AppImage} ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
      if [[ "$(grep -F "${formaterror}" $p.out 2>/dev/null)" ]]; then
        Info "AppImage magic bytes: ${AppImageMagicBytes}"
        Info "Patching out AppImage magic bytes..."
        cp -f ${AppImage} ${AppImage}.release
        dd if=/dev/zero of="${AppImage}" bs=1 count=3 seek=8 conv=notrunc
        if [[ -z "$(hexdump -Cv ${AppImage} | head -n 1 | grep '41 49 02 00')" ]]; then
          Info "Magic bytes patched: $(hexdump -Cv ${AppImage} | head -n 1)"
          ( ${AppImage} ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
          if [[ -f $p.ok ]]; then
            cat $p.ok && rm $p.ok
            AppImagePatched=LPub3D-${LP3D_VERSION}-$(uname -m).AppImage.patched
            mv -f ${AppImage} ${AppImagePatched}
            Info "${AppImage} is runnable"
          else
            Error "Command ${AppImage} ${CommandArg} after magic bytes patch FAILED"
            tail -80 $p.out
            exit 7
          fi
        else
          Error "Patch AppImage magic bytes FAILED"
          hexdump -Cv ${AppImage} | head -n 3
          exit 7
        fi
      elif [[ -f $p.ok ]]; then
        cat $p.ok && rm $p.ok
        Info "${AppImage} is runnable"
      fi
    else
      Info "Magic bytes 'AI' not found in AppImage"
      hexdump -Cv ${AppImage} | head -n 3
      ( ${AppImage} ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
      if [[ -f $p.ok ]]; then
        cat $p.ok && rm $p.ok
        Info "${AppImage} is runnable"
      else
        Error "Command ${AppImage} ${CommandArg} FAILED"
        tail -80 $p.out
        exit 7
      fi
    fi
  elif [[ -n "${LP3D_AI_BUILD_TOOLS}" ]]; then
    #CommandArg=--appimage-version
    ( ${AppImage} ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
    if [[ -f $p.ok ]]; then
      cat $p.ok && rm $p.ok
      Info "${AppImage} is runnable"
    else
      Error "Command ${AppImage} ${CommandArg} FAILED"
      tail -80 $p.out
      exit 7
    fi
  fi

  # Rename AppImage and move to $PWD
  AppImageExtension=${AppImage##*-}
  AppImageName=LPub3D-${LP3D_APP_VERSION_LONG}-${AppImageExtension}
  [[ -f ${AppImage}.release ]] && \
  mv -f ${AppImage}.release ${AppImageName} || \
  mv -f ${AppImage} ${AppImageName}

  # Crete AppImage sha512 file
  AppImage=${AppImageName}
  echo -n " CreateLinux: Creating hash file for ${AppImage}..."
  ( sha512sum "${AppImage}" > "${AppImage}.sha512" ) >$t.out 2>&1 && rm $t.out
  [[ -f $t.out ]] && \
  echo && Error "Failed to create sha512 file." && tail -20 $t.out || echo "Ok."
  Info "Application package....: ${AppImage}"
  Info "Package path...........: ${PWD}/${AppImage}"
  if [[ -f "${AppImage}.sha512" ]]; then
    Info "AppImage build completed successfully."
  else
    Error "AppImage build completed but the .sha512 file was not found."
  fi
else
  Error "AppImage file not found. ${AppImage} build FAILED."
  exit 8
fi

cd "${WD}" || exit 1

# Check AppImage build
[[ -f "${AppImagePatched}" ]] && \
AppImageCheck=$(find ./ -name ${AppImagePatched} -type f) || \
AppImageCheck=$(find ./ -name ${AppImageName} -type f)
if [[ -f "${AppImageCheck}" ]]; then
  SOURCE_DIR=${WD}
  LP3D_CHECK_STATUS="--version --app-paths"
  mkdir -p appImage_Check && cp -f ${AppImageCheck} appImage_Check/${AppImageName} && \
  Info "$(ls ./appImage_Check/*.AppImage) copied to check folder."
  if [[ -z "$(which fusermount)" || -n "${LP3D_AI_EXTRACT_PAYLOAD}" ]]; then
    ( cd appImage_Check && ./${AppImageName} --appimage-extract \
    ) >$p.out 2>&1 && rm -f $p.out
    if [[ ! -f $p.out ]]; then
      Info "Build check extracted AppImage payload..."
      LP3D_BUILD_OS=
      LPUB3D_EXE="appImage_Check/squashfs-root/usr/bin/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
    else
      Error "Build check cannot proceed. Extract ${AppImageName} FAILED"
      tail -80 $p.out
    fi
  else
    Info "Build check AppImage..."
    LP3D_BUILD_OS="appimage"
    LPUB3D_EXE="appImage_Check/${AppImageName}"
    LP3D_RENDERER_DIR=${AppDirBuildPath}/usr/bin/lpub3d/3rdParty
    test -f ${LP3D_RENDERER_DIR}/ldview-4.6/bin/ldview && LDVIEW_EXE=ldview || :
    test -f ${LP3D_RENDERER_DIR}/ldglite-1.3/bin/ldglite && LDGLITE_EXE=ldglite || :
    test -f ${LP3D_RENDERER_DIR}/lpub3d_trace_cui-3.8/bin/lpub3d_trace_cui && POVRAY_EXE=lpub3d_trace_cui || :    
  fi
  if [[ -n "${LPUB3D_EXE}" ]]; then
    set +x && source ${SOURCE_DIR}/builds/check/build_checks.sh && set -x
  fi
  if [[ "${AppImageCheck}" == *".patched"* ]]; then
    rm -fr ${AppImageCheck}
  fi
else
  Error "Build check cannot proceed. ${AppImageCheck} not found."
fi

# Move AppImage build content to output
Info "Move AppImage assets to output folder..."
mv -f ${AppDirBuildPath}/*.log /out/ 2>/dev/null || :
mv -f ./*.log /out/ 2>/dev/null || :
mv -f ~/*.log /out/ 2>/dev/null || :
mv -f ~/*_assets.tar.gz /out/ 2>/dev/null || :

if [[ ! "${BUILD_OPT}" = "verify" ]]; then
  echo -n " CreateLinux: Move AppImage build to output folder..."
  ( cd ${WD}/AppDir && \
    for file in *.AppImage*; do mv -f "${file}" /out/; done \
  ) >$t.out 2>&1 && rm $t.out
  [[ -f $t.out ]] && \
  echo && Error "Move build package to output failed." && \
  tail -80 $t.out || echo "Ok."
fi

exit 0
