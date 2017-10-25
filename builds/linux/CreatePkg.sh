#!/bin/bash
# Trevor SANDY
# Last Update October 25 2017
# To run:
# $ chmod 755 CreatePkg.sh
# $ ./CreatePkg.sh

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
BUILD_DATE=`date "+%Y%m%d"`

echo "Start $ME execution at $CWD..."

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d-ci}"
echo "   LPUB3D SOURCE DIR......${LPUB3D}"

# logging stuff
# increment log file name
f="${CWD}/$ME"
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

echo "1. create PKG working directories"
if [ ! -d pkgbuild ]
then
    mkdir -p pkgbuild/upstream
fi
cd pkgbuild/upstream

echo "2. download source"
git clone https://github.com/trevorsandy/${LPUB3D}.git

_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

WORK_DIR=${LPUB3D}-git
mv ${LPUB3D} ${WORK_DIR}

echo "3. create tarball ${WORK_DIR}.tar.gz using folder ${WORK_DIR}"
tar -czf ../${WORK_DIR}.tar.gz \
        --exclude="${WORK_DIR}/builds/linux/standard" \
        --exclude="${WORK_DIR}/builds/windows" \
        --exclude="${WORK_DIR}/builds/macx" \
        --exclude="${WORK_DIR}/lc_lib/tools" \
        --exclude="${WORK_DIR}/.travis.yml" \
        --exclude="${WORK_DIR}/.gitattributes" \
        --exclude="${WORK_DIR}/LPub3D.pro.user" \
        --exclude="${WORK_DIR}/README.md" \
        --exclude="${WORK_DIR}/_config.yml" \
        --exclude="${WORK_DIR}/.gitignore" \
        --exclude="${WORK_DIR}/appveyor.yml" ${WORK_DIR}

echo "4. copy PKGBUILD"
cp -f ${WORK_DIR}/builds/linux/obs/PKGBUILD ../
cd ../

echo "5. get LDraw archive libraries"
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -q -O lpub3dldrawunf.zip http://www.ldraw.org/library/unofficial/ldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -q http://www.ldraw.org/library/updates/complete.zip
fi

echo "6. build application package"
makepkg -s

DISTRO_FILE=`ls ${LPUB3D}-${LP3D_APP_VERSION}*.pkg.tar.xz`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "7. create update and download packages"
    IFS=- read NAME PKG_VERSION BUILD PKG_EXTENSION <<< ${DISTRO_FILE}
    cp -f ${DISTRO_FILE} "${LPUB3D}-${LP3D_APP_VERSION_LONG}_${BUILD}_${PKG_EXTENSION}"
    echo "    Download package: ${LPUB3D}-${LP3D_APP_VERSION_LONG}_${BUILD}_${PKG_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${PKG_VERSION}_${PKG_EXTENSION}"
    echo "      Update package: LPub3D-UpdateMaster_${PKG_VERSION}_${PKG_EXTENSION}"
else
    echo "7. package ${DISTRO_FILE} not found."
fi

#echo " DEBUG Package files: `find $PWD`"
echo "$ME Finished!"
# mv $LOG "${CWD}/pkgbuild/$ME.log"
