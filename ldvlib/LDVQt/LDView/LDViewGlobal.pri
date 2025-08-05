# LDView library common directives

# common directives
COMMON = LDViewGlobal
include($${PWD}/../../../common.pri)

# The ABI version.
VER_MAJ = 4
VER_MIN = 6
VER_PAT = 1
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else:  VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch
DEFINES += VERSION_INFO=\\\"$$VER_MAJ"."$$VER_MIN"."$$VER_PAT\\\"

static|staticlib {
    BUILD  = $$upper($${BUILD}) STATIC
} else {
    BUILD  = $$upper($${BUILD}) SHARED
}

# build type
CONFIG(debug, debug|release) {
    BUILD += DEBUG
    DESTDIR = $$join(ARCH,,,bit_debug)
} else {
    BUILD += RELEASE
    DESTDIR = $$join(ARCH,,,bit_release)
}
BUILD  += BUILD ON $$upper($${HOST})

CONFIG += incremental

# 3ds
!freebsd: DEFINES += EXPORT_3DS
else:     DEFINES -= EXPORT_3DS

if (win32|static|staticlib) {
    DEFINES += _TC_STATIC
    CONFIG  -= shared
}

# Basically, this project include file is set up to allow some options for selecting your LDView headers.
# The default is to select the headers in ../include. For MSVC, individual libraries also use these headers.
# To use 3rd party headers when building these libraries, set the directive CONFIG+=USE_LDV_3RD_PARTY_LIBS.
# Additionally, you have the option to use headers you may already have on your system e.g. usr/include.
# To enable this option, set SYSTEM_PREFIX_ below along with the directive CONFIG+=USE_LDV_SYSTEM_LIBS.

# 3rdParty libraries - compiled from source during build
3RD_PARTY_PREFIX_  = $${_PRO_FILE_PWD_}/../3rdParty

# pre-compiled libraries heaers location
LIBINC_            = $${_PRO_FILE_PWD_}/../include       # zlib.h and zconf.h, glext and wglext headers

# You can modify paths below to match your system
# for default settings, place headers in ../include/..
# You may also set alternative locations for your headers
win32 {
    SYSTEM_PREFIX_ = $${PREFIX}
} else {
    # System libraries - on Unix, change to or add /usr/local if you want
    SYSTEM_PREFIX_ = $${PREFIX}/usr
}

# Pre-compiled library headers
# ===============================
LIBS_INC        = $${LIBINC_}
# -------------------------------
GL2PS_INC       = $${LIBINC_}
MINIZIP_INC     = $${LIBINC_}
3DS_INC         = $${LIBINC_}
JPEG_INC        = $${LIBINC_}
PNG_INC         = $${LIBINC_}
ZLIB_INC        = $${LIBINC_}
win32-arm64-msvc|win32-msvc* {
    # Always build libgl2ps for MSVC
    GL2PS_INC   = $${3RD_PARTY_PREFIX_}/gl2ps
    USE_LDV_SYSTEM_LIBS {
    ZLIB_INC    = $$[QT_INSTALL_HEADERS]/QtZlib
    LIBS_INC   += $${ZLIB_INC}
    }
} else: {
    # Always build lib3ds except for MSVC
    3DS_INC     = $${3RD_PARTY_PREFIX_}/lib3ds
    LIBS_INC   += $${3DS_INC}
}
# Always build tinyxml
TINYXML_INC     = $${3RD_PARTY_PREFIX_}/tinyxml
LIBS_INC       += $${TINYXML_INC}

# 3rd party library headers
# ===============================
USE_LDV_3RD_PARTY_LIBS {
    # headers
    GL2PS_INC   = $${3RD_PARTY_PREFIX_}/gl2ps
    MINIZIP_INC = $${3RD_PARTY_PREFIX_}/minizip
    3DS_INC     = $${3RD_PARTY_PREFIX_}/lib3ds
    JPEG_INC    = $${3RD_PARTY_PREFIX_}/libjpeg
    PNG_INC     = $${3RD_PARTY_PREFIX_}/libpng
    ZLIB_INC    = $${3RD_PARTY_PREFIX_}/zlib
    # overwrite pre-compiled library headers (reset) with 3rd party headers path
    # ===============================
    LIBS_INC    = $${PNG_INC} \
                  $${JPEG_INC} \
                  $${TINYXML_INC} \
                  $${3DS_INC} \
                  $${MINIZIP_INC} \
                  $${GL2PS_INC} \
                  $${ZLIB_INC}
}

unix|msys {
    # append system library paths
    # ===============================
    SYS_LIBINC_          = $${SYSTEM_PREFIX_}/include
    macx {
        contains(QT_ARCH,arm64) {
            SYS_LIBINC_  = /opt/homebrew/include
        } else {
            SYS_LIBINC_  = $${SYSTEM_PREFIX_}/local/include
        }
        SYS_LIBINC_X11_  = $${SYSTEM_PREFIX_}/X11/include
        SYS_LIBINC_     += $${SYS_LIBINC_X11_}
    }
    LIBS_INC            += $${SYS_LIBINC_}
    USE_LDV_SYSTEM_LIBS {
        # reset to system library paths
        # ===============================
        LIBS_INC         = $${SYS_LIBINC_}
        # ---------------------------
        GL2PS_INC        = $${SYS_LIBINC_}
        MINIZIP_INC      = $${SYS_LIBINC_}
        3DS_INC          = $${SYS_LIBINC_}
        JPEG_INC         = $${SYS_LIBINC_}
        PNG_INC          = $${SYS_LIBINC_}
        ZLIB_INC         = $${SYS_LIBINC_}
        LIBS_INC        += $${TINYXML_INC}
    }
}

# only launch headers found display once - during TCFoundation build
equals(TARGET, TCFoundation) {
    # Always build tinyxml, libgl2ps for MSVC and lib3ds except for MSVC
    USE_LDV_3RD_PARTY_LIBS {
        message("~~~ LDVQt header option - Use 3rdParty library headers ~~~")
    } else {
        USE_LDV_SYSTEM_LIBS {
            message("~~~ LDVQt header option - Use system library headers ~~~")
        } else {
            message("~~~ LDVQt header option - Use pre-compiled library headers ~~~")
        }
    }
    if (unix|msys:exists(/usr/include/gl2ps.h)|exists($${SYSTEM_PREFIX_}/include/gl2ps.h)):!BUILD_GL2PS {
        message("~~~ LDVQt system header for library gl2ps FOUND ~~~")
    } else:exists($${3RD_PARTY_PREFIX_}/gl2ps/gl2ps.h)|exists($${LIBINC_}/gl2ps.h) {
        message("~~~ LDVQt local header for library gl2ps FOUND ~~~")
    } else {
        message("~~~ ERROR LDVQt: header for library gl2ps NOT FOUND, using local ~~~")
    }
    if (unix|msys:exists(/usr/include/minizip/unzip.h)|exists($${SYSTEM_PREFIX_}/include/minizip/unzip.h)):!BUILD_MINIZIP {
        message("~~~ LDVQt system header for library minizip FOUND ~~~")
    } else:exists($${LDV3RDHDR}/minizip/unzip.h)|exists($${LIBINC_}/minizip/unzip.h) {
        message("~~~ LDVQt local header for library minizip FOUND ~~~")
    } else {
        message("~~~ ERROR LDVQt: header for library minizip NOT FOUND ~~~")
        MINIZIP_FOUND=0
    }
    if (unix|msys:exists(/usr/include/tinyxml.h)|exists($${SYSTEM_PREFIX_}/include/tinyxml.h)):!BUILD_TINYXML {
        message("~~~ LDVQt system header for library tinyxml FOUND ~~~")
    } else:exists($${3RD_PARTY_PREFIX_}/tinyxml/tinyxml.h)|exists($${LIBINC_}/tinyxml.h) {
        message("~~~ LDVQt local header for library tinyxml FOUND ~~~")
    } else {
        message("~~~ ERROR LDVQt: header for library tinyxml NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/lib3ds.h)|exists($${SYSTEM_PREFIX_}/include/lib3ds.h)):!BUILD_3DS {
        message("~~~ LDVQt system header for library 3ds FOUND ~~~")
    } else:exists($${3RD_PARTY_PREFIX_}/lib3ds/lib3ds.h)|exists($${LIBINC_}/lib3ds.h) {
        message("~~~ LDVQt local header for library 3ds FOUND ~~~")
    } else {
        message("~~~ ERROR LDVQt: header for library 3ds NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/jpeglib.h)|exists($${SYSTEM_PREFIX_}/include/jpeglib.h)):!BUILD_JPEG {
        message("~~~ LDVQt system header for library jpeg FOUND ~~~")
    } else:exists($${3RD_PARTY_PREFIX_}/libjpeg/jpeglib.h)|exists($${LIBINC_}/jpeglib.h) {
        message("~~~ LDVQt local header for library jpeg FOUND ~~~")
    } else {
        message("~~~ ERROR LDVQt: header for library jpeg NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/png.h)|exists($${SYSTEM_PREFIX_}/include/png.h)):!BUILD_PNG {
        message("~~~ LDVQt system header for library png FOUND ~~~")
    } else:exists($${3RD_PARTY_PREFIX_}/libpng/png.h)|exists($${LIBINC_}/png.h) {
        message("~~~ LDVQt local header for library png FOUND ~~~")
    } else {
        message("~~~ ERROR LDVQt: header for library png NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/zlib.h)|exists($${SYSTEM_PREFIX_}/include/zlib.h)):!BUILD_ZLIB {
        message("~~~ LDVQt system header for library z FOUND ~~~")
    } else:exists($${3RD_PARTY_PREFIX_}/zlib/zlib.h)|exists($${LIBINC_}/zlib.h) {
        message("~~~ LDVQt local header for library z FOUND ~~~")
    } else:win32-arm64-msvc|win32-msvc*:exists($$[QT_INSTALL_HEADERS]/QtZlib) {
        message("~~~ LDVQt Qt header for libz FOUND ~~~")
    } else {
        message("~~~ ERROR LDVQt: header for library z NOT FOUND ~~~")
    }
    !contains(MINIZIP_FOUND,0) {
        DEFINES += HAVE_MINIZIP
    }
}

# objects directory
OBJECTS_DIR      = $${DESTDIR}/.obj$${POSTFIX}

# USE GNU_SOURCE
unix|msys:!macx: DEFINES += _GNU_SOURCE

# Boost
!contains(CONFIG, USE_BOOST): {
    DEFINES     += _NO_BOOST
} else:!msys {
    INCLUDEPATH += $${_PRO_FILE_PWD_}/../boost/include
    LIBS        += -L$${_PRO_FILE_PWD_}/../boost/lib
}

# Platform-specific
win32 {
    QMAKE_EXT_OBJ = .obj
    win32-arm64-msvc|win32-msvc* {
        QMAKE_CXXFLAGS += \
            /FI winsock2.h /FI winsock.h \
            /wd4675
        greaterThan(QMAKE_MSC_VER, 1933) {
            # Visual Studio 2022 (17.14) / Visual C++ 19.29 and up
            MSVC_VER          = 17.14
            MSVC_TOOLSET_VER  = 144
            DEFINES          += QMAKE_MSC_VER=$$QMAKE_MSC_VER
        }
    }
    # Windows doesn't have /dev/null but has NUL
    NULL_DEVICE = NUL
} else {
    NULL_DEVICE = /dev/null
}

unix {
    freebsd: \
    LIBS_INC    +=  /usr/local/include

    # slurm is media.peeron.com
    OSTYPE = $$system(hostname)

    OSTYPE = $$system(hostname | cut -d. -f2-)
    contains(OSTYPE, pair.com) {
        LIBS_INC    +=  /usr/local/include
        DEFINES += _GL_POPCOLOR_BROKEN
    }
}

# Includes
INCLUDEPATH += . .. $${LIBS_INC}
#message("~~~ DEBUG_INCLUDE_PATHS: $$INCLUDEPATH ~~~")
