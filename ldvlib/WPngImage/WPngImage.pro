TEMPLATE  = lib
QT       -= gui
CONFIG   += qt warn_on
CONFIG   += staticlib
macx: \
CONFIG   -= app_bundle

DEPENDPATH  += .
INCLUDEPATH += .

# The ABI version.
VER_MAJ = 1
VER_MIN = 4
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

BUILD_ARCH   = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH     = 64
    STG_ARCH = x86_64
} else {
    ARCH     = 32
    STG_ARCH = x86
}

#QMAKE_CXXFLAGS  += $(Q_CXXFLAGS)
#QMAKE_LFLAGS    += $(Q_LDFLAGS)
#QMAKE_CFLAGS    += $(Q_CFLAGS)

# USE CPP 11
unix:!freebsd:!macx {
    GCC_VERSION = $$system(g++ -dumpversion)
    greaterThan(GCC_VERSION, 4.6) {
        QMAKE_CXXFLAGS += -std=c++11
    } else {
        QMAKE_CXXFLAGS += -std=c++0x
    }
} else {
    CONFIG += c++11
}

win32 {

    QMAKE_EXT_OBJ = .obj
    CONFIG += windows

    win32-msvc* {
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1

        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            DEFINES += QT_DEBUG_MODE
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4 -WX-
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

    QMAKE_TARGET_COMPANY = "Juha Nieminen"
    QMAKE_TARGET_DESCRIPTION = "C++ library to manage images in PNG format"
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2017 Juha Nieminen"
    QMAKE_TARGET_PRODUCT = "WPngImage ($$join(ARCH,,,bit))"
}

CONFIG += skip_target_version_ext

unix: !macx: TARGET = wpngimage
else:        TARGET = WPngImage

# Indicate build type
staticlib {
    BUILD    = Static
} else {
    BUILD    = Shared
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

QMAKE_EXT_CPP = .cc

# Input files
include(WPngImage.pri)

# Suppress warnings
!win32-msvc* {
QMAKE_CFLAGS_WARN_ON += -Wall -W \
    -Wno-sign-compare
QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}
}
