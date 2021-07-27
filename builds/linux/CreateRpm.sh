#!/bin/bash
# Trevor SANDY
# Last Update July 26, 2021
# Copyright (c) 2017 - 2021 by Trevor SANDY
# To run:
# $ chmod 755 CreateDeb.sh
# $ [options] && ./builds/linux/CreateRpm.sh
# [options]:
#  - export DOCKER=true (if using Docker image)
#  - export OBS=false (if building locally)
# NOTE: elevated access required for apt-get install, execute with sudo
# or enable user with no password sudo if running noninteractive - see
# docker-compose/dockerfiles for script example of sudo, no password user.

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  [ -n "${LP3D_ARCH}" ] && ME="${ME} for (${LP3D_ARCH})" || ME="${ME} for (amd64)"
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
LP3D_TARGET_ARCH=`uname -m`
export OBS=false # OpenSUSE Build Service flag must be set for CreateRenderers.sh - called by lpub3d.spec

echo "Start $ME execution at $CWD..."

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d-ci}"
echo "   LPUB3D SOURCE DIR......${LPUB3D}"

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# logging stuff - increment log file name
f="${0##*/}"; f="${f%.*}"; [ -n "${LP3D_ARCH}" ] && f="${f}-${LP3D_ARCH}" || f="${f}-amd64"
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
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

echo "1. create RPM build working directories in rpmbuild/..."
if [ ! -d rpmbuild ]
then
    mkdir rpmbuild
fi

cd rpmbuild

BUILD_DIR=$PWD
for DIR in {BUILD,RPMS,SRPMS,SOURCES,SPECS}
do
    if [ ! -d "${DIR}" ]
    then
        mkdir "${DIR}"
    fi
done
cd ${BUILD_DIR}/SOURCES

if [ -d "/in" ]; then
    echo "2. copy ${LPUB3D} source to SOURCES/..."
    cp -rf /in/. .
else
    echo "2. download ${LPUB3D} source to SOURCES/..."
    git clone https://github.com/trevorsandy/${LPUB3D}.git
fi

# For Docker build, check if there is a tag after the last commit
if [ "$DOCKER" = "true" ]; then
   # Setup git command
   GIT_CMD="git --git-dir $PWD/${LPUB3D}/.git --work-tree $PWD/${LPUB3D}"
   # Update source
   $GI[ "${CI}" = "true" ] && $GIT_CMD pull || :
   #1. Get the latest version tag - check across all branches
   BUILD_TAG=$($GIT_CMD describe --tags --match v* $($GIT_CMD rev-list --tags --max-count=1) 2> /dev/null)
   if [ -n "$BUILD_TAG" ]; then
       #2. Get the tag datetime
       BUILD_TAG_TIME=$($GIT_CMD log -1 --format=%ai $BUILD_TAG 2> /dev/null)
       #3. Get the latest commit datetime from the build branch
       GIT_COMMIT_TIME=$($GIT_CMD log -1 --format=%ai 2> /dev/null)
           #4. If tag is newer than commit, check out the tag
       if [ $(date -d "$GIT_COMMIT_TIME" +%s) -lt $(date -d "$BUILD_TAG_TIME" +%s) ]; then
           echo "2a. checking out build tag $BUILD_TAG..."
           $GIT_CMD checkout -qf $BUILD_TAG
       fi
   fi
fi

echo "3. source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

echo "4. move ${LPUB3D}/ to ${LPUB3D}-git/ in SOURCES/..."
WORK_DIR=${LPUB3D}-git
mv -f ${LPUB3D} ${WORK_DIR}

echo "6. copy lpub3d.xpm icon to SOURCES/"
cp -f ${WORK_DIR}/mainApp/resources/lpub3d.xpm .

echo "7. copy ${LPUB3D}.spec to SPECS/"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}.spec ${BUILD_DIR}/SPECS

echo "8. copy ${LPUB3D}-rpmlintrc to SPECS/"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}-rpmlintrc ${BUILD_DIR}/SPECS

echo "9. download LDraw archive libraries to SOURCES/..."
if [ ! -f lpub3dldrawunf.zip ]
then
    curl $curlopts http://www.ldraw.org/library/unofficial/ldrawunf.zip -o lpub3dldrawunf.zip
fi
if [ ! -f complete.zip ]
then
    curl -O $curlopts http://www.ldraw.org/library/updates/complete.zip
fi
if [ ! -f tenteparts.zip ]
then
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip
fi
if [ ! -f vexiqparts.zip ]
then
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip
fi

# file copy and downloads above must happen before we make the tarball
echo "11. create tarball ${WORK_DIR}.tar.gz from ${WORK_DIR}/..."
tar -czf ${WORK_DIR}.tar.gz \
        --exclude="${WORK_DIR}/builds/linux/standard" \
        --exclude="${WORK_DIR}/builds/windows" \
        --exclude="${WORK_DIR}/builds/macx" \
        --exclude="${WORK_DIR}/lclib/tools" \
        --exclude="${WORK_DIR}/.travis.yml" \
        --exclude="${WORK_DIR}/.gitattributes" \
        --exclude="${WORK_DIR}/LPub3D.pro.user" \
        --exclude="${WORK_DIR}/README.md" \
        --exclude="${WORK_DIR}/_config.yml" \
        --exclude="${WORK_DIR}/.gitignore" \
        --exclude="${WORK_DIR}/snapcraft.yaml" \
        --exclude="${WORK_DIR}/appveyor.yml" ${WORK_DIR}

cd ${BUILD_DIR}/SPECS
echo "12. Check ${LPUB3D}.spec..."
source /etc/os-release && if [ "$ID" = "fedora" ]; then sed 's/Icon: lpub3d.xpm/# Icon: lpub3d.xpm remarked - fedora does not like/' -i "${BUILD_DIR}/SPECS/${LPUB3D}.spec"; fi
rpmlint ${LPUB3D}.spec

if [ "${GITHUB}" = "true" ]; then
    echo "13. Skipping install ${LPUB3D} build dependencies."
else
    echo "13. add ${LPUB3D} build dependencies [requires elevated access - sudo]..."
    sudo dnf builddep -y ${LPUB3D}.spec
fi

echo "    DEBUG - Checking for libXext..."
ldconfig -p | grep libXext
echo "    DEBUG END"

echo "14-1. build the RPM package..."
rpmbuild --define "_topdir ${BUILD_DIR}" -vv -bb ${LPUB3D}.spec

cd ${BUILD_DIR}/RPMS/${LP3D_TARGET_ARCH}
DISTRO_FILE=`ls ${LPUB3D}-${LP3D_APP_VERSION}*.rpm`
if [ -f ${DISTRO_FILE} ]
then
    echo "14-2. Build package: $PWD/${DISTRO_FILE}"

    if [ "${LP3D_QEMU}" = "true" ]; then
        if [ -n "$LP3D_PRE_PACKAGE_CHECK" ]; then
            echo "15-1. Pre-package build check LPub3D..."
            export LP3D_BUILD_OS=
            export SOURCE_DIR=$(readlink -f ../../SOURCES/${WORK_DIR})
            export LP3D_CHECK_LDD="1"
            export LP3D_CHECK_STATUS="--version --app-paths"
            case ${LP3D_ARCH} in
                "aarch64"|"arm64")
                    LP3D_BUILD_ARCH="64bit_release" ;;
                *)
                    LP3D_BUILD_ARCH="32bit_release" ;;
            esac
            export LPUB3D_EXE="${BUILD_DIR}/BUILD/${WORK_DIR}/mainApp/${LP3D_BUILD_ARCH}/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
            (cd ${SOURCE_DIR} && chmod a+x builds/check/build_checks.sh && ./builds/check/build_checks.sh)
        else
            echo "15-1. Building in QEMU, skipping build check."
        fi
    else
        echo "15-1. Build check ${DISTRO_FILE}"
        if [ ! -f "/usr/bin/update-desktop-database" ]; then
            echo "      Program update-desktop-database not found. Installing..."
            sudo dnf install -y desktop-file-utils
        fi

        # Install package - here we use the distro file name e.g. LPub3D-2.3.8.1566-1.fc26.x86_64.rpm
        echo "      15-1. Build check install ${LPUB3D}..."
        yes | sudo rpm -Uvh ${DISTRO_FILE}
        # Check if exe exist - here we use the executable name e.g. lpub3d22
        LPUB3D_EXE=lpub3d${LP3D_APP_VER_SUFFIX}
        SOURCE_DIR=../../SOURCES/${WORK_DIR}
        if [ -f "/usr/bin/${LPUB3D_EXE}" ]; then
            # Check commands
            LP3D_CHECK_LDD="1"
            source ${SOURCE_DIR}/builds/check/build_checks.sh
            echo "      15-1. Build check uninstall ${LPUB3D}..."
            # Cleanup - here we use the package name e.g. lpub3d-ci
            yes | sudo rpm -ev ${LPUB3D}
        else
            echo "15-1. Build check failed - /usr/bin/${LPUB3D_EXE} not found."
        fi
    fi

    echo "15-2. check ${RPM_EXTENSION} packages..."
    rpmlint ${DISTRO_FILE} ${LPUB3D}-${LP3D_APP_VERSION}*.rpm

    echo "15-3. create LPub3D ${RPM_EXTENSION} distribution packages..."
    RPM_EXTENSION="${DISTRO_FILE##*-}"
    LP3D_RPM_FILE="LPub3D-${LP3D_APP_VERSION_LONG}-${RPM_EXTENSION}"
    mv -f ${DISTRO_FILE} "${LP3D_RPM_FILE}"
    if [ -f "${LP3D_RPM_FILE}" ]; then
        if [ "${TRAVIS}" != "true" ]; then
            echo "15-4. Creating ${LP3D_DEB_FILE}.sha512 hash file..."
            sha512sum "${LP3D_RPM_FILE}" > "${LP3D_RPM_FILE}.sha512" || \
            echo "15-4. ERROR - Failed to create hash file ${LP3D_RPM_FILE}.sha512"
        fi
        if [ "${LP3D_QEMU}" = "true" ]; then
            echo "15-5. Moving ${LP3D_BASE} ${LP3D_ARCH} build assets and logs to output folder..."
            mv -f ${LP3D_RPM_FILE}* /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/BUILD/*.log 2>/dev/null || :
            mv -f ${SOURCE_DIR}/*.log /out/ 2>/dev/null || :
            mv -f ${CWD}/*.log /out/ 2>/dev/null || :
            mv -f ./*.log /out/ 2>/dev/null || :
            mv -f ~/*.log /out/ 2>/dev/null || :
        fi
        echo
        echo "    Distribution package.: ${LP3D_RPM_FILE}"
        echo "    Package path.........: $PWD/${LP3D_RPM_FILE}"
    else
        echo "15-3. ERROR - file ${LP3D_RPM_FILE} not found: "
    fi
else
    echo "14-2. ERROR - package ${DISTRO_FILE} not found."
fi

if [ "${GITHUB}" != "true" ]; then
    echo "16. cleanup cloned ${LPUB3D} repository from SOURCES/ and BUILD/..."
    rm -rf ${BUILD_DIR}/SOURCES/${WORK_DIR} ${BUILD_DIR}/BUILD/${WORK_DIR}
fi

exit 0
