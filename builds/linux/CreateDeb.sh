#!/bin/bash
# Trevor SANDY
# Last Update October 19 2017
# To run:
# $ chmod 755 CreateDeb.sh
# $ ./CreateDeb.sh

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`

echo "Start $ME execution at $CWD..."

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d-ci}"
echo "   LPUB3D SOURCE DIR......${LPUB3D}"

# when running locally, use this block...
if [ "${TRAVIS}" != "true"  ]; then
    # Travis starting point: /home/travis/build/trevorsandy/lpub3d-ci
    #
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
# For Travis CI, use this block (script called from [pwd] lpub3d/)
else
    # Travis starts in the clone directory (lpub3d/), so move outside
    cd ../
fi

echo "1. create DEB working directories in debbuild/..."
if [ ! -d debbuild ]
then
    mkdir -p debbuild/SOURCES
else
    rm -rf debbuild
    mkdir -p debbuild/SOURCES
fi

cd debbuild/SOURCES

if [ "${TRAVIS}" != "true"  ]; then
    echo "2. download source to SOURCES/..."
    git clone https://github.com/trevorsandy/${LPUB3D}.git
else
    echo "2. copy source to SOURCES/..."
    cp -rf "../../${LPUB3D}" .
fi

echo "3. source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

echo "4. move ${LPUB3D}/ to ${LPUB3D}-${LP3D_APP_VERSION}/ in SOURCES/..."
SOURCE_DIR=${LPUB3D}-${LP3D_APP_VERSION}
mv -f ${LPUB3D} ${SOURCE_DIR}

echo "5. create cleaned tarball ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz from ${SOURCE_DIR}/"
tar -czf ../${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz ${SOURCE_DIR} \
        --exclude="${SOURCE_DIR}/builds/linux/standard" \
        --exclude="${SOURCE_DIR}/builds/macx" \
        --exclude="${SOURCE_DIR}/.travis.yml" \
        --exclude="${SOURCE_DIR}/.gitattributes" \
        --exclude="${SOURCE_DIR}/LPub3D.pro.user" \
        --exclude="${SOURCE_DIR}/README.md" \
        --exclude="${SOURCE_DIR}/_config.yml" \
        --exclude="${SOURCE_DIR}/.gitignore" \
        --exclude="${SOURCE_DIR}/appveyor.yml"

echo "6. download LDraw archive libraries to SOURCES/..."
# we pull in the library archives here because the lpub3d.spec file copies them
# to the extras location. This config thus supports both Suse OBS and Travis CI build procs.
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -q http://www.ldraw.org/library/unofficial/ldrawunf.zip -O lpub3dldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -q http://www.ldraw.org/library/updates/complete.zip
fi
echo "8. download lpub3d_linux_3rdparty repository as tar.gz archive to SOURCES/..."
if [ ! -f lpub3d_linux_3rdparty.tar.gz ]
then
     wget -q -O lpub3d_linux_3rdparty.tar.gz https://github.com/trevorsandy/lpub3d_linux_3rdparty/archive/master.tar.gz
fi

echo "7. re-create (untar) soruce directory ${SOURCE_DIR}/..."
cd ../
if [  -d ${LPUB3D}_${LP3D_APP_VERSION} ]
then
   rm -rf ${LPUB3D}_${LP3D_APP_VERSION}
fi
tar zxf ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz

echo "8. copy debian/ configuration directory to ${SOURCE_DIR}/..."
cp -rf ${SOURCE_DIR}/builds/linux/obs/debian ${SOURCE_DIR}
cd ${SOURCE_DIR}/debian

echo "9. build application package from ${SOURCE_DIR}/..."
cd ../
chmod 755 debian/rules
/usr/bin/dpkg-buildpackage -us -uc

echo "10. run lintian..."
cd ../
DISTRO_FILE=`ls ${LPUB3D}-${LP3D_APP_VERSION}*.deb`
lintian ${DISTRO_FILE} ${SOURCE_DIR}/${LPUB3D}.dsc

if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "11. create LPub3D update and download packages..."
    IFS=_ read DEB_NAME DEB_VERSION DEB_EXTENSION <<< ${DISTRO_FILE}

    cp -rf ${DISTRO_FILE} "LPub3D-${LP3D_APP_VERSION_LONG}_${DEB_EXTENSION}"
    echo "    Download package..: LPub3D-${LP3D_APP_VERSION_LONG}_${DEB_EXTENSION}"

    mv -f ${DISTRO_FILE} "LPub3D-UpdateMaster_${LP3D_APP_VERSION}_${DEB_EXTENSION}"
    echo "    Update package....: LPub3D-UpdateMaster_${LP3D_APP_VERSION}_${DEB_EXTENSION}"
else
    echo "11. package ${DISTRO_FILE} not found"
fi

if [ "${TRAVIS}" == "true"  ]; then
  # export vars used by travis.yml so paths must be relative to project download dir
  export LP3D_Download_Package=`ls ../LPub3D_*.deb`
  export LP3D_Update_Package=`ls ../LPub3D-UpdateMaster_*.deb`
  echo "    DEBUG Package files: `ls ../LPub3D*.deb`"
  env | grep -P 'LP3D*'
fi

# create deb - end #
echo "$ME Finished!"
# mv $LOG "${CWD}/debbuild/$ME.log"
