# LPub3D QMake Configuration settings
# CONFIG+=ldviewqt               # build LDVQt LDView libs using Qt OpenGL
# CONFIG+=ldviewwgl              # build LDVQt LDView libs using WGL OpenGL
# CONFIG+=ldviewosmesa           # build LDVQt LDView libs using OSMesa OpenGL

# LDV Libraries QMake Configuration settings
# CONFIG+=BUILD_LDV_LIBS         # build LDView libs from source
# CONFIG+=USE_LDV_SYSTEM_LIBS    # override using pre-compiled 3rdParty libraries
# CONFIG+=USE_LDV_3RD_PARTY_LIBS # override using pre-compiled and system 3rdParty libraries
# CONFIG+=USE_SYSTEM_PNG         # override USE_LDV_3RD_PARTY_LIBS for libpng
# CONFIG+=USE_SYSTEM_GL2PS       # override USE_LDV_3RD_PARTY_LIBS for libgl2ps
# CONFIG+=USE_SYSTEM_JPEG        # override USE_LDV_3RD_PARTY_LIBS for libjpeg
# CONFIG+=USE_SYSTEM_MINIZIP     # override USE_LDV_3RD_PARTY_LIBS for libminizip
# CONFIG+=USE_SYSTEM_ZLIB        # override USE_LDV_3RD_PARTY_LIBS for libz
# CONFIG+=BUILD_3DS              # override USE_LDV_SYSTEM_LIBS for lib3ds - always built except for MSVC
# CONFIG+=BUILD_JPEG             # override USE_LDV_SYSTEM_LIBS for libjpeg
# CONFIG+=BUILD_PNG              # override USE_LDV_SYSTEM_LIBS for libpng
# CONFIG+=BUILD_GL2PS            # override USE_LDV_SYSTEM_LIBS for libgl2ps - always built for MSVC
# CONFIG+=BUILD_TINYXML          # override USE_LDV_SYSTEM_LIBS for libtinyxml
# CONFIG+=BUILD_MINIZIP          # override USE_LDV_SYSTEM_LIBS for libminizip
# CONFIG+=BUILD_ZLIB             # override USE_LDV_SYSTEM_LIBS for libz

# LPub3D QMake directory and project file structre
# ------------------------------------------
# /LPub3D
#  |--- LPub3D.pro                 Subdirs project file
#  |--- gitversion.pri             Construct project version
#  |
#  `--- /lclib
#  |     |--- lclib.pro            Library project file - consumes lclib.pri
#  |     |--- lclib.pri            Library declarations and directives project include
#  |
#  `--- /ldrawini
#  |     |--- ldrawini.pri         Library declarations and directives project include
#  |     |--- Lldrawini.pro        Library project file - consumes ldrawini.pri
#  |
#  `--- /ldvlib
#  |     |
#  |     `--- /LDVQt
#  |     |     |--- LDVQtLibs.pri  Library declarations and directives project include
#  |     |     |--- LDVQt.pro      Library project file - consumes LDVQtLibs.pri
#  |     |     |
#  |     |     `--- /LDView
#  |     |          |--- LDViewGlobal.pri                   Global declarations and directives project include
#  |     |          |--- 3rdParty.pri                       3rdParty library declarations and directives project include
#  |     |          |
#  |     |          `--- /Utilities
#  |     |          |     |--- Headerize.pro                Executable headerizer project file - declarations and directives - consumes LDViewGlobal.pri
#  |     |          |
#  |     |          `--- /LDLib
#  |     |          |     |--- LDLib.pri                    Library declarations and directives project include - consumes LDViewGlobal.pri
#  |     |          |     |--- LDLib_QT.pro                 Library Qt project file - consumes LDLib.pri
#  |     |          |     |--- LDLib_WGL.pro                Library WGL project file - consumes LDLib.pri
#  |     |          |     |--- LDLib_OSMesa.pro             Library OSMesa project file - consumes LDLib.pri
#  |     |          |
#  |     |          `--- /TRE
#  |     |          |     |--- TRE.pri                      Library declarations and directives project include - consumes LDViewGlobal.pri
#  |     |          |     |--- TRE_QT.pro                   Library Qt project file - consumes TRE.pri
#  |     |          |     |--- TRE_WGL.pro                  Library WGL project file - consumes TRE.pri
#  |     |          |     |--- TRE_OSMesa.pro               Library OSMesa project file - consumes TRE.pri
#  |     |          |
#  |     |          `--- /TCFoundation
#  |     |          |     |--- TCFoundation.pri             Library declarations and directives project include - consumes LDViewGlobal.pri
#  |     |          |     |--- TCFoundation_QT.pro          Library Qt project file - consumes TCFoundation.pri
#  |     |          |     |--- TCFoundation_WGL.pro         Library WGL project file - consumes TCFoundation.pri
#  |     |          |     |--- TCFoundation_OSMesa.pro      Library OSMesa project file - consumes TCFoundation.pri
#  |     |          |
#  |     |          `--- /LDLoader
#  |     |          |     |--- LDLoader.pri                 Library declarations and directives project include - consumes LDViewGlobal.pri
#  |     |          |     |--- LDLoader_QT.pro              Library Qt project file - consumes LDLoader.pri
#  |     |          |     |--- LDLoader_WGL.pro             Library WGL project file - consumes LDLoader.pri
#  |     |          |     |--- LDLoader_OSMesa.pro          Library OSMesa project file - consumes LDLoader.pri
#  |     |          |
#  |     |          `--- /LDExporter
#  |     |          |     |--- LDExporter.pri               Library declarations and directives project include - consumes LDViewGlobal.pri
#  |     |          |     |--- LDExporter_QT.pro            Library Qt project file - consumes LDExporter.pri
#  |     |          |     |--- LDExporter_WGL.pro           Library WGL project file - consumes LDExporter.pri
#  |     |          |     |--- LDExporter_OSMesa.pro        Library OSMesa project file - consumes LDExporter.pri
#  |     |          |
#  |     |          `--- /3rdParty
#  |     |                |
#  |     |                `--- /tinyxml
#  |     |                |     |--- 3rdParty_tinyxml.pro   3rdParty library project file - consumes 3rdParty.pri
#  |     |                |
#  |     |                `--- /gl2ps
#  |     |                |     |--- 3rdParty_gl2ps.pro     3rdParty library project file - consumes 3rdParty.pri
#  |     |                |
#  |     |                `--- /minizip
#  |     |                |     |--- 3rdParty_minizip.pro   3rdParty library project file - consumes 3rdParty.pri
#  |     |                |
#  |     |                `--- /lib3ds
#  |     |                |     |--- 3rdParty_3ds.pro       3rdParty library project file - consumes 3rdParty.pri
#  |     |                |
#  |     |                `--- /libpng
#  |     |                |     |--- 3rdParty_png.pro       3rdParty library project file - consumes 3rdParty.pri
#  |     |                |
#  |     |                `--- /libjpeg
#  |     |                |    |--- 3rdParty_jpeg.pro      3rdParty library project file - consumes 3rdParty.pri
#  |     |                |
#  |     |                `--- /zlib
#  |     |                     |--- 3rdParty_zlib.pro      3rdParty library project file - consumes 3rdParty.pri
#  |     |
#  |     `--- /WPngImage
#  |           |--- WPngImage.pri  Library declarations and directives project include
#  |           |--- WPngImage.pro  Library project file - consumes WPngImage.pri
#  |
#  `--- /mainApp
#  |     |--- macosfiledistro.pri
#  |     |    |--- install3rdpartyassets.pri Project include
#  |     |
#  |     `--- posixfiledistro.pri
#  |     |    |--- install3rdpartyassets.pri Project include
#  |     |    |--- locallibsdistro.pri
#  |     |
#  |     |--- winfiledistro.pri
#  |     |    |--- stageassets.pri           Project include
#  |     |
#  |     |--- otherfiles.pri       Project include
#  |     |--- gitversion.pri       Project include
#  |     |--- mainApp.pri          Library declarations and directives project include
#  |     |--- LDVQtLibs.pri        Library declarations and directives project include
#  |     |--- QsLog.pri            Library declarations and directives project include
#  |     |--- QSimpleUpdater.pri   Library declarations and directives project include
#  |     |--- mainApp.pro          Library project file - consumes mainApp.pri, gitversion.pri and LDVQtLibs.pri
#  |
#  `--- /QSimpleUpdater
#  |     |--- QSimpleUpdater.pri   Library declarations and directives project include
#  |
#  `--- /qslog
#  |     |--- QsLog.pri            Library declarations and directives project include
#  |
#  `--- /quazip
#  |     |--- quazip.pri           Library declarations and directives project include
#  |     |--- quazip.pro           Library project file - consumes quazip.pri
#  |
#  `--- /waitingspinner
#  |     |--- WaitingSpinner.pri   Library declarations and directives project include
#  |     |--- waitingspinner.pro   Library project file - consumes waitingspinner.pri

TEMPLATE   = subdirs

CONFIG    += ordered # This tells Qt to compile the following SUBDIRS in order

win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST  = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

BUILD_ARCH = $$(TARGET_CPU)
!contains(QT_ARCH, unknown): \
BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH): \
BUILD_ARCH = UNKNOWN ARCH
CONFIG(debug, debug|release) {
    BUILD  = DEBUG BUILD
    LPUB3D = LPub3Dd
} else {
    BUILD  = RELEASE BUILD
    LPUB3D = LPub3D
}
msys: BUILD = MSYS $$BUILD
static|staticlib: \
TYPE  = STATIC
else: \
TYPE  = SHARED

# Qt/OSMesa library identifiers
ldviewqt {
    POSTFIX  = QT
    SUFFIX  = -qt$${QT_MAJOR_VERSION}
} else:ldviewwgl {
    POSTFIX  = WGL
    SUFFIX  = -wgl
} else:!win32-msvc* {
    POSTFIX  = OSMesa
    SUFFIX  = -osmesa
}

message("~~~ $$upper($$LPUB3D) $$upper($$BUILD_ARCH) $${TYPE} $${BUILD} ON $$upper($$HOST) ~~~")

!BUILD_LDV_LIBS {
    #   Argument path - LP3D_3RD_DIST_DIR
    !isEmpty(LP3D_3RD_DIST_DIR) {
        THIRD_PARTY_DIST_DIR_PATH = $$LP3D_3RD_DIST_DIR
    } else {
    #   Environment variable path - LP3D_DIST_DIR_PATH
        THIRD_PARTY_DIST_DIR_PATH = $$(LP3D_DIST_DIR_PATH)
    }
    #   Local path
    isEmpty(THIRD_PARTY_DIST_DIR_PATH): \
    THIRD_PARTY_DIST_DIR_PATH     = $$absolute_path( builds/3rdparty )
    #   Default path
    !exists($$THIRD_PARTY_DIST_DIR_PATH) {
        unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
        else:msys:  DIST_DIR      = lpub3d_msys_3rdparty
        else:macx:  DIST_DIR      = lpub3d_macos_3rdparty
        else:win32: DIST_DIR      = lpub3d_windows_3rdparty
        THIRD_PARTY_DIST_DIR_PATH = $$absolute_path( ../$$DIST_DIR )
        !exists($$THIRD_PARTY_DIST_DIR_PATH): \
        THIRD_PARTY_DIST_DIR_PATH="undefined"
    }
    VER_LDVIEW  = ldview-4.6
    unix|msys: \
    LIB_LDVIEW  = libTCFoundation$${SUFFIX}.a
    else:win32-msvc*: \
    LIB_LDVIEW  = TCFoundation$${SUFFIX}.lib
    LIB_LDVIEW_PATH = $${THIRD_PARTY_DIST_DIR_PATH}/$${VER_LDVIEW}/lib/$${QT_ARCH}/$${LIB_LDVIEW}
    !exists($${LIB_LDVIEW_PATH}) {
        message("~~~ BUILD LDV LIBRARIES - NO LDVIEW LIBS AT $$dirname(LIB_LDVIEW_PATH) ~~~ ")
        CONFIG += BUILD_LDV_LIBS
    }
}

BUILD_LDV_LIBS {
    USE_LDV_3RD_PARTY_LIBS:USE_LDV_SYSTEM_LIBS {
        message("~~~ NOTICE 'USE_LDV_3RD_PARTY_LIBS' and 'USE_LDV_SYSTEM_LIBS' Specified. Using 'USE_LDV_3RD_PARTY_LIBS'")
        CONFIG -= USE_LDV_SYSTEM_LIBS
    }
    USE_LDV_SYSTEM_LIBS:          WHICH_LIBS = SYSTEM
    else: USE_LDV_3RD_PARTY_LIBS: WHICH_LIBS = BUILT-FROM-SOURCE
    else:                         WHICH_LIBS = PRE-COMPILED
    message("~~~ LDV LIBRARIES $$upper($$BUILD_ARCH) STATIC $${BUILD} ~~~")
    message("~~~ LDV LIBRARIES USING $${POSTFIX} OPENGL AND $${WHICH_LIBS} 3RD PARTY LIBS ~~~")

    # Except for MSVC (uses pre-built), always build lib3ds
    USE_LDV_3RD_PARTY_LIBS {
        CONFIG += BUILD_3DS
        CONFIG += BUILD_TINYXML
        !USE_SYSTEM_JPEG: \
        CONFIG += BUILD_JPEG
        !USE_SYSTEM_PNG: \
        CONFIG += BUILD_PNG
        !USE_SYSTEM_GL2PS: \
        CONFIG += BUILD_GL2PS
        !USE_SYSTEM_MINIZIP: \
        CONFIG += BUILD_MINIZIP
        !USE_SYSTEM_ZLIB: \
        CONFIG += BUILD_ZLIB
    } else {
        win32-msvc*: \
        CONFIG += BUILD_GL2PS BUILD_TINYXML
        else: \
        CONFIG += BUILD_3DS
    }

    OTHER_FILES += $$PWD/ldvlib/LDVQt/LDView/LDViewMessages.ini \
                   $$PWD/ldvlib/LDVQt/LDView/LDExporter/LDExportMessages.ini
}

# Build 3rdParty Libraries'

# Ubuntu Trusty uses libpng12 which is too old
if (USE_LDV_SYSTEM_LIBS|BUILD_GL2PS):contains(HOST, Ubuntu):contains(HOST, 14.04.5) {
    CONFIG += BUILD_PNG
}

# always built (Except for MSVC) unless prebuilt instance specified...
if (USE_LDV_3RD_PARTY_LIBS|BUILD_ZLIB) {
    SUBDIRS += 3rdParty_zlib
    3rdParty_zlib.file        = $$PWD/ldvlib/LDVQt/LDView/3rdParty/zlib/3rdParty_zlib.pro
    3rdParty_zlib.makefile    = Makefile.zlib
    3rdParty_zlib.target      = sub-3rdParty_zlib
    3rdParty_zlib.depends     =
}
if (USE_LDV_3RD_PARTY_LIBS|BUILD_3DS) {
    SUBDIRS += 3rdParty_3ds
    3rdParty_3ds.file         = $$PWD/ldvlib/LDVQt/LDView/3rdParty/lib3ds/3rdParty_3ds.pro
    3rdParty_3ds.makefile     = Makefile.3ds
    3rdParty_3ds.target       = sub-3rdParty_3ds
    3rdParty_3ds.depends      =
}
if (USE_LDV_3RD_PARTY_LIBS|BUILD_JPEG) {
    SUBDIRS += 3rdParty_jpeg
    3rdParty_jpeg.file        = $$PWD/ldvlib/LDVQt/LDView/3rdParty/libjpeg/3rdParty_jpeg.pro
    3rdParty_jpeg.makefile    = Makefile.jpeg
    3rdParty_jpeg.target      = sub-3rdParty_jpeg
    3rdParty_jpeg.depends     =
}
if (USE_LDV_3RD_PARTY_LIBS|BUILD_PNG) {
    SUBDIRS += 3rdParty_png
    3rdParty_png.file         = $$PWD/ldvlib/LDVQt/LDView/3rdParty/libpng/3rdParty_png.pro
    3rdParty_png.makefile     = Makefile.png
    3rdParty_png.target       = sub-3rdParty_png
    3rdParty_png.depends      =
}
if (USE_LDV_3RD_PARTY_LIBS|BUILD_GL2PS) {
    SUBDIRS += 3rdParty_gl2ps
    3rdParty_gl2ps.file       = $$PWD/ldvlib/LDVQt/LDView/3rdParty/gl2ps/3rdParty_gl2ps.pro
    3rdParty_gl2ps.makefile   = Makefile.gl2ps
    3rdParty_gl2ps.target     = sub-3rdParty_gl2ps
    3rdParty_gl2ps.depends    = #3rdParty_png
}
if (USE_LDV_3RD_PARTY_LIBS|BUILD_TINYXML) {
    SUBDIRS += 3rdParty_tinyxml
    3rdParty_tinyxml.file     = $$PWD/ldvlib/LDVQt/LDView/3rdParty/tinyxml/3rdParty_tinyxml.pro
    3rdParty_tinyxml.makefile = Makefile.tinyxml
    3rdParty_tinyxml.target   = sub-3rdParty_tinyxml
    3rdParty_tinyxml.depends  =
}
if (USE_LDV_3RD_PARTY_LIBS|BUILD_MINIZIP) {
    SUBDIRS += 3rdParty_minizip
    3rdParty_minizip.file     = $$PWD/ldvlib/LDVQt/LDView/3rdParty/minizip/3rdParty_minizip.pro
    3rdParty_minizip.makefile = Makefile.minizip
    3rdParty_minizip.target   = sub-3rdParty_minizip
    3rdParty_minizip.depends  =
}

# Build Qt/OSMesa/WGL LDView Libraries
BUILD_LDV_LIBS {
    SUBDIRS += \
       TRE_$${POSTFIX} \
       TCFoundation_$${POSTFIX} \
       LDLib_$${POSTFIX} \
       LDLoader_$${POSTFIX} \
       LDExporter_$${POSTFIX} \
       Headerize

    TRE_$${POSTFIX}.file              = $$PWD/ldvlib/LDVQt/LDView/TRE/TRE_$${POSTFIX}.pro
    TRE_$${POSTFIX}.makefile          = Makefile-tre.$$lower($${POSTFIX})
    TRE_$${POSTFIX}.target            = sub-TRE_$${POSTFIX}
    if (USE_LDV_3RD_PARTY_LIBS|BUILD_GL2PS): \
    TRE_$${POSTFIX}.depends           = 3rdParty_gl2ps

    TCFoundation_$${POSTFIX}.file     = $$PWD/ldvlib/LDVQt/LDView/TCFoundation/TCFoundation_$${POSTFIX}.pro
    TCFoundation_$${POSTFIX}.makefile = Makefile-tcfoundation.$$lower($${POSTFIX})
    TCFoundation_$${POSTFIX}.target   = sub-TCFoundation_$${POSTFIX}
    USE_LDV_3RD_PARTY_LIBS: \
    TCFoundation_$${POSTFIX}.depends  = 3rdParty_minizip

    LDLib_$${POSTFIX}.file            = $$PWD/ldvlib/LDVQt/LDView/LDLib/LDLib_$${POSTFIX}.pro
    LDLib_$${POSTFIX}.makefile        = Makefile-ldlib.$$lower($${POSTFIX})
    LDLib_$${POSTFIX}.target          = sub-LDLib_$${POSTFIX}
    if (USE_LDV_3RD_PARTY_LIBS|BUILD_GL2PS): \
    LDLib_$${POSTFIX}.depends         = 3rdParty_gl2ps

    LDLoader_$${POSTFIX}.file         = $$PWD/ldvlib/LDVQt/LDView/LDLoader/LDLoader_$${POSTFIX}.pro
    LDLoader_$${POSTFIX}.makefile     = Makefile-ldloader.$$lower($${POSTFIX})
    LDLoader_$${POSTFIX}.target       = sub-LDLoader_$${POSTFIX}
    LDLoader_$${POSTFIX}.depends      =

    LDExporter_$${POSTFIX}.file       = $$PWD/ldvlib/LDVQt/LDView/LDExporter/LDExporter_$${POSTFIX}.pro
    LDExporter_$${POSTFIX}.makefile   = Makefile-ldexporter.$$lower($${POSTFIX})
    LDExporter_$${POSTFIX}.target     = sub-LDExporter_$${POSTFIX}
    if (USE_LDV_3RD_PARTY_LIBS|BUILD_TINYXML): \
    LDExporter_$${POSTFIX}.depends    = 3rdParty_tinyxml

    # Headerize utility
    Headerize.file                    = $$PWD/ldvlib/LDVQt/LDView/Utilities/Headerize.pro
    Headerize.makefile                = Makefile-headerize.$$lower($${POSTFIX})
    Headerize.target                  = sub-Headerize
    Headerize.depends                 = TCFoundation_$${POSTFIX}
    USE_LDV_3RD_PARTY_LIBS: \
    Headerize.depends                += 3rdParty_zlib
}

SUBDIRS += ldrawini
ldrawini.subdir   = $$PWD/ldrawini
ldrawini.makefile = Makefile.ldrawini
ldrawini.target   = sub-ldrawini
ldrawini.depends  =

isEmpty(quazipnobuild) {
    SUBDIRS += quazip
    quazip.subdir   = $$PWD/quazip
    quazip.makefile = Makefile.quazip
    quazip.target   = sub-quazip
    quazip.depends  =
}

SUBDIRS += ldvqt_$${POSTFIX}
ldvqt_$${POSTFIX}.subdir   = $$PWD/ldvlib/LDVQt
ldvqt_$${POSTFIX}.makefile = Makefile-ldvqt.$$lower($${POSTFIX})
ldvqt_$${POSTFIX}.target   = sub-ldvqt_$${POSTFIX}
ldvqt_$${POSTFIX}.depends  =
BUILD_LDV_LIBS {
ldvqt_$${POSTFIX}.depends  = TRE_$${POSTFIX} \
                             TCFoundation_$${POSTFIX} \
                             LDLib_$${POSTFIX} \
                             LDLoader_$${POSTFIX} \
                             LDExporter_$${POSTFIX}
win32-msvc*: \
ldvqt_$${POSTFIX}.depends += 3rdParty_gl2ps 3rdParty_tinyxml
else: \
ldvqt_$${POSTFIX}.depends += 3rdParty_3ds
}

SUBDIRS += wpngimage
wpngimage.subdir   = $$PWD/ldvlib/WPngImage
wpngimage.makefile = Makefile.wpngimage
wpngimage.target   = sub-wpngimage
wpngimage.depends  =

SUBDIRS += lclib
lclib.subdir   = $$PWD/lclib
lclib.makefile = Makefile.lclib
lclib.target   = sub-lclib
lclib.depends  =

SUBDIRS += waitingspinner
waitingspinner.subdir   = $$PWD/waitingspinner
waitingspinner.makefile = Makefile.waitingspinner
waitingspinner.target   = sub-waitingspinner
waitingspinner.depends  =

SUBDIRS += mainApp
mainApp.subdir   = $$PWD/mainApp
mainApp.makefile = Makefile.mainapp
mainApp.target   = sub-mainapp
isEmpty(quazipnobuild): \
mainApp.depends  = quazip
mainApp.depends += ldrawini
mainApp.depends += lclib
mainApp.depends += ldvqt_$${POSTFIX}
mainApp.depends += wpngimage
mainApp.depends += waitingspinner

RESOURCES += \
    qsimpleupdater/etc/resources/qsimpleupdater.qrc \
    mainApp/lpub3d.qrc

msys: message("~~~ MSYS2 SYSTEM_PREFIX $${PREFIX} ~~~ ")
