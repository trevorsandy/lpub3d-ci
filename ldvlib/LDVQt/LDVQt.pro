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
INCLUDEPATH += ../WPngImage
INCLUDEPATH += ../../mainApp
INCLUDEPATH += ../../lclib/common
INCLUDEPATH += ../../qslog
INCLUDEPATH += $${SYSTEM_PREFIX_}/include

DEFINES += _NO_BOOST
DEFINES += QT_THREAD_SUPPORT
!freebsd: \
DEFINES += EXPORT_3DS

# specify flag for ARM platforms to use System OpenGL headers
!macx:contains(BUILD_ARCH,arm64)|contains(BUILD_ARCH,arm): \
UNIX_ARM_BUILD_ARCH = True

win32 {

    QMAKE_EXT_OBJ = .obj
    DEFINES      += _TC_STATIC
    DEFINES      += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1

    win32-msvc* {

        CONFIG  += windows
        CONFIG  += force_debug_info
        DEFINES += _WINSOCKAPI_
        DEFINES += _WIN_UTF8_PATHS
        QMAKE_CXXFLAGS_RELEASE += /FI winsock2.h /FI winsock.h
        QMAKE_LFLAGS_WINDOWS += /IGNORE:4099
        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -WX- -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4  -wd"4005" -wd"4456" -wd"4458" -wd"4459" -wd"4127" -wd"4701"
            QMAKE_CFLAGS_DEBUG   += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
            QMAKE_CXXFLAGS_DEBUG += $$QMAKE_ADDL_MSVC_DEBUG_FLAGS
        }
        CONFIG(release, debug|release) {
            QMAKE_ADDL_MSVC_RELEASE_FLAGS = $$QMAKE_ADDL_MSVC_FLAGS -GF -Gy
            QMAKE_CFLAGS_OPTIMIZE += -Ob1 -Oi -Ot
            QMAKE_CFLAGS_WARN_ON  += -W1 -WX- -wd"4005" -wd"4456" -wd"4458" -wd"4805"
            QMAKE_CFLAGS_RELEASE  += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
            QMAKE_CXXFLAGS_RELEASE += $$QMAKE_ADDL_MSVC_RELEASE_FLAGS
        }
        QMAKE_CXXFLAGS_WARN_ON = $$QMAKE_CFLAGS_WARN_ON
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

# specify ARM platforms that need to suppress local glext.h header
contains(UNIX_ARM_BUILD_ARCH,True): contains(BUILD_TARGET,suse): contains(HOST_VERSION,1320) {
    DEFINES += ARM_SKIP_GL_HEADERS
    message("~~~ lib$${TARGET} $$upper($$QT_ARCH) build - $${BUILD_TARGET}-$${HOST_VERSION}-$${BUILD_ARCH} define SKIP_LDV_GL_HEADERS ~~~")
} else {
    message("~~~ lib$${TARGET} $$upper($$QT_ARCH) build - $${BUILD_TARGET}-$${HOST_VERSION}-$${BUILD_ARCH} ~~~")
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
    LDVImageMatte.cpp \
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
    LDVImageMatte.h \
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

#~~ suppress warnings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

unix|msys {
QMAKE_CFLAGS_WARN_ON += \
    -Wall -W \
    -Wno-deprecated-declarations
QMAKE_CXXFLAGS_WARN_ON  = $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-deprecated-copy
} # unix|msys
if (unix|msys):!macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-comment \
    -Wno-format-security \
    -Wno-format \
    -Wno-implicit-fallthrough \
    -Wno-parentheses \
    -Wno-return-type \
    -Wno-sign-compare \
    -Wno-uninitialized \
    -Wno-unknown-pragmas \
    -Wno-unused-parameter \
    -Wno-unused-result \
    -Wno-unused-variable \
    -Wno-clobbered \
    -Wno-switch \
    -Wno-unused-but-set-variable \
    -Wno-overloaded-virtual
msys {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-attributes
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CFLAGS_WARN_ON += \
    -Wno-misleading-indentation  \
    -Wno-implicit-function-declaration \
    -Wno-incompatible-pointer-types
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-template-id-cdtor \
    -Wno-cast-function-type \
    -Wno-class-memaccess \
    -Wno-type-limits \
    -Wno-cpp
} else: \
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
} # unix|msys:!macx
macx {
QMAKE_CFLAGS_WARN_ON += \
    -Wno-for-loop-analysis \
    -Wno-incompatible-pointer-types-discards-qualifiers \
    -Wno-int-conversion \
    -Wno-invalid-source-encoding \
    -Wno-mismatched-new-delete \
    -Wno-nullability-completeness \
    -Wno-reorder \
    -Wno-undefined-bool-conversion
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
}
