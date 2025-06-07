TEMPLATE = app
TARGET   = LPub3D
QT      += core
QT      += gui
QT      += opengl
QT      += network
QT      += xml
QT      += concurrent
QT      *= printsupport
CONFIG  += warn_on
CONFIG  += exceptions
CONFIG  += incremental

lessThan(QT_MAJOR_VERSION, 5) {
    error("$${TARGET} requires Qt5.4 or later.")
}

equals(  QT_MAJOR_VERSION, 5): \
lessThan(QT_MINOR_VERSION, 4) {
    error("$${TARGET} requires Qt5.4 or later.")
}

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core5compat openglwidgets
    DEFINES += QOPENGLWIDGET
}

CONFIG(debug, debug|release) { LPUB3D = $${TARGET}d } else { LPUB3D = $${TARGET} }

include(../gitversion.pri)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32:macx: \
GAMEPAD {
    qtHaveModule(gamepad) {
        QT += gamepad
        DEFINES += LC_ENABLE_GAMEPAD
    }
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

win32 {

    QMAKE_TARGET_COMPANY = "$${TARGET} Software"
    QMAKE_TARGET_DESCRIPTION = "$${TARGET} - An LDraw Building Instruction Editor."
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2015 - 2025 Trevor SANDY"
    QMAKE_TARGET_PRODUCT = "$${TARGET} ($$join(ARCH,,,bit))"
    RC_LANG = "English (United Kingdom)"
    RC_ICONS = "lpub3d.ico"

    QMAKE_EXT_OBJ = .obj

    win32-msvc* {

        CONFIG  += windows
        CONFIG  += force_debug_info
        DEFINES += _WINSOCKAPI_
        DEFINES += QT_NODLL
        DEFINES += _WIN_UTF8_PATHS
        QMAKE_CXXFLAGS_RELEASE += /FI winsock2.h /FI winsock.h
        QMAKE_LFLAGS += -NODEFAULTLIB:LIBCMT
        QMAKE_LFLAGS_WINDOWS += /STACK:4194304 /IGNORE:4099
        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -WX- -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4  -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701" -wd"4714" -wd"4305" -wd"4099"
            QMAKE_CFLAGS_DEBUG   += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
            QMAKE_CXXFLAGS_DEBUG += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
        }
        CONFIG(release, debug|release) {
            QMAKE_ADDL_MSVC_RELEASE_FLAGS = $$QMAKE_ADDL_MSVC_FLAGS -GF -Gy
            QMAKE_CFLAGS_OPTIMIZE += -Ob1 -Oi -Ot
            QMAKE_CFLAGS_WARN_ON  += -W1 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4805" -wd"4838" -wd"4700" -wd"4098"
            QMAKE_CFLAGS_RELEASE  += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
            QMAKE_CXXFLAGS_RELEASE += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
        }
        QMAKE_CXXFLAGS_WARN_ON = $$QMAKE_CFLAGS_WARN_ON
    } else: msys {
        QMAKE_LFLAGS += -Wl,--allow-multiple-definition
    }
} else {
    macx: \
    LIBS += -framework CoreFoundation -framework CoreServices
}

if (unix|msys):!macx: \
TARGET      = $$lower($$TARGET)
STG_TARGET  = $${TARGET}
DIST_TARGET = $${TARGET}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HOST_VERSION   = $$(PLATFORM_VER)
BUILD_TARGET   = $$(TARGET_VENDOR)
BUILD_ARCH     = $$(TARGET_CPU)

# platform architecture, name and version fallback
!contains(QT_ARCH, unknown): BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):   BUILD_ARCH = $$system(uname -m)
isEmpty(BUILD_ARCH):         BUILD_ARCH = UNKNOWN ARCH
isEmpty(BUILD_TARGET) {
    msys:BUILD_TARGET = MSYS2
    win32-msvc*:BUILD_TARGET = $$system(systeminfo | findstr /B /C:\"OS Name\")
    unix:!macx:BUILD_TARGET = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
    macx:BUILD_TARGET = $$system(echo `sw_vers -productName`)
}
isEmpty(HOST_VERSION) {
    win32-msvc*:HOST_VERSION = $$system(systeminfo | findstr /B /C:\"OS Version\")
    unix:!macx:HOST_VERSION = $$system(. /etc/os-release 2>/dev/null; [ -n \"$VERSION_ID\" ] && echo \"$VERSION_ID\")
    macx:HOST_VERSION = $$system(echo `sw_vers -productVersion`)
    mingw:ide_qtcreator:HOST_VERSION = MinGW_2025
    else:msys:HOST_VERSION = $$system(VER=$(echo $(uname -a) | grep -oP \"\b\d{4}-\d{2}-\d{2}\b\") && echo ${MSYSTEM}_${VER//-/.})
}

message("~~~ $${LPUB3D} $$upper($${TARGET}) $$upper($$QT_ARCH) BUILD: $${BUILD_TARGET}-$${HOST_VERSION}-$${BUILD_ARCH} ~~~")
message("~~~ $${LPUB3D} BUILDING WITH QT VERSION: $$QT_VERSION ~~~")

# for aarch64, QT_ARCH = arm64, for arm7l, QT_ARCH = arm
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH     = 64
    STG_ARCH = x86_64
    LIB_ARCH = 64
} else {
    ARCH     = 32
    STG_ARCH = x86
    LIB_ARCH =
}

# define chipset
if (contains(QT_ARCH, arm)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)): \
CHIPSET  = ARM
else: \
CHIPSET  = AMD

DEFINES += VER_ARCH=\\\"$$ARCH\\\"
DEFINES += VER_CHIPSET=\\\"$$CHIPSET\\\"

# special case for OpenSuse 1320
contains(HOST_VERSION, 1320):contains(BUILD_TARGET, suse):contains(BUILD_ARCH, aarch64): \
DEFINES += OPENSUSE_1320_ARM

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

VER_LDVIEW  = ldview-4.6
VER_LDGLITE = ldglite-1.3
VER_POVRAY  = lpub3d_trace_cui-3.8
DEFINES    += VER_LDVIEW=\\\"$$VER_LDVIEW\\\"
DEFINES    += VER_LDGLITE=\\\"$$VER_LDGLITE\\\"
DEFINES    += VER_POVRAY=\\\"$$VER_POVRAY\\\"

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DEPENDPATH  += .
INCLUDEPATH += .
INCLUDEPATH += ../lclib/common ../lclib/qt ../ldvlib ../waitingspinner ../ldrawini jsonconfig
win32-msvc*: \
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

msys: \
SYSTEM_PREFIX_ = $${PREFIX}
else: \
SYSTEM_PREFIX_ = $${PREFIX}/usr/local
macx {
    contains(QT_ARCH,arm64): \
    SYSTEM_PREFIX_ = /opt/homebrew
    CONFIG += sdk_no_version_check
}

#~~~~ third party distro folder ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# - Set enviroinment variable LP3D_DIST_DIR_PATH as needed.
# - 3rd party libraries, executables, documentation and resources.
# - When building on macOS, it is necessary to add CONFIG+=dmg at
#   Projects/Build Steps/Qmake/'Additional arguments' because,
#   macOS build will also bundle all deliverables.

#   Argument path - LP3D_3RD_DIST_DIR
!isEmpty(LP3D_3RD_DIST_DIR) {
    THIRD_PARTY_DIST_DIR_PATH = $$LP3D_3RD_DIST_DIR
    3RD_DIR_SOURCE = LP3D_3RD_DIST_DIR
} else {
#   Environment variable path - LP3D_DIST_DIR_PATH
    THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
    !isEmpty(THIRD_PARTY_DIST_DIR_PATH): \
    3RD_DIR_SOURCE = LP3D_DIST_DIR_PATH
}
#   Local path
isEmpty(THIRD_PARTY_DIST_DIR_PATH): \
THIRD_PARTY_DIST_DIR_PATH     = $$absolute_path( $$PWD/../builds/3rdparty )
exists($$THIRD_PARTY_DIST_DIR_PATH): \
3RD_DIR_SOURCE = LOCAL_3RD_DIST_DIR
else {
    unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
    else:msys:  DIST_DIR      = lpub3d_msys_3rdparty
    else:macx:  DIST_DIR      = lpub3d_macos_3rdparty
    else:win32: DIST_DIR      = lpub3d_windows_3rdparty
    THIRD_PARTY_DIST_DIR_PATH = $$absolute_path( $$_PRO_FILE_PWD_/../../$$DIST_DIR )
    exists($$THIRD_PARTY_DIST_DIR_PATH) {
        3RD_DIR_SOURCE_UNSPECIFIED = "INFO - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED, USING $$THIRD_PARTY_DIST_DIR_PATH"
    } else {
        3RD_DIR_SOURCE_UNSPECIFIED = "ERROR - THIRD_PARTY_DIST_DIR_PATH WAS NOT SPECIFIED!"
        THIRD_PARTY_DIST_DIR_PATH="undefined"
    }
    3RD_DIR_SOURCE = DEFAULT_3RD_PARTY_DIR
}

#~~ LDVQt dependencies ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VER_USE_LDVIEW_DEV = False
CONFIG(debug, debug|release) {
    # These lines requires a git extract of ldview at the same location as the lpub3d git extract
    # and defines the ldview git extract folder name, you can set as you like
    mingw:ide_qtcreator: VER_LDVIEW_DEV = undefined
    else:unix|msys:      VER_LDVIEW_DEV = ldview
    else:win32-msvc*:    VER_LDVIEW_DEV = ldview_vs_build
    # This line defines the path of the ldview git extract relative to this project file
    VER_LDVIEW_DEV_REPOSITORY = $$absolute_path( $$PWD/../../$${VER_LDVIEW_DEV} )
    exists($$VER_LDVIEW_DEV_REPOSITORY) {
        VER_USE_LDVIEW_DEV = True
        message("~~~ $${LPUB3D} LINK LDVQt USING LDVIEW DEVELOPMENT REPOSITORY ~~~ ")
    } else:!msys {
        message("~~~ $${LPUB3D} WARNING - COULD NOT LOAD LDVIEW DEV FROM: $$VER_LDVIEW_DEV_REPOSITORY ~~~ ")
    }
}
# Load LDView libraries for LDVQt
LOAD_LDV_LIBS = True
!freebsd: \
DEFINES += EXPORT_3DS

#~~ ldsearchDirs (uses LDView's LDrawIni)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if (equals(VER_USE_LDVIEW_DEV,True)) {
    INCLUDEPATH += $${VER_LDVIEW_DEV_REPOSITORY}/3rdParty
} else {
    INCLUDEPATH += $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include/3rdParty
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# USE CPP 11
contains(USE_CPP11,NO) {
    message("NO CPP11")
} else {
    DEFINES += USE_CPP11
}

# Greater than or equal to Qt 5.4
greaterThan(QT_MAJOR_VERSION, 4): \
greaterThan(QT_MINOR_VERSION, 3) {
    win32-msvc* {
        QMAKE_CXXFLAGS += /std:c++17
    } else:unix|msys {
        greaterThan(QT_MINOR_VERSION, 11) {
            CONFIG += c++17
        } else {
            QMAKE_CXXFLAGS += -std=c++17
        }
    }
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static {                                     # everything below takes effect with CONFIG ''= static
    BUILD    = Static
    CONFIG  += static
    LIBS    += -static
    DEFINES += STATIC
    DEFINES += QUAZIP_STATIC                 # this is so the compiler can detect quazip static
    macx:  TARGET = $$join(TARGET,,,_static) # this adds an _static in the end, so you can seperate static build from non static build
    win32: TARGET = $$join(TARGET,,,s)       # this adds an s in the end, so you can seperate static build from non static build
} else {
    BUILD   = Shared
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# NOTE: Reminder to update MacOS library links on QuaZip and LDrawIni major version change
#       - Files inpacted: build_checks.sh, CreateDmg.sh and macosfiledistro.pri

# LDVQT Qt/OSMesa/WGL library identifiers
ldviewqt: \
POSTFIX  = -qt$${QT_MAJOR_VERSION}
else:msys:ldviewwgl: \
POSTFIX  = -wgl
else:!win32-msvc*: \
POSTFIX  = -osmesa

# Always use local QuaZip for added minizip unzOpen calls
CONFIG(debug, debug|release) {
    DEFINES += QT_DEBUG_MODE
    ARCH_BLD = bit_debug
    BUILD_CONF = Debug

    win32 {
        LDRAWINI_LIB = LDrawInid161
        QUAZIP_LIB = QuaZIPd11
        LC_LIB = LCd233
        LDVQT_LIB = LDVQtd46
        WPNGIMAGE_LIB = WPngImaged14
        WAITING_SPINNER_LIB = WaitingSpinnerd10
        msys {
            LDRAWINI_LIB = $$lower($$LDRAWINI_LIB)
            QUAZIP_LIB = $$lower(lib$$QUAZIP_LIB)
            LC_LIB = $$lower($$LC_LIB)
            LDVQT_LIB = $$lower($$LDVQT_LIB)
            WPNGIMAGE_LIB = $$lower($$WPNGIMAGE_LIB)
            WAITING_SPINNER_LIB = $$lower($$WAITING_SPINNER_LIB)
        }
    }

    macx {
        LDRAWINI_LIB = LDrawIni_debug
        QUAZIP_LIB = libQuaZIP_debug
        LC_LIB = LC_debug
        LDVQT_LIB = LDVQt_debug
        WPNGIMAGE_LIB = WPngImage_debug
        WAITING_SPINNER_LIB = WaitingSpinner_debug
    }

    unix:!macx {
        LDRAWINI_LIB = ldrawinid
        QUAZIP_LIB = libquazipd
        LC_LIB = lcd
        LDVQT_LIB = ldvqtd
        WPNGIMAGE_LIB = wpngimaged
        WAITING_SPINNER_LIB = waitingspinnerd

        # For Linux builds, simplify debug ops by using runtime content in build folders
        DEFINES += DEBUG_MODE_USE_BUILD_FOLDERS
        message("~~~ $${LPUB3D} INFO - DEBUG-MODE RUNTIME PATHS USING BUILD FOLDERS ~~~")
    }

    LDVQT_LIB = $${LDVQT_LIB}$${POSTFIX}

    # executable target name
    macx:       TARGET = $$join(TARGET,,,_debug)
    win32:      TARGET = $$join(TARGET,,,d)
    unix:!macx: TARGET = $$join(TARGET,,,d$$VER_MAJOR$$VER_MINOR)

    # enable copy ldvMessages to OUT_PWD/mainApp/extras (except macOS)
    !macx:DEVL_LDV_MESSAGES_INI = True

} else {

    ARCH_BLD = bit_release
    BUILD_CONF = Release

    win32 {
        LDRAWINI_LIB = LDrawIni161
        QUAZIP_LIB = QuaZIP11
        LC_LIB = LC233
        LDVQT_LIB = LDVQt46
        WPNGIMAGE_LIB = WPngImage14
        WAITING_SPINNER_LIB = WaitingSpinner10
        msys {
            LDRAWINI_LIB = $$lower($$LDRAWINI_LIB)
            QUAZIP_LIB = $$lower(lib$$QUAZIP_LIB)
            LC_LIB = $$lower($$LC_LIB)
            LDVQT_LIB = $$lower($$LDVQT_LIB)
            WPNGIMAGE_LIB = $$lower($$WPNGIMAGE_LIB)
            WAITING_SPINNER_LIB = $$lower($$WAITING_SPINNER_LIB)
        }
    }

    macx {
        LDRAWINI_LIB = LDrawIni
        QUAZIP_LIB = libQuaZIP
        LC_LIB = LC
        LDVQT_LIB = LDVQt
        WPNGIMAGE_LIB = WPngImage
        WAITING_SPINNER_LIB = WaitingSpinner
    }

    unix:!macx {
        LDRAWINI_LIB = ldrawini
        QUAZIP_LIB = libquazip
        LC_LIB = lc
        LDVQT_LIB = ldvqt
        WPNGIMAGE_LIB = wpngimage
        WAITING_SPINNER_LIB = waitingspinner
    }

    LDVQT_LIB = $${LDVQT_LIB}$${POSTFIX}

    # executable target
    !macx:!win32: TARGET = $$join(TARGET,,,$$VER_MAJOR$$VER_MINOR)
}

#~~build path components~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

BUILD += $$BUILD_CONF

#manpage
MAN_PAGE = $$join(TARGET,,,.1)

message("~~~ $${LPUB3D} LPUB3D $$join(ARCH,,,bit) $$upper($${BUILD}) ($${TARGET}) $${CHIPSET} CHIPSET ~~~")

#~~file distributions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

!isEmpty(3RD_DIR_SOURCE_UNSPECIFIED): \
message("~~~ $${LPUB3D} $$3RD_DIR_SOURCE_UNSPECIFIED ~~~")
message("~~~ $${LPUB3D} 3RD PARTY DISTRIBUTION REPO ($$3RD_DIR_SOURCE): $$THIRD_PARTY_DIST_DIR_PATH ~~~")

#~~configuration options~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# To stage and install LPub3D content, a configuration option must be specified
# using CONFIG+=<option>. When building from QtCreator, set CONFIG+=<option> in
# project additional arguments. When building from a script call, you can set
# CONFIG+=<option> as a qmake argument. Example CONFIG additions:
# CONFIG-=release CONFIG-=debug_and_release CONFIG+=stagerenderers CONFIG+=exe
config_options = exe dmg deb rpm pkg api snp flp con msys
for(config_option, config_options) {
    contains(CONFIG, $$config_option): \
    option = $$config_option
}
if(!isEmpty(option)) {
    contains(option, api) {
        DEFINES += LP3D_APPIMAGE
        DISTRO_PACKAGE = AppImage
    } else:contains(option, snp) {
        DEFINES += LP3D_SNAP
        DISTRO_PACKAGE = Snap
    } else:contains(option, flp) {
        DEFINES += LP3D_FLATPACK
        DISTRO_PACKAGE = FlatPak
    } else:contains(option, con) {
        DEFINES += LP3D_CONDA
        DISTRO_PACKAGE = Conda
    } else:contains(option, msys) {
        DEFINES += LP3D_MSYS2
        DISTRO_PACKAGE = $$(MSYSTEM)
        isEmpty(DISTRO_PACKAGE): \
        DISTRO_PACKAGE = MSYS2
        isEmpty(INSTALL_PREFIX): INSTALL_PREFIX = $$PREFIX
        message("~~~ $${LPUB3D} MSYS2 INSTALL_PREFIX $${INSTALL_PREFIX} ~~~")
    } else {
        contains(option, exe): DISTRO_PACKAGE = Windows
        contains(option, dmg): DISTRO_PACKAGE = macOS
        contains(option, deb): DISTRO_PACKAGE = Debian
        contains(option, rpm): DISTRO_PACKAGE = RedHat
        contains(option, pkg): DISTRO_PACKAGE = Arch
        IN_APP_UPDATE = Enabled
    }

    !equals(IN_APP_UPDATE, Enabled): DEFINES += DISABLE_IN_APP_UPDATE

    message("~~~ $${LPUB3D} BUILD DISTRIBUTION PACKAGE: $$DISTRO_PACKAGE ($$option) ~~~")

    if (unix|msys|install3rd) {
        CONFIG += install3rdexe
        CONFIG += install3rdassets
        CONFIG += install3rdconfig
        CONFIG += install3rdcontent
        macx:include(macosfiledistro.pri)
        else:include(posixfiledistro.pri)
    }

    win32-msvc* {
        CONFIG += stage3rdexe
        CONFIG += stage3rdassets
        CONFIG += stage3rdconfig
        CONFIG += stage3rdcontent
        include(winfiledistro.pri)
    }
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AUTO_RESTART = 0
AUTO_RESTART_STR = NOT DEFINED
AUTO_RESTART_ENABLED = $$(LPUB3D_AUTO_RESTART_ENABLED)
!isEmpty(AUTO_RESTART_ENABLED) {
    AUTO_RESTART_STR = SET TO DISABLED
    equals(AUTO_RESTART_ENABLED, 1) {
        AUTO_RESTART = 1
        AUTO_RESTART_STR = SET TO ENABLED
    }
    DEFINES += AUTO_RESTART=$$AUTO_RESTART
}
message("~~~ $${LPUB3D} AUTO_RESTART ENVIRONMENT VARIABLE $${AUTO_RESTART_STR} ~~~ ")

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Specify LDView modules develpment or third party header paths
equals(VER_USE_LDVIEW_DEV,True) {
    INCLUDEPATH += $${VER_LDVIEW_DEV_REPOSITORY} $${VER_LDVIEW_DEV_REPOSITORY}/include
} else {
    INCLUDEPATH +=$${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include
}

# Needed to access ui header from LDVQt
INCLUDEPATH += $$absolute_path( $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR/.ui )

# System headers
INCLUDEPATH += $${SYSTEM_PREFIX_}/include

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# 18/11/2024 - Always use local QuaZip for added minizip unzOpen calls to accommodate LDView
INCLUDEPATH += ../quazip

win32-msvc* {
    EXT_S = lib
    EXT_D = so
} else {
    EXT_S = a
    EXT_D = dll
}

#~~ includes~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include(../qslog/QsLog.pri)
include(../qsimpleupdater/QSimpleUpdater.pri)

#~~~ libraries~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# 18/11/2024 - Always use local QuaZip for added minizip unzOpen calls to accommodate LDView
LIBS += $$absolute_path($$OUT_PWD/../quazip/$${DESTDIR}/$${QUAZIP_LIB}.$${EXT_S})
#message("~~~ DEBUG_QUAZIP_LIB: $$absolute_path($$OUT_PWD/../quazip/$${DESTDIR}/$${QUAZIP_LIB}.$${EXT_S}) ~~~")

LIBS += -L$$absolute_path($$OUT_PWD/../ldrawini/$$DESTDIR) -l$$LDRAWINI_LIB

LIBS += -L$$absolute_path($$OUT_PWD/../waitingspinner/$$DESTDIR) -l$$WAITING_SPINNER_LIB

LIBS += -L$$absolute_path($$OUT_PWD/../lclib/$$DESTDIR) -l$$LC_LIB

LIBS += -L$$absolute_path($$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR) -l$$LDVQT_LIB

# WPngImage must follow LDVQT or else there will be compile errors
LIBS += -L$$absolute_path($$OUT_PWD/../ldvlib/WPngImage/$$DESTDIR) -l$$WPNGIMAGE_LIB

include(../ldvlib/LDVQt/LDVQtLibs.pri)

msys {
    ldviewosmesa: \
    OPENGL_LIBS   = $${SYSTEM_PREFIX_}/lib/osmesa.dll.a \
                    $${SYSTEM_PREFIX_}/lib/opengl32.dll.a \
                    -L$${SYSTEM_PREFIX_}/lib -lglu32
    else: \
    OPENGL_LIBS   = -L$${SYSTEM_PREFIX_}/lib -lOpenGL32 $$QMAKE_LIBS_OPENGL
    MSYS_LIBS_MS  = -lucrt -lwinmm -lcomdlg32 -lole32 -lbz2
    MSYS_LIBS_GUI = $$QMAKE_LIBS_GUI
} else:win32-msvc* {
    OPENGL_LIBS   = -lopengl32 -lglu32
}
win32: \
LIBS += -lshlwapi -ladvapi32 $$MSYS_LIBS_MS -lshell32 -lwininet -luser32 \
        -lgdi32 $$QMAKE_LIBS_NETWORK $$OPENGL_LIBS $$MSYS_LIBS_GUI
else:!macx: \
LIBS += -lGL -lGLU
!win32-msvc*: \
LIBS += -lz

#~~ update check ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# set config to enable/disable initial update check
# CONFIG+=disable_update_check
disable_update_check: \
DEFINES += DISABLE_UPDATE_CHECK

#~~ source and headers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#message("~~~ DEBUG_CONFIG_$$upper($$TARGET): $$CONFIG ~~~")
#message("~~~ DEBUG_INCLUDEPATH_$$upper($$TARGET): $$INCLUDEPATH ~~~")
#message("~~~ DEBUG_LIBS_$$upper($$TARGET): $$LIBS ~~~")

#~~ source and headers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HEADERS += \
    aboutdialog.h \
    annotations.h \
    application.h \
    archiveparts.h \
    backgrounddialog.h \
    backgrounditem.h \
    blenderpreferences.h \
    bomoptionsdialog.h \
    borderdialog.h \
    borderedlineitem.h \
    callout.h \
    calloutbackgrounditem.h \
    calloutpointeritem.h \
    color.h \
    commands.h \
    commands/command.h \
    commands/commandcollection.h \
    commands/commandcompleter.h \
    commands/commandlistmodel.h \
    commands/commandsdialog.h \
    commands/commandstablemodel.h \
    commands/commandstextedit.h \
    commands/filterlineedit.h \
    commands/jsoncommandtranslator.h \
    commands/jsoncommandtranslatorfactory.h \
    commands/snippets/jsonsnippettranslator.h \
    commands/snippets/jsonsnippettranslatorfactory.h \
    commands/snippets/snippet.h \
    commands/snippets/snippetcollection.h \
    commands/snippets/snippetcompleter.h \
    commands/snippets/snippetlistmodel.h \
    commands/snippets/snippetstablemodel.h \
    commonmenus.h \
    csiannotation.h \
    csiitem.h \
    declarations.h \
    dependencies.h \
    dialogexportpages.h \
    dividerdialog.h \
    dividerpointeritem.h \
    editwindow.h \
    excludedparts.h \
    fadehighlightdialog.h \
    findreplace.h \
    globals.h \
    gradients.h \
    highlighter.h \
    highlightersimple.h \
    historylineedit.h \
    hoverpoints.h \
    ldrawcolordialog.h \
    ldrawcolourparts.h \
    ldrawfiles.h \
    ldrawfilesload.h \
    ldsearchdirs.h \
    lgraphicsscene.h \
    lgraphicsview.h \
    lpub.h \
    lpub_object.h \
    lpub_preferences.h \
    lpub_qtcompat.h \
    messageboxresizable.h \
    meta.h \
    metagui.h \
    metaitem.h \
    metatypes.h \
    numberitem.h \
    options.h \
    pageattributepixmapitem.h \
    pageattributetextitem.h \
    pagebackgrounditem.h \
    pageorientationdialog.h \
    pagepointer.h \
    pagepointerbackgrounditem.h \
    pagepointeritem.h \
    pagesizedialog.h \
    pagesizes.h \
    pairdialog.h \
    parmshighlighter.h \
    parmswindow.h \
    paths.h \
    placement.h \
    placementdialog.h \
    pli.h \
    pliannotationdialog.h \
    pliconstraindialog.h \
    plisortdialog.h \
    plisubstituteparts.h \
    pointer.h \
    pointerattribdialog.h \
    pointeritem.h \
    pointerplacementdialog.h \
    preferencesdialog.h \
    preferredrendererdialog.h \
    range.h \
    range_element.h \
    ranges.h \
    ranges_element.h \
    ranges_item.h \
    render.h \
    renderdialog.h \
    reserve.h \
    resize.h \
    resolution.h \
    rotateiconitem.h \
    rotateiconsizedialog.h \
    rotstepdialog.h \
    rx.h \
    scaledialog.h \
    separatorcombobox.h \
    sizeandorientationdialog.h \
    step.h \
    stickerparts.h \
    submodelcolordialog.h \
    submodelitem.h \
    substitutepartdialog.h \
    texteditdialog.h \
    textitem.h \
    threadworkers.h \
    updatecheck.h \
    version.h \
    where.h

SOURCES += \
    aboutdialog.cpp \
    annotations.cpp \
    application.cpp \
    archiveparts.cpp \
    assemglobals.cpp \
    backgrounddialog.cpp \
    backgrounditem.cpp \
    blenderpreferences.cpp \
    bomoptionsdialog.cpp \
    borderdialog.cpp \
    borderedlineitem.cpp \
    callout.cpp \
    calloutbackgrounditem.cpp \
    calloutglobals.cpp \
    calloutpointeritem.cpp \
    color.cpp \
    commands/commandcollection.cpp \
    commands/commandcompleter.cpp \
    commands/commandlistmodel.cpp \
    commands/commandsdialog.cpp \
    commands/commandstablemodel.cpp \
    commands/commandstextedit.cpp \
    commands/filterlineedit.cpp \
    commands/jsoncommandtranslator.cpp \
    commands/snippets/jsonsnippettranslator.cpp \
    commands/snippets/snippetcollection.cpp \
    commands/snippets/snippetcompleter.cpp \
    commands/snippets/snippetlistmodel.cpp \
    commands/snippets/snippetstablemodel.cpp \
    commandline.cpp \
    commands.cpp \
    commonmenus.cpp \
    csiannotation.cpp \
    csiitem.cpp \
    dependencies.cpp \
    dialogexportpages.cpp \
    dividerdialog.cpp \
    dividerpointeritem.cpp \
    editwindow.cpp \
    excludedparts.cpp \
    fadehighlightdialog.cpp \
    fadestepglobals.cpp \
    findreplace.cpp \
    formatpage.cpp \
    gradients.cpp \
    highlighter.cpp \
    highlightersimple.cpp \
    highlightstepglobals.cpp \
    historylineedit.cpp \
    hoverpoints.cpp \
    lclibpreferences.cpp \
    ldrawcolordialog.cpp \
    ldrawcolourparts.cpp \
    ldrawfiles.cpp \
    ldrawfilesload.cpp \
    ldrawpartdialog.cpp \
    ldsearchdirs.cpp \
    lgraphicsscene.cpp \
    lgraphicsview.cpp \
    lpub.cpp \
    lpub_object.cpp \
    lpub_preferences.cpp \
    messageboxresizable.cpp \
    meta.cpp \
    metagui.cpp \
    metaitem.cpp \
    multistepglobals.cpp \
    numberitem.cpp \
    openclose.cpp \
    pageattributepixmapitem.cpp \
    pageattributetextitem.cpp \
    pagebackgrounditem.cpp \
    pageglobals.cpp \
    pageorientationdialog.cpp \
    pagepointer.cpp \
    pagepointerbackgrounditem.cpp \
    pagepointeritem.cpp \
    pagesizedialog.cpp \
    pagesizes.cpp \
    pairdialog.cpp \
    parmshighlighter.cpp \
    parmswindow.cpp \
    paths.cpp \
    placement.cpp \
    placementdialog.cpp \
    pli.cpp \
    pliannotationdialog.cpp \
    pliconstraindialog.cpp \
    pliglobals.cpp \
    plisortdialog.cpp \
    plisubstituteparts.cpp \
    pointerattribdialog.cpp \
    pointeritem.cpp \
    pointerplacementdialog.cpp \
    preferencesdialog.cpp \
    preferredrendererdialog.cpp \
    printfile.cpp \
    projectglobals.cpp \
    range.cpp \
    range_element.cpp \
    ranges.cpp \
    ranges_element.cpp \
    ranges_item.cpp \
    render.cpp \
    renderdialog.cpp \
    reserve.cpp \
    resize.cpp \
    resolution.cpp \
    rotate.cpp \
    rotateiconitem.cpp \
    rotateiconsizedialog.cpp \
    rotstepdialog.cpp \
    rx.cpp \
    scaledialog.cpp \
    separatorcombobox.cpp \
    sizeandorientationdialog.cpp \
    step.cpp \
    stickerparts.cpp \
    submodelcolordialog.cpp \
    submodelglobals.cpp \
    submodelitem.cpp \
    substitutepartdialog.cpp \
    texteditdialog.cpp \
    textitem.cpp \
    threadworkers.cpp \
    traverse.cpp \
    undoredo.cpp \
    updatecheck.cpp \
    visualeditor.cpp

FORMS += \
    aboutdialog.ui \
    dialogexportpages.ui \
    ldrawcolordialog.ui \
    ldrawfilesloaddialog.ui \
    ldrawpartdialog.ui \
    preferences.ui \
    renderdialog.ui \
    substitutepartdialog.ui \
    texteditdialog.ui

include(otherfiles.pri)

RESOURCES += \
    ../lclib/lclib.qrc \
    ../lclib/resources/stylesheet/stylesheet.qrc \
    ../ldvlib/LDVQt/resources.qrc \
    resources/lsynth/lsynth.qrc \
    lpub3d.qrc

#~~ suppress warnings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

!win32-msvc*:!macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-deprecated-declarations \
    -Wno-deprecated-copy \
    -Wno-unused-parameter \
    -Wno-sign-compare
}

macx {

QMAKE_CFLAGS_WARN_ON += \
    -Wall -W \
    -Wno-deprecated-copy \
    -Wno-deprecated-declarations \
    -Wno-overloaded-virtual \
    -Wno-self-assign \
    -Wno-sometimes-uninitialized \
    -Wno-unused-result
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}

} else: win32 {

!win32-msvc* {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-attributes
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}

QMAKE_CFLAGS_WARN_ON += \
   -Wno-misleading-indentation
QMAKE_CXXFLAGS_WARN_ON += \
   -Wno-cpp \
   -Wno-implicit-fallthrough \
   -Wno-maybe-uninitialized \
   -Wno-strict-aliasing \
   -Wno-unused-result
}

} else {

QMAKE_CFLAGS_WARN_ON += \
    -Wno-strict-aliasing
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}

}

#message($$CONFIG)
