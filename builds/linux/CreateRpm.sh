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

echo "1. create RPM build working directories in rpmbuild/..."
if [ ! -d rpmbuild ]
then
    mkdir rpmbuild
fi
cd rpmbuild
RPM_BUILD_DIR=$PWD
for DIR in {BUILD,RPMS,SRPMS,SOURCES,SPECS}
do
    if [ ! -d "${DIR}" ]
    then
        mkdir "${DIR}"
    fi
done
cd SOURCES

echo "2. download source to SOURCES/..."
git clone https://github.com/trevorsandy/${LPUB3D}.git

echo "3. source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

echo "4. move ${LPUB3D}/ to ${LPUB3D}-git/ in SOURCES/..."
WORK_DIR=${LPUB3D}-git
mv ${LPUB3D} ${WORK_DIR}

echo "5. copy ${LPUB3D}.spec.git.version to SOURCES/..."
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}.spec.git.version .
if [ -f "${LPUB3D}.spec.git.version" ]; then echo "   DEBUG ${LPUB3D}.spec.git.version copied"; else echo "   DEBUG ${LPUB3D}.spec.git.version not found!"; fi

echo "6. copy xpm icon to SOURCES/"
cp -f ${WORK_DIR}/mainApp/images/lpub3d.xpm .
if [ -f "lpub3d.xpm" ]; then echo "   DEBUG lpub3d.xpm copied"; else echo "   DEBUG lpub3d.xpm not found!"; fi

echo "7. copy spec to SPECS/"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}.spec ../SPECS
if [ -f "../SPECS/${LPUB3D}.spec" ]; then echo "   DEBUG ${LPUB3D}.spec copied"; else echo "   DEBUG ${LPUB3D}.spec not found!"; fi

echo "8. download LDraw archive libraries to SOURCES/..."
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -q -O lpub3dldrawunf.zip http://www.ldraw.org/library/unofficial/ldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -q http://www.ldraw.org/library/updates/complete.zip
fi

# file copy and downloads above must happen before we make the tarball
echo "9. create tarball ${WORK_DIR}.tar.gz from ${WORK_DIR}/..."
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

cd ../SPECS
echo "10. download build dependenvies..."
dnf builddep -y ${LPUB3D}.spec

echo "10. build the RPM package..."
# check the spec
rpmlint ${LPUB3D}.spec

#rpmbuild --define "_topdir ${WORK_DIR}/rpmbuild" -v -ba ${LPUB3D}.spec
rpmbuild --define "_topdir ${RPM_BUILD_DIR}" -vv -ba ${LPUB3D}.spec

cd ../RPMS/${LP3D_TARGET_ARCH}
DISTRO_FILE=`ls ${LPUB3D}*.rpm`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "11a. check rpm packages..."
    rpmlint ${DISTRO_FILE} ../../SRPMS/${LPUB3D}*.rpm

    echo "11. create update and download packages..."
    IFS=- read NAME RPM_VERSION RPM_EXTENSION <<< ${DISTRO_FILE}

    cp -f ${DISTRO_FILE} "${LPUB3D}-${LP3D_APP_VERSION_LONG}_${RPM_EXTENSION}"
    echo "    Download package: ${LPUB3D}_${LP3D_APP_VERSION_LONG}_${RPM_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${RPM_VERSION}_${RPM_EXTENSION}"
    echo "      Update package: LPub3D-UpdateMaster_${RPM_VERSION}_${RPM_EXTENSION}"
else
    echo "11. package ${DISTRO_FILE} not found."
fi

echo "12. cleanup cloned ${LPUB3D} repository from SOURCES/ and BUILD/..."
rm -rf ../../SOURCES/${WORK_DIR} ../../BUILD/${WORK_DIR}

echo " DEBUG Package files:" `ls ../RPMS`

# debug
echo "********** RPM-TMP Logs ******************"
if [ -d "~/rpm/tmp" ]; then
    rpm-temp-logs=`ls ~/rpm/tmp/rpm-tmp*`
fi
if [ -d "/var/tmp" ]; then
    rpm-temp-logs=${rpm-temp-logs} `ls /var/tmp/rpm-tmp*`
fi
for FILE in ${rpm-temp-logs}; do echo "* rpm-tmp log: $FILE" && cat $FILE; done
echo
#echo " DEBUG Package files: `find $PWD`"
echo "$ME Finished!"
#mv $LOG "${CWD}/rpmbuild/$ME.log"
