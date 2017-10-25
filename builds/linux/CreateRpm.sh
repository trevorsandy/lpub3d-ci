#!/bin/bash
# Trevor SANDY
# Last Update October 25 2017
# To run:
# $ chmod 755 CreateRpm.sh
# $ ./CreateRpm.sh

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
BUILD_DATE=`date "+%Y%m%d"`
CHANGE_DATE=`date "+%a %b %d %Y"`
LP3D_TARGET_ARCH=`uname -m`

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

echo "1. create RPM build working directories"
if [ ! -d rpmbuild ]
then
    mkdir rpmbuild
fi
cd rpmbuild
for DIR in {BUILD,RPMS,SRPMS,SOURCES,SPECS}
do
    if [ ! -d "${DIR}" ]
    then
        mkdir "${DIR}"
    fi
done
cd SOURCES

echo "2. download source"
git clone https://github.com/trevorsandy/${LPUB3D}.git

echo "3. source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

WORK_DIR=${LPUB3D}-git
mv ${LPUB3D} ${WORK_DIR}

echo "4. create tarball ${WORK_DIR}.tar.gz using folder ${WORK_DIR}"
tar -czvf ${WORK_DIR}.tar.gz \
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

echo "5. copy ${LPUB3D}.git.version to SOURCES"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}.spec.git.version .

echo "6. copy xpm icon to SOURCES/"
cp -f ${WORK_DIR}/mainApp/images/lpub3d.xpm .

echo "7. copy spec to SPECS/"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}.spec ../SPECS

echo "8. get LDraw archive libraries"
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -q -O lpub3dldrawunf.zip http://www.ldraw.org/library/unofficial/ldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -q http://www.ldraw.org/library/updates/complete.zip
fi

echo "8. build the RPM package (success = 'exit 0')"
cd ../SPECS
rpmbuild --define "_topdir ${WORK_DIR}/rpmbuild" -v -ba ${LPUB3D}.spec

cd ../RPMS/${LP3D_TARGET_ARCH}
DISTRO_FILE=`ls ${LPUB3D}-${APP_VERSION}*.rpm`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "9. create update and download packages"
    IFS=- read NAME VERSION ARCH_EXTENSION <<< ${DISTRO_FILE}

    cp -f ${DISTRO_FILE} "${LPUB3D}-${APP_VERSION_LONG}_${ARCH_EXTENSION}"
    echo "    Download package: ${LPUB3D}_${APP_VERSION_LONG}_${ARCH_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
    echo "      Update package: LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
else
    echo "9. package ${DISTRO_FILE} not found."
fi

echo "10. cleanup cloned ${LPUB3D} repository from SOURCES/ and BUILD/"
rm -rf ../../SOURCES/${WORK_DIR} ../../BUILD/${WORK_DIR}

echo " DEBUG Package files: `find $PWD`"
echo "$ME Finished!"
#mv $LOG "${CWD}/rpmbuild/$ME.log"
