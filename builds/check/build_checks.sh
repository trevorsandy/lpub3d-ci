#!/bin/bash
# Trevor SANDY
# Last Update August 10, 2018
# LPub3D Unix checks
# NOTE: source with:
#       $LP3D_COMPILE_SOURCE = true,
#       $LP3D_BUILD_PKG = <empty>,
#       $SOURCE_DIR = <lpub3d source folder>

cd ${SOURCE_DIR}/mainApp/64bit_release
if [[ "${XMING}" != "true" && ("${DOCKER}" = "true" || "$TRAVIS_OS_NAME" = "linux") ]]; then
    echo && echo "- Using XVFB from working directory: ${PWD}"
    USE_XVFB="true"
fi

# Build checks setup
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    echo "- macOS build checks..."
    LPUB3D_EXE="LPub3D.app/Contents/MacOS/LPub3D"
    echo "- install library links..."
    /usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
    /usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Libs/libQuaZIP.0.dylib
    echo "- change mapping to LPub3D..."
    /usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
    /usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/MacOS/LPub3D
    echo "- bundle LPub3D..."
    macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite
    echo "- change library dependency mapping..."
    /usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
    /usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
elif [ "$TRAVIS_OS_NAME" = "linux" ]; then
    echo "- Linux build checks..."
    LPUB3D_EXE="lpub3d${LP3D_APP_VER_SUFFIX}"

    # 3rdParty renderer paths
    LP3D_3RD_LDGLITE="${LP3D_DIST_DIR_PATH}/${LP3D_VER_LDGLITE}"
    LP3D_3RD_LDVIEW="${LP3D_DIST_DIR_PATH}/${LP3D_VER_LDVIEW}"
    LP3D_3RD_POVRAY="${LP3D_DIST_DIR_PATH}/${LP3D_VER_POVRAY}"
	
    # 3rdParty renderer exe paths
    LP3D_3RD_LDGLITE_EXE="${LP3D_3RD_LDGLITE}/bin/x86_64/ldglite"
    LP3D_3RD_LDVIEW_EXE="${LP3D_3RD_LDVIEW}/bin/x86_64/ldview"
    LP3D_3RD_POVRAY_EXE="${LP3D_3RD_POVRAY}/bin/x86_64/lpub3d_trace_cui"
	
    echo "- setup share contents..." # share
    LP3D_3RD_SHARE="share/lpub3d/3rdParty"
    LP3D_LDGLITE_SHARE="${LP3D_3RD_SHARE}/${LP3D_VER_LDGLITE}"
    LP3D_LDVIEW_SHARE="${LP3D_3RD_SHARE}/${LP3D_VER_LDVIEW}"
    LP3D_POVRAY_SHARE="${LP3D_3RD_SHARE}/${LP3D_VER_POVRAY}"
    mkdir -p ${LP3D_LDGLITE_SHARE}
    mkdir -p ${LP3D_LDVIEW_SHARE}
    mkdir -p ${LP3D_POVRAY_SHARE}
    cp -rf ${LP3D_3RD_LDGLITE}/docs ${LP3D_LDGLITE_SHARE}/
    cp -rf ${LP3D_3RD_LDGLITE}/resources ${LP3D_LDGLITE_SHARE}/
    cp -rf ${LP3D_3RD_LDVIEW}/docs ${LP3D_LDVIEW_SHARE}/
    cp -rf ${LP3D_3RD_LDVIEW}/resources ${LP3D_LDVIEW_SHARE}/
    cp -rf ${LP3D_3RD_POVRAY}/docs ${LP3D_POVRAY_SHARE}/
    cp -rf ${LP3D_3RD_POVRAY}/resources ${LP3D_POVRAY_SHARE}/
	
    echo "- setup mainApp/extras contents..." # mainApp/extras
    cp -rf mainApp/extras share/lpub3d/
	
    echo "- setup 3rdParty bin contents..." # mainApp/opt
    LP3D_OPT_BIN="mainApp/opt/lpub3d/3rdParty"
    LP3D_LDGLITE_BIN="${LP3D_OPT_BIN}/${LP3D_VER_LDGLITE}/bin"
    LP3D_LDVIEW_BIN="${LP3D_OPT_BIN}/${LP3D_VER_LDVIEW}/bin"
    LP3D_POVRAY_BIN="${LP3D_OPT_BIN}/${LP3D_VER_POVRAY}/bin"
    mkdir -p ${LP3D_LDGLITE_BIN}
    mkdir -p ${LP3D_LDVIEW_BIN}
    mkdir -p ${LP3D_POVRAY_BIN}
    cp -rf ${LP3D_3RD_LDGLITE_EXE} ${LP3D_LDGLITE_BIN}/
    cp -rf ${LP3D_3RD_LDVIEW_EXE} ${LP3D_LDVIEW_BIN}/
    cp -rf ${LP3D_3RD_POVRAY_EXE} ${LP3D_POVRAY_BIN}/

    echo "- update LD_LIBRARY_PATH..."  #Qt libs, ldrawini and quazip
    LP3D_QUAZIP_LIB=$(realpath quazip/64bit_release/)
    LP3D_LDRAWINI_LIB=$(realpath ldrawini/64bit_release/)
    LP3D_QT_LIB=/opt/qt${LP3D_QT_BASE}/lib
    export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${LP3D_QT_LIB}:${LP3D_QUAZIP_LIB}:${LP3D_LDRAWINI_LIB}"
	
    echo "- run LDD..." 
    if [ -d "mainApp/64bit_release" ]; then
        ldd mainApp/64bit_release/${LPUB3D_EXE} 2>/dev/null
    else
        echo "ERROR - LDD check failed for $(realpath mainApp/64bit_release/${LPUB3D_EXE})"
    fi
fi

echo && echo "------------Build checks start--------------" && echo

LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/build_checks.mpd"

for LP3D_BUILD_CHECK in CHECK01 CHECK02 CHECK03 CHECK04; do
    case ${LP3D_BUILD_CHECK} in
    CHECK01)
        LP3D_CHECK_HDR="- Check 1 of 4: Native File Process Check..."
        LP3D_CHECK_OPTIONS="--process-file --preferred-renderer native"
        ;;
    CHECK02)
        LP3D_CHECK_HDR="- Check 2 of 4: LDView File Process Check..."
        LP3D_CHECK_OPTIONS="--process-file --clear-cache --preferred-renderer ldview"
        ;;
    CHECK03)
        LP3D_CHECK_HDR="- Check 3 of 4: LDGLite Export Range Check..."
        LP3D_CHECK_OPTIONS="--process-export --range 1-3 --clear-cache --preferred-renderer ldglite"
        ;;
    CHECK04)
        LP3D_CHECK_HDR="- Check 4 of 4: Native POV Generation Check..."
        LP3D_CHECK_OPTIONS="--process-file --clear-cache --preferred-renderer povray"
        ;;
      esac

    echo && echo ${LP3D_CHECK_HDR}
    echo "Command: ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE}
    [ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} || \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE}
done

echo && echo "------------Build checks completed----------" && echo

