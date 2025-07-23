# ==LDVQt LOAD_LDV_HEADERS=====================

contains(LOAD_LDV_HEADERS,True) {
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    BUILD_LDV_LIBS {
        VER_LDVIEW_DIR_PATH    = LDView
        VER_LDVIEW_INCLUDE     = $${VER_LDVIEW_DIR_PATH}
        VER_LDVIEW_THIRD_PARTY = $${VER_LDVIEW_DIR_PATH}/3rdParty
    } else {
        VER_LDVIEW_INCLUDE     = $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/include
        VER_LDVIEW_THIRD_PARTY = $${VER_LDVIEW_INCLUDE}/3rdParty
    }
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Header source path
    equals(VER_USE_LDVIEW_DEV,True) {
        isEmpty(LDVHDRDIR):LDVHDRDIR       = $${VER_LDVIEW_DEV_REPOSITORY}/include
        isEmpty(LDV3RDHDRDIR):LDV3RDHDRDIR = $$LDVHDRDIR
        isEmpty(LDV3RDHDR):LDV3RDHDR       = $${VER_LDVIEW_DEV_REPOSITORY}/3rdParty
    } else {
        isEmpty(LDVHDRDIR):LDVHDRDIR       = $${VER_LDVIEW_INCLUDE}
        isEmpty(LDV3RDHDRDIR):LDV3RDHDRDIR = $${VER_LDVIEW_THIRD_PARTY}
        isEmpty(LDV3RDHDR):LDV3RDHDR       = $${LDV3RDHDRDIR}
    }

    USE_LDV_SYSTEM_LIBS:win32-arm64-msvc|win32-msvc*: \
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
    DEPENDPATH  += $${LDVHDRDIR}
    INCLUDEPATH += $${LDVHDRDIR}
    INCLUDEPATH += $${LDV3RDHDRDIR}
    BUILD_LDV_LIBS {
        LDVHDRDIR    = $${VER_LDVIEW_DIR_PATH}/include
        INCLUDEPATH += $${LDVHDRDIR}
    }

    message("~~~ lib$${TARGET} ADD LDVIEW 3RDPARTY HEADERS TO INCLUDEPATH: $$LDVHDRDIR ~~~ ")

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

    if (unix|msys:exists(/usr/include/gl2ps.h)|exists($${SYSTEM_PREFIX_}/include/gl2ps.h)):!BUILD_GL2PS {
        message("~~~ lib$${TARGET} system library header for gl2ps FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/gl2ps.h)|exists($${LDVHDRDIR}/gl2ps.h) {
        message("~~~ lib$${TARGET} local library header for gl2ps FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for gl2ps NOT FOUND, using local ~~~")
    }
    if (unix|msys:exists(/usr/include/minizip/unzip.h)|exists($${SYSTEM_PREFIX_}/include/minizip/unzip.h)):!BUILD_MINIZIP {
        message("~~~ lib$${TARGET} system library header for minizip FOUND ~~~")
        LDVMINIZIPFOUND=1
    } else:exists($${LDV3RDHDR}/minizip/unzip.h)|exists($${LDVHDRDIR}/minizip/unzip.h) {
        message("~~~ lib$${TARGET} local library header for minizip FOUND ~~~")
        LDVMINIZIPFOUND=1
    } else {
        LDVMINIZIPFOUND=0
        message("~~~ ERROR lib$${TARGET}: library header for minizip NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/tinyxml.h)|exists($${SYSTEM_PREFIX_}/include/tinyxml.h)):!BUILD_TINYXML {
        message("~~~ lib$${TARGET} system library header for tinyxml FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/tinyxml.h)|exists($${LDVHDRDIR}/tinyxml.h) {
        message("~~~ lib$${TARGET} local library header for tinyxml FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for tinyxml NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/lib3ds.h)|exists($${SYSTEM_PREFIX_}/include/lib3ds.h)):!BUILD_3DS {
        message("~~~ lib$${TARGET} system library header for 3ds FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/lib3ds.h)|exists($${LDVHDRDIR}/lib3ds.h) {
        message("~~~ lib$${TARGET} local library header for 3ds FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for 3ds NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/jpeglib.h)|exists($${SYSTEM_PREFIX_}/include/jpeglib.h)):!BUILD_JPEG {
        message("~~~ lib$${TARGET} system library header for jpeg FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/jpeglib.h)|exists($${LDVHDRDIR}/jpeglib.h) {
        message("~~~ lib$${TARGET} local library header for jpeg FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for jpeg NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/png.h)|exists($${SYSTEM_PREFIX_}/include/png.h)):!BUILD_PNG {
        message("~~~ lib$${TARGET} system library header for png FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/png.h)|exists($${LDVHDRDIR}/png.h) {
        message("~~~ lib$${TARGET} local library header for png FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for png NOT FOUND ~~~")
    }
    if (unix|msys:exists(/usr/include/zlib.h)|exists($${SYSTEM_PREFIX_}/include/zlib.h)):!BUILD_ZLIB {
        message("~~~ lib$${TARGET} system library header for libz FOUND ~~~")
    } else:exists($${LDV3RDHDRDIR}/zlib.h)|exists($${LDVHDRDIR}/zlib.h) {
        message("~~~ lib$${TARGET} local library header for libz FOUND ~~~")
    } else:win32-arm64-msvc|win32-msvc*:exists($$[QT_INSTALL_HEADERS]/QtZlib) {
        message("~~~ lib$${TARGET} Qt library header for libz FOUND ~~~")
    } else {
        message("~~~ ERROR lib$${TARGET}: library header for libz NOT FOUND ~~~")
    }
    contains(LDVMINIZIPFOUND,1) {
        DEFINES += HAVE_MINIZIP
        INCLUDEPATH += $${LDV3RDHDR}
    }
}
# END LOAD_LDV_HEADERS

# ==LPub3D LOAD_LDV_LIBS=====================

# This block is executed by LPub3D mainApp to enable linking the LDVlib
contains(LOAD_LDV_LIBS,True) {
    if (win32-arm64-msvc|win32-msvc*):CONFIG(debug, debug|release):!BUILD_LDV_LIBS {
        equals(VER_USE_LDVIEW_DEV,True) {
            LDVLIBDIR    = $${VER_LDVIEW_DEV_REPOSITORY}/Build/Debug$$LIB_ARCH
            LDV3RDLIBDIR = $${VER_LDVIEW_3RD_LIBS}
        } else {
            LDVLIBDIR    = $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/Build/Debug$$LIB_ARCH
            LDV3RDLIBDIR = $$LDVLIBDIR
        }
    } else {
        BUILD_LDV_LIBS {
            LDVLIBDIR    = $$absolute_path( $${OUT_PWD}/../ldvlib/LDVQt/LDView )
            LDV3RDLIBDIR = $${LDVLIBDIR}/3rdParty
            LDV3RDLIBS   = $${VER_LDVIEW_3RD_LIBS}
        } else {
            LDVLIBDIR    = $${THIRD_PARTY_DIST_DIR_PATH}/$$VER_LDVIEW/lib/$$QT_ARCH
            LDV3RDLIBDIR = $$LDVLIBDIR
        }
    }
    BUILD_LDV_LIBS {
        LIBS            += -L$${LDV3RDLIBS}
    } else {
        LIBS            += -L$${LDVLIBDIR}
        if (win32-arm64-msvc|win32-msvc*):CONFIG(debug, debug|release): \
        LIBS            += -L$${LDV3RDLIBDIR}
    }
    message("~~~ $${LPUB3D} ADD LDVIEW LIBRARIES PATH TO LIBS: $$LDVLIBDIR ~~~ ")
    message("~~~ $${LPUB3D} ADD LDVIEW 3RDPARTY LIBRARIES PATH TO LIBS: $$LDV3RDLIBDIR ~~~ ")

#isEmpty(LDVLIBRARY):LDVLIBRARY = $$absolute_path( $$OUT_PWD/../ldvlib/LDVQt/$${DESTDIR} )
#message("~~~ $${LPUB3D} Library path: $$LDVLIBRARY ~~~ ")

    # Set library names, source paths and local paths
    win32-arm64-msvc|win32-msvc* {
        # library name
        LDLIB_LIB        = -lLDLib
        LDEXPORTER_LIB   = -lLDExporter
        LDLOADER_LIB     = -lLDLoader
        TRE_LIB          = -lTRE
        TCFOUNDATION_LIB = -lTCFoundation

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml_STL
        BUILD_3DS: \
        3DS_LIB          = -llib3ds
        else: \
        3DS_LIB          = -llib3ds-$${VSVER}
        BUILD_PNG: \
        PNG_LIB          = -llibpng16
        else: \
        PNG_LIB          = -llibpng16-$${VSVER}
        BUILD_JPEG: \
        JPEG_LIB         = -llibjpeg
        else: \
        JPEG_LIB         = -llibjpeg-$${VSVER}
        BUILD_ZLIB: \
        ZLIB_LIB         = -lz
        else: \
        ZLIB_LIB         = -lzlib-$${VSVER}

        # source paths
        BUILD_LDV_LIBS {
            LDLIB_SRC        = $${LDVLIBDIR}/LDLib/$${DESTDIR}/LDLib$${POSTFIX}.lib
            LDEXPORTER_SRC   = $${LDVLIBDIR}/LDExporter/$${DESTDIR}/LDExporter$${POSTFIX}.lib
            LDLOADER_SRC     = $${LDVLIBDIR}/LDLoader/$${DESTDIR}/LDLoader$${POSTFIX}.lib
            TRE_SRC          = $${LDVLIBDIR}/TRE/$${DESTDIR}/TRE$${POSTFIX}.lib
            TCFOUNDATION_SRC = $${LDVLIBDIR}/TCFoundation/$${DESTDIR}/TCFoundation$${POSTFIX}.lib

            GL2PS_SRC        = $${LDV3RDLIBDIR}/gl2ps/$${DESTDIR}/gl2ps.lib
            TINYXML_SRC      = $${LDV3RDLIBDIR}/tinyxml/$${DESTDIR}/tinyxml_STL.lib
            BUILD_3DS: \
            3DS_SRC          = $${LDV3RDLIBDIR}/lib3ds/$${DESTDIR}/lib3ds.lib
            else: \
            3DS_SRC          = $${LDV3RDLIBS}/lib3ds-$${VSVER}.lib
            BUILD_PNG: \
            PNG_SRC          = $${LDV3RDLIBDIR}/libpng/$${DESTDIR}/libpng16.lib
            else: \
            PNG_SRC          = $${LDV3RDLIBS}/libpng16-$${VSVER}.lib
            BUILD_JPEG: \
            JPEG_SRC         = $${LDV3RDLIBDIR}/libjpeg/$${DESTDIR}/libjpeg.lib
            else: \
            JPEG_SRC         = $${LDV3RDLIBS}/libjpeg-$${VSVER}.lib
            BUILD_ZLIB: \
            ZLIB_SRC         = $${LDV3RDLIBDIR}/zlib/$${DESTDIR}/libz.lib
            else: \
            ZLIB_SRC         = $${LDV3RDLIBS}/zlib-$${VSVER}.lib
        } else {
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
        }
    } else {
        # library name
        LDLIB_LIB        = -lLDLib$${POSTFIX}
        LDEXPORTER_LIB   = -lLDExporter$${POSTFIX}
        LDLOADER_LIB     = -lLDLoader$${POSTFIX}
        TRE_LIB          = -lTRE$${POSTFIX}
        TCFOUNDATION_LIB = -lTCFoundation$${POSTFIX}

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml
        3DS_LIB          = -l3ds
        msys:!BUILD_PNG: \
        PNG_LIB          = -lpng
        else: \
        PNG_LIB          = -lpng16
        JPEG_LIB         = -ljpeg
        ZLIB_LIB         = -lz

        # source paths
        BUILD_LDV_LIBS {
            LDLIB_SRC        = $${LDVLIBDIR}/LDLib/$${DESTDIR}/libLDLib$${POSTFIX}.a
            LDEXPORTER_SRC   = $${LDVLIBDIR}/LDExporter/$${DESTDIR}/libLDExporter$${POSTFIX}.a
            LDLOADER_SRC     = $${LDVLIBDIR}/LDLoader/$${DESTDIR}/libLDLoader$${POSTFIX}.a
            TRE_SRC          = $${LDVLIBDIR}/TRE/$${DESTDIR}/libTRE$${POSTFIX}.a
            TCFOUNDATION_SRC = $${LDVLIBDIR}/TCFoundation/$${DESTDIR}/libTCFoundation$${POSTFIX}.a
            
            GL2PS_SRC        = $${LDV3RDLIBDIR}/gl2ps/$${DESTDIR}/libgl2ps.a
            TINYXML_SRC      = $${LDV3RDLIBDIR}/tinyxml/$${DESTDIR}/libtinyxml.a
            3DS_SRC          = $${LDV3RDLIBDIR}/lib3ds/$${DESTDIR}/lib3ds.a
            PNG_SRC          = $${LDV3RDLIBDIR}/libpng/$${DESTDIR}/libpng16.a
            JPEG_SRC         = $${LDV3RDLIBDIR}/libjpeg/$${DESTDIR}/libjpeg.a
            ZLIB_SRC         = $${LDV3RDLIBDIR}/zlib/$${DESTDIR}/libz.a
        } else {
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
            ZLIB_SRC         = NA
        }
    }

    # Set 'use local' flags
    BUILD_GL2PS {
        message("~~~ $${LPUB3D} WILL BUILD GL2PS LIBRARY ~~~")
    } else:!exists($${GL2PS_SRC}) {
        USE_LOCAL_GL2PS_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM GL2PS LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL GL2PS LIBRARY FOUND $${GL2PS_SRC} ~~~")

    BUILD_TINYXML {
        message("~~~ $${LPUB3D} WILL BUILD TINYXML LIBRARY ~~~")
    } else:!exists($${TINYXML_SRC}) {
        USE_LOCAL_TINYXML_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM TINYXML LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL TINYXML LIBRARY FOUND $${TINYXML_SRC} ~~~")

    BUILD_3DS {
        message("~~~ $${LPUB3D} WILL BUILD 3DS LIBRARY ~~~")
    } else:!exists($${3DS_SRC}) {
        USE_LOCAL_3DS_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM 3DS LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL 3DS LIBRARY FOUND $${3DS_SRC} ~~~")

    BUILD_PNG {
        message("~~~ $${LPUB3D} WILL BUILD PNG LIBRARY ~~~")
    } else:!exists($${PNG_SRC}) {
        USE_LOCAL_PNG_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM PNG LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL PNG LIBRARY FOUND $${PNG_SRC} ~~~")

    BUILD_JPEG {
        message("~~~ $${LPUB3D} WILL BUILD JPEG LIBRARY ~~~")
    } else:!exists($${JPEG_SRC}) {
        USE_LOCAL_JPEG_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM JPEG LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL JPEG LIBRARY FOUND $${JPEG_SRC} ~~~")

    BUILD_ZLIB {
        message("~~~ $${LPUB3D} WILL BUILD Z LIBRARY ~~~")
    } else:!exists($${ZLIB_SRC}) {
        USE_LOCAL_ZLIB_LIB = False
        message("~~~ $${LPUB3D} USING SYSTEM Z LIBRARY ~~~")
    } else:message("~~~ $${LPUB3D} LOCAL Z LIBRARY FOUND $${ZLIB_SRC} ~~~")

    BUILD_LDV_LIBS: \
    LIBS += \
        $${LDLIB_SRC} \
        $${LDEXPORTER_SRC} \
        $${LDLOADER_SRC} \
        $${TRE_SRC} \
        $${TCFOUNDATION_SRC}
    else: \
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
    } else:BUILD_GL2PS {
        LIBS               += $$GL2PS_SRC
    } else {
        LIBS               += $$GL2PS_LIB
    }

    if (contains(USE_LOCAL_TINYXML_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/libtinyxml.dylib
        else:LIBS          += -ltinyxml
    } else:BUILD_TINYXML {
        LIBS               += $$TINYXML_SRC
    } else {
        LIBS               += $$TINYXML_LIB
    }

    if (contains(USE_LOCAL_PNG_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/libpng.dylib
        else:LIBS          += -lpng
    } else:BUILD_PNG {
        LIBS               += $$PNG_SRC
    } else {
        LIBS               += $$PNG_LIB
    }

    if (contains(USE_LOCAL_JPEG_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/libjpeg.dylib
        else:LIBS          += -ljpeg
    } else:BUILD_JPEG {
        LIBS               += $$JPEG_SRC
    } else {
        LIBS               += $$JPEG_LIB
    }

    if (contains(USE_LOCAL_3DS_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/lib3ds.dylib
        else:LIBS          += -l3ds
    } else:BUILD_3DS {
        LIBS               += $$3DS_SRC
    } else {
        LIBS               += $$3DS_LIB
    }

    if (contains(USE_LOCAL_3DS_LIB,False)) {
        macx:LIBS          += $${SYSTEM_PREFIX_}/lib/libz.dylib
        else:LIBS          += -lz
    } else:BUILD_ZLIB {
        LIBS               += $$ZLIB_SRC
    } else {
        LIBS               += $$ZLIB_LIB
    }

    # message("~~~ $${LPUB3D} INFO - LDVQt LIBRARIES: $${LIBS} ~~~")

#~~ Merge ldv messages ini files and move to extras dir ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    if (mingw:ide_qtcreator)|win32-arm64-msvc|win32-msvc* {
        LDV_COPY_CMD    = COPY /V /Y
        LDV_CONCAT_CMD  = TYPE
    } else {
        LDV_COPY_CMD    = cp -f
        LDV_CONCAT_CMD  = cat
    }
    # LDV_LDVIEW_RESOURCE_DIR, LDV_EXPORT_RESOURCE_DIR, LDV_LDVQT_DIR and LDV_MESSAGES_INI defined in mainApp.pro
    LDV_LDVIEW_MESSAGES = $$shell_path( $$absolute_path( $${LDV_LDVIEW_RESOURCE_DIR}/LDViewMessages.ini ) )
    LDV_EXPORT_MESSAGES = $$shell_path( $$absolute_path( $${LDV_EXPORT_RESOURCE_DIR}/LDExportMessages.ini ) )
    LDV_WIDGET_MESSAGES = $$shell_path( $$absolute_path( $${LDV_LDVQT_DIR}/LDVWidgetMessages.ini ) )
    LDV_CONCAT_MESSAGES = $$shell_path( $$absolute_path( $$_PRO_FILE_PWD_/extras/$$LDV_MESSAGES_INI ) )
    LDV_CONCAT_MESSAGES_CMD = \
    $$LDV_CONCAT_CMD $$LDV_LDVIEW_MESSAGES $$LDV_EXPORT_MESSAGES $$LDV_WIDGET_MESSAGES > $$LDV_CONCAT_MESSAGES
    # When compiling from QtCreator, add ldvMessages.ini to destination directory extras folder - except for macOS
    contains(DEVL_LDV_MESSAGES_INI,True) {
        LDV_MESSAGES_DEVL = $$shell_path( $${OUT_PWD}/$${DESTDIR}/extras/$$LDV_MESSAGES_INI )
        message("~~~ $${LPUB3D} COPY LDV_MESSAGES_INI TO: ./$${DESTDIR}/extras/$$LDV_MESSAGES_INI ~~~")
        LDV_CONCAT_MESSAGES_CMD += \
        $$escape_expand(\n\t) \
        $$LDV_COPY_CMD \
        $$LDV_CONCAT_MESSAGES $$LDV_MESSAGES_DEVL
    }
    #message("~~~ DEBUG_$$upper($${TARGET}) LDV_CONCAT_MESSAGES_CMD: $$LDV_CONCAT_MESSAGES_CMD ~~~ ")
    ldvmsg_concat_msg.target   = ConcatProjectMessage
    ldvmsg_concat_msg.commands = @echo Project MESSAGE: ~~~ $${LPUB3D} Creating $${TARGET} $${LDV_MESSAGES_INI}... ~~~
    ldvmsg_concat.target       = ConcatenateMessages
    ldvmsg_concat.commands     = $$LDV_CONCAT_MESSAGES_CMD
    ldvmsg_concat.depends      = $${LDV_LDVIEW_MESSAGES} \
                                 $${LDV_EXPORT_MESSAGES} \
                                 $${LDV_WIDGET_MESSAGES} \
                                 ConcatProjectMessage

    QMAKE_EXTRA_TARGETS       += ldvmsg_concat ldvmsg_concat_msg
    PRE_TARGETDEPS            += ConcatenateMessages
    QMAKE_CLEAN               += $$LDV_CONCAT_MESSAGES
}
# END LPub3D LOAD_LDV_LIBS
