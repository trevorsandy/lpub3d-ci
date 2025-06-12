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
CONFIG  += skip_target_version_ext
macx: \
CONFIG  -= app_bundle

greaterThan(QT_MAJOR_VERSION, 5) {
  QT += core5compat openglwidgets
  DEFINES += QOPENGLWIDGET
}

# The ABI version.
VER_MAJ = 4
VER_MIN = 6
VER_PAT = 0
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

msys: \
SYSTEM_PREFIX_ = $${PREFIX}
else: \
SYSTEM_PREFIX_ = $${PREFIX}/usr/local
macx {
    contains(QT_ARCH,arm64): \
    SYSTEM_PREFIX_ = /opt/homebrew
}
INCLUDEPATH += $${SYSTEM_PREFIX_}/include

DEFINES += _NO_BOOST
DEFINES += QT_THREAD_SUPPORT
!freebsd: \
DEFINES += EXPORT_3DS

# platform name and version
BUILD_TARGET   = $$(TARGET_VENDOR)  # Platform ID
HOST_VERSION   = $$(PLATFORM_VER)   # Platform Version
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

# for aarch64, QT_ARCH = arm64, for arm7l, QT_ARCH = arm
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH  = 64
    LIB_ARCH = 64
} else {
    ARCH  = 32
    LIB_ARCH =
}

# specify flag for ARM platforms to use System OpenGL headers
!macx:contains(BUILD_ARCH,arm64)|contains(BUILD_ARCH,arm): \
UNIX_ARM_BUILD_ARCH = True

CONFIG += incremental

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
    BUILD    = Static
    QMAKE_LFLAGS += -static
    unix|msys: \
    DEFINES += _TC_STATIC
} else {
    BUILD    = Shared
    msys: \
    CONFIG  -= staticlib
}

# LDVQT Qt/OSMesa/WGL library identifiers
ldviewqt {
    CONFIG  += CUI_QT
    DEFINES += _QT
    POSTFIX  = -qt$${QT_MAJOR_VERSION}
} else:ldviewwgl {
    CONFIG  += CUI_WGL
    POSTFIX  = -wgl
} else:!win32-msvc* {
    CONFIG  += OSMesa
    DEFINES += _OSMESA
    POSTFIX  = -osmesa
}

# 3rd party executables, documentation and resources.
!isEmpty(LP3D_3RD_DIST_DIR): \
THIRD_PARTY_DIST_DIR_PATH     = $$LP3D_3RD_DIST_DIR
else: \
THIRD_PARTY_DIST_DIR_PATH     = $$(LP3D_DIST_DIR_PATH)
isEmpty(THIRD_PARTY_DIST_DIR_PATH): \
THIRD_PARTY_DIST_DIR_PATH     = $$absolute_path( $$PWD/../../builds/3rdparty )
!exists($$THIRD_PARTY_DIST_DIR_PATH) {
    unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
    else:msys:  DIST_DIR      = lpub3d_msys_3rdparty
    else:macx:  DIST_DIR      = lpub3d_macos_3rdparty
    else:win32: DIST_DIR      = lpub3d_windows_3rdparty
    THIRD_PARTY_DIST_DIR_PATH = $$absolute_path( $$PWD/../../../$$DIST_DIR )
    !exists($$THIRD_PARTY_DIST_DIR_PATH) {
        message("~~~ ERROR lib$${TARGET} - THIRD_PARTY_DIST_DIR_PATH (LDVLIB) WAS NOT FOUND! ~~~ ")
        THIRD_PARTY_DIST_DIR_PATH="undefined"
    }
}

!BUILD_LDV_LIBS {
    unix|msys: \
    LIB_LDVIEW  = libTCFoundation$${POSTFIX}.a
    else:win32-msvc*: \
    LIB_LDVIEW  = TCFoundation$${POSTFIX}.lib
    LIB_LDVIEW_PATH = $${THIRD_PARTY_DIST_DIR_PATH}/$${VER_LDVIEW}/lib/$${QT_ARCH}/$${LIB_LDVIEW} 
    !exists($${LIB_LDVIEW_PATH}): \
    CONFIG += BUILD_LDV_LIBS
}

CONFIG(debug, debug|release) {
    VER_USE_LDVIEW_DEV = False
    DEFINES += QT_DEBUG_MODE
    BUILD_CONF = Debug
    ARCH_BLD = bit_debug
    macx: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d$${VER_MAJ}$${VER_MIN})
    unix:!macx: TARGET = $$join(TARGET,,,d)
    # The remaining lines in this block adds the LDView header and source files...
    # This line requires a git extract of ldview at the same location as the lpub3d git extract
    # These lines defines the ldview git extract folder name, you can set as you like
    mingw:ide_qtcreator: VER_LDVIEW_DEV = undefined
    else:unix|msys:      VER_LDVIEW_DEV = ldview
    else:win32-msvc*:    VER_LDVIEW_DEV = ldview_vs_build
    BUILD_LDV_LIBS {
        VER_USE_LDVIEW_DEV = False
    } else {
        # This line defines the path of the ldview git extract relative to this project file
        VER_LDVIEW_DEV_REPOSITORY = $$absolute_path( ../../../$${VER_LDVIEW_DEV} )
        message("~~~ lib$${TARGET} BUILD LDVQt USING LDVIEW DEVELOPMENT REPOSITORY ~~~ ")
        exists($$VER_LDVIEW_DEV_REPOSITORY) {
            message("~~~ lib$${TARGET} ADD LDVIEW HEADERS TO INCLUDEPATH: $$VER_LDVIEW_DEV_REPOSITORY ~~~ ")
            INCLUDEPATH += $${VER_LDVIEW_DEV_REPOSITORY}
            VER_USE_LDVIEW_DEV = True
        } else {
            message("~~~ WARNING lib$${TARGET}: - COULD NOT LOAD LDVIEW DEV FROM: $$VER_LDVIEW_DEV_REPOSITORY ~~~ ")
        }
    }
} else {
    BUILD_CONF = Release
    ARCH_BLD = bit_release
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

# USE CPP 11
contains(USE_CPP11,NO) {
  message("NO CPP11")
} else {
  DEFINES += USE_CPP11
}

contains(QT_VERSION, ^5\\..*) {
  if (unix|msys):!macx {
    GCC_VERSION = $$system(g++ -dumpversion)
    greaterThan(GCC_VERSION, 4.8) {
      QMAKE_CXXFLAGS += -std=c++11
    } else {
      QMAKE_CXXFLAGS += -std=c++0x
    }
  } else {
    CONFIG += c++11
  }
}

contains(QT_VERSION, ^6\\..*) {
  win32-msvc* {
    QMAKE_CXXFLAGS += /std:c++17
  }
  macx {
    QMAKE_CXXFLAGS+= -std=c++17
  }
  if (unix|msys):!macx {
    GCC_VERSION = $$system(g++ -dumpversion)
    greaterThan(GCC_VERSION, 5) {
      QMAKE_CXXFLAGS += -std=c++17
    } else {
      QMAKE_CXXFLAGS += -std=c++0x
    }
  }
}

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
