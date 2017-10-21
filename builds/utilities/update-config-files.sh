#!/bin/bash
# Trevor SANDY
# Last Update 19 October 2017
# This script is automatically executed by qmake from mainApp.pro

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CHANGE_DATE_LONG=`date +%a,\ %d\ %b\ %Y\ %H:%M:%S\ %z`
CHANGE_DATE=`date +%a\ %b\ %d\ %Y`
DATE_TIME=`date +%d\ %m\ %Y\ %H:%M:%S`
BUILD_DATE=`date "+%Y%m%d"`
CALL_DIR=`pwd`
OS=`uname`

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d-ci}"
OLD_VAR="${OLD_VAR:-lpub3d-ci}"

LP3D_PWD=$1
if [ "$LP3D_PWD" = "" ] && [ "${_PRO_FILE_PWD_}" = "" ]
then
    echo "Error: Did not receive required argument _PRO_FILE_PWD_"
    echo "$ME terminated!"
    exit 1
fi

# logging stuff
LOG="$LP3D_UTIL_DIR/$ME.log"
if [ -f ${LOG} -a -r ${LOG} ]
then
        rm ${LOG}
fi
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start $ME execution..."
echo "1. capture version info"

if [ "$3" != "" ]
then
    echo "   using version arguments..."
    VER_MAJOR=$2
    VER_MINOR=$3
    VER_PATCH=$4
    VER_REVISION=$5
    VER_BUILD=$6
    VER_SHA_HASH=$7

    # echo "   reading version info from builds/utilities/version.info"
    # FILE="$LP3D_UTIL_DIR/version.info"
    # if [ -f ${FILE} -a -r ${FILE} ]
    # then
    # 	VERSION_INFO=`cat ${FILE}`
    #     #         1 2  3  4   5       6
    #     # format "2 0 20 17 663 410fdd7"
    #     read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION VER_BUILD VER_SHA_HASH THE_REST <<< ${VERSION_INFO//'"'}
    #     echo
    #     echo "   VERSION_INFO...........${VERSION_INFO//'"'}"
    # else
    #     echo "Error: Cannot read ${FILE} from ${CALL_DIR}"
    #     echo "$ME terminated!"
    # 	exit 1
    # fi
else
    echo "   using git queries..."
    cd "$LP3D_PWD/.."
    lp3d_git_ver_tag_long=`git describe --tags --long`
    lp3d_git_ver_tag_short=`git describe --tags --abbrev=0`
    lp3d_git_ver_commit_count=`git rev-list HEAD --count`
    lp3d_git_ver_sha_hash_short=`git rev-parse --short HEAD`
    cd "${CALL_DIR}"
    lp3d_ver_tmp1=${lp3d_git_ver_tag_long#*-}       # remove prefix ending in "-"
    lp3d_ver_tmp2=${lp3d_git_ver_tag_short//./" "}  # replace . with " "
    lp3d_version_=${lp3d_ver_tmp2/v/}               # replace v with ""
    lp3d_revision_=${lp3d_ver_tmp1%-*}
    VERSION_INFO=${lp3d_version_}" "${lp3d_revision_}" "${lp3d_git_ver_commit_count}" "${lp3d_git_ver_sha_hash_short}

    echo "   writing version info to builds/utilities/version.info"
    FILE="$LP3D_UTIL_DIR/version.info"
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        rm ${FILE}
    fi
    cat <<EOF >${FILE}
${VERSION_INFO} ${DATE_TIME}
EOF
fi
APP_VER_SUFFIX=${VER_MAJOR}${VER_MINOR}
LP3D_VERSION=${VER_MAJOR}"."${VER_MINOR}"."${VER_PATCH}
LP3D_APP_VERSION=${LP3D_VERSION}"."${VER_BUILD}
LP3D_APP_VERSION_LONG=${LP3D_VERSION}"."${VER_REVISION}"."${VER_BUILD}_${BUILD_DATE}
BUILDVERSION=${LP3D_VERSION}"."${VER_REVISION}"."${VER_BUILD}" ("${DATE_TIME}")"
# export LP3D_VERSION=$LP3D_VERSION
# export LP3D_APP_VERSION=$LP3D_APP_VERSION
# export LP3D_APP_VERSION_LONG=$LP3D_APP_VERSION_LONG

echo "   LPUB3D_DIR.............${LPUB3D}"
echo "   LP3D_PWD...............${LP3D_PWD}"
echo "   CALL_DIR...............${CALL_DIR}"
echo "   VER_MAJOR..............${VER_MAJOR}"
echo "   VER_MINOR..............${VER_MINOR}"
echo "   VER_PATCH..............${VER_PATCH}"
echo "   VER_REVISION...........${VER_REVISION}"
echo "   VER_BUILD..............${VER_BUILD}"
echo "   VER_SHA_HASH...........${VER_SHA_HASH}"
echo "   APP_VER_SUFFIX.........${APP_VER_SUFFIX}"
echo "   BUILDVERSION...........${BUILDVERSION}"
echo "   DATE_TIME..............${DATE_TIME}"
echo "   CHANGE_DATE_LONG.......${CHANGE_DATE_LONG}"

echo "   LP3D_VERSION...........${LP3D_VERSION}"
echo "   LP3D_APP_VERSION.......${LP3D_APP_VERSION}"
echo "   LP3D_APP_VERSION_LONG..${LP3D_APP_VERSION_LONG}"

echo "   SOURCE_DIR.............${LPUB3D}-${LP3D_APP_VERSION}"

echo "2. set root directory name for linux config files..."
LP3D_OBS_DIR=$LP3D_PWD/../builds/linux/obs
LP3D_UTIL_DIR=$LP3D_PWD/../builds/utilities
if [ "${OLD_VAR}" = "${FILE}" ];
then
    echo "   nothing to do, skipping"
else
    LP3D_DEB_DSC_FILE=$LP3D_OBS_DIR/debian/${OLD_VAR}.dsc
    LP3D_OBS_SPEC_FILE=$LP3D_OBS_DIR/${OLD_VAR}.spec
    if [ -d "${LP3D_DEB_DSC_FILE}" ]
    then
        mv -f "${LP3D_DEB_DSC_FILE}" "$LP3D_OBS_DIR/debian/${LPUB3D}.dsc"
    fi
    if [ -d "${LP3D_OBS_SPEC_FILE}" ]
    then
        mv -f "${LP3D_OBS_SPEC_FILE}" "$LP3D_OBS_DIR/${LPUB3D}.spec"
    fi
    for FILE in \
        $LP3D_OBS_DIR/debian/source/include-binaries \
        $LP3D_OBS_DIR/debian/changelog \
        $LP3D_OBS_DIR/debian/control \
        $LP3D_OBS_DIR/debian/copyright \
        $LP3D_OBS_DIR/debian/install \
        $LP3D_OBS_DIR/debian/${LPUB3D}.dsc \
        $LP3D_OBS_DIR/_service \
        $LP3D_OBS_DIR/${LPUB3D}.spec \
        $LP3D_OBS_DIR/PKGBUILD \
        $LP3D_PWD/../builds/linux/Dockerfile-ubuntu_xenial
    do
        if [ "$OS" = Darwin ]; then
            sed -i "" -e "s/\b${OLD_VAR}\b/${LPUB3D}/g" "${FILE}"
        else
            sed -i -e "s/\b${OLD_VAR}\b/${LPUB3D}/g" "${FILE}"
        fi
    done
fi
echo "3. update desktop configuration         - add version suffix"
FILE="$LP3D_PWD/lpub3d.desktop"
LineToReplace=10
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]; then
        sed -i "" "${LineToReplace}s/.*/Exec=${LPUB3D}${APP_VER_SUFFIX} %f/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/Exec=${LPUB3D}${APP_VER_SUFFIX} %f/" "${FILE}"
    fi
else
    echo "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

echo "4. update man page                      - add version suffix"
FILE="$LP3D_PWD/docs/lpub3d${APP_VER_SUFFIX}.1"
LineToReplace=61
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]; then
        sed -i "" "${LineToReplace}s/.*/     \/usr\/bin\/${LPUB3D}${APP_VER_SUFFIX}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/     \/usr\/bin\/${LPUB3D}${APP_VER_SUFFIX}/" "${FILE}"
    fi
else
    echo "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

echo "5. update PKGBUILD                      - add app version"
FILE="$LP3D_OBS_DIR/PKGBUILD"
LineToReplace=3
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]; then
        sed -i "" "${LineToReplace}s/.*/pkgver=${LP3D_APP_VERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/pkgver=${LP3D_APP_VERSION}/" "${FILE}"
    fi
else
    echo "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

echo "6. create changelog                     - add app version and change date"
FILE="$LP3D_OBS_DIR/debian/changelog"
if [ -f ${FILE} -a -r ${FILE} ]
then
	rm ${FILE}
fi
cat <<EOF >${FILE}
${LPUB3D} (${LP3D_APP_VERSION}) xenial; urgency=medium

  * LPub3D version ${LP3D_APP_VERSION_LONG} for Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${CHANGE_DATE_LONG}
EOF

echo "7. update ${LPUB3D}.dsc                 - add app version"
FILE="$LP3D_OBS_DIR/debian/${LPUB3D}.dsc"
LineToReplace=5
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]; then
        sed -i "" "${LineToReplace}s/.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
    fi
else
    echo "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

echo "8. update readme.txt                    - add app version"
FILE="$LP3D_PWD/docs/README.txt"
LineToReplace=1
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]; then
        sed -i "" "${LineToReplace}s/.*/LPub3D ${BUILDVERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/LPub3D ${BUILDVERSION}/" "${FILE}"
    fi
else
    echo "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

echo "9. update ${LPUB3D}.spec                - add app version and change date"
FILE="$LP3D_OBS_DIR/${LPUB3D}.spec"
LineToReplace=256
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]; then
        sed -i "" "${LineToReplace}s/.*/* ${CHANGE_DATE} - trevor.dot.sandy.at.gmail.dot.com ${LP3D_APP_VERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/* ${CHANGE_DATE} - trevor.dot.sandy.at.gmail.dot.com ${LP3D_APP_VERSION}/" "${FILE}"
    fi
else
    echo "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

echo "10. create '${LPUB3D}.spec.git.version' - version for OBS builds"
FILE="$LP3D_OBS_DIR/${LPUB3D}.spec.git.version"
if [ -f ${FILE} -a -r ${FILE} ]
then
    rm ${FILE}
fi
cat <<EOF >${FILE}
${LP3D_APP_VERSION}
EOF

if [ "$OS" = Darwin ];
then
    echo "11. update the Info.plist with version major, version minor, build and git sha hash"
    FILE="$LP3D_PWD/Info.plist"
    if [ -f "${FILE}" ];
    then
        PLIST_COMMAND=/usr/libexec/PlistBuddy -c
        $PLIST_COMMAND "\"Set :CFBundleShortVersionString ${LP3D_VERSION}\"" "${FILE}"
        $PLIST_COMMAND "\"Set :CFBundleVersion ${VER_BUILD}\"" "${FILE}"
        $PLIST_COMMAND "\"Set :CFBundleGetInfoString LPub3D ${LP3D_VERSION} https://github.com/trevorsandy/${LPUB3D}\"" "${FILE}"
        $PLIST_COMMAND "\"Set :com.trevorsandy.${LPUB3D}.GitSHA ${VER_SHA_HASH}\"" "${FILE}"
    else
        echo "   Error: update failed, ${FILE} not found."
    fi
fi
echo "Script $ME execution finshed."
