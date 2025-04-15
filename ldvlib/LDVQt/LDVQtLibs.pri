win32-msvc* {
    BUILD_WORKER_VERSION = $$(LP3D_VSVERSION)
    isEmpty(BUILD_WORKER_VERSION): BUILD_WORKER_VERSION = 2019
    lessThan(BUILD_WORKER_VERSION, 2019) {
        VSVER=vs2015
    } else {
        contains(QT_ARCH,i386): VSVER=vs2017
        else: VSVER=vs2019
    }
    !contains(LOAD_LDV_LIBS,True) {
        message("~~~ lib$${TARGET} BUILD WORKER: Visual Studio $$BUILD_WORKER_VERSION ~~~")
        message("~~~ lib$${TARGET} $$upper($$QT_ARCH) MSVS LIBRARY VERSION: $$VSVER ~~~")
    }
}

contains(LOAD_LDV_HEADERS,True) {
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # 3rd party executables, documentation and resources.
    !isEmpty(LP3D_3RD_DIST_DIR): \
    THIRD_PARTY_DIST_DIR_PATH     = $$LP3D_3RD_DIST_DIR
    else: \
    THIRD_PARTY_DIST_DIR_PATH     = $$(LP3D_DIST_DIR_PATH)
    isEmpty(THIRD_PARTY_DIST_DIR_PATH): \
    THIRD_PARTY_DIST_DIR_PATH     = $$absolute_path( $$PWD/../../builds/3rdparty )
    !exists($$THIRD_PARTY_DIST_DIR_PATH) {
        unix:!macx: DIST_DIR      = lpub3d_linux_3rdparty
        else:macx:  DIST_DIR      = lpub3d_macos_3rdparty
        else:win32: DIST_DIR      = lpub3d_windows_3rdparty
        THIRD_PARTY_DIST_DIR_PATH = $$absolute_path( $$PWD/../../../$$DIST_DIR )
        !exists($$THIRD_PARTY_DIST_DIR_PATH) {
            message("~~~ ERROR lib$${TARGET} - THIRD_PARTY_DIST_DIR_PATH (LDVLIB) WAS NOT FOUND! ~~~ ")
            THIRD_PARTY_DIST_DIR_PATH="undefined"
        }
    }

    VER_LDVIEW = ldview-4.5
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Header soruce path
    equals(VER_USE_LDVIEW_DEV,True) {
        isEmpty(LDVHDRDIR):LDVHDRDIR       = $${VER_LDVIEW_DEV_REPOSITORY}/include
        isEmpty(LDV3RDHDRDIR):LDV3RDHDRDIR = $$LDVHDRDIR
        isEmpty(LDV3RDHDR):LDV3RDHDR       = $${VER_LDVIEW_DEV_REPOSITORY}/3rdParty
    } else {
        isEmpty(LDVHDRDIR):LDVHDRDIR       = $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include
        isEmpty(LDV3RDHDRDIR):LDV3RDHDRDIR = $${LDVHDRDIR}/3rdParty
        isEmpty(LDV3RDHDR):LDV3RDHDR       = $${LDV3RDHDRDIR}
    }

    message("~~~ lib$${TARGET} ADD LDVIEW 3RDPARTY HEADERS TO INCLUDEPATH: $$LDVHDRDIR ~~~ ")

    DEPENDPATH  += $${LDVHDRDIR}
    INCLUDEPATH += $${LDVHDRDIR}
    INCLUDEPATH += $${LDV3RDHDRDIR}
    if (contains(UNIX_ARM_BUILD_ARCH,True)) {
        if(!contains(DEFINES,ARM_SKIP_GL_HEADERS)) {
            system( mkdir -p ./include/GL && touch ./include/GL/glext.h )
            INCLUDEPATH += ./include/GL
            message("~~~ lib$${TARGET} ADD FAKE LDVIEW GLEXT HEADER TO INCLUDEPATH: ./include/GL ~~~ ")
        }
        exists ($${LDVHDRDIR}/GL): \
        system(rm -rf $${LDVHDRDIR}/GL)
        !exists ($${LDVHDRDIR}/GL): \
        message("~~~ lib$${TARGET} REMOVED LDVIEW GL HEADERS FOR $$upper($$QT_ARCH) BUILD ~~~ ")
        message("~~~ lib$${TARGET} USING SYSTEM GL HEADERS FOR $$upper($$QT_ARCH) BUILD ~~~ ")
    } else {
        if (!contains(SKIP_LDV_GL_HEADERS,True)) {
            INCLUDEPATH += $${LDVHDRDIR}/GL
            message("~~~ lib$${TARGET} ADD LDVIEW GL HEADERS TO INCLUDEPATH: $$LDVHDRDIR/GL ~~~ ")
        }
    }

    if (unix:exists(/usr/include/tinyxml.h)|exists($${SYSTEM_PREFIX_}/include/tinyxml.h)) {
        message("~~~ lib$${TARGET} system library header for tinyxml FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/tinyxml.h) {
        message("~~~ lib$${TARGET} local library header for tinyxml FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for tinyxml NOT FOUND ~~~")
    }
    if (unix:exists(/usr/include/gl2ps.h)|exists($${SYSTEM_PREFIX_}/include/gl2ps.h)) {
        message("~~~ lib$${TARGET} system library header for gl2ps FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/gl2ps.h) {
        message("~~~ lib$${TARGET} local library header for gl2ps FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for gl2ps NOT FOUND, using local ~~~")
    }
    if (unix:exists(/usr/include/lib3ds.h)|exists($${SYSTEM_PREFIX_}/include/lib3ds.h)) {
        message("~~~ lib$${TARGET} system library header for 3ds FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/lib3ds.h) {
        message("~~~ lib$${TARGET} local library header for 3ds FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for 3ds NOT FOUND ~~~")
    }
    if (unix:exists(/usr/include/minizip/unzip.h)|exists($${SYSTEM_PREFIX_}/include/minizip/unzip.h)) {
        message("~~~ lib$${TARGET} system library header for minizip FOUND ~~~")
        LDVMINIZIPFOUND=1
    } else:exists($${LDV3RDHDR}/minizip/unzip.h) {
        message("~~~ lib$${TARGET} local library header for minizip FOUND ~~~")
        LDVMINIZIPFOUND=1
    } else {
        LDVMINIZIPFOUND=0
        message("~~~ ERROR lib$${TARGET}: library header for minizip NOT FOUND ~~~")
    }
    if (unix:exists(/usr/include/png.h)|exists($${SYSTEM_PREFIX_}/include/png.h)) {
        message("~~~ lib$${TARGET} system library header for png FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/png.h) {
        message("~~~ lib$${TARGET} local library header for png FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for png NOT FOUND ~~~")
    }
    if (unix:exists(/usr/include/jpeglib.h)|exists($${SYSTEM_PREFIX_}/include/jpeglib.h)) {
        message("~~~ lib$${TARGET} system library header for jpeg FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/gl2ps.h) {
        message("~~~ lib$${TARGET} local library header for jpeg FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for jpeg NOT FOUND ~~~")
    }
    contains(LDVMINIZIPFOUND,1) {
        DEFINES += HAVE_MINIZIP
        INCLUDEPATH += $${LDV3RDHDR}
    }

} # LOAD_LDV_HEADERS,True


# This block is executed by LPub3D mainApp to enable linking the LDVlib
contains(LOAD_LDV_LIBS,True) {
    win32-msvc*:CONFIG(debug, debug|release) {
        equals(VER_USE_LDVIEW_DEV,True) {
            contains(LIB_ARCH, 64): LDVLIB_ARCH = /x64
            LDVLIBDIR    = $${VER_LDVIEW_DEV_REPOSITORY}/Build/Debug$$LIB_ARCH
            LDV3RDLIBDIR = $${VER_LDVIEW_DEV_REPOSITORY}/lib$$LDVLIB_ARCH
        } else {
            LDVLIBDIR    = $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/Build/Debug$$LIB_ARCH
            LDV3RDLIBDIR = $$LDVLIBDIR
        }
    } else {
        LDVLIBDIR       = $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/lib/$$QT_ARCH
        LDV3RDLIBDIR    = $$LDVLIBDIR
    }

    message("~~~ $${LPUB3D} ADD LDVIEW LIBRARIES PATH TO LIBS: $$LDVLIBDIR ~~~ ")
    LIBS        += -L$${LDVLIBDIR}

    message("~~~ $${LPUB3D} ADD LDVIEW 3RDPARTY LIBRARIES PATH TO LIBS: $$LDV3RDLIBDIR ~~~ ")
    LIBS        += -L$${LDV3RDLIBDIR}

#    isEmpty(LDVLIBRARY):LDVLIBRARY = $$absolute_path( $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR )
#    message("~~~ $${LPUB3D} Library path: $$LDVLIBRARY ~~~ ")

    # Set library names, source paths and local paths
    win32-msvc* {
        # library name
        LDLIB_LIB        = -lLDLib
        LDEXPORTER_LIB   = -lLDExporter
        LDLOADER_LIB     = -lLDLoader
        TRE_LIB          = -lTRE
        TCFOUNDATION_LIB = -lTCFoundation

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml_STL
        3DS_LIB          = -llib3ds-$${VSVER}
        PNG_LIB          = -llibpng16-$${VSVER}
        JPEG_LIB         = -llibjpeg-$${VSVER}
        ZLIB_LIB         = -lzlib-$${VSVER}

        # source paths
        LDLIB_SRC        = $${LDVLIBDIR}/LDLib.lib
        LDEXPORTER_SRC   = $${LDVLIBDIR}/LDExporter.lib
        LDLOADER_SRC     = $${LDVLIBDIR}/LDLoader.lib
        TRE_SRC          = $${LDVLIBDIR}/TRE.lib
        TCFOUNDATION_SRC = $${LDVLIBDIR}/TCFoundation.lib

        GL2PS_SRC        = $${LDVLIBDIR}/gl2ps.lib
        TINYXML_SRC      = $${LDVLIBDIR}/tinyxml_STL.lib
        3DS_SRC          = $${LDV3RDLIBDIR}/lib3ds-$${VSVER}.lib
        PNG_SRC          = $${LDV3RDLIBDIR}/libpng16-$${VSVER}.lib
        JPEG_SRC         = $${LDV3RDLIBDIR}/libjpeg-$${VSVER}.lib
        ZLIB_SRC         = $${LDV3RDLIBDIR}/zlib-$${VSVER}.lib
    } else {
        POSTFIX          = -osmesa
        # library name
        LDLIB_LIB        = -lLDLib$${POSTFIX}
        LDEXPORTER_LIB   = -lLDExporter$${POSTFIX}
        LDLOADER_LIB     = -lLDLoader$${POSTFIX}
        TRE_LIB          = -lTRE$${POSTFIX}
        TCFOUNDATION_LIB = -lTCFoundation$${POSTFIX}

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml
        3DS_LIB          = -l3ds
        PNG_LIB          = -lpng16
        JPEG_LIB         = -ljpeg

        # source paths
        LDLIB_SRC        = $${LDVLIBDIR}/libLDLib$${POSTFIX}.a
        LDEXPORTER_SRC   = $${LDVLIBDIR}/libLDExporter$${POSTFIX}.a
        LDLOADER_SRC     = $${LDVLIBDIR}/libLDLoader$${POSTFIX}.a
        TRE_SRC          = $${LDVLIBDIR}/libTRE$${POSTFIX}.a
        TCFOUNDATION_SRC = $${LDVLIBDIR}/libTCFoundation$${POSTFIX}.a

        GL2PS_SRC        = $${LDVLIBDIR}/libgl2ps.a
        TINYXML_SRC      = $${LDVLIBDIR}/libtinyxml.a
        3DS_SRC          = $${LDV3RDLIBDIR}/lib3ds.a
        msys: \
        PNG_SRC          = $${LDV3RDLIBDIR}/libpng.a
        else: \
        PNG_SRC          = $${LDV3RDLIBDIR}/libpng16.a
        JPEG_SRC         = $${LDV3RDLIBDIR}/libjpeg.a
    }

    # Set 'use local' flags
    !exists($${GL2PS_SRC}) {
        USE_LOCAL_GL2PS_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM GL2PS LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL GL2PS LIBRARY FOUND $${GL2PS_SRC} ~~~")

    !exists($${TINYXML_SRC}) {
        USE_LOCAL_TINYXML_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM TINYXML LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL TINYXML LIBRARY FOUND $${TINYXML_SRC} ~~~")

    !exists($${3DS_SRC}) {
        USE_LOCAL_3DS_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM 3DS LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL 3DS LIBRARY FOUND $${3DS_SRC} ~~~")

    !exists($${PNG_SRC}) {
        USE_LOCAL_PNG_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM PNG LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL PNG LIBRARY FOUND $${PNG_SRC} ~~~")

    !exists($${JPEG_SRC}) {
        USE_LOCAL_JPEG_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM JPEG LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL JPEG LIBRARY FOUND $${JPEG_SRC} ~~~")

    win32-msvc* {
        !exists($${ZLIB_SRC}) {
            USE_LOCAL_ZLIB_LIB = False
            message("~~~ $${LPUB3D} USING SYSTEM Z LIBRARY ~~~")
        } else:message("~~~ $${LPUB3D} LOCAL Z LIBRARY FOUND $${ZLIB_SRC} ~~~")
    }

    LIBS += \
        $${LDLIB_LIB} \
        $${LDEXPORTER_LIB} \
        $${LDLOADER_LIB} \
        $${TRE_LIB} \
        $${TCFOUNDATION_LIB}

    # 3rd party libraries should come after main libraries

    if (contains(USE_LOCAL_GL2PS_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/libgl2ps.dylib
        else:LIBS          += -lgl2ps
    } else {
        LIBS               += $$GL2PS_LIB
    }

    if (contains(USE_LOCAL_TINYXML_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/libtinyxml.dylib
        else:LIBS          += -ltinyxml
    } else {
        LIBS               += $$TINYXML_LIB
    }

    if (contains(USE_LOCAL_PNG_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/libpng.dylib
        else:LIBS          += -lpng
    } else {
        LIBS               += $$PNG_LIB
    }

    if (contains(USE_LOCAL_JPEG_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/libjpeg.dylib
        else:LIBS          += -ljpeg
    } else {
        LIBS               += $$JPEG_LIB
    }

    if (contains(USE_LOCAL_3DS_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/lib3ds.dylib
        else:LIBS          += -l3ds
    } else {
        LIBS               += $$3DS_LIB
    }

    if (!contains(USE_LOCAL_ZLIB_LIB,False)) {
        LIBS               += $$ZLIB_LIB
    }

    # message("~~~ $${LPUB3D} INFO - LDVQt LIBRARIES: $${LIBS} ~~~")

#~~ Merge ldv messages ini files and move to extras dir ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    #LDVMESSAGESINI defined in mainApp.pro
    if (mingw:ide_qtcreator)|win32-msvc* {
        COPY_CMD        = COPY /V /Y
        LDV_CONCAT_CMD  = TYPE
    } else {
        COPY_CMD        = cp -f
        LDV_CONCAT_CMD  = cat
    }
    LDV_RESOURCE_DIR    = $$shell_path( $$absolute_path( $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/resources ) )
    LDV_MESSAGES        = $$shell_path( $$absolute_path( $${LDV_RESOURCE_DIR}/LDViewMessages.ini ) )
    LDV_EXPORT_MESSAGES = $$shell_path( $$absolute_path( $${LDV_RESOURCE_DIR}/LDExportMessages.ini ) )
    LDV_WIDGET_MESSAGES = $$shell_path( $$absolute_path( $$PWD/LDVWidgetMessages.ini ) )
    LDV_CONCAT_MESSAGES = $$shell_path( $$absolute_path( $$_PRO_FILE_PWD_/extras/$$LDVMESSAGESINI ) )
    #message("~~~ DEBUG_$$upper($${TARGET}) LDV_CONCAT_MESSAGES: $$LDV_CONCAT_MESSAGES ~~~ ")
    #message("~~~ DEBUG_$$upper($${TARGET}) LDV_RESOURCE_DIR: $$LDV_RESOURCE_DIR ~~~ ")
    LDV_CONCAT_MESSAGES_CMD = \
    $$LDV_CONCAT_CMD $$LDV_MESSAGES $$LDV_EXPORT_MESSAGES $$LDV_WIDGET_MESSAGES > $$LDV_CONCAT_MESSAGES
    # When compiling from QtCreator, add ldvMessages.ini to destination directory extras folder - except for macOS
    contains(DEVL_LDV_MESSAGES_INI,True) {
        LDV_MESSAGES_DEVL = $$shell_path( $${OUT_PWD}/$${DESTDIR}/extras/$$LDVMESSAGESINI )
        message("~~~ $${LPUB3D} COPY LDVMESSAGES.INI TO: ./$${DESTDIR}/extras/$$LDVMESSAGESINI ~~~")
        LDV_CONCAT_MESSAGES_CMD += \
        $$escape_expand(\n\t) \
        $$COPY_CMD \
        $$LDV_CONCAT_MESSAGES $$LDV_MESSAGES_DEVL
    }
    #message("~~~ DEBUG_$$upper($${TARGET}) LDV_CONCAT_MESSAGES_CMD: $$LDV_CONCAT_MESSAGES_CMD ~~~ ")
    ldvmsg_concat_msg.target   = ConcatProjectMessage
    ldvmsg_concat_msg.commands = @echo Project MESSAGE: ~~~ $${LPUB3D} Creating $${TARGET} $${LDVMESSAGESINI}... ~~~
    ldvmsg_concat.target       = ConcatenateMessages
    ldvmsg_concat.commands     = $$LDV_CONCAT_MESSAGES_CMD
    ldvmsg_concat.depends      = $${LDV_MESSAGES} \
                                 $${LDV_EXPORT_MESSAGES} \
                                 $${LDV_WIDGET_MESSAGES} \
                                 ConcatProjectMessage

    QMAKE_EXTRA_TARGETS     += ldvmsg_concat ldvmsg_concat_msg
    PRE_TARGETDEPS          += $$LDV_CONCAT_MESSAGES
    QMAKE_CLEAN             += $$LDV_CONCAT_MESSAGES
} # LPub3D LOAD_LDV_LIBS,True
