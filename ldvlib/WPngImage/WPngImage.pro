TEMPLATE  = lib
TARGET    = WPngImage
QT       -= gui
CONFIG   += qt warn_on
CONFIG   += staticlib
CONFIG   += skip_target_version_ext
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

win32 {

    QMAKE_TARGET_COMPANY = "Juha Nieminen"
    QMAKE_TARGET_DESCRIPTION = "C++ library to manage images in PNG format"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2017 Juha Nieminen"
    QMAKE_TARGET_PRODUCT = "$${TARGET} ($$join(ARCH,,,bit))"

    QMAKE_EXT_OBJ = .obj

    win32-msvc* {

        CONFIG  += windows
        CONFIG  += force_debug_info
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -WX- -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
            DEFINES += QT_DEBUG_MODE
            QMAKE_ADDL_MSVC_DEBUG_FLAGS = -RTC1 $$QMAKE_ADDL_MSVC_FLAGS
            QMAKE_CFLAGS_WARN_ON += -W4 
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

QMAKE_EXT_CPP = .cc

# Input files
include(WPngImage.pri)

# Suppress warnings
unix|msys {
QMAKE_CFLAGS_WARN_ON += \
    -Wall -W \
    -Wno-sign-compare \
    -Wno-overloaded-virtual \
    -Wno-deprecated-declarations
QMAKE_CXXFLAGS_WARN_ON  = $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-deprecated-copy
} # unix|msys
