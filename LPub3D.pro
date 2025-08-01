# LPub3D QMake Configuration settings
# CONFIG+=ldviewqt                # build LDVQt LDView libs using Qt OpenGL
# CONFIG+=ldviewwgl               # build LDVQt LDView libs using WGL OpenGL - default for Windows
# CONFIG+=ldviewosmesa            # build LDVQt LDView libs using OSMesa OpenGL - default for Linux and macOS

# LDV Libraries QMake Configuration settings
# CONFIG+=BUILD_LDV_LIBS          # build LDView libs from source
# CONFIG+=USE_LDV_SYSTEM_LIBS     # override using pre-compiled 3rdParty libraries
# CONFIG+=USE_LDV_3RD_PARTY_LIBS  # override using pre-compiled and system 3rdParty libraries
# CONFIG+=USE_3RD_PARTY_DIST_HDRS # use third party distribution LDView library headers
# CONFIG+=USE_SYSTEM_PNG          # override USE_LDV_3RD_PARTY_LIBS for libpng
# CONFIG+=USE_SYSTEM_GL2PS        # override USE_LDV_3RD_PARTY_LIBS for libgl2ps
# CONFIG+=USE_SYSTEM_JPEG         # override USE_LDV_3RD_PARTY_LIBS for libjpeg
# CONFIG+=USE_SYSTEM_MINIZIP      # override USE_LDV_3RD_PARTY_LIBS for libminizip
# CONFIG+=USE_SYSTEM_ZLIB         # override USE_LDV_3RD_PARTY_LIBS for libz
# CONFIG+=BUILD_3DS               # override USE_LDV_SYSTEM_LIBS for lib3ds - always built except for MSVC
# CONFIG+=BUILD_JPEG              # override USE_LDV_SYSTEM_LIBS for libjpeg
# CONFIG+=BUILD_PNG               # override USE_LDV_SYSTEM_LIBS for libpng
# CONFIG+=BUILD_GL2PS             # override USE_LDV_SYSTEM_LIBS for libgl2ps - always built for MSVC
# CONFIG+=BUILD_TINYXML           # override USE_LDV_SYSTEM_LIBS for libtinyxml - always built
# CONFIG+=BUILD_MINIZIP           # override USE_LDV_SYSTEM_LIBS for libminizip
# CONFIG+=BUILD_ZLIB              # override USE_LDV_SYSTEM_LIBS for libz

# LPub3D QMake directory and project file structre
# ------------------------------------------
# /LPub3D
#  |--- LPub3D.pro                 Subdirs project file - consumes common.pri
#  |--- gitversion.pri             Construct project version
#  |--- common.pri                 Common project declarations and directives
#  |
#  `--- /lclib
#  |     |--- lclib.pro            Library project file - consumes common.pri, lclib.pri
#  |     |--- lclib.pri            Library declarations and directives project include
#  |
#  `--- /ldrawini
#  |     |--- ldrawini.pri         Library declarations and directives project include
#  |     |--- Lldrawini.pro        Library project file - consumes common.pri, ldrawini.pri
#  |
#  `--- /ldvlib
#  |     |
#  |     `--- /LDVQt
#  |           |--- LDVQtLibs.pri  Library declarations and directives project include
#  |           |--- LDVQt.pro      Library project file - consumes common.pri, LDVQtLibs.pri
#  |           |
#  |           `--- /LDView
#  |                |--- LDViewGlobal.pri                   Global declarations and directives project include - consumes common.pri
#  |                |--- 3rdParty.pri                       3rdParty library declarations and directives project include - consumes common.pri
#  |                |
#  |                `--- /Utilities
#  |                |     |--- Headerize.pro                Executable headerizer project file - declarations and directives - consumes LDViewGlobal.pri
#  |                |
#  |                `--- /LDLib
#  |                |     |--- LDLib.pri                    Library declarations and directives project include - consumes LDViewGlobal.pri
#  |                |     |--- LDLib_QT.pro                 Library Qt project file - consumes LDLib.pri
#  |                |     |--- LDLib_WGL.pro                Library WGL project file - consumes LDLib.pri
#  |                |     |--- LDLib_OSMesa.pro             Library OSMesa project file - consumes LDLib.pri
#  |                |
#  |                `--- /TRE
#  |                |     |--- TRE.pri                      Library declarations and directives project include - consumes LDViewGlobal.pri
#  |                |     |--- TRE_QT.pro                   Library Qt project file - consumes TRE.pri
#  |                |     |--- TRE_WGL.pro                  Library WGL project file - consumes TRE.pri
#  |                |     |--- TRE_OSMesa.pro               Library OSMesa project file - consumes TRE.pri
#  |                |
#  |                `--- /TCFoundation
#  |                |     |--- TCFoundation.pri             Library declarations and directives project include - consumes LDViewGlobal.pri
#  |                |     |--- TCFoundation_QT.pro          Library Qt project file - consumes TCFoundation.pri
#  |                |     |--- TCFoundation_WGL.pro         Library WGL project file - consumes TCFoundation.pri
#  |                |     |--- TCFoundation_OSMesa.pro      Library OSMesa project file - consumes TCFoundation.pri
#  |                |
#  |                `--- /LDLoader
#  |                |     |--- LDLoader.pri                 Library declarations and directives project include - consumes LDViewGlobal.pri
#  |                |     |--- LDLoader_QT.pro              Library Qt project file - consumes LDLoader.pri
#  |                |     |--- LDLoader_WGL.pro             Library WGL project file - consumes LDLoader.pri
#  |                |     |--- LDLoader_OSMesa.pro          Library OSMesa project file - consumes LDLoader.pri
#  |                |
#  |                `--- /LDExporter
#  |                |     |--- LDExporter.pri               Library declarations and directives project include - consumes LDViewGlobal.pri
#  |                |     |--- LDExporter_QT.pro            Library Qt project file - consumes LDExporter.pri
#  |                |     |--- LDExporter_WGL.pro           Library WGL project file - consumes LDExporter.pri
#  |                |     |--- LDExporter_OSMesa.pro        Library OSMesa project file - consumes LDExporter.pri
#  |                |
#  |                `--- /3rdParty
#  |                      |
#  |                      `--- /tinyxml
#  |                      |     |--- 3rdParty_tinyxml.pro   3rdParty library project file - consumes 3rdParty.pri
#  |                      |
#  |                      `--- /gl2ps
#  |                      |     |--- 3rdParty_gl2ps.pro     3rdParty library project file - consumes 3rdParty.pri
#  |                      |
#  |                      `--- /minizip
#  |                      |     |--- 3rdParty_minizip.pro   3rdParty library project file - consumes 3rdParty.pri
#  |                      |
#  |                      `--- /lib3ds
#  |                      |     |--- 3rdParty_3ds.pro       3rdParty library project file - consumes 3rdParty.pri
#  |                      |
#  |                      `--- /libpng
#  |                      |     |--- 3rdParty_png.pro       3rdParty library project file - consumes 3rdParty.pri
#  |                      |
#  |                      `--- /libjpeg
#  |                      |    |--- 3rdParty_jpeg.pro       3rdParty library project file - consumes 3rdParty.pri
#  |                      |
#  |                      `--- /zlib
#  |                           |--- 3rdParty_zlib.pro       3rdParty library project file - consumes 3rdParty.pri
#  |
#  `--- /mainApp
#  |     |--- macosfiledistro.pri            Process Unix and MSYS2 distribution project include
#  |     |    |--- install3rdpartyassets.pri Unix and MSYS2 distributionpackaging assets project include
#  |     |
#  |     `--- posixfiledistro.pri            Process Unix and MSYS2 distribution project include
#  |     |    |--- install3rdpartyassets.pri Unix and MSYS2 distribution packaging assets project include
#  |     |    |--- locallibsdistro.pri       Process local libraries project include - used by legacy Linux build
#  |     |
#  |     |--- winfiledistro.pri              Process Windows MSVC distribution project include
#  |     |    |--- stageassets.pri           Stage Windows MSVC distribution packaging assets project include
#  |     |
#  |     |--- otherfiles.pri       Add 'other' project files project include
#  |     |--- gitversion.pri       Process git version project include
#  |     |--- mainApp.pri          Library declarations and directives project include
#  |     |--- LDVQtLibs.pri        Library declarations and directives project include
#  |     |--- QsLog.pri            Library declarations and directives project include
#  |     |--- QSimpleUpdater.pri   Library declarations and directives project include
#  |     |--- mainApp.pro          Library project file - consumes common.pri, gitversion.pri, mainApp.pri, and LDVQtLibs.pri
#  |
#  `--- /QSimpleUpdater
#  |     |--- QSimpleUpdater.pri   Library declarations and directives project include
#  |
#  `--- /qslog
#  |     |--- QsLog.pri            Library declarations and directives project include
#  |
#  `--- /quazip
#  |     |--- quazip.pri           Library declarations and directives project include
#  |     |--- quazip.pro           Library project file - consumes common.pri, quazip.pri
#  |
#  `--- /waitingspinner
#        |--- WaitingSpinner.pri   Library declarations and directives project include
#        |--- waitingspinner.pro   Library project file - consumes common.pri, waitingspinner.pri

TEMPLATE    = subdirs

CONFIG     += ordered # This tells Qt to compile the following SUBDIRS in order

# common directives
include(common.pri)

# build, type and opengl - postfix
CONFIG(debug, debug|release) {
    BUILD   = $$upper($${BUILD}) DEBUG BUILD
    LPUB3D  = LPub3Dd
} else {
    BUILD   = $$upper($${BUILD}) RELEASE BUILD
    LPUB3D  = LPub3D
}
msys {
    BUILD   = MSYS $${BUILD}
}
static|staticlib {
    TYPE    = STATIC
} else {
    TYPE    = SHARED
}
POSTFIX     = $${SUFFIX}

message("~~~ $$upper($${LPUB3D}) $$upper($$BUILD_ARCH) $${TYPE} $${BUILD} ON $$upper($$HOST) ~~~")
!isEmpty(USE_LDV_LIBRARIES_MSG): message("~~~ $$upper($${LPUB3D}) $${USE_LDV_LIBRARIES_MSG} ~~~")
!isEmpty(LDVIEW_DEV_REPO_MSG): message("~~~ $$upper($${LPUB3D}) $${LDVIEW_DEV_REPO_MSG} ~~~")

BUILD_LDV_LIBS {
    USE_LDV_SYSTEM_LIBS:          WHICH_LIBS = SYSTEM
    else: USE_LDV_3RD_PARTY_LIBS: WHICH_LIBS = BUILT-FROM-SOURCE
    else:                         WHICH_LIBS = PRE-COMPILED
    equals(NO_LDVIEW_DIST_LIBS, True): \
    message("~~~ BUILD LDV LIBRARIES - NO LDVIEW LIBS AT $$dirname(LIB_LDVIEW_PATH) ~~~ ")
    message("~~~ BUILD LDV LIBRARIES $$upper($${BUILD_ARCH}) STATIC $${BUILD} ~~~")
    message("~~~ BUILD LDV LIBRARIES USING $${POSTFIX} OPENGL AND $${WHICH_LIBS} 3RD PARTY LIBS ~~~")
    OTHER_FILES += \
        ldvlib/LDVQt/LDView/LDViewMessages.ini \
        ldvlib/LDVQt/LDView/LDExporter/LDExportMessages.ini
}

# Ubuntu Trusty uses libpng12 which is too old
if (USE_LDV_SYSTEM_LIBS|BUILD_GL2PS):contains(HOST, Ubuntu):contains(HOST, 14.04.5) {
    CONFIG += BUILD_PNG
}

BUILD_ZLIB {
    SUBDIRS                  += 3rdParty_zlib
    3rdParty_zlib.file        = ldvlib/LDVQt/LDView/3rdParty/zlib/3rdParty_zlib.pro
    3rdParty_zlib.makefile    = Makefile.zlib
    3rdParty_zlib.target      = sub-3rdParty_zlib
    3rdParty_zlib.depends     =
}
BUILD_3DS {
    SUBDIRS                  += 3rdParty_3ds
    3rdParty_3ds.file         = ldvlib/LDVQt/LDView/3rdParty/lib3ds/3rdParty_3ds.pro
    3rdParty_3ds.makefile     = Makefile.3ds
    3rdParty_3ds.target       = sub-3rdParty_3ds
    3rdParty_3ds.depends      =
}
BUILD_JPEG {
    SUBDIRS                  += 3rdParty_jpeg
    3rdParty_jpeg.file        = ldvlib/LDVQt/LDView/3rdParty/libjpeg/3rdParty_jpeg.pro
    3rdParty_jpeg.makefile    = Makefile.jpeg
    3rdParty_jpeg.target      = sub-3rdParty_jpeg
    3rdParty_jpeg.depends     =
}
BUILD_PNG {
    SUBDIRS                  += 3rdParty_png
    3rdParty_png.file         = ldvlib/LDVQt/LDView/3rdParty/libpng/3rdParty_png.pro
    3rdParty_png.makefile     = Makefile.png
    3rdParty_png.target       = sub-3rdParty_png
    3rdParty_png.depends      =
}
BUILD_GL2PS {
    SUBDIRS                  += 3rdParty_gl2ps
    3rdParty_gl2ps.file       = ldvlib/LDVQt/LDView/3rdParty/gl2ps/3rdParty_gl2ps.pro
    3rdParty_gl2ps.makefile   = Makefile.gl2ps
    3rdParty_gl2ps.target     = sub-3rdParty_gl2ps
    3rdParty_gl2ps.depends    =
}
BUILD_TINYXML {
    SUBDIRS                  += 3rdParty_tinyxml
    3rdParty_tinyxml.file     = ldvlib/LDVQt/LDView/3rdParty/tinyxml/3rdParty_tinyxml.pro
    3rdParty_tinyxml.makefile = Makefile.tinyxml
    3rdParty_tinyxml.target   = sub-3rdParty_tinyxml
    3rdParty_tinyxml.depends  =
}
BUILD_MINIZIP {
    SUBDIRS                  += 3rdParty_minizip
    3rdParty_minizip.file     = ldvlib/LDVQt/LDView/3rdParty/minizip/3rdParty_minizip.pro
    3rdParty_minizip.makefile = Makefile.minizip
    3rdParty_minizip.target   = sub-3rdParty_minizip
    3rdParty_minizip.depends  =
}

# Build Qt/OSMesa/WGL LDView Libraries
BUILD_LDV_LIBS {
    SUBDIRS                          += \
                                        TRE_$${POSTFIX} \
                                        TCFoundation_$${POSTFIX} \
                                        LDLib_$${POSTFIX} \
                                        LDLoader_$${POSTFIX} \
                                        LDExporter_$${POSTFIX} #\
#                                        Headerize

    TRE_$${POSTFIX}.file              = ldvlib/LDVQt/LDView/TRE/TRE_$${POSTFIX}.pro
    TRE_$${POSTFIX}.makefile          = Makefile-tre.$$lower($${POSTFIX})
    TRE_$${POSTFIX}.target            = sub-TRE_$${POSTFIX}
    BUILD_GL2PS: \
    TRE_$${POSTFIX}.depends           = 3rdParty_gl2ps

    TCFoundation_$${POSTFIX}.file     = ldvlib/LDVQt/LDView/TCFoundation/TCFoundation_$${POSTFIX}.pro
    TCFoundation_$${POSTFIX}.makefile = Makefile-tcfoundation.$$lower($${POSTFIX})
    TCFoundation_$${POSTFIX}.target   = sub-TCFoundation_$${POSTFIX}
    BUILD_MINIZIP: \
    TCFoundation_$${POSTFIX}.depends  = 3rdParty_minizip

    LDLib_$${POSTFIX}.file            = ldvlib/LDVQt/LDView/LDLib/LDLib_$${POSTFIX}.pro
    LDLib_$${POSTFIX}.makefile        = Makefile-ldlib.$$lower($${POSTFIX})
    LDLib_$${POSTFIX}.target          = sub-LDLib_$${POSTFIX}
    BUILD_GL2PS: \
    LDLib_$${POSTFIX}.depends         = 3rdParty_gl2ps

    LDLoader_$${POSTFIX}.file         = ldvlib/LDVQt/LDView/LDLoader/LDLoader_$${POSTFIX}.pro
    LDLoader_$${POSTFIX}.makefile     = Makefile-ldloader.$$lower($${POSTFIX})
    LDLoader_$${POSTFIX}.target       = sub-LDLoader_$${POSTFIX}
    LDLoader_$${POSTFIX}.depends      =

    LDExporter_$${POSTFIX}.file       = ldvlib/LDVQt/LDView/LDExporter/LDExporter_$${POSTFIX}.pro
    LDExporter_$${POSTFIX}.makefile   = Makefile-ldexporter.$$lower($${POSTFIX})
    LDExporter_$${POSTFIX}.target     = sub-LDExporter_$${POSTFIX}
    BUILD_TINYXML: \
    LDExporter_$${POSTFIX}.depends    = 3rdParty_tinyxml

    Headerize.file                    = ldvlib/LDVQt/LDView/Utilities/Headerize.pro
    Headerize.makefile                = Makefile-headerize.$$lower($${POSTFIX})
    Headerize.target                  = sub-Headerize
    Headerize.depends                 = TCFoundation_$${POSTFIX}
    BUILD_ZLIB: \
    Headerize.depends                += 3rdParty_zlib
}

SUBDIRS                   += ldrawini
ldrawini.subdir            = ldrawini
ldrawini.makefile          = Makefile.ldrawini
ldrawini.target            = sub-ldrawini
ldrawini.depends           =

SUBDIRS                   += quazip
quazip.subdir              = quazip
quazip.makefile            = Makefile.quazip
quazip.target              = sub-quazip
quazip.depends             =

SUBDIRS                   += ldvqt_$${POSTFIX}
ldvqt_$${POSTFIX}.subdir   = ldvlib/LDVQt
ldvqt_$${POSTFIX}.makefile = Makefile-ldvqt.$$lower($${POSTFIX})
ldvqt_$${POSTFIX}.target   = sub-ldvqt_$${POSTFIX}
BUILD_LDV_LIBS {
ldvqt_$${POSTFIX}.depends  = TRE_$${POSTFIX} \
                             TCFoundation_$${POSTFIX} \
                             LDLib_$${POSTFIX} \
                             LDLoader_$${POSTFIX} \
                             LDExporter_$${POSTFIX} \
                             3rdParty_tinyxml
win32-arm64-msvc|win32-msvc*: \
ldvqt_$${POSTFIX}.depends += 3rdParty_gl2ps
else: \
ldvqt_$${POSTFIX}.depends += 3rdParty_3ds
} # BUILD_LDV_LIBS

SUBDIRS                   += lclib
lclib.subdir               = lclib
lclib.makefile             = Makefile.lclib
lclib.target               = sub-lclib
lclib.depends              =

SUBDIRS                   += waitingspinner
waitingspinner.subdir      = waitingspinner
waitingspinner.makefile    = Makefile.waitingspinner
waitingspinner.target      = sub-waitingspinner
waitingspinner.depends     =

SUBDIRS                   += mainApp
mainApp.subdir             = mainApp
mainApp.makefile           = Makefile.mainapp
mainApp.target             = sub-mainapp
mainApp.depends            = quazip \
                             ldrawini \
                             lclib \
                             ldvqt_$${POSTFIX} \
                             waitingspinner

RESOURCES                 += \
                             qsimpleupdater/etc/resources/qsimpleupdater.qrc \
                             mainApp/lpub3d.qrc

msys: message("~~~ $$upper($${LPUB3D}) MSYS2 SYSTEM_PREFIX $${PREFIX} ~~~ ")
