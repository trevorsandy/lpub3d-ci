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
    mkdir pkgbuild
    mkdir pkgbuild/upstream
fi
cd pkgbuild/upstream
WORK_DIR=${LPUB3D}-git

echo "2. download source"
git clone https://github.com/trevorsandy/${LPUB3D}.git
mv ${LPUB3D} ${WORK_DIR}

_PRO_FILE_PWD_=`pwd`/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

echo "4. create tarball"
tar -czvf ../${WORK_DIR}.tar.gz \
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

echo "5. copy PKGBUILD"
cp -f ${WORK_DIR}/builds/linux/obs/PKGBUILD ../
cd ../

echo "6. get LDraw archive libraries"
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -N -O lpub3dldrawunf.zip http://www.ldraw.org/library/unofficial/ldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -N http://www.ldraw.org/library/updates/complete.zip
fi

echo "7. build application package"
makepkg -s

DISTRO_FILE=`ls ${LPUB3D}-${APP_VERSION}*.pkg.tar.xz`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "8. create update and download packages"
    IFS=- read NAME VERSION BUILD ARCH_EXTENSION <<< ${DISTRO_FILE}
    cp -f ${DISTRO_FILE} "${LPUB3D}-${APP_VERSION_LONG}_${BUILD}_${ARCH_EXTENSION}"
    echo "    Download package: ${LPUB3D}-${APP_VERSION_LONG}_${BUILD}_${ARCH_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
    echo "      Update package: LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
else
    echo "8. package ${DISTRO_FILE} not found."
fi
echo "12. Package files: `find $PWD`"
echo "$ME Finished!"
# mv $LOG "${CWD}/pkgbuild/$ME.log"
