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

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
}

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
win32-arm64-msvc|win32-msvc*: \
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib

CONFIG += incremental precompile_header

win32 {
    QMAKE_EXT_OBJ = .obj
    DEFINES += _TC_STATIC
    PRECOMPILED_HEADER = common/lc_global.h
    PRECOMPILED_SOURCE = common/lc_global.cpp
    PRECOMPILED_HEADER = common/lc_global.h
    PRECOMPILED_SOURCE = common/lc_global.cpp
    DEFINES += _WINSOCKAPI_

    LIBS += -ladvapi32 -lshell32 -lopengl32 -lwininet -luser32
    !win32-arm64-msvc:!win32-msvc*: \
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
!isEmpty(DIST_DIR_NOT_FOUND_MSG): message("~~~ lib$${TARGET} $${DIST_DIR_NOT_FOUND_MSG} ~~~")
!isEmpty(CPP11_MSG):  message("~~~ lib$${TARGET} $${CPP11_MSG} ~~~")
!isEmpty(CPP17_MSG):  message("~~~ lib$${TARGET} $${CPP17_MSG} ~~~")

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

include(lclib.pri)
