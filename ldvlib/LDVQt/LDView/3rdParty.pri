# 3rd party library directives

#requires(USE_THIRD_PARTY_LIBS)

TEMPLATE = lib
QT      -= core
QT      -= gui
CONFIG  -= qt
CONFIG  += warn_on
CONFIG  += staticlib

# common directives
COMMON = LDViewGlobal
include($${PWD}/../../../common.pri)

static|staticlib {
    BUILD   = $$upper($${BUILD}) STATIC
} else {
    BUILD   = $$upper($${BUILD}) SHARED
}

# build type
CONFIG(debug, debug|release) {
    BUILD  += DEBUG
    DESTDIR = $$join(ARCH,,,bit_debug)
} else {
    BUILD  += RELEASE
    DESTDIR = $$join(ARCH,,,bit_release)
}
BUILD += BUILD ON $$upper($$HOST)

# Pre-compiled library headers
# ===============================
LIBINC_                  = $${_PRO_FILE_PWD_}/../../include       # zlib.h and zconf.h
LIBS_INC                 = $${LIBINC_}
# 3rd party library headers
# ===============================
BUILD_ZLIB {
    ZLIB_INC             = $${_PRO_FILE_PWD_}/../zlib
} else: win32-arm64-msvc|win32-msvc* {
    ZLIB_INC             = $${LIBINC_}
} else:unix|msys {
    # System prefix
    msys {
        SYSTEM_PREFIX_   = $${PREFIX}
    } else {
        # System libraries
        SYSTEM_PREFIX_   = $${PREFIX}/usr
    }
    # append system library paths
    SYS_LIBINC_          = $${SYSTEM_PREFIX_}/include
    macx {
        contains(QT_ARCH,arm64) {
            SYS_LIBINC_  = /opt/homebrew/include
        } else {
            SYS_LIBINC_  = $${SYSTEM_PREFIX_}/local/include
        }
    }
    LIBS_INC            += $${SYS_LIBINC_}
    USE_LDV_SYSTEM_LIBS {
        # reset to system library paths
        LIBS_INC         = $${SYS_LIBINC_}
    }
    ZLIB_INC             = $${SYS_LIBINC_}
} else {
    ZLIB_INC             = $${LIBINC_}
}

INCLUDEPATH             += .

unix: \
!equals(TARGET, z): \
INCLUDEPATH             += $${ZLIB_INC}

win32-arm64-msvc|win32-msvc*: \
!equals(TARGET, zlib): \
INCLUDEPATH             += $${ZLIB_INC}

win32-arm64-msvc {
    if (equals(TARGET, 3ds)|equals(TARGET, jpeg)|equals(TARGET, png16)): \
        TARGET=lib$${TARGET}-$${VSVER}

    if (equals(TARGET, zlib)): \
        TARGET=$${TARGET}-$${VSVER}
}

# USE GNU_SOURCE
unix|msys:!macx: \
DEFINES                 += _GNU_SOURCE

win32-arm64-msvc|win32-msvc* {
    QMAKE_CXXFLAGS_RELEASE += \
    /FI winsock2.h /FI winsock.h \
    /wd4675
    QMAKE_LFLAGS += \
    -Wl,--allow-multiple-definition
}

OBJECTS_DIR       = $${DESTDIR}/.obj
win32: \
QMAKE_EXT_OBJ     = .obj
