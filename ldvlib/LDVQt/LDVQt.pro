TEMPLATE = lib
TARGET   = LDVQt
QT      += core
QT      += opengl
QT      += network
QT      += widgets
QT      += gui
QT      += concurrent
QT      *= printsupport
CONFIG  += thread
CONFIG  += staticlib
CONFIG  += warn_on
CONFIG  += incremental
CONFIG  += skip_target_version_ext
macx: \
CONFIG  -= app_bundle

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
    DEFINES += QOPENGLWIDGET
}

# common directives
include(../../common.pri)

# The ABI version.
VER_MAJ = 4
VER_MIN = 6
VER_PAT = 1
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch
VER_LDVIEW     = ldview-$$VER_MAJ"."$$VER_MIN

DEPENDPATH  += .
INCLUDEPATH += .
INCLUDEPATH += ../../mainApp
INCLUDEPATH += ../../lclib/common
INCLUDEPATH += ../../qslog
INCLUDEPATH += $${SYSTEM_PREFIX_}/include

DEFINES += _NO_BOOST
DEFINES += QT_THREAD_SUPPORT
!contains(DEFINES, _QT): \
DEFINES += _QT
!freebsd: \
DEFINES += EXPORT_3DS

# specify flag for ARM platforms to use System OpenGL headers
unix:!macx:contains(BUILD_ARCH,arm64)|contains(BUILD_ARCH,arm): \
UNIX_ARM_BUILD_ARCH = True

win32 {
    QMAKE_EXT_OBJ  = .obj
    DEFINES       += _TC_STATIC
    win32-arm64-msvc|win32-msvc* {
        DEFINES   += _WINSOCKAPI_
        DEFINES   += _WIN_UTF8_PATHS
    }
}

if (unix|msys):!macx: TARGET = $$lower($$TARGET)

# Indicate build type
staticlib {
    BUILD    = $${BUILD} Static
    QMAKE_LFLAGS += -static
    unix|msys: \
    DEFINES += _TC_STATIC
} else {
    BUILD    = $${BUILD} Shared
    msys: \
    CONFIG  -= staticlib
}

CONFIG(debug, debug|release) {
    DEFINES      += QT_DEBUG_MODE
    BUILD_CONF    = Debug
    ARCH_BLD      = bit_debug
    macx: TARGET  = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d$${VER_MAJ}$${VER_MIN})
    unix:!macx: TARGET = $$join(TARGET,,,d)
    equals(VER_USE_LDVIEW_DEV,True): \
    INCLUDEPATH  += $${VER_LDVIEW_DEV_REPOSITORY}
} else {
    BUILD_CONF    = Release
    ARCH_BLD      = bit_release
    win32: TARGET = $$join(TARGET,,,$${VER_MAJ}$${VER_MIN})
}

TARGET = $${TARGET}$${POSTFIX}
BUILD += $$BUILD_CONF Build
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

#~~ LDView headers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LOAD_LDV_HEADERS = True
include(LDVQtLibs.pri)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")
win32-arm64-msvc|win32-msvc*: message("~~~ lib$${TARGET} QMAKE_MSC_VER $$QMAKE_MSC_VER ~~~")
!isEmpty(DIST_DIR_NOT_FOUND_MSG): message("~~~ lib$${TARGET} $${DIST_DIR_NOT_FOUND_MSG} ~~~")
!isEmpty(LDVIEW_DEV_REPO_MSG): message("~~~ lib$${TARGET} $${LDVIEW_DEV_REPO_MSG} ~~~")
!isEmpty(LDVIEW_DEV_HDRS_MSG): message("~~~ lib$${TARGET} $${LDVIEW_DEV_HDRS_MSG} ~~~")
!isEmpty(VS_BUILD_WORKER_MSG): message("~~~ lib$${TARGET} $${VS_BUILD_WORKER_MSG} ~~~")
!isEmpty(VS_LIBRARY_VER_MSG): message("~~~ lib$${TARGET} $${VS_LIBRARY_VER_MSG} ~~~")
!isEmpty(CPP11_MSG): message("~~~ lib$${TARGET} $${CPP11_MSG} ~~~")
!isEmpty(CPP17_MSG): message("~~~ lib$${TARGET} $${CPP17_MSG} ~~~")

# specify ARM platforms that need to suppress local glext.h header
contains(UNIX_ARM_BUILD_ARCH,True): contains(BUILD_TARGET,suse): contains(HOST_VERSION,1320) {
    DEFINES += ARM_SKIP_GL_HEADERS
    message("~~~ lib$${TARGET} $$upper($$QT_ARCH) BUILD - $${BUILD_TARGET}-$${HOST_VERSION}-$${BUILD_ARCH} define SKIP_LDV_GL_HEADERS ~~~")
} else {
    message("~~~ lib$${TARGET} $$upper($$QT_ARCH) BUILD - $${BUILD_TARGET}-$${HOST_VERSION}-$${BUILD_ARCH} ~~~")
}

OBJECTS_DIR     = $$DESTDIR/.obj$${POSTFIX}
MOC_DIR         = $$DESTDIR/.moc$${POSTFIX}
RCC_DIR         = $$DESTDIR/.qrc$${POSTFIX}
UI_DIR          = $$DESTDIR/.ui$${POSTFIX}

# USE GNU_SOURCE
unix|msys: DEFINES += _GNU_SOURCE

# stdlib.h fix placeholder - do not remove

#~~ source and headers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SOURCES += \
    LDVAlertHandler.cpp \
    LDViewExportOption.cpp \
    LDVHtmlInventory.cpp \
    LDViewPartList.cpp \
    LDVMisc.cpp \
    LDVPreferences.cpp \
    LDVWidget.cpp
win32:!contains(DEFINES, _OSMESA): \
SOURCES += \
    LDVExtensionsSetup.cpp

HEADERS += \
    LDVWidgetDefaultKeys.h \
    LDVAlertHandler.h \
    LDVHtmlInventory.h \
    LDViewExportOption.h \
    LDViewPartList.h \
    LDVMisc.h \
    LDVPreferences.h \
    LDVWidget.h
win32:!contains(DEFINES, _OSMESA): \
HEADERS += \
    LDVExtensionsSetup.h

FORMS += \
    LDVExportOptionPanel.ui \
    LDVPreferencesPanel.ui \
    LDVPartList.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    LDVWidgetMessages.ini
