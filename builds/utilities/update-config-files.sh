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

if [ "$1" = "" ]; then SOURCED="true"; LP3D_PWD=${_PRO_FILE_PWD_}; else SOURCED="false"; LP3D_PWD=$1; fi
LP3D_OBS_DIR=$LP3D_PWD/../builds/linux/obs
LP3D_UTIL_DIR=$LP3D_PWD/../builds/utilities
Info () {
    if [ "${SOURCED}" = "true" ]
    then
        echo "   update-config: ${*}" >&2
    else
        echo "${*}" >&2
    fi
}

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d-ci}"
OLD_VAR="${OLD_VAR:-lpub3d-ci}"

if [ "$LP3D_PWD" = "" ] && [ "${_PRO_FILE_PWD_}" = "" ]
then
    Info "Error: Did not receive required argument _PRO_FILE_PWD_"
    Info "$ME terminated!"
    exit 1
fi

if [ "${SOURCED}" != "true" ]
then
    # logging stuff
    LOG="$CALL_DIR/$ME.log"
    if [ -f ${LOG} -a -r ${LOG} ]
    then
            rm ${LOG}
    fi
    exec > >(tee -a ${LOG} )
    exec 2> >(tee -a ${LOG} >&2)
fi

#Info "   DEBUG INPUT ARGS \$0 [$0], \$1 [$1], \$2 [$2], \$3 [$3], \$4 [$4], \$5 [$5], \$6 [$6], \$7 [$7]"
Info "1. capture version info"
if [ "${SOURCED}" = "true" ]
then
    Info "   using git queries..."
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
    #         1 2 3  4  5   6
    # format "2 0 20 17 663 410fdd7"
    read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION VER_BUILD VER_SHA_HASH THE_REST <<< ${VERSION_INFO//'"'}
    Info "   writing version info to builds/utilities/version.info..."
    Info "   VERSION_INFO...........${VERSION_INFO//'"'}"
    FILE="$LP3D_UTIL_DIR/version.info"
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        rm ${FILE}
    fi
    cat <<EOF >${FILE}
${VERSION_INFO} ${DATE_TIME}
EOF
else
    Info "   Start $ME execution..."
    Info "   using input arguments..."
    VER_MAJOR=$2
    VER_MINOR=$3
    VER_PATCH=$4
    VER_REVISION=$5
    VER_BUILD=$6
    VER_SHA_HASH=$7
fi
APP_VER_SUFFIX=${VER_MAJOR}${VER_MINOR}
LP3D_VERSION=${VER_MAJOR}"."${VER_MINOR}"."${VER_PATCH}
LP3D_APP_VERSION=${LP3D_VERSION}"."${VER_BUILD}
LP3D_APP_VERSION_LONG=${LP3D_VERSION}"."${VER_REVISION}"."${VER_BUILD}_${BUILD_DATE}
LP3D_BUILD_VERSION=${LP3D_VERSION}"."${VER_REVISION}"."${VER_BUILD}" ("${DATE_TIME}")"

Info "   LPUB3D_DIR.............${LPUB3D}"
Info "   LP3D_PWD...............${LP3D_PWD}"
Info "   CALL_DIR...............${CALL_DIR}"
Info "   VER_MAJOR..............${VER_MAJOR}"
Info "   VER_MINOR..............${VER_MINOR}"
Info "   VER_PATCH..............${VER_PATCH}"
Info "   VER_REVISION...........${VER_REVISION}"
Info "   VER_BUILD..............${VER_BUILD}"
Info "   VER_SHA_HASH...........${VER_SHA_HASH}"
Info "   APP_VER_SUFFIX.........${APP_VER_SUFFIX}"
Info "   DATE_TIME..............${DATE_TIME}"
Info "   CHANGE_DATE_LONG.......${CHANGE_DATE_LONG}"

Info "   LP3D_VERSION...........${LP3D_VERSION}"
Info "   LP3D_APP_VERSION.......${LP3D_APP_VERSION}"
Info "   LP3D_APP_VERSION_LONG..${LP3D_APP_VERSION_LONG}"
Info "   LP3D_BUILD_VERSION.....${LP3D_BUILD_VERSION}"

Info "   SOURCE_DIR.............${LPUB3D}-${LP3D_APP_VERSION}"

Info "2. set top-level build directory name for linux config files..."
if [ "${OLD_VAR}" = "${LPUB3D}" ];
then
    Info "   nothing to do, skipping set top-level build directory name"
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
        $LP3D_OBS_DIR/debian/${LPUB3D}.dsc \
        $LP3D_OBS_DIR/_service \
        $LP3D_OBS_DIR/${LPUB3D}.spec \
        $LP3D_OBS_DIR/PKGBUILD \
        $LP3D_PWD/../builds/utilities/docker/Dockerfile-ubuntu_xenial
    do
        if [ "$OS" = Darwin ]
        then
            sed -i "" -e "s/\b${OLD_VAR}\b/${LPUB3D}/g" "${FILE}"
        else
            sed -i -e "s/\b${OLD_VAR}\b/${LPUB3D}/g" "${FILE}"
        fi
    done
fi

Info "3. update desktop configuration         - add version suffix"
FILE="$LP3D_PWD/lpub3d.desktop"
LineToReplace=10
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]
    then
        sed -i "" "${LineToReplace}s/.*/Exec=${LPUB3D}${APP_VER_SUFFIX} %f/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/Exec=${LPUB3D}${APP_VER_SUFFIX} %f/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

Info "4. update man page                      - add version suffix"
FILE="$LP3D_PWD/docs/lpub3d${APP_VER_SUFFIX}.1"
LineToReplace=61
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]
    then
        sed -i "" "${LineToReplace}s/.*/     \/usr\/bin\/${LPUB3D}${APP_VER_SUFFIX}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/     \/usr\/bin\/${LPUB3D}${APP_VER_SUFFIX}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

Info "5. update PKGBUILD                      - add app version"
FILE="$LP3D_OBS_DIR/PKGBUILD"
LineToReplace=3
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]
    then
        sed -i "" "${LineToReplace}s/.*/pkgver=${LP3D_APP_VERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/pkgver=${LP3D_APP_VERSION}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

Info "6. create changelog                     - add app version and change date"
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

Info "7. update ${LPUB3D}.dsc                 - add app version"
FILE="$LP3D_OBS_DIR/debian/${LPUB3D}.dsc"
LineToReplace=5
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]
    then
        sed -i "" "${LineToReplace}s/.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

Info "8. update readme.txt                    - add app version"
FILE="$LP3D_PWD/docs/README.txt"
LineToReplace=1
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]
    then
        sed -i "" "${LineToReplace}s/.*/LPub3D ${LP3D_BUILD_VERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/LPub3D ${LP3D_BUILD_VERSION}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

Info "9. update ${LPUB3D}.spec                - add app version and change date"
FILE="$LP3D_OBS_DIR/${LPUB3D}.spec"
LineToReplace=263
if [ -f ${FILE} -a -r ${FILE} ]
then
    if [ "$OS" = Darwin ]
    then
        sed -i "" "${LineToReplace}s/.*/* ${CHANGE_DATE} - trevor.dot.sandy.at.gmail.dot.com ${LP3D_APP_VERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/* ${CHANGE_DATE} - trevor.dot.sandy.at.gmail.dot.com ${LP3D_APP_VERSION}/" "${FILE}"
    fi
    #cat "${FILE}"
else
    Info "   Error: Cannot read ${FILE} from ${CALL_DIR}"
fi

Info "10. create '${LPUB3D}.spec.git.version' - version for OBS builds"
FILE="$LP3D_OBS_DIR/${LPUB3D}.spec.git.version"
if [ -f ${FILE} -a -r ${FILE} ]
then
    rm ${FILE}
fi
cat <<EOF >${FILE}
${LP3D_APP_VERSION}
EOF

if [ "$OS" = Darwin ]
then
    Info "11. update the Info.plist with version major, version minor, build and git sha hash"
    INFO_PLIST_FILE="$LP3D_PWD/Info.plist"
    if [ -f "${INFO_PLIST_FILE}" ]
    then
        /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString ${LP3D_VERSION}" "${INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :CFBundleVersion ${VER_BUILD}" "${INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :CFBundleGetInfoString LPub3D ${LP3D_VERSION} https://github.com/trevorsandy/${LPUB3D}" "${INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :com.trevorsandy.lpub3d.GitSHA ${VER_SHA_HASH}" "${INFO_PLIST_FILE}"
    else
        Info "   Error: update failed, ${INFO_PLIST_FILE} not found."
    fi
fi
if [ "${SOURCED}" = "false" ]
then
    Info "Script $ME execution finshed."
fi
echo
