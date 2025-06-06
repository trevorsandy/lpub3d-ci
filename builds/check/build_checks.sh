#!/bin/bash
# Trevor SANDY
# Last Update May 22, 2025
# Copyright (C) 2018 - 2025 by Trevor SANDY
# LPub3D Unix build checks - for remote CI (Travis, OBS)
# NOTE: Source with variables as appropriate:
#       BUILD_OPT = compile   (macOS only)
#       XSERVER = true|false  (used when running local XServer)
#       LP3D_CHECK_LDD = check library dependencies using ldd
#       LP3D_BUILD_OS = appimage|flatpak|snap,
#       SOURCE_DIR = <lpub3d source folder>
#       LPUB3D_EXE = <lpub3d executable path>
#       LDGLITE_EXE = <LDGLite renderer executable path>
#       LDVIEW_EXE = <LDView renderer executable path>
#       POVRAY_EXE = <POVRay renderer executable path>

# Startup
BUILD_DIR=$PWD
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
if [ "${ME}" = "build_checks.sh" ]; then
    # not sourced
    SCRIPT_NAME=$0
    SCRIPT_ARGS=$*
    echo && echo "Start $ME execution at $BUILD_DIR..."
else
    # sourced
    echo && echo "Start build_checks.sh execution at $BUILD_DIR..."
fi

# Initialize build check elapsed time
lp3d_elapsed_check_start=$SECONDS

function show_settings
{
    echo
    echo "--Buld Check Settings:"
    [ -n "${LP3D_BUILD_OS}" ] && echo "--LP3D_BUILD_OS......$LP3D_BUILD_OS"
    echo "--LPUB3D_EXE.........$LPUB3D_EXE"
    [ -n "$LDGLITE_EXE" ] && echo "--LP3D_LDGLITE.......$LP3D_LDGLITE"
    [ -n "$LDVIEW_EXE" ] && echo "--LP3D_LDVIEW........$LP3D_LDVIEW"
    [ -n "$POVRAY_EXE" ] && echo "--LP3D_POVRAY........$LP3D_POVRAY"
    echo "--SOURCE_DIR.........$SOURCE_DIR"
    [ -n "$XDG_RUNTIME_DIR" ] && echo "--XDG_RUNTIME_DIR....$XDG_RUNTIME_DIR"
    [ "${USE_XVFB}" = "true" ] && echo "--USE_XVFB...........YES"
    [ "${XSERVER}" = "true" ] && echo "--XSERVER............YES"
    [ "${DOCKER}" = "true" ] && echo "--DOCKER.............YES"
    echo "--LP3D_OS_NAME.......$LP3D_OS_NAME"
    echo "--LP3D_TARGET_ARCH...$LP3D_TARGET_ARCH"
    echo "--LP3D_PLATFORM......$LP3D_PLATFORM"
    [ -n "${LP3D_CHECK_LDD}" ] && echo "--LP3D_CHECK_LDD.....YES"
    [ -n "${SCRIPT_NAME}" ] && echo "--SCRIPT_NAME.......$SCRIPT_NAME"
    echo
}

function show_settings_and_exit
{
    show_settings
    [ -n "$1" ] && echo "$1" && echo
    # Exit and allow the build process to continue
    exit 0
}

# Build Check Timer args: 1 = <start> (seconds mark)
ElapsedCheckTime() {
    if test -z "$1"; then return 0; fi
    TIME_ELAPSED="$(((SECONDS - $1) % 60))sec"
    TIME_MINUTES=$((((SECONDS - $1) / 60) % 60))
    TIME_HOURS=$(((SECONDS - $1) / 3600))
    if [ "$TIME_MINUTES" -gt 0 ]; then
        TIME_ELAPSED="${TIME_MINUTES}mins $TIME_ELAPSED"
    fi
    if [ "$TIME_HOURS" -gt 0 ]; then
        TIME_ELAPSED="${TIME_HOURS}hrs $TIME_ELAPSED"
    fi
    echo "$TIME_ELAPSED"
}

# Initialize platform variables
LP3D_OS_NAME=$(uname | awk '{print tolower($0)}')
LP3D_TARGET_ARCH="$(uname -m)"
if [[ "${LP3D_OS_NAME}" = "darwin" ]]; then
    LP3D_PLATFORM=$(echo `sw_vers -productName`)
else
    LP3D_PLATFORM=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $OS_NAME | awk '{print tolower($0)}') #'
    [ "${LP3D_PLATFORM}" = "msys2" ] && LP3D_OS_NAME="${LP3D_PLATFORM}" && MSYS2=1 || :
fi

# Check if renderers are available
if [ -n "$LDGLITE_EXE" ]; then
    [ -f "$LDGLITE_EXE" ] && LP3D_LDGLITE="Available" || LP3D_LDGLITE="Not Available"
fi
if [ -n "$LDVIEW_EXE" ]; then
    [ -f "$LDVIEW_EXE" ] && LP3D_LDVIEW="Available" || LP3D_LDVIEW="Not Available"
fi
if [ -n "$POVRAY_EXE" ]; then
    [ -f "$POVRAY_EXE" ] && LP3D_POVRAY="Available" || LP3D_POVRAY="Not Available"
fi

unset LP3D_BUNDLED_APP
if [[ "$LP3D_BUILD_OS" = "flatpak" || "$LP3D_BUILD_OS" = "snap" ]]; then
    LP3D_BUNDLED_APP=1
fi

# Flatpak and Snap validate and set executable permissions
if [ -n "$LP3D_BUNDLED_APP" ]; then
    LPUB3D_EXE=$(find ${LPUB3D_DEST}/bin -name lpub3d* -type f)
    if [[ -f "$LPUB3D_EXE" ]]; then
        if [[ -d "$SOURCE_DIR/builds/check" ]]; then
            cd ${SOURCE_DIR}
        else
            show_settings_and_exit "ERROR - Invalid source path specified. Build check cannot be executed."
        fi
    else
        show_settings_and_exit "ERROR - LPub3D executable was not found. Build check cannot be executed."
    fi
fi

# AppImage validate and set executable permissions

case "${LPUB3D_EXE}" in
*.AppImage)
    VALID_APPIMAGE=1 ;;
*)
    VALID_APPIMAGE=0 ;;
esac
if [[ "$LP3D_BUILD_OS" = "appimage" && $VALID_APPIMAGE -eq 1 ]]; then
    if [[ -f "${LPUB3D_EXE}" ]]; then
        chmod u+x ${LPUB3D_EXE}
        cd ${SOURCE_DIR}
    else
        show_settings_and_exit "ERROR - $(realpath ${LPUB3D_EXE}) not found. Build check cannot be executed."
    fi
fi

# Set XDG_RUNTIME_DIR
if [[ "${LP3D_OS_NAME}" != "darwin" && "${LP3D_OS_NAME}" != "msys2" ]]; then
    VALID_UID=0
    if [[ "$LP3D_BUILD_OS" = "snap" ]]; then
        [ -n $UID ] && VALID_UID=1
    else
        [ $UID -ge 1000 ] && VALID_UID=1
    fi
    if [[ $VALID_UID -eq 1 && -z "$(printenv | grep XDG_RUNTIME_DIR)" ]]; then
        runtime_dir="/tmp/runtime-user-$UID"
        if [ ! -d "$runtime_dir" ]; then
           mkdir -p $runtime_dir
           chmod 700 $runtime_dir
        fi
        export XDG_RUNTIME_DIR="$runtime_dir"
    fi
fi

# Set USE_XVFB flag
if [[ "${XSERVER}" != "true" && ("${DOCKER}" = "true" && "${LP3D_OS_NAME}" != "darwin" && "${LP3D_OS_NAME}" != "msys2") ]]; then
    if [ -z "$(command -v xvfb-run)" ]; then
        show_settings_and_exit "ERROR - XVFB (xvfb-run) could not be found. Build check cannot be executed."
    else
        USE_XVFB="true"
    fi
fi

# Set the log output path
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$HOME || :

show_settings

# macOS, compile only, initialize build paths and libraries
if [[ "${LP3D_OS_NAME}" = "darwin" && "$BUILD_OPT" = "compile" ]]; then
    cd ${SOURCE_DIR}/mainApp/64bit_release

    echo "- set macOS LPub3D executable..."
    LPUB3D_EXE="LPub3D.app/Contents/MacOS/LPub3D"

    echo "- bundle LPub3D..."
    macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite
fi

# Initialize variables
LP3D_CONSOLE="$([ -n "${MSYS2}" ] && echo "--no-console-redirect " || echo "--no-stdout-log ")"
LP3D_CHECK_STATUS=${LP3D_CHECK_STATUS:-${LP3D_CONSOLE}--version}
LP3D_CHECK_PATH="$(realpath ${SOURCE_DIR})/builds/check"
LP3D_CHECK_FILE="${LP3D_CHECK_PATH}/build_checks.mpd"
LP3D_CHECK_SUCCESS="Application terminated with return code 0."
LP3D_XVFB_ERROR="xvfb-run: error: "
LP3D_LOG_FILE="Check.out"
LP3D_ERROR=0
LP3D_COUNT=0
let LP3D_CHECK_PASS=0
let LP3D_CHECK_FAIL=0
LP3D_CHECKS_PASS=
LP3D_CHECKS_FAIL=
LP3D_CHECK_STDLOG=

# Application status check
[ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
${LPUB3D_EXE} ${LP3D_CHECK_STATUS} 2> ${LP3D_LOG_FILE} || \
${LPUB3D_EXE} ${LP3D_CHECK_STATUS} 2> ${LP3D_LOG_FILE}
if [[ $? -ne 0 && -s "${LP3D_LOG_FILE}" ]];then
    echo "- LPub3D Status Log Trace..."
    cat "${LP3D_LOG_FILE}"
else
        # remove empty file
        rm -f "${LP3D_LOG_FILE}"
fi

if [[ -n "${LP3D_CHECK_LDD}" && ${VALID_APPIMAGE} -eq 0 ]]; then
    echo && echo "-----------Library Dependencies-------------" && echo
    LPUB3D_LDD_EXE=$(which ${LPUB3D_EXE})
    find ${LPUB3D_LDD_EXE} -executable -type f -exec ldd {} \;
fi

# Set build checks based on available renderers
LP3D_BUILD_CHECK_LIST=( CHECK_01 )
[ "$LP3D_LDGLITE" != "Not Available" ] && LP3D_BUILD_CHECK_LIST+=( CHECK_02 ) || :
[ "$LP3D_POVRAY" != "Not Available" ] && LP3D_BUILD_CHECK_LIST+=( CHECK_03 ) || :
[ "$LP3D_LDVIEW" != "Not Available" ] && LP3D_BUILD_CHECK_LIST+=( CHECK_04 CHECK_05 CHECK_06 CHECK_07 ) || :

NUM_CHECKS=${#LP3D_BUILD_CHECK_LIST[@]}

# disable automatic restart on abnormal end
export LPUB3D_AUTO_RESTART_ENABLED=0

# disable available versions and update check
export LPUB3D_DISABLE_UPDATE_CHECK=1

echo && echo "------------Build Checks Start--------------" && echo

for LP3D_BUILD_CHECK in ${LP3D_BUILD_CHECK_LIST[@]}; do
    lp3d_check_start=$SECONDS
    LP3D_LOG_FILE="${LP3D_BUILD_CHECK}.out"
        LP3D_COUNT=$((LP3D_COUNT + 1))
    case ${LP3D_BUILD_CHECK} in
    CHECK_01)
        QT_DEBUG_PLUGINS=1
        LP3D_CHECK_LBL="Native File Process"
        LP3D_CHECK_HDR="- Check ${LP3D_COUNT} of ${NUM_CHECKS}: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="${LP3D_CONSOLE}--process-file --liblego --preferred-renderer native"
        ;;
    CHECK_02)
        LP3D_CHECK_LBL="LDGLite Export Range"
        LP3D_CHECK_HDR="- Check ${LP3D_COUNT} of ${NUM_CHECKS}: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="${LP3D_CONSOLE}--process-export --range 1-3 --clear-cache --liblego --preferred-renderer ldglite"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_PATH}/stderr-ldglite"
        ;;
    CHECK_03)
        LP3D_CHECK_LBL="Native POV Generation"
        LP3D_CHECK_HDR="- Check ${LP3D_COUNT} of ${NUM_CHECKS}: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="${LP3D_CONSOLE}--process-file --clear-cache --liblego --preferred-renderer povray"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_PATH}/stderr-povray"
        ;;
    CHECK_04)
        LP3D_CHECK_LBL="LDView File Process"
        LP3D_CHECK_HDR="- Check ${LP3D_COUNT} of ${NUM_CHECKS}: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="${LP3D_CONSOLE}--process-file --clear-cache --liblego --preferred-renderer ldview"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_PATH}/stdout-ldview"
        ;;
    CHECK_05)
        LP3D_CHECK_LBL="LDView (Single Call) File Process"
        LP3D_CHECK_HDR="- Check ${LP3D_COUNT} of ${NUM_CHECKS}: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="${LP3D_CONSOLE}--process-file --clear-cache --liblego --preferred-renderer ldview-sc"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_PATH}/stdout-ldview"
        ;;
    CHECK_06)
        LP3D_CHECK_LBL="LDView TENTE Model"
        LP3D_CHECK_HDR="- Check ${LP3D_COUNT} of ${NUM_CHECKS}: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="${LP3D_CONSOLE}--process-file --clear-cache --libtente --preferred-renderer ldview"
        LP3D_CHECK_FILE="${LP3D_CHECK_PATH}/TENTE/astromovil.ldr"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_PATH}/TENTE/stdout-ldview"
        ;;
    CHECK_07)
        LP3D_CHECK_LBL="LDView (Snapshot List) VEXIQ Model"
        LP3D_CHECK_HDR="- Check ${LP3D_COUNT} of ${NUM_CHECKS}: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="${LP3D_CONSOLE}--process-file --clear-cache --libvexiq --preferred-renderer ldview-scsl"
        LP3D_CHECK_FILE="${LP3D_CHECK_PATH}/VEXIQ/spider.mpd"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_PATH}/VEXIQ/stdout-ldview"
        ;;
      esac

    # Remove old log if exist
    if [ -f "${LP3D_LOG_FILE}" ]; then
        rm -rf "${LP3D_LOG_FILE}"
    fi

    # Initialize XVFB and execute check
    [ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} &> ${LP3D_LOG_FILE} || \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} &> ${LP3D_LOG_FILE}
    # allow some time between checks
    sleep 4
    # check output log for build check status
    if [ -f "${LP3D_LOG_FILE}" ]; then
        LP3D_ERROR=0
        if grep -q "${LP3D_CHECK_SUCCESS}" "${LP3D_LOG_FILE}"; then
            if grep -q "${LP3D_XVFB_ERROR}" "${LP3D_LOG_FILE}"; then
                LP3D_ERROR=1
            fi
            echo "${LP3D_CHECK_HDR} PASSED, ELAPSED TIME [`ElapsedCheckTime $lp3d_check_start`]"
            echo "${LP3D_CHECK_LBL} Command: ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE}"
            let LP3D_CHECK_PASS++
            LP3D_CHECKS_PASS="${LP3D_CHECKS_PASS},$(echo ${LP3D_CHECK_HDR} | cut -d " " -f 3)"
            echo
        else
            LP3D_ERROR=2
            echo "${LP3D_CHECK_HDR} FAILED, ELAPSED TIME [`ElapsedCheckTime $lp3d_check_start`]"
            let LP3D_CHECK_FAIL++
            LP3D_CHECKS_FAIL="${LP3D_CHECKS_FAIL},$(echo ${LP3D_CHECK_HDR} | cut -d " " -f 3)"
            echo "- LPub3D Log Trace: ${LP3D_LOG_FILE}"
            cat "${LP3D_LOG_FILE}"
            [ -s "${LP3D_CHECK_STDLOG}" ] && \
            echo "- Standard Error Log Trace: ${LP3D_CHECK_STDLOG}" && \
            cat "${LP3D_CHECK_STDLOG}" || true
            if [ -z "$LP3D_BUNDLED_APP" ]; then
                if [ -d "${LP3D_CHECK_PATH}" ]; then
                    [ -z "${MSYS2}" ] && \
                    cp -f "${LP3D_LOG_FILE}" "${LP3D_CHECK_PATH}" || \
                    find "$(dirname ${LPUB3D_EXE})" -maxdepth 1 -iname "LPub3D.*" -type f -exec cp -f {} "${LP3D_CHECK_PATH}" \;
                    LP3D_CHECK_ASSETS="$(ls -A ${LP3D_CHECK_PATH})"
                    if [ "${LP3D_CHECK_ASSETS}" ]; then
                        echo "${LP3D_BUILD_CHECK} assets found:" && echo "${LP3D_CHECK_ASSETS}" && \
                        echo "- Archiving assets to ${LP3D_LOG_PATH}/${LP3D_BUILD_CHECK}_assets.tar.gz" >/dev/null 2>&1
                        if tar -czvf "${LP3D_LOG_PATH}/${LP3D_BUILD_CHECK}_assets.tar.gz" "${LP3D_CHECK_PATH}/"; then
                            echo "Success"
                        else
                            echo "Oops - tar failed!"
                        fi
                    else
                        echo "Nothing to archive. Directory ${LP3D_CHECK_PATH} is empty."
                    fi
                else
                    echo "Directory ${LP3D_CHECK_PATH} was not found."
                fi
            fi
            echo
        fi
        # Report Xvfb errors
        if [ "${LP3D_ERROR}" -eq "1" ]; then
            if grep -q "${LP3D_XVFB_ERROR}" "${LP3D_LOG_FILE}"; then
                echo "- LPub3D Xvfb Log Trace: ${LP3D_LOG_FILE}"
                cat "${LP3D_LOG_FILE}"
            fi
        fi
        # Remove log file
        if [ -z "$LP3D_BUNDLED_APP" ]; then
            rm -rf "${LP3D_LOG_FILE}"
        fi
        # Cleanup check output
        find ${LP3D_CHECK_PATH} -depth -iname "LPub3D" -type d -prune -exec rm -rf {} \; >/dev/null 2>&1
        find ${LP3D_CHECK_PATH} \( -name "std*" -o -name "*.pdf" \) -type f -exec rm -rf {} \; >/dev/null 2>&1
    else
        echo "ERROR - ${LP3D_LOG_FILE} was not generated."
    fi
done

# remove leading ','
if [ "${LP3D_CHECK_PASS}" -gt "0" ]; then
    LP3D_CHECKS_PASS="$(echo ${LP3D_CHECKS_PASS} | cut -c 2-)"
fi
if [ "${LP3D_CHECK_FAIL}" -gt "0" ]; then
    LP3D_CHECKS_FAIL="$(echo ${LP3D_CHECKS_FAIL} | cut -c 2-)"
fi

# move the run log to user folder for output capture
if [ -z "$LP3D_BUNDLED_APP" ]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        RUN_LOG=$(find ${HOME}/Library/Application\ Support/LPub3D\ Software -type f -name "*Log.txt")
    elif [ -n "${MSYS2}" ]; then
        RUN_LOG=$(find "$(dirname "${LPUB3D_EXE}")" -type f -name "*Log.txt")
    else
        RUN_LOG=$(find ${HOME}/.local/share -type f -name "*Log.txt")
    fi
    if [ -n "${RUN_LOG}" ]; then
        mv -f "${RUN_LOG}" "${LP3D_LOG_PATH}/LPub3DRun.log" && \
        echo "Moved ${RUN_LOG} to ${LP3D_LOG_PATH}/LPub3DRun.log" || \
        echo "WARNING - ${RUN_LOG} was not moved to ${LP3D_LOG_PATH}/LPub3DRun.log"
    else
        echo "WARNING - RunLog [${RUN_LOG}] was not found"
    fi
fi

unset SUMMARY_MSG
SUMMARY_MSG+="----Build Check Completed: PASS (${LP3D_CHECK_PASS})[${LP3D_CHECKS_PASS}], "
(( ${LP3D_CHECK_FAIL} > 0 )) && \
SUMMARY_MSG+="FAIL (${LP3D_CHECK_FAIL})[${LP3D_CHECKS_FAIL}], " || :
SUMMARY_MSG+="ELAPSED TIME [$(ElapsedCheckTime $lp3d_elapsed_check_start)]----"
echo && echo $SUMMARY_MSG && echo
