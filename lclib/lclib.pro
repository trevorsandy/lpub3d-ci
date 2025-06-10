TEMPLATE = lib
TARGET   = LC
QT      += core
QT      += gui
QT      += widgets
QT      += opengl
QT      += network
QT      += xml
QT      += concurrent
QT      *= printsupport
CONFIG  += staticlib
CONFIG  += warn_on
CONFIG  += skip_target_version_ext

# The ABI version.
# REMINDER: Update LC_LIB = LC233 in mainApp.pro on version change
VER_MAJ = 23
VER_MIN = 3
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

# common directives
include(../common.pri)

win32:macx: \
GAMEPAD {
    qtHaveModule(gamepad) {
        QT += gamepad
        DEFINES += LC_ENABLE_GAMEPAD
    }
}

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += qt common
INCLUDEPATH += ../mainApp ../qsimpleupdater/include ../qsimpleupdater/src
INCLUDEPATH += ../qslog ../ldrawini ../quazip ../qsimpleupdater/src/progress_bar
win32-msvc*: \
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib

CONFIG += incremental precompile_header

win32 {

    QMAKE_EXT_OBJ = .obj
    DEFINES += _TC_STATIC
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
    PRECOMPILED_HEADER = common/lc_global.h
    PRECOMPILED_SOURCE = common/lc_global.cpp

    PRECOMPILED_HEADER = common/lc_global.h
    PRECOMPILED_SOURCE = common/lc_global.cpp
    DEFINES += _WINSOCKAPI_

    win32-msvc* {

        CONFIG  += windows
        CONFIG  += force_debug_info
        DEFINES += _WINSOCKAPI_
        QMAKE_LFLAGS_WINDOWS += /IGNORE:4099
        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -WX- -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            DEFINES += QT_DEBUG_MODE
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4  -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701"
            QMAKE_CFLAGS_DEBUG   += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
            QMAKE_CXXFLAGS_DEBUG += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
        }
        CONFIG(release, debug|release) {
            QMAKE_ADDL_MSVC_RELEASE_FLAGS = $$QMAKE_ADDL_MSVC_FLAGS -GF -Gy
            QMAKE_CFLAGS_OPTIMIZE += -Ob1 -Oi -Ot
            QMAKE_CFLAGS_WARN_ON  += -W1 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4714" -wd"4805"
            QMAKE_CFLAGS_RELEASE  += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
            QMAKE_CXXFLAGS_RELEASE += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
        }
        QMAKE_CXXFLAGS_WARN_ON = $$QMAKE_CFLAGS_WARN_ON
    }

    LIBS += -ladvapi32 -lshell32 -lopengl32 -lwininet -luser32
    !win32-msvc*: \
    LIBS += -lz

} else {

    PRECOMPILED_HEADER = common/lc_global.h
    LIBS += -lz

}

if (unix|msys):!macx: TARGET = $$lower($$TARGET)

#~~ LDView headers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BUILD_LDV_LIBS {
    VER_LDVIEW_DIR_PATH = $$absolute_path( ../ldvlib/LDVQt/LDView )
    VER_LDVIEW_INCLUDE  = $${VER_LDVIEW_DIR_PATH}/include
    INCLUDEPATH += $$VER_LDVIEW_DIR_PATH $$VER_LDVIEW_INCLUDE
} else {
    equals(VER_USE_LDVIEW_DEV,True): \
    VER_LDVIEW_INCLUDE = $${VER_LDVIEW_DEV_REPOSITORY}
    else: \
    VER_LDVIEW_INCLUDE = $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include
    INCLUDEPATH += $$VER_LDVIEW_INCLUDE
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DEFINES += LC_DISABLE_UPDATE_CHECK=1

# Indicate build type
staticlib {
    BUILD    = Static
    QMAKE_LFLAGS += -static
    unix|msys: \
    DEFINES += _TC_STATIC
} else {
    BUILD    = Shared
    msys: \
    CONFIG  -= staticlib
}

CONFIG(debug, debug|release) {
    BUILD += Debug Build
    ARCH_BLD = bit_debug
    macx: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d$${VER_MAJ}$${VER_MIN})
    unix:!macx: TARGET = $$join(TARGET,,,d)
} else {
    BUILD += Release Build
    ARCH_BLD = bit_release
    win32: TARGET = $$join(TARGET,,,$${VER_MAJ}$${VER_MIN})
}
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

include(lclib.pri)

# Suppress warnings
unix|msys {
QMAKE_CFLAGS_WARN_ON += \
    -Wall -W \
    -Wno-deprecated-declarations \
    -Wno-unknown-pragmas \
    -Wno-overloaded-virtual
QMAKE_CXXFLAGS_WARN_ON  = $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-deprecated-copy
} # unix|msys
if (unix|msys):!macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-implicit-fallthrough \
    -Wno-unused-parameter \
    -Wno-sign-compare \
    -Wno-unknown-pragmas \
    -Wno-strict-aliasing
msys {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-attributes
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CFLAGS_WARN_ON += \
    -Wno-misleading-indentation
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-template-id-cdtor \
    -Wno-class-memaccess \
    -Wno-stringop-overflow \
    -Wno-stringop-truncation \
    -Wno-type-limits \
    -Wno-maybe-uninitialized \
    -Wno-unused-result \
    -Wno-cpp
} else: \
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
} # unix|msys:!macx
macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-sometimes-uninitialized \
    -Wno-self-assign \
    -Wno-unused-result
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
}
