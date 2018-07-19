# Discard STDERR so get path to NULL device
win32: NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
else:  NULL_DEVICE = /dev/null

# LDView source path components
win32 {
    COPY_CMD = COPY /y /a /b
    LDVBLD   = $$BUILD_CONF                   # Debug|Release
    LDVARCH  = $$LIB_ARCH                     # 64|null
} else:unix {
    COPY_CMD = cp -f
    LDVBLD   = $$DESTDIR                      # 64bit_debug|64bit_release|32bit_debug|32bit_release
    LDVARCH  =                                # null
}
LDVHDRDIR    = $$PWD/../../../ldview          # called from <base>/ldvlib/LDVQt
LDVLIBDIR    = $$_PRO_FILE_PWD_/../../ldview  # called from <base>/mainApp
LDVDESTDIR   = $$join(LDVBLD,,,$$LDVARCH)
THIRDLIBDIR  = $${LDVHDRDIR}/3rdParty
PREBUILTDIR  = $${LDVHDRDIR}/lib
contains(LIB_ARCH,64): PREBUILTDIR = $${LDVHDRDIR}/lib/x64

contains(LOAD_LDVHEADERS,True) {
    # LDVQt include path
    LDVINCLUDE  = $$PWD/include
    message("~~~ LDVQt Headers path: $$LDVINCLUDE ~~~ ")

    # Copy headers from LDView
    win32 {
        Make_hdr_dirs_cmd = cd include && \
                            if not exist LDExporter mkdir LDExporter && \
                            if not exist LDLib mkdir LDLib && \
                            if not exist LDLoader mkdir LDLoader && \
                            if not exist TRE mkdir TRE && \
                            if not exist TCFoundation mkdir TCFoundation && \
                            if not exist 3rdParty mkdir 3rdParty
    } else {
        Make_hdr_dirs_cmd = cd include && \
                            if ! test -e LDExporter; then mkdir LDExporter; fi && \
                            if ! test -e LDLib; then mkdir LDLib; fi && \
                            if ! test -e LDLoader; then mkdir LDLoader; fi && \
                            if ! test -e TRE; then mkdir TRE; fi && \
                            if ! test -e TCFoundation; then mkdir TCFoundation; fi && \
                            if ! test -e 3rdParty; then mkdir 3rdParty; fi
    }
    LDLib_hdr_cmd        = $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLib/*.h) $$system_path(include/LDLib/)
    LDExporter_hdr_cmd   = $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDExporter/*.h) $$system_path(include/LDExporter/)
    LDLoader_hdr_cmd     = $$COPY_CMD $$system_path( $${LDVHDRDIR}/LDLoader/*.h) $$system_path(include/LDLoader/)
    TRE_hdr_cmd          = $$COPY_CMD $$system_path( $${LDVHDRDIR}/TRE/*.h) $$system_path(include/TRE/)
    TCFoundation_hdr_cmd = $$COPY_CMD $$system_path( $${LDVHDRDIR}/TCFoundation/*.h) $$system_path(include/TCFoundation/)

    3rdParty_hdr_cmd     = $$COPY_CMD $$system_path( $${LDVHDRDIR}/include/*.h) $$system_path(include/3rdParty/)
    GL_hdr_cmd           = $$COPY_CMD $$system_path( $${LDVHDRDIR}/include/GL/*.h) $$system_path(include/GL/)

    # Make local directories
    system( $$Make_hdr_dirs_cmd > $$NULL_DEVICE )
    # Copy LDView headers
    system( $$LDExporter_hdr_cmd > $$NULL_DEVICE )
    system( $$LDLib_hdr_cmd > $$NULL_DEVICE )
    system( $$LDLoader_hdr_cmd > $$NULL_DEVICE )
    system( $$TRE_hdr_cmd > $$NULL_DEVICE )
    system( $$TCFoundation_hdr_cmd > $$NULL_DEVICE )
    # Copy 3rdParty headers
    system( $$3rdParty_hdr_cmd > $$NULL_DEVICE )
    system( $$GL_hdr_cmd > $$NULL_DEVICE )

    # LDView include path
    INCLUDEPATH += $${LDVINCLUDE}/
    DEPENDPATH  += $${LDVINCLUDE}/

    # 3rdParty include path
    INCLUDEPATH += $${LDVINCLUDE}/3rdParty/
    DEPENDPATH  += $${LDVINCLUDE}/3rdParty/
}

contains(LOAD_LDVLIBS,True) {

    # LDVQt library path - put everything in single lcation
    LDVLIBRARY = $$OUT_PWD/../ldvlib/LDVQt/$$DESTDIR/
    message("~~~ LDVQt Library path: $$LDVLIBRARY ~~~ ")

    # Set 'use local' flags
    if (unix:exists(/usr/include/tinyxml.h)|exists(/usr/local/include/tinyxml.h)) { USE_LOCAL_TINYXML_LIB = False }
    if (unix:exists(/usr/include/gl2ps.h)|exists(/usr/local/include/gl2ps.h)) { USE_LOCAL_GL2PS_LIB = False }
    if (unix:exists(/usr/include/zip.h)|exists(/usr/local/include/minizip/zip.h)) { USE_LOCAL_MINIZIP_LIB = False }
    if (unix:exists(/usr/include/png.h)|exists(/usr/local/include/png.h)) { USE_LOCAL_PNG_LIB = False }
    if (unix:exists(/usr/include/jpeglib.h)|exists(/usr/local/include/jpeglib.h)) { USE_LOCAL_JPEG_LIB = False }
    !win32-msvc*: \
    USE_LOCAL_ZLIB_LIB = False    # Use local zlib if win32-msvc*
    USE_LOCAL_3DS_LIB =           # Always use local lib3ds

    # Set source and local paths and library names
    win32-msvc* {
        # source paths
        CONFIG(debug, debug|release) {
            _LDLIB_DEP        = $${LDVLIBDIR}/Build/$${LDVDESTDIR}/LDLib.lib
            _LDEXPORTER_DEP   = $${LDVLIBDIR}/Build/$${LDVDESTDIR}/LDExporter.lib
            _LDLOADER_DEP     = $${LDVLIBDIR}/Build/$${LDVDESTDIR}/LDLoader.lib
            _TRE_DEP          = $${LDVLIBDIR}/Build/$${LDVDESTDIR}/TRE.lib
            _TCFOUNDATION_DEP = $${LDVLIBDIR}/Build/$${LDVDESTDIR}/TCFoundation.lib

            _GL2PS_DEP        = $${LDVLIBDIR}/Build/$${LDVDESTDIR}/gl2ps.lib
            _TINYXML_DEP      = $${LDVLIBDIR}/Build/$${LDVDESTDIR}/tinyxml_STL.lib
        } else:CONFIG(release, debug|release) {
            _LDLIB_DEP        = $${LDVLIBDIR}/LDLib/Build/$${LDVDESTDIR}/LDLib.lib
            _LDEXPORTER_DEP   = $${LDVLIBDIR}/LDExporter/Build/$${LDVDESTDIR}/LDExporter.lib
            _LDLOADER_DEP     = $${LDVLIBDIR}/LDLoader/Build/$${LDVDESTDIR}/LDLoader.lib
            _TRE_DEP          = $${LDVLIBDIR}/TRE/Build/$${LDVDESTDIR}/TRE.lib
            _TCFOUNDATION_DEP = $${LDVLIBDIR}/TCFoundation/Build/$${LDVDESTDIR}/TCFoundation.lib

            _GL2PS_DEP        = $${THIRDLIBDIR}/gl2ps/Build/$${LDVDESTDIR}/gl2ps.lib
            _TINYXML_DEP      = $${THIRDLIBDIR}/tinyxml/Build/$${LDVDESTDIR}/tinyxml_STL.lib
        }
        _3DS_DEP         = $${PREBUILTDIR}/lib3ds-vs2015.lib
        _ZIP_DEP         = $${PREBUILTDIR}/unzip32-vs2015.lib
        _PNG_DEP         = $${PREBUILTDIR}/libpng16-vs2015.lib
        _JPEG_DEP        = $${PREBUILTDIR}/libjpeg-vs2015.lib
        _ZLIB_DEP        = $${PREBUILTDIR}/zlib-vs2015.lib

        # library name
        LDLIB_LIB        = -lLDLib
        LDEXPORTER_LIB 	 = -lLDExporter
        LDLOADER_LIB     = -lLDLoader
        TRE_LIB          = -lTRE
        TCFOUNDATION_LIB = -lTCFoundation

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml_STL
        3DS_LIB          = -llib3ds-vs2015
        ZIP_LIB          = -lunzip32-vs2015
        PNG_LIB          = -llibpng16-vs2015
        JPEG_LIB         = -llibjpeg-vs2015
        ZLIB_LIB         = -lzlib-vs2015

        # local library paths
        LDLIB_DEP        = $${LDVLIBRARY}/LDLib.lib
        LDEXPORTER_DEP 	 = $${LDVLIBRARY}/LDExporter.lib
        LDLOADER_DEP     = $${LDVLIBRARY}/LDLoader.lib
        TRE_DEP          = $${LDVLIBRARY}/TRE.lib
        TCFOUNDATION_DEP = $${LDVLIBRARY}/TCFoundation.lib

        GL2PS_DEP        = $${LDVLIBRARY}/gl2ps.lib
        TINYXML_DEP      = $${LDVLIBRARY}/tinyxml_STL.lib
        3DS_DEP          = $${LDVLIBRARY}/lib3ds-vs2015.lib
        ZIP_DEP          = $${LDVLIBRARY}/unzip32-vs2015.lib
        PNG_DEP          = $${LDVLIBRARY}/libpng16-vs2015.lib
        JPEG_DEP         = $${LDVLIBRARY}/libjpeg-vs2015.lib
        ZLIB_DEP         = $${LDVLIBRARY}/zlib-vs2015.lib
    } else:unix {
        # source paths
        _LDLIB_DEP = $${LDVLIBDIR}/LDLib/$${LDVDESTDIR}/libLDraw-osmesa.a
        _LDEXPORTER_DEP = $${LDVLIBDIR}/LDExporter/$${LDVDESTDIR}/libLDExporter-osmesa.a
        _LDLOADER_DEP = $${LDVLIBDIR}/LDLoader/$${LDVDESTDIR}/libLDLoader-osmesa.a
        _TRE_DEP = $${LDVLIBDIR}/TRE/$${LDVDESTDIR}/libTRE-osmesa.a
        _TCFOUNDATION_DEP = $${LDVLIBDIR}/TCFoundation/$${LDVDESTDIR}/libTCFoundation-osmesa.a

        _GL2PS_DEP = $${THIRDLIBDIR}/gl2ps/$${LDVDESTDIR}/libgl2ps.a
        _TINYXML_DEP = $${THIRDLIBDIR}/tinyxml/$${LDVDESTDIR}/libtinyxml.a
        _3DS_DEP         = $${THIRDLIBDIR}/lib3ds/$${LDVDESTDIR}/lib3ds.a
        macx {
            _ZIP_DEP     = $${PREBUILTDIR}/MacOSX/libminizip.a
            _PNG_DEP     = $${PREBUILTDIR}/MacOSX/libpng.a
            _JPEG_DEP    = $${PREBUILTDIR}/MacOSX/libjpeg.a
        } else {
            _ZIP_DEP     =
            _PNG_DEP     = $${THIRDLIBDIR}/libpng/$${LDVDESTDIR}/libpng.a
            _JPEG_DEP    = $${THIRDLIBDIR}/libjpeg/$${LDVDESTDIR}/libjpeg.a
            IS_LINUX     = True
        }

        # library name
        LDLIB_LIB        = -lLDraw-osmesa
        LDEXPORTER_LIB 	 = -lLDExporter-osmesa
        LDLOADER_LIB     = -lLDLoader-osmesa
        TRE_LIB          = -lTRE-osmesa
        TCFOUNDATION_LIB = -lTCFoundation-osmesa

        GL2PS_LIB        = -lgl2ps
        TINYXML_LIB      = -ltinyxml
        3DS_LIB          = -l3ds
        macx:ZIP_LIB     = -lminizip
        else:ZIP_LIB     =
        PNG_LIB          = -llibpng
        JPEG_LIB         = -llibjpeg

        # local paths
        LDLIB_DEP        = $${LDVLIBRARY}/libLDraw-osmesa.a
        LDEXPORTER_DEP 	 = $${LDVLIBRARY}/libLDExporter-osmesa.a
        LDLOADER_DEP     = $${LDVLIBRARY}/libLDLoader-osmesa.a
        TRE_DEP          = $${LDVLIBRARY}/libTRE-osmesa.a
        TCFOUNDATION_DEP = $${LDVLIBRARY}/libTCFoundation-osmesa.a

        GL2PS_DEP        = $${LDVLIBRARY}/libgl2ps.a
        TINYXML_DEP      = $${LDVLIBRARY}/libtinyxml.a
        3DS_DEP          = $${LDVLIBRARY}/lib3ds.a
        macx:ZIP_LIB     = $${LDVLIBRARY}/libminizip.a
        else:ZIP_LIB     =
        PNG_DEP          = $${LDVLIBRARY}/libpng.a
        JPEG_DEP         = $${LDVLIBRARY}/libjpeg.a
    }

    # Copy libraries from LDView
    LDLIB_LIB_cmd             = $$COPY_CMD $$system_path( $${_LDLIB_DEP} ) $$system_path( $${LDVLIBRARY} )
    LDEXPORTER_LIB_cmd        = $$COPY_CMD $$system_path( $${_LDEXPORTER_DEP} ) $$system_path( $${LDVLIBRARY} )
    LDLOADER_LIB_cmd          = $$COPY_CMD $$system_path( $${_LDLOADER_DEP} ) $$system_path( $${LDVLIBRARY} )
    TRE_LIB_cmd               = $$COPY_CMD $$system_path( $${_TRE_DEP} ) $$system_path( $${LDVLIBRARY} )
    TCFOUNDATION_LIB_cmd      = $$COPY_CMD $$system_path( $${_TCFOUNDATION_DEP} ) $$system_path( $${LDVLIBRARY} )

    TINYXML_LIB_cmd           = $$COPY_CMD $$system_path( $${_TINYXML_DEP} ) $$system_path( $${LDVLIBRARY} )
    GL2PS_LIB_cmd             = $$COPY_CMD $$system_path( $${_GL2PS_DEP} ) $$system_path( $${LDVLIBRARY} )
    3DS_LIB_cmd               = $$COPY_CMD $$system_path( $${_3DS_DEP} ) $$system_path( $${LDVLIBRARY} )
    ZIP_LIB_cmd               = $$COPY_CMD $$system_path( $${_ZIP_DEP} ) $$system_path( $${LDVLIBRARY} )
    PNG_LIB_cmd               = $$COPY_CMD $$system_path( $${_PNG_DEP} ) $$system_path( $${LDVLIBRARY} )
    JPEG_LIB_cmd              = $$COPY_CMD $$system_path( $${_JPEG_DEP} ) $$system_path( $${LDVLIBRARY} )
    ZLIB_LIB_cmd              = $$COPY_CMD $$system_path( $${_ZLIB_DEP} ) $$system_path( $${LDVLIBRARY} )

    LDLib_lib.target          = $$LDLIB_DEP
    LDLib_lib.commands        = $$LDLIB_LIB_cmd
    LDLib_lib.depends         = LDLib_lib_msg LDExporter_lib
    LDLib_lib_msg.commands    = @echo Copying LDLib library...

    LDExporter_lib.target     = $$LDEXPORTER_DEP
    LDExporter_lib.commands   = $$LDEXPORTER_LIB_cmd
    LDExporter_lib.depends    = LDExporter_lib_msg LDLoader_lib
    LDExporter_lib_msg.commands = @echo Copying LDExporter library...

    LDLoader_lib.target       = $$LDLOADER_DEP
    LDLoader_lib.commands     = $$LDLOADER_LIB_cmd
    LDLoader_lib.depends      = LDLoader_lib_msg TRE_lib
    LDLoader_lib_msg.commands = @echo Copying LDLoader library...

    TRE_lib.target            = $$TRE_DEP
    TRE_lib.commands          = $$TRE_LIB_cmd
    TRE_lib.depends           = TRE_lib_msg TCFoundation_lib
    TRE_lib_msg.commands      = @echo Copying TRE library...

    TCFoundation_lib.target   = $$TCFOUNDATION_DEP
    TCFoundation_lib.commands = $$TCFOUNDATION_LIB_cmd
    TCFoundation_lib.depends  = TCFoundation_lib_msg
    TCFoundation_lib_msg.commands = @echo Copying TCFoundation library...

    QMAKE_EXTRA_TARGETS += \
        LDLib_lib LDLib_lib_msg \
        LDExporter_lib LDExporter_lib_msg \
        LDLoader_lib LDLoader_lib_msg \
        TRE_lib TRE_lib_msg \
        TCFoundation_lib TCFoundation_lib_msg

    if (!contains(USE_LOCAL_TINYXML_LIB,False)) {
      Tinyxml_lib.target       = $$TINYXML_DEP
      Tinyxml_lib.commands     = $$TINYXML_LIB_cmd
      Tinyxml_lib.depends      = Tinyxml_lib_msg
      Tinyxml_lib_msg.commands = @echo Copying Tinyxml library...
      QMAKE_EXTRA_TARGETS     += Tinyxml_lib Tinyxml_lib_msg
    }

    if (!contains(USE_LOCAL_GL2PS_LIB,False)) {
      Gl2ps_lib.target       = $$GL2PS_DEP
      Gl2ps_lib.commands     = $$GL2PS_LIB_cmd
      Gl2ps_lib.depends      = Gl2ps_lib_msg
      Gl2ps_lib_msg.commands = @echo Copying Gl2ps library...
      QMAKE_EXTRA_TARGETS   += Gl2ps_lib Gl2ps_lib_msg
    }

    if (!contains(USE_LOCAL_3DS_LIB,False)) {
      3ds_lib.target       = $$3DS_DEP
      3ds_lib.commands     = $$3DS_LIB_cmd
      3ds_lib.depends      = 3ds_lib_msg
      3ds_lib_msg.commands = @echo Copying 3ds library...
      QMAKE_EXTRA_TARGETS += 3ds_lib 3ds_lib_msg
    }

    if (!contains(USE_LOCAL_MINIZIP_LIB,False):!contains(IS_LINUX,True)) {
      Minizip_lib.target       = $$ZIP_DEP
      Minizip_lib.commands     = $$ZIP_LIB_cmd
      Minizip_lib.depends      = Minizip_lib_msg
      Minizip_lib_msg.commands = @echo Copying Minizip library...
      QMAKE_EXTRA_TARGETS     += Minizip_lib Minizip_lib_msg
    }

    if (!contains(USE_LOCAL_PNG_LIB,False)) {
      Png_lib.target       = $$PNG_DEP
      Png_lib.commands     = $$PNG_LIB_cmd
      Png_lib.depends      = Png_lib_msg
      Png_lib_msg.commands = @echo Copying Png library...
      QMAKE_EXTRA_TARGETS += Png_lib Png_lib_msg
    }

    if (!contains(USE_LOCAL_JPEG_LIB,False)) {
      Jpeg_lib.target       = $$JPEG_DEP
      Jpeg_lib.commands     = $$JPEG_LIB_cmd
      Jpeg_lib.depends      = Jpeg_lib_msg
      Jpeg_lib_msg.commands = @echo Copying Jpeg library...
      QMAKE_EXTRA_TARGETS  += Jpeg_lib Jpeg_lib_msg
    }

    if (!contains(USE_LOCAL_ZLIB_LIB,False)) {
      Zlib_lib.target       = $$ZLIB_DEP
      Zlib_lib.commands     = $$ZLIB_LIB_cmd
      Zlib_lib.depends      = Zlib_lib_msg
      Zlib_lib_msg.commands = @echo Copying Zlib library...
      QMAKE_EXTRA_TARGETS  += Zlib_lib Zlib_lib_msg
    }

    LIBS += \
        $${LDLIB_LIB} \
        $${LDEXPORTER_LIB} \
        $${LDLOADER_LIB} \
        $${TRE_LIB} \
        $${TCFOUNDATION_LIB}

    PRE_TARGETDEPS += \
        $${LDLIB_DEP} \
        $${LDEXPORTER_DEP} \
        $${LDLOADER_DEP} \
        $${TRE_DEP} \
        $${TCFOUNDATION_DEP}

    QMAKE_CLEAN += \
        $${LDLIB_DEP} \
        $${LDEXPORTER_DEP} \
        $${LDLOADER_DEP} \
        $${TRE_DEP} \
        $${TCFOUNDATION_DEP}

    # 3rd party libraries should come after main libraries

    if (contains(USE_LOCAL_GL2PS_LIB,False)) {
        macx:LIBS += /usr/local/lib/libgl2ps.dylib
        else:LIBS += -lgl2ps
    } else {
        LIBS           += $$GL2PS_LIB
        PRE_TARGETDEPS += $$GL2PS_DEP
        QMAKE_CLEAN    += $$GL2PS_DEP
    }

    if (contains(USE_LOCAL_TINYXML_LIB,False)) {
        macx:LIBS += /usr/local/lib/libtinyxml.dylib
        else:LIBS += -ltinyxml
    } else {
        LIBS           += $$TINYXML_LIB
        PRE_TARGETDEPS += $$TINYXML_DEP
        QMAKE_CLEAN    += $$TINYXML_DEP
    }

    if (contains(USE_LOCAL_PNG_LIB,False)) {
        if (contains(HOST, Ubuntu):contains(HOST, 14.04.5)) {
            LIBS           += $$PNG_LIB
            PRE_TARGETDEPS += $$PNG_DEP
            QMAKE_CLEAN    += $$PNG_DEP
        } else {
            macx:LIBS += /usr/local/lib/libpng.dylib
            else:LIBS += -lpng
        }
    } else {
        LIBS           += $$PNG_LIB
        PRE_TARGETDEPS += $$PNG_DEP
        QMAKE_CLEAN    += $$PNG_DEP
    }

    if (contains(USE_LOCAL_JPEG_LIB,False)) {
        macx:LIBS += /usr/local/lib/libjpeg.dylib
        else:LIBS += -ljpeg
    } else {
        LIBS           += $$JPEG_LIB
        PRE_TARGETDEPS += $$JPEG_DEP
        QMAKE_CLEAN    += $$JPEG_DEP
    }

    if (contains(USE_LOCAL_MINIZIP_LIB,False)) {
        macx:LIBS += /usr/local/lib/libminizip.dylib
        else:LIBS += -lminizip
    } else {
        LIBS           += $$ZIP_LIB
        PRE_TARGETDEPS += $$ZIP_DEP
        QMAKE_CLEAN    += $$ZIP_DEP
    }

    if (contains(USE_LOCAL_3DS_LIB,False)) {
        macx:LIBS += /usr/local/lib/lib3ds.dylib
        else:LIBS += -l3ds
    } else {
        LIBS           += $$3DS_LIB
        PRE_TARGETDEPS += $$3DS_DEP
        QMAKE_CLEAN    += $$3DS_DEP
    }

    if (!contains(USE_LOCAL_ZLIB_LIB,False)) {
        LIBS           += $$ZLIB_LIB
        PRE_TARGETDEPS += $$ZLIB_DEP
        QMAKE_CLEAN    += $$ZLIB_DEP
    }

#~~ Merge ldv messages ini files and move to extras dir ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    LDVMESSAGESINI     = LDVMessages.ini
    LDVMESSAGESINI_DEP = $$_PRO_FILE_PWD_/extras/$$LDVMESSAGESINI

    win32 {
        PLUS_CMD      = +
        REDIRECT_CMD  =
    } else:unix {
        COPY_CMD      = cat
        PLUS_CMD      =
        REDIRECT_CMD  = >
    }

    ldvmsg_copy_cmd = \
    $$COPY_CMD \
    $$system_path( $${LDVLIBDIR}/LDViewMessages.ini ) $$PLUS_CMD \
    $$system_path( $${LDVLIBDIR}/LDExporter/LDExportMessages.ini ) $$REDIRECT_CMD \
    $$system_path( $$LDVMESSAGESINI_DEP )

    ldvmsg_copy.target         = $$LDVMESSAGESINI_DEP
    ldvmsg_copy.commands       = $$ldvmsg_copy_cmd
    ldvmsg_copy.depends        = $${LDVLIBDIR}/LDViewMessages.ini \
                                 $${LDVLIBDIR}/LDExporter/LDExportMessages.ini \
                                 ldvmsg_copy_msg
    ldvmsg_copy_msg.commands   = @echo Copying $${LDVMESSAGESINI}...

    QMAKE_EXTRA_TARGETS += ldvmsg_copy ldvmsg_copy_msg
    PRE_TARGETDEPS      += $$LDVMESSAGESINI_DEP
    QMAKE_CLEAN         += $$LDVMESSAGESINI_DEP
}
