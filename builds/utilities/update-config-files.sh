#!/bin/bash
# Trevor SANDY
# Last Update November 01 2017
# This script is automatically executed by qmake from mainApp.pro

LP3D_ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
LP3D_CHANGE_DATE_LONG=`date +%a,\ %d\ %b\ %Y\ %H:%M:%S\ %z`
LP3D_CHANGE_DATE=`date +%a\ %b\ %d\ %Y`
LP3D_DATE_TIME=`date +%d\ %m\ %Y\ %H:%M:%S`
LP3D_BUILD_DATE=`date "+%Y%m%d"`
LP3D_CALL_DIR=`pwd`
LP3D_OS=`uname`

if [ "$1" = "" ]; then SOURCED="true"; LP3D_PWD=${_PRO_FILE_PWD_}; else SOURCED="false"; LP3D_PWD=$1; fi
cd $LP3D_PWD/.. && basedir=$PWD && cd $LP3D_CALL_DIR

# Change these when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="$(basename "$(echo "$basedir")")"
OLD_LPUB3D="${OLD_LPUB3D:-lpub3d-ci}"

Info () {
    if [ "${SOURCED}" = "true" ]
    then
        echo "   update-config: ${*}" >&2
    else
        echo "${*}" >&2
    fi
}

LP3D_OBS_DIR=$LP3D_PWD/../builds/linux/obs
LP3D_UTIL_DIR=$LP3D_PWD/../builds/utilities

# Change these accordingly when respective config files are modified
LINE_DESKTOP=10             # Exec=lpub3d20 %f
LINE_MANPAGE=61             # /usr/bin/lpub3d20
LINE_PKGBUILD=3             # pkgver=2.0.21.129
LINE_DSC=5                  # Version: 2.0.21.129
LINE_README=1               # LPub3D 2.0.21.59.126...
LINE_SPEC="94 288"          # 1st 2.0.0.21.166 2nd * Fri Oct 27 2017...

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
    Info "1. capture version info using git queries"
    cd "$LP3D_PWD/.."
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
    Info "   Start $LP3D_ME execution..."
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

if [ "${CONTINUOUS_INTEGRATION}" = "true" ];
then
    # Stop at the end of this block during Travis-CI builds
    export LP3D_APP_VERSION=${LP3D_APP_VERSION}
    export LP3D_APP_VERSION_LONG=${LP3D_APP_VERSION_LONG}

    Info "   LP3D_VERSION_INFO......${LP3D_VERSION_INFO}"
    Info "   LP3D_APP_VERSION.......${LP3D_APP_VERSION}"
    Info "   LP3D_APP_VERSION_LONG..${LP3D_APP_VERSION_LONG}"
else
    # AppVeyor 64bit Qt MinGW build has git.exe/cygwin conflict returning no .git directory found so generate version.info file
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
        Info "   FILE version.info......written to builds/utilities/version.info";
    else
        Info "   FILE version.info......error, file not found";
    fi

    Info "   LPUB3D_DIR.............${LPUB3D}"
    Info "   LP3D_PWD...............${LP3D_PWD}"
    Info "   LP3D_CALL_DIR..........${LP3D_CALL_DIR}"

    Info "   LP3D_VERSION_INFO......${LP3D_VERSION_INFO}"
    Info "   VER_MAJOR..............${VER_MAJOR}"
    Info "   VER_MINOR..............${VER_MINOR}"
    Info "   VER_PATCH..............${VER_PATCH}"
    Info "   VER_REVISION...........${VER_REVISION}"
    Info "   VER_BUILD..............${VER_BUILD}"
    Info "   VER_SHA_HASH...........${VER_SHA_HASH}"
    Info "   LP3D_APP_VER_SUFFIX....${LP3D_APP_VER_SUFFIX}"
    Info "   LP3D_DATE_TIME.........${LP3D_DATE_TIME}"
    Info "   LP3D_CHANGE_DATE_LONG..${LP3D_CHANGE_DATE_LONG}"

    Info "   LP3D_VERSION...........${LP3D_VERSION}"
    Info "   LP3D_APP_VERSION.......${LP3D_APP_VERSION}"
    Info "   LP3D_APP_VERSION_LONG..${LP3D_APP_VERSION_LONG}"
    Info "   LP3D_BUILD_VERSION.....${LP3D_BUILD_VERSION}"

    Info "   LP3D_SOURCE_DIR........${LPUB3D}-${LP3D_APP_VERSION}"

    Info "2. set top-level build directory name for linux config files..."
    if [ "${OLD_LPUB3D}" = "${LPUB3D}" ];
    then
        Info "   nothing to do, skipping set top-level build directory name"
    else
        LP3D_DEB_DSC_FILE=$LP3D_OBS_DIR/debian/${OLD_LPUB3D}.dsc
        LP3D_DEB_LINT_FILE=$LP3D_OBS_DIR/debian/${OLD_LPUB3D}.lintian-overrides
        LP3D_OBS_SPEC_FILE=$LP3D_OBS_DIR/${OLD_LPUB3D}.spec
        LP3D_RPM_LINT_FILE=$LP3D_OBS_DIR/${OLD_LPUB3D}-rpmlintrc
        if [ -d "${LP3D_DEB_DSC_FILE}" ]
        then
            mv -f "${LP3D_DEB_DSC_FILE}" "$LP3D_OBS_DIR/debian/${LPUB3D}.dsc"
        fi
        if [ -d "${LP3D_DEB_LINT_FILE}" ]
        then
            mv -f "${LP3D_DEB_LINT_FILE}" "$LP3D_OBS_DIR/debian/${LPUB3D}.lintian-overrides"
        fi
        if [ -d "${LP3D_OBS_SPEC_FILE}" ]
        then
            mv -f "${LP3D_OBS_SPEC_FILE}" "$LP3D_OBS_DIR/${LPUB3D}.spec"
        fi
        if [ -d "${LP3D_RPM_LINT_FILE}" ]
        then
            mv -f "${LP3D_RPM_LINT_FILE}" "$LP3D_OBS_DIR/${LPUB3D}-rpmlintrc"
        fi
        for FILE in \
            $LP3D_OBS_DIR/debian/source/include-binaries \
            $LP3D_OBS_DIR/debian/changelog \
            $LP3D_OBS_DIR/debian/control \
            $LP3D_OBS_DIR/debian/copyright \
            $LP3D_OBS_DIR/debian/${LPUB3D}.dsc \
            $LP3D_OBS_DIR/debian/${LPUB3D}.lintian-overrides \
            $LP3D_OBS_DIR/_service \
            $LP3D_OBS_DIR/PKGBUILD \
            $LP3D_OBS_DIR/${LPUB3D}-rpmlintrc \
            $LP3D_PWD/../builds/linux/docker-compose/docker-compose-linux.yml \
            $LP3D_PWD/../builds/utilities/docker/Dockerfile-archlinux_2017.10.01 \
            $LP3D_PWD/../builds/utilities/docker/Dockerfile-fedora_25 \
            $LP3D_PWD/../builds/utilities/docker/Dockerfile-ubuntu_xenial \
            $LP3D_PWD/../appveyor.yml \
            $LP3D_PWD/../travis.yml
        do
            if [ "$LP3D_OS" = Darwin ]
            then
                sed -i "" -e "s/\b${OLD_LPUB3D}\b/${LPUB3D}/g" "${FILE}"
            else
                sed -i -e "s/\b${OLD_LPUB3D}\b/${LPUB3D}/g" "${FILE}"
            fi
        done
    fi

    Info "3. update desktop configuration         - add version suffix"
    FILE="$LP3D_PWD/lpub3d.desktop"
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

    Info "4. update man page                      - add version suffix"
    FILE="$LP3D_PWD/docs/lpub3d${LP3D_APP_VER_SUFFIX}.1"
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

    Info "5. update PKGBUILD                      - add app version"
    FILE="$LP3D_OBS_DIR/PKGBUILD"
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

    Info "6. create changelog                     - add app version and change date"
    FILE="$LP3D_OBS_DIR/debian/changelog"
    if [ -f ${FILE} -a -r ${FILE} ]
    then
    	rm ${FILE}
    fi
    cat <<EOF >${FILE}
${LPUB3D} (${LP3D_APP_VERSION}) xenial; urgency=medium

  * LPub3D version ${LP3D_APP_VERSION_LONG} for Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${LP3D_CHANGE_DATE_LONG}
EOF

    Info "7. update ${LPUB3D}.dsc                 - add app version"
    FILE="$LP3D_OBS_DIR/debian/${LPUB3D}.dsc"
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

    Info "8. update README.txt                    - add app version"
    FILE="$LP3D_PWD/docs/README.txt"
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

    Info "9. update ${LPUB3D}.spec                - add app version and change date"
    FILE="$LP3D_OBS_DIR/${LPUB3D}.spec"
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

    if [ "$LP3D_OS" = Darwin ]
    then
        Info "10. update the Info.plist with version major, version minor, build and git sha hash"
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

    if [ "${SOURCED}" = "false" ]
    then
        Info "   Script $LP3D_ME execution finshed."
    fi
    echo
fi
