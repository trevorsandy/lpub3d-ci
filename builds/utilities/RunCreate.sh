#!/bin/bash
# shellcheck source=/dev/null
# Trevor SANDY
# Last Update June 12, 2025
# Copyright (C) 2024 - 2025 by Trevor SANDY

LOCAL=${LOCAL:-true}
DOCKER=${DOCKER:-true}
LPUB3D=${LPUB3D:-lpub3d-ci}
PRESERVE=${PRESERVE:-true}
UPDATE_SH=${UPDATE_SH:-true}
XSERVER=${XSERVER:-false}
LP3D_ARCH=${LP3D_ARCH:-amd64}
DEB_EXT=${DEB_EXTENSION:-$LP3D_ARCH.Deb}
LOCAL_PATH=${LOCAL_RESOURCE_PATH:-${HOME}/resources}
LOG_PATH=${LP3D_LOG_PATH:-${HOME}/resources/logs}
WORK_PATH=${WORK_PATH:-${HOME}}
SKIP_CLEANUP=${SKIP_CLEANUP:-0}
EXPORT_ARTIFACTS=${EXPORT_ARTIFACTS:-0}

function ShowHelp() {
    echo
    echo "Build LPub3D Linux distribution"
    echo
    echo "You can run this script from the Docker image user HOME"
    echo
    echo "[optoinal arguments]:"
    echo " - Deb - run CreateDeb.sh build script"
    echo " - Rpm - run CreateRpm.sh build script"
    echo " - Pkg - run CreatePkg.sh build script"
    echo "[environment variable options]:"
    echo " - LOCAL=$LOCAL - local build - use local versus download renderer and library source"
    echo " - DOCKER=$DOCKER - using Docker image"
    echo " - LPUB3D=$LPUB3D - repository name"
    echo " - PRESERVE=$PRESERVE - clone remote repository"
    echo " - UPDATE_SH=$UPDATE_SH - overwrite Create script when building in local Docker"
    echo " - XSERVER=$XSERVER - use Docker host XMing/XSrv XServer"
    echo " - LP3D_ARCH=$LP3D_ARCH - set build architecture"
    echo " - DEB_EXTENSION=$DEB_EXT - distribution file suffix"
    echo " - LOCAL_RESOURCE_PATH=$LOCAL_RESOURCE_PATH - path (or Docker volume mount) where lpub3d and renderer sources and library archives are located"
    echo " - LP3D_LOG_PATH=$LP3D_LOG_PATH - log path"
    echo " - WORK_PATH=$WORK_PATH - work directory"
    echo " - EXPORT_ARTIFACTS=$EXPORT_ARTIFACTS - export build artifacts"
    echo " - SKIP_CLEANUP=$SKIP_CLEANUP - do not clean work directory after build"
    echo "NOTE: elevated access required for apt-get install, execute with sudo"
    echo "or enable user with no password sudo if running noninteractive - see"
    echo "docker-compose/dockerfiles for script example of sudo, no password user."
    echo
    echo "To run:"
    echo "chmod a+x $0"
    echo "env [option option ...] ./builds/utilities/$0 [Deb|Pkg|Rpm]"
    echo
    echo "Configure (set SCRIPT_DIR), copy and paste the following 3 lines"
    echo "in the command console."
    echo
    echo "RUN_CREATE=$0"
    echo "RUN_CREATE_DIR=\${SCRIPT_DIR:-$HOME/resources/lpub3d-ci}"
    echo "cd ~/ && cp -f \${RUN_CREATE_DIR}/builds/utilities/$RUN_CREATE . \\"
    echo "&& chmod a+x $RUN_CREATE && ./$RUN_CREATE"
    echo
}

function ShowOptions ()
{
    echo
    echo "LPUB3D...........[${LPUB3D}]"
    echo "OPTION...........[$([ "$opt" = "Deb" ] && echo Debian || ([ "$opt" = "Pkg" ] && echo Arch || echo RedHat))]"
    echo "LOCAL............[${LOCAL}]"
    echo "DOCKER...........[${DOCKER}]"
    echo "PRESERVE.........[${PRESERVE}]"
    echo "UPDATE_SH........[${UPDATE_SH}]"
    echo "XSERVER..........[${XSERVER}]"
    echo "LP3D_ARCH........[${LP3D_ARCH}]"
    echo "SKIP_CLEANUP.....[$([ "${SKIP_CLEANUP}" -eq 1 ] && echo "Yes" || echo "No")]"
    echo "EXPORT_ARTIFACTS.[$([ "${EXPORT_ARTIFACTS}" -eq 1 ] && echo "Yes" || echo "No")]"
    [ "$opt" = "Deb" ] && \
    echo "DEB_EXTENSION....[${DEB_EXT}]" || :
    echo "WORK_PATH........[${WORK_PATH}]"
    echo "LOCAL_PATH.......[${LOCAL_PATH}]"
    echo "LOG_PATH.........[${LOG_PATH}]"
    echo
}

function ExportArtifacts() {
  if ! test -d "$buildFolder"; then echo "Build folder '$buildFolder' is invalid." && return 1; fi
  if ! test -d /buildpkg; then echo "Artifact folder '/buildpkg' is invalid." && return 1; fi
  fileTypeList="$fileTypeList .sha512 .log .sh assets.tar.gz"
  for fileType in $fileTypeList; do
    case $fileType in
    assets.tar.gz)
      files=$(find "$WORK_PATH" -maxdepth 1 -name "*$fileType" -type f)
      for file in $files; do sudo cp -f "$file" /buildpkg/; done
    ;;
    .log|.sh)
      f1=$(find "$buildFolder" -maxdepth 1 -name "*$fileType" -type f)
      f2=$(find "$WORK_PATH" -maxdepth 1 -name "*$fileType" -type f)
      if test "$1" = "Rpm"; then f3=$(find / -maxdepth 1 -name "*$fileType" -type f); fi
      files="$(echo "$f1 $f2 $f3" | tr " " "\n")"
      for file in $files; do sudo cp -f "$file" /buildpkg/; done
    ;;
    *)
      f4=$(find "$buildFolder" -maxdepth 1 -name "*$fileType" -type f)
      if test "$1" = "Rpm"; then f5=$(find "$buildFolder/../RPMS" -maxdepth 1 -name "*$fileType" -type f); fi
      if test "$1" = "Pkg"; then f6=$(find "$buildFolder/.." -maxdepth 1 -name "*$fileType" -type f); fi
      files="$(echo "$f4 $f5 $f6" | tr " " "\n")"
      for file in $files; do sudo cp -f "$file" /buildpkg/; done
    ;;
    esac
  done
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -\?|-h|--help) ShowHelp; exit 0 ;;
        Deb|Pkg|Rpm) break ;;
        *) echo "Unknown parameter passed: '$1'. Use -? to show help."; exit 1 ;;
    esac
    shift
done

echo && echo "Run $0 at $PWD..."

if ! test "$1"; then
  os=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo "$ID")
  case $os in
    ubuntu|debian) opt=Deb ;;
    fedora|suse|centos) opt=Rpm ;;
    arch) opt=Pkg ;;
    *) echo "Could not identify the OS flavour as Debian, Redhat or Arch derived" && exit 1 ;;
  esac
else
  opt=$1
fi

case $opt in
  Deb|Rpm|Pkg) : ;;
  *) echo "Invalid option specified. Valid argument is Deb, Rpm, or Pkg." && exit 1 ;;
esac

ShowOptions

# Confirmation
echo
sleep 1s && read -p "  Do you want to continue (y/n)? " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
  # handle exits from shell or function but don't exit interactive shell
  [[ "$0" == "$BASH_SOURCE" ]] && rm -f ${LOG} && exit 1 || return 1
fi

createScript="Create$opt.sh"

if test "$opt" != "Deb"; then unset DEB_EXT; fi

declare -r l=run

case $opt in
  Deb) buildFolder=$WORK_PATH/debbuild ;;
  Rpm) buildFolder=$WORK_PATH/rpmbuild ;;
  Pkg) buildFolder=$WORK_PATH/pkgbuild ;;
esac

if test "${PRESERVE}" = "true"; then
  if [ "$SKIP_CLEANUP" -eq 1 ]; then
    echo && echo -n "Skipping clean up $buildFolder folder..."
  else
    echo && echo -n "Cleaning up $buildFolder folder..."
    case $opt in
      Deb)
      if test -d "$buildFolder/SOURCES"; then
        dirs="ldglite ldview povray"
        ( cd "$buildFolder" && \
          for dir in $dirs; do if test -d "$dir"; then mv "$dir" SOURCES; fi; done && \
          mv SOURCES "$WORK_PATH" && rm -rf ./* && mv "$WORK_PATH/SOURCES" . && \
          for dir in $dirs; do if test -d "SOURCES/$dir"; then mv "SOURCES/$dir" .; fi; done && \
          cd "$WORK_PATH" && rm -rf ./*.log ./*assets.tar.gz ) >$l.out 2>&1 && rm $l.out
      fi
      ;;
      Rpm)
      if test -d "$buildFolder/SOURCES"; then
        ( cd "$buildFolder" && \
          for dir in BUILDROOT RPMS SRPMS SPECS; do if test -d "$dir"; then rm -rf $dir; fi; done && \
          if test -d "BUILD"; then rm -rf BUILD/*.log BUILD/lpub3d*; fi && \
          cd SOURCES && if test -f "${LPUB3D}-git.tar.gz"; then tar -xf lpub3d-ci-git.tar.gz; fi && \
          if test -d "${LPUB3D}-git"; then mv "${LPUB3D}-git" "${LPUB3D}"; fi && \
          rm -rf "${LPUB3D}-git.tar.gz" ) >$l.out 2>&1 && rm $l.out
      fi
      ;;
      Pkg)
      if test -d "$buildFolder/src"; then
        ( cd "$WORK_PATH" && rm -rf ./*.log ./*assets* && \
          cd "$buildFolder" && if test -f "PKGBUILD"; then rm -rf PKGBUILD; fi && rm -rf pkg ./*.zst* ./*.out lpub3d*  && \
          cd "$buildFolder/src" && rm -rf ./*.log lpub3d* ./*.zip ) >$l.out 2>&1 && rm $l.out
      fi
      ;;
    esac
  fi

  [ -f $l.out ] && \
  echo "ERROR - Could not clean up $buildFolder folder." && \
  tail -80 $l.out && exit 1 || echo "Ok."

  if test "${UPDATE_SH}" = "true"; then
    cp -rf "${LOCAL_PATH}/${LPUB3D}/builds/linux/${createScript}" . && \
    dos2unix "${createScript}" && chmod a+x "${createScript}"
  fi
else
  echo
  if test -d "$buildFolder"; then rm -rf "$buildFolder"; fi
  cp -f "${LOCAL_PATH}/${LPUB3D}/builds/linux/${createScript}" .
  dos2unix "${createScript}" && chmod a+x "${createScript}"
fi

if [ ! -d "${LOG_PATH}" ]; then
  echo -n "Creating log path: ${LOG_PATH}..."
  (cd ~/ && mkdir -p "$LOG_PATH") >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
fi
if [ ! -d "/out" ]; then
  echo -n "Linking /out to ${LOG_PATH}..."
  (cd / && sudo ln -sf "$LOG_PATH" out) >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
fi

env \
LOCAL="${LOCAL}" \
DOCKER="${DOCKER}" \
LPUB3D="${LPUB3D}" \
PRESERVE="${PRESERVE}" \
UPDATE_SH="${UPDATE_SH}" \
XSERVER="${XSERVER}" \
LP3D_ARCH="${LP3D_ARCH}" \
DEB_EXTENSION="${DEB_EXT}" \
LOCAL_RESOURCE_PATH="${LOCAL_PATH}" \
LP3D_LOG_PATH="${LOG_PATH}" \
LP3D_CPU_CORES="$(nproc)" \
LP3D_3RD_DIST_DIR="lpub3d_linux_3rdparty" \
"./${createScript}"

status=$?
if [ ${status} -eq 0 ]; then
  case $opt in
    Deb)
      fileTypeList=".deb .ddeb .dsc .changes"
    ;;
    Rpm)
      fileTypeList=".rpm"
      buildFolder=$buildFolder/BUILD
    ;;
    Pkg)
      fileTypeList=".pkg.tar.zst"
      buildFolder=$buildFolder/src
    ;;
  esac

  if [ "$EXPORT_ARTIFACTS" -eq 1 ]; then
    echo -n "Exporting artifacts..." && \
    ExportArtifacts "$opt" && echo "Done."
  fi
  echo "----------------------------------------------------"
fi
