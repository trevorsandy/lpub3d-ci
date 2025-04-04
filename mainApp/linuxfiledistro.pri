    # These settings are used for the Linux package distribution
unix:!macx {
    DIST_TARGET = $$lower($$DIST_TARGET)

    binarybuild {
        # To build a binary distribution that will not require elevated rights to install,
        # pass CONFIG+=binarybuild to qmake (i.e. in QtCreator, set in qmake Additional Arguments)
        # The installer which uses Qt installer framework can be found in the "builds" source folder.
        # This macro is used to properly load parameter files on initial launch
        DEFINES += X11_BINARY_BUILD
        # Linker flag setting to properly direct LPub3D to ldrawini and quazip shared libraries.
        # This setting assumes dependent libraries are deposited at <exe location>/lib by the installer.
        QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/lib\'"
    }

    # These defines point LPub3D to the platform-specific content
    # when performing 'check for update' downloads
    # Don't forget to set CONFIG+=<deb|rpm|pkg|exe|api|snp|flp|dmg> accordingly

    # <BUILD_CODE>-<PLATFORM_CODE>-<HOST_VERSION>-<TARGET_CPU>
    !isEmpty(option): BUILD_CODE = $$option
    if (api|snp|flp|msys) {
        THIRD_PARTY_EXEC_DIR = $$(LP3D_3RD_EXE_DIR)
        _PLATFORM_CODE = ap
        COPY_CMD = cp -f
        EXTRAS_PATH = $$system_path( $${_PRO_FILE_PWD_}/extras)
        system( $$COPY_CMD $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/complete.zip) $${EXTRAS_PATH}/ )
        ! exists( $${EXTRAS_PATH}/complete.zip ): message( "~~~ $${LPUB3D} ERROR: - $${EXTRAS_PATH}/complete.zip not copied ~~~" )
        system( $$COPY_CMD $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/lpub3dldrawunf.zip) $${EXTRAS_PATH}/ )
        ! exists( $${EXTRAS_PATH}/lpub3dldrawunf.zip ): message( "~~~ $${LPUB3D} ERROR: - $${EXTRAS_PATH}/lpub3dldrawunf.zip not copied ~~~" )
        system( $$COPY_CMD $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/tenteparts.zip) $${EXTRAS_PATH}/ )
        ! exists( $${EXTRAS_PATH}/tenteparts.zip ): message( "~~~ $${LPUB3D} ERROR: - $${EXTRAS_PATH}/tenteparts.zip not copied ~~~" )
        system( $$COPY_CMD $$system_path( $${THIRD_PARTY_DIST_DIR_PATH}/vexiqparts.zip) $${EXTRAS_PATH}/ )
        ! exists( $${EXTRAS_PATH}/vexiqparts.zip ): message( "~~~ $${LPUB3D} ERROR: - $${EXTRAS_PATH}/vexiqparts.zip not copied ~~~" )
    }

    _PLATFORM_ID = $$system(. /etc/os-release 2>/dev/null; [ -n \"$ID\" ] && echo \"$ID\")
    if (contains(_PLATFORM_ID, ubuntu)|contains(_PLATFORM_ID, debian)) {
       contains(_PLATFORM_ID, ubuntu): _PLATFORM_CODE = ubu
       else: _PLATFORM_CODE = db
       contains(QT_ARCH, x86_64): _TARGET_CPU = amd64
       else: _TARGET_CPU = $$QT_ARCH
    } else {
       isEmpty(_PLATFORM_CODE): _PLATFORM_CODE = $$(PLATFORM_CODE)
       _TARGET_CPU = $${BUILD_ARCH}
       contains(BUILD_ARCH, UNKNOWN ARCH): _TARGET_CPU =
    }

    contains(BUILD_ARCH, arm64): BUILD_ARCH_POVRAY = aarch64
    else: BUILD_ARCH_POVRAY = $${BUILD_ARCH}

    isEmpty(HOST_VERSION):   message("~~~ $${LPUB3D} ERROR: - PLATFORM_VERSION NOT DETECTED ~~~")
    isEmpty(_PLATFORM_CODE): message("~~~ $${LPUB3D} ERROR: - PLATFORM_CODE NOT DETECTED ~~~")
    isEmpty(_TARGET_CPU):    message("~~~ $${LPUB3D} ERROR: - PLATFORM_CPU NOT DETECTED ~~~")

    DISTRO_PACKAGE = $${BUILD_CODE}-$${_PLATFORM_CODE}-$${HOST_VERSION}-$${_TARGET_CPU}
    message("~~~ $${LPUB3D} DISTRO_PACKAGE_ID: $$DISTRO_PACKAGE ~~~")
    DEFINES += DISTRO_PACKAGE=\\\"$$DISTRO_PACKAGE\\\"

    MAN_PAGE = $$DIST_TARGET$$VER_MAJOR$$VER_MINOR
    MAN_PAGE = $$join(MAN_PAGE,,,.1)

    # These settings are used for package distributions that will require elevated rights to install
    isEmpty(INSTALL_PREFIX): INSTALL_PREFIX = /usr
    else: \
    DEFINES += OPT_PREFIX=\\\"$$INSTALL_PREFIX\\\"
    
    isEmpty(BIN_DIR): BIN_DIR               = $$INSTALL_PREFIX/bin
    isEmpty(SHARE_DIR): SHARE_DIR           = $$INSTALL_PREFIX/share
    isEmpty(THIRD_PARTY_EXEC_DIR): \
    THIRD_PARTY_EXEC_DIR                    = $$INSTALL_PREFIX/bin/$$DIST_TARGET

    isEmpty(DOCS_DIR): DOCS_DIR             = $$SHARE_DIR/doc/$$DIST_TARGET
    isEmpty(ICON_DIR): ICON_DIR             = $$SHARE_DIR/icons
    isEmpty(APPDATA_DIR): APPDATA_DIR       = $$SHARE_DIR/metainfo
    isEmpty(MAN_DIR): MAN_DIR               = $$SHARE_DIR/man/man1   
    isEmpty(DESKTOP_DIR): DESKTOP_DIR       = $$SHARE_DIR/applications
    isEmpty(MIME_DIR): MIME_DIR             = $$SHARE_DIR/mime/packages
    isEmpty(MIME_ICON_DIR): MIME_ICON_DIR   = $$SHARE_DIR/icons/hicolor/scalable/mimetypes
   
    isEmpty(RESOURCE_DIR): RESOURCE_DIR     = $$SHARE_DIR/$$DIST_TARGET

    target.path = $$BIN_DIR

    docs.files += $$_PRO_FILE_PWD_/docs/README.txt $$_PRO_FILE_PWD_/docs/RELEASE_NOTES.html $$_PRO_FILE_PWD_/docs/CREDITS.txt $$_PRO_FILE_PWD_/docs/COPYING.txt
    docs.path = $$DOCS_DIR

    man.files += $$_PRO_FILE_PWD_/docs/$$MAN_PAGE
    man.path = $$MAN_DIR

    # lpub3d.desktop
    desktop.files += $$_PRO_FILE_PWD_/$$join(DIST_TARGET,,,.desktop)
    desktop.path = $$DESKTOP_DIR

    # lpub3d.appdata.xml
    appstream_appdata.files += $$_PRO_FILE_PWD_/$$join(DIST_TARGET,,,.appdata.xml)
    appstream_appdata.path = $$APPDATA_DIR
    
    icons.path   = $$ICON_DIR/hicolor
    icons.files += $$_PRO_FILE_PWD_/resources/icon/128x128
    icons.files += $$_PRO_FILE_PWD_/resources/icon/16x16
    icons.files += $$_PRO_FILE_PWD_/resources/icon/24x24
    icons.files += $$_PRO_FILE_PWD_/resources/icon/256x256
    icons.files += $$_PRO_FILE_PWD_/resources/icon/32x32
    icons.files += $$_PRO_FILE_PWD_/resources/icon/48x48
    icons.files += $$_PRO_FILE_PWD_/resources/icon/512x512
    icons.files += $$_PRO_FILE_PWD_/resources/icon/64x64
    icons.files += $$_PRO_FILE_PWD_/resources/icon/scalable

    mime.files += $$_PRO_FILE_PWD_/$$join(DIST_TARGET,,,.xml)
    mime.path = $$MIME_DIR

    mime_ldraw_icon.files += $$_PRO_FILE_PWD_/resources/x-ldraw.svg
    mime_ldraw_icon.path = $$MIME_ICON_DIR

    mime_multi_part_ldraw_icon.files += $$_PRO_FILE_PWD_/resources/x-multi-part-ldraw.svg
    mime_multi_part_ldraw_icon.path = $$MIME_ICON_DIR

    mime_multipart_ldraw_icon.files += $$_PRO_FILE_PWD_/resources/x-multipart-ldraw.svg
    mime_multipart_ldraw_icon.path = $$MIME_ICON_DIR

    excluded_count_parts.files += $$_PRO_FILE_PWD_/extras/excludedParts.lst
    excluded_count_parts.path = $$RESOURCE_DIR

    fadestep_color_parts.files += $$_PRO_FILE_PWD_/extras/fadeStepColorParts.lst
    fadestep_color_parts.path = $$RESOURCE_DIR

    pli_freeform_annotations.files += $$_PRO_FILE_PWD_/extras/freeformAnnotations.lst
    pli_freeform_annotations.path = $$RESOURCE_DIR

    pli_title_annotations.files += $$_PRO_FILE_PWD_/extras/titleAnnotations.lst
    pli_title_annotations.path = $$RESOURCE_DIR

    pli_orientation.files += $$_PRO_FILE_PWD_/extras/pli.mpd
    pli_orientation.path = $$RESOURCE_DIR

    pli_substitution_parts.files += $$_PRO_FILE_PWD_/extras/pliSubstituteParts.lst
    pli_substitution_parts.path = $$RESOURCE_DIR

    ldraw_unofficial_library.files += $$_PRO_FILE_PWD_/extras/lpub3dldrawunf.zip
    ldraw_unofficial_library.path = $$RESOURCE_DIR

    ldraw_official_library.files += $$_PRO_FILE_PWD_/extras/complete.zip
    ldraw_official_library.path = $$RESOURCE_DIR

    ldraw_tente_library.files += $$_PRO_FILE_PWD_/extras/tenteparts.zip
    ldraw_tente_library.path = $$RESOURCE_DIR

    ldraw_vexiq_library.files += $$_PRO_FILE_PWD_/extras/vexiqparts.zip
    ldraw_vexiq_library.path = $$RESOURCE_DIR

    ldglite_ini.files += $$_PRO_FILE_PWD_/extras/ldglite.ini
    ldglite_ini.path = $$RESOURCE_DIR

    nativexport_ini.files += $$_PRO_FILE_PWD_/extras/ldvExport.ini
    nativexport_ini.path = $$RESOURCE_DIR

    ldv_messages_ini.files += $$_PRO_FILE_PWD_/extras/$$LDVMESSAGESINI
    ldv_messages_ini.path = $$RESOURCE_DIR

    INSTALLS += \
    target \
    docs \
    man \
    desktop \
    icons\
    mime\
    mime_ldraw_icon \
    mime_multi_part_ldraw_icon \
    mime_multipart_ldraw_icon \
    excluded_count_parts \
    fadestep_color_parts \
    pli_freeform_annotations \
    pli_title_annotations \
    pli_orientation \
    pli_substitution_parts \
    ldraw_unofficial_library \
    ldraw_official_library \
    ldraw_tente_library \
    ldraw_vexiq_library \
    ldglite_ini \
    nativexport_ini \
    ldv_messages_ini

    api {
    # On OBS, the AppImage appstream appdata scheme is fucked!
    # No matter what I try, validation across x86 and ARM AppImage builds fail.
    # On x86_64 build validation is reported as successful but there is also a message
    # stating no appstream meta info was found. On ARM builds validation fails with
    # invalid tag 'launchable' - this tag is valid. I spent 2 days on this and zero.
    # See this going forward: https://github.com/AppImage/AppImageKit/issues/603#issuecomment-355105387
    # I give up!
        OPEN_BUILD_SERVICE = $$(OBS)
        isEmpty(OPEN_BUILD_SERVICE) {
            INSTALLS += appstream_appdata
        }
    } else {
        if (snp|flp) {
            INSTALLS += appstream_appdata
        }
    }

    # The package distribution settings below requires a specific dev env configuration.
    # Basically, if you create a projects folder e.g. c:\Users\<user>\Projects and
    # clone the listed GitHub repos under the Projects folder, your dev env will
    # conform to these settings.
    # GitHub clone list:
    # LPub3D           = https://github.com/trevorsandy/lpub3d [Branch: master]
    # 3rdParty Content = https://github.com/trevorsandy/lpub3d_linux_3rdparty [Branch: master]
    #
    # At the end, your Projects folder should look like this:
    # C:\Users\<user>\Projects\
    #                   |
    #                   ---\lpub3d_linux_3rdparty\
    #                   |
    #                   ---\LPub3D\
    #
    # Additionally, when using QtCreator be sure 'Shadow Build' is enabled.
    #
    # source path
    isEmpty(THIRD_PARTY_SRC):THIRD_PARTY_SRC = $$THIRD_PARTY_DIST_DIR_PATH

    # source executables - 3rd party components
    isEmpty(LDGLITE_INS_EXE):LDGLITE_INS_EXE   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/bin/$$BUILD_ARCH/ldglite
    isEmpty(LDVIEW_INS_EXE):LDVIEW_INS_EXE     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/bin/$$BUILD_ARCH/ldview
    isEmpty(RAYTRACE_INS_EXE):RAYTRACE_INS_EXE = $$THIRD_PARTY_SRC/$$VER_POVRAY/bin/$$BUILD_ARCH_POVRAY/lpub3d_trace_cui

    # source directories - 3rd party components
    isEmpty(LDGLITE_INS_DOC):LDGLITE_INS_DOC   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/docs
    isEmpty(LDVIEW_INS_DOC):LDVIEW_INS_DOC     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/docs
    isEmpty(RAYTRACE_INS_DOC):RAYTRACE_INS_DOC = $$THIRD_PARTY_SRC/$$VER_POVRAY/docs

    isEmpty(LDGLITE_INS_RES):LDGLITE_INS_RES   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/resources
    isEmpty(LDVIEW_INS_RES):LDVIEW_INS_RES     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/resources
    isEmpty(RAYTRACE_INS_RES):RAYTRACE_INS_RES = $$THIRD_PARTY_SRC/$$VER_POVRAY/resources

    # installed data directories - 3rd party renderer executables
    isEmpty(LDGLITE_INS_DIR):LDGLITE_INS_DIR           = $$THIRD_PARTY_EXEC_DIR/3rdParty/$$VER_LDGLITE/bin
    isEmpty(LDVIEW_INS_DIR):LDVIEW_INS_DIR             = $$THIRD_PARTY_EXEC_DIR/3rdParty/$$VER_LDVIEW/bin
    isEmpty(RAYTRACE_INS_DIR):RAYTRACE_INS_DIR         = $$THIRD_PARTY_EXEC_DIR/3rdParty/$$VER_POVRAY/bin

    # installed data directories - 3rd party components
    isEmpty(LDGLITE_INS_DOC_DIR):LDGLITE_INS_DOC_DIR   = $$RESOURCE_DIR/3rdParty/$$VER_LDGLITE/doc
    isEmpty(LDGLITE_INS_RES_DIR):LDGLITE_INS_RES_DIR   = $$RESOURCE_DIR/3rdParty/$$VER_LDGLITE/resources
    isEmpty(LDVIEW_INS_DOC_DIR):LDVIEW_INS_DOC_DIR     = $$RESOURCE_DIR/3rdParty/$$VER_LDVIEW/doc
    isEmpty(LDVIEW_INS_RES_DIR):LDVIEW_INS_RES_DIR     = $$RESOURCE_DIR/3rdParty/$$VER_LDVIEW/resources
    isEmpty(RAYTRACE_INS_DOC_DIR):RAYTRACE_INS_DOC_DIR = $$RESOURCE_DIR/3rdParty/$$VER_POVRAY/doc
    isEmpty(RAYTRACE_INS_RES_DIR):RAYTRACE_INS_RES_DIR = $$RESOURCE_DIR/3rdParty/$$VER_POVRAY/resources

    # install 3rd party content
    include(install3rdpartycontent.pri)
    # install local libraries - used for RHEL builds
    include(locallibsdistro.pri)
}
