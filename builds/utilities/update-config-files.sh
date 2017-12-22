#!/bin/bash
# Trevor SANDY
# Last Update December 22, 2017
# This script is automatically executed by qmake from mainApp.pro
# It is also called by other config scripts accordingly

echo "   Start update-config-files.sh execution..."

LP3D_ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
LP3D_CHANGE_DATE_LONG=`date +%a,\ %d\ %b\ %Y\ %H:%M:%S\ %z`
LP3D_CHANGE_DATE=`date +%a\ %b\ %d\ %Y`
LP3D_DATE_TIME=`date +%d\ %m\ %Y\ %H:%M:%S`
LP3D_BUILD_DATE=`date "+%Y%m%d"`
LP3D_CALL_DIR=`pwd`
LP3D_OS=`uname`
GIT_DEPTH=500

if [ "$1" = "" ]; then SOURCED="true"; LP3D_PWD=${_PRO_FILE_PWD_}; else SOURCED="false"; LP3D_PWD=$1; fi
cd $LP3D_PWD/.. && basedir=$PWD && cd $LP3D_CALL_DIR

# Change these when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="$(basename "$(echo "$basedir")")"

Info () {
    if [ "${SOURCED}" = "true" ]
    then
        echo "   update-config: ${*}" >&2
    else
        echo "${*}" >&2
    fi
}

# Fake realpath
realpath() {
  OURPWD=$PWD
  cd "$(dirname "$1")"
  LINK=$(readlink "$(basename "$1")")
  while [ "$LINK" ]; do
    cd "$(dirname "$LINK")"
    LINK=$(readlink "$(basename "$1")")
  done
  REALPATH_="$PWD/$(basename "$1")"
  cd "$OURPWD"
  echo "$REALPATH_"
}

# Change these accordingly when respective config files are modified
LINE_DESKTOP=10                 # Exec=lpub3d20 %f
LINE_MANPAGE=61                 # /usr/bin/lpub3d20
LINE_README=1                   # LPub3D 2.0.21.59.126...
if [ "$OBS" = true ]; then
    USING_OBS=Yes
    LINE_PKGBUILD=3             # pkgver=2.0.21.129
    LINE_DSC=5                  # Version: 2.0.21.129
    LINE_SPEC="95 530"          # 1st 2.0.0.21.166 2nd * Fri Oct 27 2017...
    LP3D_OBS_DIR=$(realpath $LP3D_PWD/../builds/linux/obs/alldeps)
else
    USING_OBS=No
    LINE_PKGBUILD=3
    LINE_DSC=5
    LINE_SPEC="93 293"
    LP3D_OBS_DIR=$(realpath $LP3D_PWD/../builds/linux/obs)
fi
LP3D_UTIL_DIR=$(realpath $LP3D_PWD/../builds/utilities)

if [ "$LP3D_PWD" = "" ] && [ "${_PRO_FILE_PWD_}" = "" ]
then
    Info "Error: Did not receive required argument _PRO_FILE_PWD_"
    Info "$LP3D_ME terminated!"
    exit 1
fi

if [ "${SOURCED}" != "true" ]
then
    # logging stuff
    LOG="$LP3D_CALL_DIR/$LP3D_ME.log"
    if [ -f ${LOG} -a -r ${LOG} ]
    then
        rm ${LOG}
    fi
    exec > >(tee -a ${LOG} )
    exec 2> >(tee -a ${LOG} >&2)
fi

#Info "   DEBUG INPUT ARGS \$0 [$0], \$1 [$1], \$2 [$2], \$3 [$3], \$4 [$4], \$5 [$5], \$6 [$6], \$7 [$7]"
if [ "${SOURCED}" = "true" ]
then
    cd "$(realpath $LP3D_PWD/..)"
    if [ "${CONTINUOUS_INTEGRATION}" = "true" ];
    then
        # if Travis, update refs and tags
        Info "1. update git tags and capture version info using git queries"
        git fetch -qfup --depth=${GIT_DEPTH} origin +${TRAVIS_BRANCH} +refs/tags/*:refs/tags/*
        git checkout -qf ${TRAVIS_COMMIT}
    else
        Info "1. capture version info using git queries"
    fi
    lp3d_git_ver_tag_long=`git describe --tags --long`
    lp3d_git_ver_tag_short=`git describe --tags --abbrev=0`
    lp3d_git_ver_commit_count=`git rev-list HEAD --count`
    lp3d_git_ver_sha_hash_short=`git rev-parse --short HEAD`
    cd "${LP3D_CALL_DIR}"
    lp3d_ver_tmp1=${lp3d_git_ver_tag_long#*-}       # remove prefix ending in "-"
    lp3d_ver_tmp2=${lp3d_git_ver_tag_short//./" "}  # replace . with " "
    lp3d_version_=${lp3d_ver_tmp2/v/}               # replace v with ""
    lp3d_revision_=${lp3d_ver_tmp1%-*}
    LP3D_VERSION_INFO=${lp3d_version_}" "${lp3d_revision_}" "${lp3d_git_ver_commit_count}" "${lp3d_git_ver_sha_hash_short}
else
    Info "1. capture version info using input arguments"
    LP3D_VERSION_INFO=$2" "$3" "$4" "$5" "$6" "$7
fi
#         1 2 3  4  5   6
# format "2 0 20 17 663 410fdd7"
read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION VER_BUILD VER_SHA_HASH THE_REST <<< ${LP3D_VERSION_INFO//'"'}
LP3D_APP_VER_SUFFIX=${VER_MAJOR}${VER_MINOR}
LP3D_VERSION=${VER_MAJOR}"."${VER_MINOR}"."${VER_PATCH}
LP3D_APP_VERSION=${LP3D_VERSION}"."${VER_BUILD}
LP3D_APP_VERSION_LONG=${LP3D_VERSION}"."${VER_REVISION}"."${VER_BUILD}_${LP3D_BUILD_DATE}
LP3D_BUILD_VERSION=${LP3D_VERSION}"."${VER_REVISION}"."${VER_BUILD}" ("${LP3D_DATE_TIME}")"

Info "   LPUB3D_DIR.............${LPUB3D}"
Info "   USING_OBS..............${USING_OBS}"
Info "   GIT_DEPTH..............${GIT_DEPTH}"
Info "   VER_MAJOR..............${VER_MAJOR}"
Info "   VER_MINOR..............${VER_MINOR}"
Info "   VER_PATCH..............${VER_PATCH}"
Info "   VER_REVISION...........${VER_REVISION}"
Info "   VER_BUILD..............${VER_BUILD}"
Info "   VER_SHA_HASH...........${VER_SHA_HASH}"

if [ ! "${CONTINUOUS_INTEGRATION}" = "true" ];
then
    Info "   LP3D_PWD...............${LP3D_PWD}"
    Info "   LP3D_CALL_DIR..........${LP3D_CALL_DIR}"

    Info "   LP3D_VERSION_INFO......${LP3D_VERSION_INFO}"
    Info "   LP3D_APP_VERSION.......${LP3D_APP_VERSION}"
    Info "   LP3D_APP_VERSION_LONG..${LP3D_APP_VERSION_LONG}"
    Info "   LP3D_APP_VER_SUFFIX....${LP3D_APP_VER_SUFFIX}"
    Info "   LP3D_DATE_TIME.........${LP3D_DATE_TIME}"
    Info "   LP3D_CHANGE_DATE_LONG..${LP3D_CHANGE_DATE_LONG}"

    Info "   LP3D_VERSION...........${LP3D_VERSION}"
    Info "   LP3D_BUILD_VERSION.....${LP3D_BUILD_VERSION}"

    Info "   LP3D_SOURCE_DIR........${LPUB3D}-${LP3D_APP_VERSION}"
fi

if [ "$LP3D_OS" = Darwin ]
then
    Info "2. update the Info.plist with version major, version minor, build and git sha hash"
    LP3D_INFO_PLIST_FILE="$LP3D_PWD/Info.plist"
    if [ -f "${LP3D_INFO_PLIST_FILE}" ]
    then
        /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString ${LP3D_VERSION}" "${LP3D_INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :CFBundleVersion ${VER_BUILD}" "${LP3D_INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :CFBundleGetInfoString LPub3D ${LP3D_VERSION} https://github.com/trevorsandy/${LPUB3D}" "${LP3D_INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :com.trevorsandy.lpub3d.GitSHA ${VER_SHA_HASH}" "${LP3D_INFO_PLIST_FILE}"
    else
        Info "   Error: update failed, ${LP3D_INFO_PLIST_FILE} not found."
    fi
fi

if [ "${CONTINUOUS_INTEGRATION}" = "true" ];
then
    # Stop at the end of this block during Travis-CI builds
    export LP3D_APP_VERSION=${LP3D_APP_VERSION}
    export LP3D_APP_VERSION_LONG=${LP3D_APP_VERSION_LONG}
    Info " update-config-files.sh execution finished."
    echo
else
    # generate version.info file
    FILE="$LP3D_UTIL_DIR/version.info"
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        rm ${FILE}
    fi
    cat <<EOF >${FILE}
${LP3D_VERSION_INFO} ${LP3D_DATE_TIME}
EOF
    if [ -f "${FILE}" ];
    then
        Info "   FILE version.info written to...................[$FILE]";
    else
        Info "   FILE version.info error, file not found";
    fi

    FILE="$LP3D_PWD/docs/README.txt"
    Info "2. update README.txt      - add version           [$FILE]"
    LineToReplace=${LINE_README}
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        if [ "$LP3D_OS" = Darwin ]
        then
            sed -i "" "${LineToReplace}s/.*/LPub3D ${LP3D_BUILD_VERSION}/" "${FILE}"
        else
            sed -i "${LineToReplace}s/.*/LPub3D ${LP3D_BUILD_VERSION}/" "${FILE}"
        fi
    else
        Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
    fi

    # -----
    FILE="$LP3D_PWD/lpub3d.desktop"
    Info "3. update desktop config  - add version suffix    [$FILE]"
    LineToReplace=${LINE_DESKTOP}
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        if [ "$LP3D_OS" = Darwin ]
        then
            sed -i "" "${LineToReplace}s/.*/Exec=lpub3d${LP3D_APP_VER_SUFFIX} %f/" "${FILE}"
        else
            sed -i "${LineToReplace}s/.*/Exec=lpub3d${LP3D_APP_VER_SUFFIX} %f/" "${FILE}"
        fi
    else
        Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
    fi

    FILE="$LP3D_PWD/docs/lpub3d${LP3D_APP_VER_SUFFIX}.1"
    Info "4. update man page        - add version suffix    [$FILE]"
    LineToReplace=${LINE_MANPAGE}
    FILE_TEMPLATE=`ls $LP3D_PWD/docs/lpub3d.*`
    if [ -f ${FILE_TEMPLATE} ];
    then
        if [ -f ${FILE} ];
        then
            rm -f "${FILE}"
        fi
        cp "${FILE_TEMPLATE}" "${FILE}"
    fi
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        if [ "$LP3D_OS" = Darwin ]
        then
            sed -i "" "${LineToReplace}s/.*/     \/usr\/bin\/lpub3d${LP3D_APP_VER_SUFFIX}/" "${FILE}"
        else
            sed -i "${LineToReplace}s/.*/     \/usr\/bin\/lpub3d${LP3D_APP_VER_SUFFIX}/" "${FILE}"
        fi
    else
        Info "   Error: Cannot read ${FILE} (be sure ${FILE_TEMPLATE} exsit) from ${LP3D_CALL_DIR}"
    fi

    FILE="$LP3D_OBS_DIR/debian/changelog"
    Info "5. create changelog       - add version and date  [$FILE]"
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        rm ${FILE}
    fi
    cat <<EOF >${FILE}
${LPUB3D} (${LP3D_APP_VERSION}) debian; urgency=medium

  * LPub3D version ${LP3D_APP_VERSION_LONG} for Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${LP3D_CHANGE_DATE_LONG}
EOF

    FILE="$LP3D_OBS_DIR/debian/${LPUB3D}.dsc"
    Info "6. update ${LPUB3D}.dsc   - add version           [$FILE]"
    LineToReplace=${LINE_DSC}
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        if [ "$LP3D_OS" = Darwin ]
        then
            sed -i "" "${LineToReplace}s/.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
        else
            sed -i "${LineToReplace}s/.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
        fi
    else
        Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
    fi

    FILE="$LP3D_OBS_DIR/PKGBUILD"
    Info "7. update PKGBUILD        - add version           [$FILE]"
    LineToReplace=${LINE_PKGBUILD}
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        if [ "$LP3D_OS" = Darwin ]
        then
            sed -i "" "${LineToReplace}s/.*/pkgver=${LP3D_APP_VERSION}/" "${FILE}"
        else
            sed -i "${LineToReplace}s/.*/pkgver=${LP3D_APP_VERSION}/" "${FILE}"
        fi
    else
        Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
    fi

    FILE="$LP3D_OBS_DIR/${LPUB3D}.spec"
    Info "8. update ${LPUB3D}.spec  - add version and date  [$FILE]"
    LinesToReplace=${LINE_SPEC}
    LastLine=`wc -l < ${FILE}`
    if [ -f ${FILE} -a -r ${FILE} ]
    then
        read FirstLine SecondLine <<< ${LinesToReplace}
        for LineToReplace in ${LinesToReplace}; do
            case $LineToReplace in
            $FirstLine)
                if [ "$LP3D_OS" = Darwin ]; then
                    sed -i "" "${LineToReplace}s/.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
                else
                    sed -i "${LineToReplace}s/.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
                fi
                ;;
            $SecondLine)
                if [ "$LP3D_OS" = Darwin ]; then
                    sed -i "" "${LineToReplace}s/.*/* ${LP3D_CHANGE_DATE} - trevor.dot.sandy.at.gmail.dot.com ${LP3D_APP_VERSION}/" "${FILE}"
                else
                    sed -i "${LineToReplace}s/.*/* ${LP3D_CHANGE_DATE} - trevor.dot.sandy.at.gmail.dot.com ${LP3D_APP_VERSION}/" "${FILE}"
                fi
                ;;
            esac
        done
        if [ "$doDebChange" != "" ];
        then
            ((LastLine++))
            if [ "$LP3D_OS" = Darwin ]
            then
                sed -i "" "${LastLine}s/.*/- /" "${FILE}"
            else
                sed -i "${LastLine}s/.*/- /" "${FILE}"
            fi
        fi
        #cat "${FILE}"
    else
        Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
    fi

    # -----
    if [ "${SOURCED}" = "false" ]
    then
        Info "   Script $LP3D_ME execution finshed."
    fi
    echo
fi
