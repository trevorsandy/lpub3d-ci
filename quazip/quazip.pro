TEMPLATE = lib
TARGET   = QuaZIP
CONFIG   += qt warn_on
QT       -= gui
CONFIG   += staticlib
CONFIG   += skip_target_version_ext

# 1.0.0 is the first stable ABI.
# The next binary incompatible change will be 2.0.0 and so on.
# The existing QuaZIP policy on changing ABI requires to bump the
# major version of QuaZIP itself as well. Note that there may be
# other reasons for chaging the major version of QuaZIP, so
# in case where there is a QuaZIP major version bump but no ABI change,
# the VERSION variable will stay the same.

# For example:

# QuaZIP 1.0 is released after some 0.x, keeping binary compatibility.
# VERSION stays 1.0.0.
# Then some binary incompatible change is introduced. QuaZIP goes up to
# 2.0, VERSION to 2.0.0.
# And so on.

# The ABI version.
VER_MAJ = 1
VER_MIN = 1
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

    QMAKE_TARGET_COMPANY = "Sergey A. Tachenov"
    QMAKE_TARGET_DESCRIPTION = "C++ wrapper over Gilles Vollant's ZIP/UNZIP"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2005-2014 Sergey A. Tachenov"
    QMAKE_TARGET_PRODUCT = "$${TARGET} ($$join(ARCH,,,bit))"

    QMAKE_EXT_OBJ = .obj

    win32-msvc* {

        CONFIG += windows
        CONFIG += force_debug_info
        INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
        QMAKE_CFLAGS_WARN_ON -= -W3
        QMAKE_ADDL_MSVC_FLAGS = -WX- -GS -Gd -fp:precise -Zc:forScope
        CONFIG(debug, debug|release) {
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
    } else {
        LIBS += -lz
    }

} else {
    LIBS += -lz
}

if (unix|msys):!macx: TARGET = $$lower($$TARGET)

contains(QT_VERSION, ^5\\..*) {
  if (unix|msys):!macx {
    GCC_VERSION = $$system(g++ -dumpversion)
    greaterThan(GCC_VERSION, 4.8) {
      QMAKE_CXXFLAGS += -std=c++11
    } else {
      QMAKE_CXXFLAGS += -std=c++0x
    }
  }  else {
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

# You'll need to define this one manually if using a build system other
# than qmake or using QuaZIP sources directly in your project.

# Indicate build type
staticlib {
    BUILD    = Static
    DEFINES += QUAZIP_STATIC
} else {
    # This one handles dllimport/dllexport directives.
    BUILD    = Shared
    DEFINES += QUAZIP_BUILD
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

# Input files
include(quazip.pri)

# Suppress warnings
unix|msys {
QMAKE_CFLAGS_WARN_ON += \
    -Wall -W \
    -Wno-sign-compare \
    -Wno-deprecated-declarations
QMAKE_CXXFLAGS_WARN_ON  = $${QMAKE_CFLAGS_WARN_ON}
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-deprecated-copy
} # unix|msys
