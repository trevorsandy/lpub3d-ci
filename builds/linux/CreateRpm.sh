#!/bin/bash
# Trevor SANDY
# Last Update November 02 2017
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

echo "2. download ${LPUB3D} source to SOURCES/..."
git clone https://github.com/trevorsandy/${LPUB3D}.git

echo "3. source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

echo "4. move ${LPUB3D}/ to ${LPUB3D}-git/ in SOURCES/..."
WORK_DIR=${LPUB3D}-git
mv -f ${LPUB3D} ${WORK_DIR}

echo "6. copy lpub3d.xpm icon to SOURCES/"
cp -f ${WORK_DIR}/mainApp/images/lpub3d.xpm .

echo "7. copy ${LPUB3D}.spec to SPECS/"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}.spec ${BUILD_DIR}/SPECS

echo "8. copy ${LPUB3D}-rpmlintrc to SPECS/"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}-rpmlintrc ${BUILD_DIR}/SPECS

echo "9. download LDraw archive libraries to SOURCES/..."
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -q -O lpub3dldrawunf.zip http://www.ldraw.org/library/unofficial/ldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -q -O complete.zip http://www.ldraw.org/library/updates/complete.zip
fi
# echo "10. download lpub3d_linux_3rdparty repository as tar.gz archive to SOURCES/..."
# if [ ! -f lpub3d_linux_3rdparty.tar.gz ]
# then
#      wget -q -O lpub3d_linux_3rdparty.tar.gz https://github.com/trevorsandy/lpub3d_linux_3rdparty/archive/master.tar.gz
# fi
echo "10. source CreateRenderers from  SOURCES/..."
env OBS=false source ${WORK_DIR}/builds/utilities/CreateRenderers.sh

# file copy and downloads above must happen before we make the tarball
echo "11. create tarball ${WORK_DIR}.tar.gz from ${WORK_DIR}/..."
tar -czf ${WORK_DIR}.tar.gz \
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

cd ${BUILD_DIR}/SPECS
echo "12. download build dependenvies..."
source /etc/os-release && if [ "$ID" = "fedora" ]; then sed 's/Icon: lpub3d.xpm/# Icon: lpub3d.xpm remarked - fedora does not like/' -i "${BUILD_DIR}/SPECS/${LPUB3D}.spec"; fi
# check the spec
rpmlint ${LPUB3D}.spec
# add lpub3d build dependencies
dnf builddep -y ${LPUB3D}.spec

echo "13. build the RPM package..."
rpmbuild --define "_topdir ${BUILD_DIR}" -vv -bb ${LPUB3D}.spec

cd ${BUILD_DIR}/RPMS/${LP3D_TARGET_ARCH}
DISTRO_FILE=`ls ${LPUB3D}-${LP3D_APP_VERSION}*.rpm`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "14. check rpm packages..."
    rpmlint ${DISTRO_FILE} ${LPUB3D}-${LP3D_APP_VERSION}*.rpm

    echo "15. create update and download packages..."
    #IFS=- read RPM_NAME RPM_VERSION RPM_EXTENSION <<< ${DISTRO_FILE}
    RPM_EXTENSION="${DISTRO_FILE##*-}"

    cp -f ${DISTRO_FILE} "LPub3D-${LP3D_APP_VERSION_LONG}_${RPM_EXTENSION}"
    echo "    Download package..: LPub3D-${LP3D_APP_VERSION_LONG}_${RPM_EXTENSION}"

    mv -f ${DISTRO_FILE} "LPub3D-UpdateMaster_${LP3D_APP_VERSION}_${RPM_EXTENSION}"
    echo "    Update package....: LPub3D-UpdateMaster_${LP3D_APP_VERSION}_${RPM_EXTENSION}"
else
    echo "14. package ${DISTRO_FILE} not found."
fi

echo "15. cleanup cloned ${LPUB3D} repository from SOURCES/ and BUILD/..."
rm -rf ${BUILD_DIR}/SOURCES/${WORK_DIR} ${BUILD_DIR}/BUILD/${WORK_DIR}

#echo " DEBUG Package files:" `ls ${BUILD_DIR}/RPMS/${LP3D_TARGET_ARCH}`

echo "$ME Finished!"
#mv $LOG "${CWD}/rpmbuild/$ME.log"
