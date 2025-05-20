# 3rd party library directives

#requires(USE_THIRD_PARTY_LIBS)

TEMPLATE = lib
QT      -= core
QT      -= gui
CONFIG  -= qt
CONFIG  += warn_on
CONFIG  += staticlib

# fine-grained host identification
win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

# platform switch
BUILD_ARCH = $$(TARGET_CPU)
isEmpty(BUILD_ARCH): \
!contains(QT_ARCH, unknown): \
BUILD_ARCH = $$QT_ARCH
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH  = 64
} else {
    ARCH  = 32
}

# build type
CONFIG(debug, debug|release) {
    BUILD = DEBUG
    DESTDIR = $$join(ARCH,,,bit_debug)
} else {
    BUILD = RELEASE
    DESTDIR = $$join(ARCH,,,bit_release)
}
BUILD += BUILD ON $$upper($$HOST)

INCLUDEPATH += $$PWD
!equals(TARGET, z) {
    !USE_SYSTEM_ZLIB {
        INCLUDEPATH  += $$_PRO_FILE_PWD_/../zlib
    } else {
        INCLUDEPATH  += $$_PRO_FILE_PWD_/../../include   # for zlib.h and zconf.h
    }
}

# USE GNU_SOURCE
unix|msys:!macx: \
DEFINES += _GNU_SOURCE

# USE CPP 11
contains(USE_CPP11,NO) {
    message("~~~ DO NOT USE CPP11 SPECIFIED ~~~")
} else {
    DEFINES += USE_CPP11
}

contains(QT_VERSION, ^5\\..*) {
    unix|msys:!macx {
        GCC_VERSION = $$system(g++ -dumpversion)
        greaterThan(GCC_VERSION, 4.8) {
            QMAKE_CXXFLAGS += -std=c++11
        } else {
            QMAKE_CXXFLAGS += -std=c++0x
        }
    }
}

contains(QT_VERSION, ^6\\..*) {
    win32-msvc* {
        QMAKE_CXXFLAGS += /std:c++17
    }
    macx {
        QMAKE_CXXFLAGS+= -std=c++17
    }
    unix|msys:!macx {
        GCC_VERSION = $$system(g++ -dumpversion)
        greaterThan(GCC_VERSION, 5) {
            QMAKE_CXXFLAGS += -std=c++17
        } else {
            QMAKE_CXXFLAGS += -std=c++0x
        }
    }
}

unix|msys {
    EXT_S = a
    macx: \
    EXT_D = dylib
    else:msys: \
    EXT_D = dll
    else: \
    EXT_D = so
}

win32-msvc* {
    DEFINES += \
        _CRT_SECURE_NO_WARNINGS \
        _CRT_SECURE_NO_DEPRECATE=1 \
        _CRT_NONSTDC_NO_WARNINGS=1
    QMAKE_CXXFLAGS_RELEASE += \
        /FI winsock2.h /FI winsock.h \
        /wd4675
    QMAKE_LFLAGS += \
    -Wl,--allow-multiple-definition

    EXT_S = lib
    EXT_D = dll
}

OBJECTS_DIR   = $$DESTDIR/.obj
win32: \
QMAKE_EXT_OBJ = .obj

# suppress warnings
unix|msys {
QMAKE_CFLAGS_WARN_ON = \
                     -Wall -W \
                     -Wno-unused-parameter \
                     -Wno-parentheses \
                     -Wno-unused-variable \
                     -Wno-deprecated-declarations \
                     -Wno-return-type \
                     -Wno-sign-compare \
                     -Wno-uninitialized \
                     -Wno-format \
                     -Wno-switch \
                     -Wno-comment \
                     -Wno-unused-result \
                     -Wno-unused-but-set-variable \
                     -Wno-unused-variable \
                     -Wno-implicit-fallthrough \
                     -Wno-misleading-indentation \
                     -Wno-format-security                     

QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}

QMAKE_CFLAGS_WARN_ON += \
                     -Wno-implicit-function-declaration \
                     -Wno-incompatible-pointer-types
macx {
QMAKE_CFLAGS_WARN_ON += \
                     -Wno-incompatible-pointer-types-discards-qualifiers \
                     -Wno-undefined-bool-conversion \
                     -Wno-invalid-source-encoding \
                     -Wno-mismatched-new-delete \
                     -Wno-for-loop-analysis \
                     -Wno-int-conversion \
                     -Wno-reorder
} else {
QMAKE_CFLAGS_WARN_ON += \
                     -Wno-calloc-transposed-args
}
} # unix|msys
