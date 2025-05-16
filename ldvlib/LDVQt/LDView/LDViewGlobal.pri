# LDView global directives

# Get fine-grained host identification
win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

# The ABI version.
VER_MAJ = 4
VER_MIN = 6
VER_PAT = 1
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else:  VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch
DEFINES += VERSION_INFO=\\\"$$VER_MAJ"."$$VER_MIN"."$$VER_PAT\\\"

static|staticlib { 
    BUILD  = STATIC 
} else { 
    BUILD  = SHARED 
}

# Qt/OSMesa/WGL library identifiers
contains(DEFINES, _QT) {
    POSTFIX  = -qt$${QT_MAJOR_VERSION}
    BUILD   += QT
} else: contains(DEFINES, _OSMESA) {
    POSTFIX  = -osmesa
    BUILD   += OSMESA
    QT      -= core
} else:CUI_WGL {
    DEFINES += _LP3D_CUI_WGL
    POSTFIX  = -wgl
    BUILD   += WGL
    QT      -= core
}

# Except for MSVC (use pre-built), always build 3rd party lib3ds
win32-msvc* {
    CONFIG += BUILD_MINIZIP BUILD_GL2PS BUILD_TINYXML
} else {
    !USE_3RD_PARTY_PREBUILT_3DS: CONFIG += BUILD_3DS
}

BUILD_3DS {
    USE_3RD_PARTY_3DS = YES
}

# Open Build Service overrides
BUILD_TINYXML {
    USE_3RD_PARTY_TINYXML = YES
}

BUILD_GL2PS {
    USE_3RD_PARTY_GL2PS = YES
}

BUILD_MINIZIP {
    USE_3RD_PARTY_MINIZIP = YES
}

# Ubuntu Trusty uses libpng12 which is too old
if (contains(HOST, Ubuntu):contains(HOST, 14.04.5):USE_SYSTEM_LIBS|BUILD_PNG) {
    USE_3RD_PARTY_PNG = YES
}

# for aarch64, QT_ARCH = arm64, for arm7l, QT_ARCH = arm
BUILD_ARCH = $$(TARGET_CPU)
isEmpty(BUILD_ARCH): \
!contains(QT_ARCH, unknown): \
BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH): BUILD_ARCH = $$(TARGET_CPU)
if (contains(BUILD_ARCH,x86_64)|contains(BUILD_ARCH,arm64)|contains(BUILD_ARCH,aarch64)) {
    ARCH     = 64
    LIB_ARCH = 64
} else {
    ARCH     = 32
    LIB_ARCH =
}

# build type
CONFIG(debug, debug|release) {
    BUILD += DEBUG
    DESTDIR = $$join(ARCH,,,bit_debug)
} else {
    BUILD += RELEASE
    DESTDIR = $$join(ARCH,,,bit_release)
}
BUILD  += BUILD ON $$upper($$HOST)

CONFIG += incremental

# 3ds
!freebsd: DEFINES += EXPORT_3DS
else:     DEFINES -= EXPORT_3DS

if (win32|static|staticlib) {
    DEFINES += _TC_STATIC
    CONFIG  -= shared
}

# Basically, this project include file is set up to allow some options for selecting your LDView libraries.
# The default is to select the pre-defined libraries in ../lib and headers in ../include.
# There is an option to build all the libraries dynamically as you compile the solution.
# This option is enabled by setting the directive CONFIG+=USE_3RD_PARTY_LIBS.
# Additionally, you have the option to use system library path(s) to access libraries
# you may already have on your system e.g. usr/include and usr/lib. To enable this
# option, be sure to set SYSTEM_PREFIX_ below along with the directive CONFIG+=USE_SYSTEM_LIBS

# 3rdParty libraries - compiled from source during build (some, not all)
3RD_PARTY_PREFIX_  = $$_PRO_FILE_PWD_/../3rdParty

# pre-compiled libraries heaers location
LIBINC_            = $$_PRO_FILE_PWD_/../include       # zlib.h and zconf.h, glext and wglext headers

# You can modify library paths below to match your system
# for default settings, place headers in ../include/..
# place required pre-compiled static libs in ../lib/..
# You may also set alternative locations for your libraries and headers
unix|msys {
    # System libraries - on Unix, change to or add /usr/local if you want
    msys: \
    SYSTEM_PREFIX_ = $${PREFIX}
    else: \
    SYSTEM_PREFIX_ = $${PREFIX}/usr

    # Static library extension
    EXT_S          = a

    # base names
    USE_SYSTEM_LIBS {
        LIB_PNG    = png
    } else {
        # MSYS2 always uses system libs
        LIB_PNG    = png16
    }
    msys {
        LIB_GLU    = glu32
        LIB_GL     = opengl32
    } else {
        LIB_GLU    = GLU
        LIB_GL     = GL
    }
    LIB_JPEG       = jpeg
    LIB_GL2PS      = gl2ps
    LIB_Z          = z
    LIB_TINYXML    = tinyxml
    LIB_3DS        = 3ds
    LIB_MINIZIP    = minizip

    macx {
        # pre-compiled libraries location
        LIBDIR_     = $$_PRO_FILE_PWD_/../lib/MacOSX
        # dynamic library extension
        EXT_D       = dylib
        # frameworks
        OSX_FRAMEWORKS_CORE = -framework CoreFoundation -framework CoreServices
    } else {
        # dynamic library extension
        msys: \
        EXT_D       = dll
        else: \
        EXT_D       = so

        !msys {
            # pre-compiled libraries location
            CUI_WGL:equals(TARGET, LDView) {
                equals(ARCH, 64): LIBDIR_ = $$_PRO_FILE_PWD_/lib/Linux/x86_64
                else:             LIBDIR_ = $$_PRO_FILE_PWD_/lib
            } else {
                equals(ARCH, 64): LIBDIR_ = $$_PRO_FILE_PWD_/../lib/Linux/x86_64
                else:             LIBDIR_ = $$_PRO_FILE_PWD_/../lib
            }
        }
    }

    # library names
    LIB_CUI          = libCUI$${POSTFIX}.$${EXT_S}
    LIB_LDLIB        = libLDLib$${POSTFIX}.$${EXT_S}
    LIB_LDEXPORTER   = libLDExporter$${POSTFIX}.$${EXT_S}
    LIB_LDLOADER     = libLDLoader$${POSTFIX}.$${EXT_S}
    LIB_TRE          = libTRE$${POSTFIX}.$${EXT_S}
    LIB_TCFOUNDATION = libTCFoundation$${POSTFIX}.$${EXT_S}

} else:win32-msvc* {
    # Windows MSVC stuff...
    SYSTEM_PREFIX_ = $${PREFIX}

    BUILD_WORKER_VERSION = $$(LP3D_VSVERSION)
    isEmpty(BUILD_WORKER_VERSION): BUILD_WORKER_VERSION = 2019
    lessThan(BUILD_WORKER_VERSION, 2019) {
        VSVER=vs2015
    } else {
        contains(QT_ARCH,i386): VSVER=vs2017
        else: VSVER=vs2019
    }

    # pre-compiled libraries location
    CUI_WGL:equals(TARGET, LDView): \
    LIB_REL_PATH = $$_PRO_FILE_PWD_/lib
    else: \
    LIB_REL_PATH = $$_PRO_FILE_PWD_/../lib
    equals(ARCH, 64): \
    LIBDIR_      = $${LIB_REL_PATH}/x64
    else: \
    LIBDIR_      = $${LIB_REL_PATH}

    # static library extensions
    EXT_S           = lib
    # dynamic library extensions
    EXT_D           = dll

    # base names
    LIB_MINIZIP   = minizip
    LIB_GL2PS     = gl2ps
    LIB_TINYXML   = tinyxml_STL
    LIB_3DS       = lib3ds-$${VSVER}
    LIB_PNG       = libpng16-$${VSVER}
    LIB_JPEG      = libjpeg-$${VSVER}
    LIB_Z         = zlib-$${VSVER}

    # library names
    LIB_LDLIB        = LDLib$${POSTFIX}.$${EXT_S}
    LIB_LDEXPORTER   = LDExporter$${POSTFIX}.$${EXT_S}
    LIB_LDLOADER     = LDLoader$${POSTFIX}.$${EXT_S}
    LIB_TRE          = TRE$${POSTFIX}.$${EXT_S}
    LIB_TCFOUNDATION = TCFoundation$${POSTFIX}.$${EXT_S}
}

# pre-compiled libraries
# Library variables - you can modify the items below if
# you want to set individual libray paths/names etc...
# ===============================
LIBS_INC            = $${LIBINC_}
LIBS_DIR            = -L$${LIBDIR_}
# -------------------------------
PNG_INC             = $${LIBINC_}
PNG_LIBDIR          = -L$${LIBDIR_}

JPEG_INC            = $${LIBINC_}
JPEG_LIBDIR         = -L$${LIBDIR_}

GL2PS_INC           = $${LIBINC_}
GL2PS_LIBDIR        = -L$${LIBDIR_}

TINYXML_INC         = $${LIBINC_}
TINYXML_LIBDIR      = -L$${LIBDIR_}

3DS_INC             = $${LIBINC_}
3DS_LIBDIR          = -L$${LIBDIR_}

MINIZIP_INC         = $${3RD_PARTY_PREFIX_}/minizip
MINIZIP_LIBDIR      = -L$${LIBDIR_}

ZLIB_INC            = $${LIBINC_}
ZLIB_LIBDIR         = -L$${LIBDIR_}

# Update Libraries
# ===============================

LIBS_PRI            = -l$${LIB_PNG} \
                      -l$${LIB_JPEG} \
                      -l$${LIB_GL2PS} \
                      -l$${LIB_MINIZIP} \
                      -l$${LIB_TINYXML} \
                      -l$${LIB_Z}

# conditional libraries
contains(DEFINES, EXPORT_3DS): \
LIBS_PRI           += -l$${LIB_3DS}

# 3rd party libreries - compiled from source
# Be careful not to move this chunk. moving it will affect to overall logic flow.
# ===============================
USE_3RD_PARTY_LIBS {
    # headers and static compiled libs
    GL2PS_INC       = $${3RD_PARTY_PREFIX_}/gl2ps
    GL2PS_LIBDIR    = -L$${3RD_PARTY_PREFIX_}/gl2ps/$$DESTDIR

    MINIZIP_INC     = $${3RD_PARTY_PREFIX_}/minizip
    MINIZIP_LIBDIR  = -L$${3RD_PARTY_PREFIX_}/minizip/$$DESTDIR

    TINYXML_INC     = $${3RD_PARTY_PREFIX_}/tinyxml
    TINYXML_LIBDIR  = -L$${3RD_PARTY_PREFIX_}/tinyxml/$$DESTDIR

    3DS_INC         = $${3RD_PARTY_PREFIX_}/lib3ds
    3DS_LIBDIR      = -L$${3RD_PARTY_PREFIX_}/lib3ds/$$DESTDIR

    JPEG_INC        = $${3RD_PARTY_PREFIX_}/libjpeg
    JPEG_LIBDIR     = -L$${3RD_PARTY_PREFIX_}/libjpeg/$$DESTDIR

    PNG_INC         = $${3RD_PARTY_PREFIX_}/libpng
    PNG_LIBDIR      = -L$${3RD_PARTY_PREFIX_}/libpng/$$DESTDIR

    ZLIB_INC        = $${3RD_PARTY_PREFIX_}/zlib
    ZLIB_LIBDIR     = -L$${3RD_PARTY_PREFIX_}/zlib/$$DESTDIR

    # overwrite pre-compiled path (reset) with 3rd party path
    # ===============================
    LIBS_INC =  $${PNG_INC} \
                $${JPEG_INC} \
                $${TINYXML_INC} \
                $${3DS_INC} \
                $${MINIZIP_INC} \
                $${GL2PS_INC} \
                $${ZLIB_INC}

    LIBS_DIR =  $${PNG_LIBDIR} \
                $${JPEG_LIBDIR} \
                $${TINYXML_LIBDIR} \
                $${3DS_LIBDIR} \
                $${MINIZIP_LIBDIR} \
                $${GL2PS_LIBDIR} \
                $${ZLIB_LIBDIR}
}

win32-msvc* {
    # Be careful not to move this chunk. Moving it will affect to overall logic flow.

    3DS_LDLIBS  = $${LIBDIR_}/lib3ds-$${VSVER}.lib
    PNG_LDLIBS  = $${LIBDIR_}/libpng16-$${VSVER}.lib
    JPEG_LDLIBS = $${LIBDIR_}/libjpeg-$${VSVER}.lib
    ZLIB_LDLIBS = $${LIBDIR_}/zlib-$${VSVER}.lib

    # override pre-compiled libraries with 3rd party demand built library paths as specified
    contains(USE_3RD_PARTY_MINIZIP, YES) {
        # remove lib reference
        LIBS_PRI           -= -l$${LIB_MINIZIP}
        # update base name
        LIB_MINIZIP         = minizip
        # reset individual library entry
        MINIZIP_INC         = $${3RD_PARTY_PREFIX_}
        USE_3RD_PARTY_PREBUILT_MINIZIP {
            MINIZIP_LIBDIR  = -L$${LIBDIR_}
            MINIZIP_LDLIBS  = $${LIBDIR_}/lib$${LIB_3DS}.$${EXT_S}
        } else {
            MINIZIP_LIBDIR  = -L$${3RD_PARTY_PREFIX_}/minizip/$$DESTDIR
            MINIZIP_LDLIBS  = $${3RD_PARTY_PREFIX_}/minizip/$$DESTDIR/$${LIB_MINIZIP}.$${EXT_S}
        }
        # update libs path
        LIBS_INC           += $${MINIZIP_INC}
        DEFINES            += HAVE_MINIZIP
    }
    contains(USE_3RD_PARTY_GL2PS, YES) {
        # remove lib reference
        LIBS_PRI           -= -l$${LIB_GL2PS}
        # update base name
        LIB_GL2PS           = gl2ps
        # reset individual library entry
        GL2PS_INC           = $${3RD_PARTY_PREFIX_}/libgl2ps
        GL2PS_LIBDIR        = -L$${3RD_PARTY_PREFIX_}/gl2ps/$$DESTDIR
        GL2PS_LDLIBS        = $${3RD_PARTY_PREFIX_}/gl2ps/$$DESTDIR/$${LIB_GL2PS}.$${EXT_S}
        # update libs path
        LIBS_INC           += $${GL2PS_INC}
    }
    contains(USE_3RD_PARTY_TINYXML, YES) {
        # remove lib reference
        LIBS_PRI           -= -l$${LIB_TINYXML}
        # update base name
        LIB_TINYXML         = tinyxml_STL
        # reset individual library entry
        TINYXML_INC         = $${3RD_PARTY_PREFIX_}/tinyxml
        TINYXML_LIBDIR      = -L$${3RD_PARTY_PREFIX_}/tinyxml/$$DESTDIR
        TINYXML_LDLIBS      = $${3RD_PARTY_PREFIX_}/tinyxml/$$DESTDIR/$${LIB_TINYXML}.$${EXT_S}
        # update libs path
        LIBS_INC           += $${TINYXML_INC}
    }
}

unix|msys {
    # Be careful not to move this chunk. Moving it will affect to overall logic flow.

    # detect system libraries paths
    SYS_LIBINC_         = $${SYSTEM_PREFIX_}/include
    macx {                                                             # OSX
        contains(QT_ARCH,arm64) {
            SYS_LIBINC_ = /opt/homebrew/include
            SYS_LIBDIR_ = /opt/homebrew/lib
        } else {
            SYS_LIBINC_ = $${SYSTEM_PREFIX_}/local/include
            SYS_LIBDIR_ = $${SYSTEM_PREFIX_}/local/lib
        }
        SYS_LIBINC_X11_ = $${SYSTEM_PREFIX_}/X11/include
        SYS_LIBDIR_X11_ = $${SYSTEM_PREFIX_}/X11/lib
    } else: exists($${SYSTEM_PREFIX_}/lib/$$QT_ARCH-linux-gnu) {       # Debian
        SYS_LIBDIR_     = $${SYSTEM_PREFIX_}/lib/$$QT_ARCH-linux-gnu
    } else: exists($${SYSTEM_PREFIX_}/lib$${LIB_ARCH}) {               # RedHat, Arch - lIB_ARCH is empyt for 32bit
        SYS_LIBDIR_     = $${SYSTEM_PREFIX_}/lib$${LIB_ARCH}
    } else {                                                           # Arch, MSYS2
        SYS_LIBDIR_     = $${SYSTEM_PREFIX_}/lib
    }

    # ===============================
    USE_3RD_PARTY_LIBS {
        # detect system libraries
        _LIB_PNG    = png
        USE_SYSTEM_PNG: !USE_3RD_PARTY_PNG: exists($${SYS_LIBDIR_}/lib$${_LIB_PNG}.$${EXT_D}): USE_SYSTEM_PNG_LIB = YES
        USE_SYSTEM_JPEG: exists($${SYS_LIBDIR_}/lib$${LIB_JPEG}.$${EXT_D}): USE_SYSTEM_JPEG_LIB = YES
        USE_SYSTEM_MINIZIP: exists($${SYS_LIBDIR_}/lib$${LIB_MINIZIP}.$${EXT_D}): USE_SYSTEM_MINIZIP_LIB = YES
        USE_SYSTEM_Z: exists($${SYS_LIBDIR_}/lib$${LIB_Z}.$${EXT_D}): USE_SYSTEM_Z_LIB = YES

        # override 3rd party library paths
        USE_3RD_PARTY_PREBUILT_3DS {
            3DS_LIBDIR  = -L$${LIBDIR_}
            3DS_LDLIBS  = $${LIBDIR_}/lib$${LIB_3DS}.$${EXT_S}
        }

        contains(USE_SYSTEM_PNG_LIB, YES) {
            # remove lib reference
            LIBS_PRI     -= -l$${LIB_PNG}
            # use sytem lib name - only macos will trigger this logic
            LIB_PNG       = png
            # remove 3rdParty lib reference
            LIBS_INC     -= $${PNG_INC}
            LIBS_DIR     -= $${PNG_LIBDIR}
            # reset individual library entry
            PNG_INC       = $${SYS_LIBINC_}
            PNG_LIBDIR    = -L$${SYS_LIBDIR_}
            PNG_LDLIBS    = $${SYS_LIBDIR_}/lib$${LIB_PNG}.$${EXT_D}
        }

        contains(USE_SYSTEM_JPEG_LIB, YES) {
            # remove lib reference
            LIBS_PRI     -= -l$${LIB_JPEG}
            # remove 3rdParty lib reference
            LIBS_INC     -= $${JPEG_INC}
            LIBS_DIR     -= $${JPEG_LIBDIR}
            # reset individual library entry
            JPEG_INC      = $${SYS_LIBINC_}
            JPEG_LIBDIR   = -L$${SYS_LIBDIR_}
            JPEG_LDLIBS   = $${SYS_LIBDIR_}/lib$${LIB_JPEG}.$${EXT_D}
        }

        contains(USE_SYSTEM_MINIZIP_LIB, YES) {
            # remove lib reference
            LIBS_PRI       -= -l$${LIB_MINIZIP}
            # remove 3rdParty lib reference
            LIBS_INC       -= $${MINIZIP_INC}
            LIBS_DIR       -= $${MINIZIP_LIBDIR}
            # reset individual library entry
            MINIZIP_INC     = $${SYS_LIBINC_}
            MINIZIP_LIBDIR  = -L$${SYS_LIBDIR_}
            MINIZIP_LDLIBS  = $${SYS_LIBDIR_}/lib$${LIB_MINIZIP}.$${EXT_D}
            DEFINES        += HAVE_MINIZIP
        } else:exists($${MINIZIP_INC}/unzip.h) {
            USE_3RD_PARTY_PREBUILT_MINIZIP {
                MINIZIP_LIBDIR  = -L$${LIBDIR_}
                MINIZIP_LDLIBS  = $${LIBDIR_}/lib$${LIB_MINIZIP}.$${EXT_S}
            }
            # set HAVE_MINIZIP preprocessor directive
            #message("~~~ 3rdParty Minizip header found ~~~")
            DEFINES += HAVE_MINIZIP
        }

        contains(USE_SYSTEM_Z_LIB, YES) {
            # remove lib reference
            LIBS_PRI     -= -l$${LIB_Z}
            # remove 3rdParty lib reference
            LIBS_INC     -= $${ZLIB_INC}
            LIBS_DIR     -= $${ZLIB_LIBDIR}
            # reset individual library entry
            ZLIB_INC      = $${SYS_LIBINC_}
            ZLIB_LIBDIR   = -L$${SYS_LIBDIR_}
            ZLIB_LDLIBS   = $${SYS_LIBDIR_}/lib$${LIB_Z}.$${EXT_D}
        }
    } # USE_3RD_PARTY_LIBS

    # ===============================
    USE_SYSTEM_LIBS {
        # update base names
        LIB_PNG             = png

        # remove pre-compiled libs path
        LIBS_INC           -= $${LIBINC_}
        LIBS_DIR           -= -L$${LIBDIR_}

        # append system library paths
        # ===============================
        LIBS_INC           += $${SYS_LIBINC_}
        LIBS_DIR           += -L$${SYS_LIBDIR_}

        macx {
            LIBS_INC       += $${SYS_LIBINC_X11_}
            LIBS_DIR       += -L$${SYS_LIBDIR_X11_}
        }
        # -------------------------------

        GL2PS_INC           = $${SYS_LIBINC_}
        GL2PS_LIBDIR        = -L$${SYS_LIBDIR_}

        TINYXML_INC         = $${SYS_LIBINC_}
        TINYXML_LIBDIR      = -L$${SYS_LIBDIR_}

        MINIZIP_INC         = $${SYS_LIBINC_}
        MINIZIP_LIBDIR      = -L$${SYS_LIBDIR_}

        3DS_INC             = $${SYS_LIBINC_}
        3DS_LIBDIR          = -L$${SYS_LIBDIR_}

        PNG_INC             = $${SYS_LIBINC_}
        PNG_LIBDIR          = -L$${SYS_LIBDIR_}

        JPEG_INC            = $${SYS_LIBINC_}
        JPEG_LIBDIR         = -L$${SYS_LIBDIR_}

        ZLIB_INC            = $${SYS_LIBINC_}
        ZLIB_LIBDIR         = -L$${SYS_LIBDIR_}

        # override system libraries with 3rd party pre/demand built library paths as specified
        contains(USE_3RD_PARTY_TINYXML, YES) {
            # remove lib reference
            LIBS_PRI           -= -l$${LIB_TINYXML}
            # update base name
            LIB_TINYXML         = tinyxml
            # reset individual library entry
            TINYXML_INC         = $${3RD_PARTY_PREFIX_}/tinyxml
            TINYXML_LIBDIR      = -L$${3RD_PARTY_PREFIX_}/tinyxml/$$DESTDIR
            TINYXML_LDLIBS      = $${3RD_PARTY_PREFIX_}/tinyxml/$$DESTDIR/lib$${LIB_TINYXML}.$${EXT_S}
            # update libs path
            LIBS_INC           += $${TINYXML_INC}
        }
        contains(USE_3RD_PARTY_GL2PS, YES) {
            # remove lib reference
            LIBS_PRI         -= -l$${LIB_GL2PS}
            # update base name
            LIB_GL2PS         = gl2ps
            # reset individual library entry
            GL2PS_INC         = $${3RD_PARTY_PREFIX_}/libgl2ps
            GL2PS_LIBDIR      = -L$${3RD_PARTY_PREFIX_}/gl2ps/$$DESTDIR
            GL2PS_LDLIBS      = $${3RD_PARTY_PREFIX_}/gl2ps/$$DESTDIR/lib$${LIB_GL2PS}.$${EXT_S}
            # update libs path
            LIBS_INC         += $${GL2PS_INC}
        }
        contains(USE_3RD_PARTY_MINIZIP, YES) {
            # remove lib reference
            LIBS_PRI         -= -l$${LIB_MINIZIP}
            # update base name
            LIB_MINIZIP       = minizip
            # reset individual library entry
            MINIZIP_INC       = $${3RD_PARTY_PREFIX_}/minizip
            USE_3RD_PARTY_PREBUILT_MINIZIP {
                MINIZIP_LIBDIR  = -L$${LIBDIR_}
                MINIZIP_LDLIBS  = $${LIBDIR_}/lib$${LIB_3DS}.$${EXT_S}
            } else {
                MINIZIP_LIBDIR  = -L$${3RD_PARTY_PREFIX_}/minizip/$$DESTDIR
                MINIZIP_LDLIBS  = $${3RD_PARTY_PREFIX_}/minizip/$$DESTDIR/lib$${LIB_MINIZIP}.$${EXT_S}
            }
            # update libs path
            LIBS_INC         += $${MINIZIP_INC}
            DEFINES          += HAVE_MINIZIP

        } else:macx:contains(QT_ARCH,arm64):exists(/opt/homebrew/include/minizip/unzip.h) {
            # message("~~~ macOS System $$QT_ARCH Minizip header found ~~~")
            DEFINES += HAVE_MINIZIP
        } else:exists(/usr/include/minizip/unzip.h)|exists(/usr/local/include/minizip/unzip.h) {
            # set HAVE_MINIZIP preprocessor directive
            # message("~~~ System $$QT_ARCH Minizip header found ~~~")
            DEFINES += HAVE_MINIZIP
        }
        contains(USE_3RD_PARTY_PNG, YES) {
            # remove lib reference
            LIBS_PRI       -= -l$${LIB_PNG}
            # update base name
            LIB_PNG         = png16
            # reset individual library entry
            PNG_INC         = $${3RD_PARTY_PREFIX_}/libpng
            PNG_LIBDIR      = -L$${3RD_PARTY_PREFIX_}/libpng/$$DESTDIR
            PNG_LDLIBS      = $${3RD_PARTY_PREFIX_}/libpng/$$DESTDIR/lib$${LIB_PNG}.$${EXT_S}
            # update libs path
            LIBS_INC       += $${PNG_INC}
        }
        contains(USE_3RD_PARTY_JPEG, YES) {
            # remove lib reference
            LIBS_PRI       -= -l$${LIB_JPEG}
            # reset individual library entry
            JPEG_INC        = $${3RD_PARTY_PREFIX_}/libJPEG
            JPEG_LIBDIR     = -L$${3RD_PARTY_PREFIX_}/libJPEG/$$DESTDIR
            JPEG_LDLIBS     = $${3RD_PARTY_PREFIX_}/libJPEG/$$DESTDIR/lib$${LIB_JPEG}.$${EXT_S}
            # update libs path
            LIBS_INC       += $${JPEG_INC}
        }
        contains(USE_3RD_PARTY_3DS, YES) {
            # remove lib reference
            LIBS_PRI       -= -l$${LIB_3DS}
            # reset individual library entry
            3DS_INC         = $${3RD_PARTY_PREFIX_}/lib3ds
            3DS_LIBDIR      = -L$${3RD_PARTY_PREFIX_}/lib3ds/$$DESTDIR
            3DS_LDLIBS      = $${3RD_PARTY_PREFIX_}/lib3ds/$$DESTDIR/lib$${LIB_3DS}.$${EXT_S}
            # update libs path
            LIBS_INC       += $${3DS_INC}
        } else {
            # remove lib reference
            LIBS_PRI       -= -l$${LIB_3DS}
            # reset individual library entry
            3DS_INC         = $${LIBINC_}
            3DS_LIBDIR      = -L$${LIBDIR_}
            3DS_LDLIBS      = $${LIBDIR_}/lib$${LIB_3DS}.$${EXT_S}
            # update libs path
            !contains(LIBS_INC, $${3DS_INC}): LIBS_INC += $${3DS_INC}
        }
    } # USE_SYSTEM_LIBS
}

!win32-msvc*:isEmpty(USE_3RD_PARTY_3DS):message("~~~ USING STATIC PRE-BUILT 3DS LIB ~~~")

# Includes
INCLUDEPATH += . .. $${LIBS_INC}
#message("~~~ DEBUG_INCLUDE_PATHS: $$INCLUDEPATH ~~~")

# USE GNU_SOURCE
unix|msys:!macx: DEFINES += _GNU_SOURCE

# USE CPP 11
contains(USE_CPP11,NO): \
message("~~~ DO NOT USE CPP11 SPECIFIED ~~~")
else: \
DEFINES += USE_CPP11

contains(QT_VERSION, ^5\\..*) {
    win32-msvc* {
        QMAKE_CXXFLAGS += /std:c++17
    }
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

# Windows doesn't have /dev/null but has NUL
win32: NULL_DEVICE = NUL
else:  NULL_DEVICE = /dev/null

# Boost
!contains(CONFIG, USE_BOOST): {
    DEFINES     += _NO_BOOST
} else:!msys {
    INCLUDEPATH += $$_PRO_FILE_PWD_/../boost/include
    LIBS        += -L$$_PRO_FILE_PWD_/../boost/lib
}

# dirs
OBJECTS_DIR      = $$DESTDIR/.obj$${POSTFIX}

# Platform-specific
win32 {
    QMAKE_EXT_OBJ = .obj

    win32-msvc* {
        DEFINES  += \
            _CRT_SECURE_NO_WARNINGS \
            _CRT_NONSTDC_NO_WARNINGS=1
        QMAKE_CXXFLAGS += \
            /FI winsock2.h /FI winsock.h \
            /wd4675
    }
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
                     -Wno-unused-result \
                     -Wno-implicit-fallthrough \
                     -Wno-stringop-overflow
CUI_WGL: \
QMAKE_CFLAGS_WARN_ON += \
                     -Wno-missing-field-initializers \
                     -Wno-unused-but-set-variable \
                     -Wno-switch
msys {
QMAKE_CFLAGS_WARN_ON += \
                     -Wno-attributes \
                     -Wno-unknown-pragmas \
                     -Wno-type-limits \
                     -Wno-cast-function-type \
                     -Wno-implicit-fallthrough \
                     -Wno-stringop-truncation \
                     -Wno-calloc-transposed-args \
                     -Wno-template-id-cdtor
}
macx {
QMAKE_CFLAGS_WARN_ON += \
                     -Wno-implicit-function-declaration \
                     -Wno-incompatible-pointer-types-discards-qualifiers \
                     -Wno-incompatible-pointer-types \
                     -Wno-undefined-bool-conversion \
                     -Wno-invalid-source-encoding \
                     -Wno-mismatched-new-delete \
                     -Wno-for-loop-analysis \
                     -Wno-int-conversion \
                     -Wno-reorder
} else {
QMAKE_CFLAGS_WARN_ON += \
                     -Wno-clobbered
}
QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}
} # unix|msys
