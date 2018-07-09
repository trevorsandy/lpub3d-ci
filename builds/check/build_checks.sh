#!/bin/bash
# Trevor SANDY
# Last Update July 09, 2018
# LPub3D Unix checks
# NOTE: this file must be sourced. with ${SOURCE_DIR} and ${PUB3D_EXE} predefined.

if [[ "${DOCKER}" = "true" && ${XMING} != "true" ]]; then
LP3D_XVFB_CMD="xvfb-run --auto-servernum --server-num=1 -s \"-screen 0 1024x768x24\""
else
LP3D_XVFB_CMD=""
fi

LP3D_CHECK_FILE="${PWD}/${SOURCE_DIR}/builds/check/build_checks.mpd"

echo && echo "- Check 1 of 4: file process check..."
LP3D_CHECK_OPTIONS="--ignore-console-redirect --process-file --preferred-renderer native"
${LP3D_XVFB_CMD} ${LPUB3D_EXE} $LP3D_CHECK_OPTIONS $LP3D_CHECK_FILE

echo && echo "- Check 2 of 4: export check..."
LP3D_CHECK_OPTIONS="--ignore-console-redirect --process-file --clear-cache --preferred-renderer ldview"
${LP3D_XVFB_CMD} ${LPUB3D_EXE} $LP3D_CHECK_OPTIONS $LP3D_CHECK_FILE

echo && echo "- Check 3 of 4: range check..."
LP3D_CHECK_OPTIONS="--ignore-console-redirect --process-export --range 1-3 --clear-cache --preferred-renderer ldglite"
${LP3D_XVFB_CMD} ${LPUB3D_EXE} $LP3D_CHECK_OPTIONS $LP3D_CHECK_FILE

echo && echo "- Check 4 of 4: povray check..."
LP3D_CHECK_OPTIONS="--ignore-console-redirect --process-file --clear-cache --preferred-renderer povray"
${LP3D_XVFB_CMD} ${LPUB3D_EXE} $LP3D_CHECK_OPTIONS $LP3D_CHECK_FILE

echo "------------Build checks completed----------" && echo

