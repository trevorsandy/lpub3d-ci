# install 3rd party executables, documents and resources
install3rdassets {
    win32-arm64-msvc|win32-msvc*: THIRD_PARTY_DEST = $$LPUB3D_INS_CONTENT_PATH
    else:macx:   THIRD_PARTY_DEST = $$DESTDIR/$${TARGET}.app/Contents/3rdParty
    message("~~~ $${LPUB3D} 3RD_INSTALL FROM REPO: $$THIRD_PARTY_SRC ~~~")
    if (unix|msys):!macx {
        THIRD_PARTY_DEST = $$RESOURCE_DIR/3rdParty
        message("~~~ $${LPUB3D} 3RD_INSTALL TO EXE DEST: $$THIRD_PARTY_EXEC_DIR/3rdParty ~~~")
    }
    message("~~~ $${LPUB3D} 3RD_INSTALL TO DEST: $$THIRD_PARTY_DEST ~~~")
}

install3rdexe {
    # renderer executables
    message("~~~ $${LPUB3D} 3RD PARTY RENDERER ASSETS WILL BE INSTALLED ~~~")

    exists($${LDGLITE_INS_EXE}) {
        ldglite_ins.files       += $$LDGLITE_INS_EXE
        ldglite_ins.path         = $$LDGLITE_INS_DIR

        macx: QMAKE_BUNDLE_DATA += ldglite_ins
        else: INSTALLS          += ldglite_ins

        CONFIG                  += ldglite_install

        message("~~~ $${LPUB3D} 1 OF 3 $$upper($${VER_LDGLITE}) RENDERER INSTALLED ~~~")
    } else:BUILD_LDV_LIBS {
        message("~~~ $${LPUB3D} 1 OF 3 $$upper($${VER_LDVIEW}) RENDERER BUILT FROM SOURCE ~~~")
    } else {
        message("~~~ NOTICE 1 OF 3 $$upper($${VER_LDGLITE}) RENDERER NOT FOUND ~~~")
    }

    exists($${LDVIEW_INS_EXE}) {
        ldview_ins.files        += $$LDVIEW_INS_EXE
        ldview_ins.path          = $$LDVIEW_INS_DIR

        macx: QMAKE_BUNDLE_DATA += ldview_ins
        else: INSTALLS          += ldview_ins

        CONFIG                  += ldview_install

        message("~~~ $${LPUB3D} 2 OF 3 $$upper($${VER_LDVIEW}) RENDERER INSTALLED ~~~")
    } else:BUILD_LDV_LIBS {
        message("~~~ $${LPUB3D} 2 OF 3 $$upper($${VER_LDVIEW}) RENDERER BUILT FROM SOURCE ~~~")
    } else {
        message("~~~ NOTICE 2 OF 3 $$upper($${VER_LDVIEW}) RENDERER NOT FOUND ~~~")
    }

    exists($${RAYTRACE_INS_EXE}) {
        raytracer_ins.files     += $$RAYTRACE_INS_EXE
        raytracer_ins.path       = $$RAYTRACE_INS_DIR

        macx: QMAKE_BUNDLE_DATA += raytracer_ins
        else: INSTALLS          += raytracer_ins

        CONFIG                  += raytracer_install

        message("~~~ $${LPUB3D} 3 OF 3 $$upper($${VER_POVRAY}) RENDERER INSTALLED ~~~")
    } else:BUILD_LDV_LIBS {
        message("~~~ $${LPUB3D} 3 OF 3 $$upper($${VER_POVRAY}) RENDERER BUILT FROM SOURCE ~~~")
    } else {
        message("~~~ NOTICE 3 OF 3 $$upper($${VER_POVRAY}) RENDERER NOT FOUND ~~~")
    }
}

install3rdconfig {
    message("~~~ $${LPUB3D} 3RD PARTY RENDERER EXECUTABLE CONFIG FILES WILL BE INSTALLED ~~~")

    raytracer_install {
        win32-arm64-msvc|win32-msvc*: \
        raytracer_ins_resources_config.files += \
            $$RAYTRACE_INS_RES/config/$$QT_ARCH/povray.ini \
            $$RAYTRACE_INS_RES/config/$$QT_ARCH/povray.conf
        else: \
        raytracer_ins_resources_config.files += \
            $$RAYTRACE_INS_RES/config/povray.ini \
            $$RAYTRACE_INS_RES/config/povray.conf
        raytracer_ins_resources_config.path = $$RAYTRACE_INS_RES_DIR/config

        macx: QMAKE_BUNDLE_DATA += raytracer_ins_resources_config
        else: INSTALLS          += raytracer_ins_resources_config

        message("~~~ $${LPUB3D} 1 OF 2 $$upper($${VER_POVRAY}) CONFIG FILES INSTALLED ~~~")
    }

    ldview_install {
        ldview_ins_resources_config.files += \
            $$LDVIEW_INS_RES/config/ldview.ini \
            $$LDVIEW_INS_RES/config/ldviewPOV.ini \
            $$LDVIEW_INS_RES/config/LDViewCustomini
        ldview_ins_resources_config.path = $$LDVIEW_INS_RES_DIR/config

        macx: QMAKE_BUNDLE_DATA += ldview_ins_resources_config
        else: INSTALLS          += ldview_ins_resources_config

        message("~~~ $${LPUB3D} 2 OF 2 $$upper($${VER_LDVIEW}) CONFIG FILES INSTALLED ~~~")
    }
}

install3rdcontent {
    message("~~~ $${LPUB3D} 3RD PARTY RENDERER DOCUMENTATION AND RESOURCES WILL BE INSTALLED ~~~")

    # ldglite content
    ldglite_install {
        ldglite_ins_docs.files += \
            $$LDGLITE_INS_DOC/README.TXT \
            $$LDGLITE_INS_DOC/LICENCE
        ldglite_ins_docs.path = $$LDGLITE_INS_DOC_DIR

        macx {
            ldglite_ins_resources.files += \
                $$LDGLITE_INS_RES/set-ldrawdir.command
            ldglite_ins_resources.path = $$LDGLITE_INS_RES_DIR
            QMAKE_BUNDLE_DATA += \
            ldglite_ins_docs \
            ldglite_ins_resources
        } else {
            INSTALLS += \
            ldglite_ins_docs
        }

        message("~~~ $${LPUB3D} 1 OF 3 $$upper($${VER_LDGLITE}) DOCUMENTATION AND RESOURCES INSTALLED ~~~")
    }

    # ldview contnet
    ldview_install {
        ldview_ins_docs.files += \
            $$LDVIEW_INS_DOC/ChangeHistory.html \
            $$LDVIEW_INS_DOC/Help.html \
            $$LDVIEW_INS_DOC/License.txt \
            $$LDVIEW_INS_DOC/Readme.txt
        ldview_ins_docs.path = $$LDVIEW_INS_DOC_DIR

        ldview_ins_resources.files += \
            $$LDVIEW_INS_RES/8464.mpd \
            $$LDVIEW_INS_RES/m6459.ldr \
            $$LDVIEW_INS_RES/LGEO.xml
        ldview_ins_resources.path = $$LDVIEW_INS_RES_DIR

        macx: QMAKE_BUNDLE_DATA += \
            ldview_ins_docs \
            ldview_ins_resources
        else: INSTALLS += \
            ldview_ins_docs \
            ldview_ins_resources

        message("~~~ $${LPUB3D} 2 OF 3 $$upper($${VER_LDVIEW}) DOCUMENTATION AND RESOURCES INSTALLED ~~~")
    }

    # raytracer contnet
    raytracer_install {
        win32: EXT = .txt
        else: EXT =
        raytracer_ins_docs.files += \
            $$RAYTRACE_INS_DOC/AUTHORS$$EXT \
            $$RAYTRACE_INS_DOC/ChangeLog$$EXT \
            $$RAYTRACE_INS_DOC/CUI_README$$EXT \
            $$RAYTRACE_INS_DOC/LICENSE$$EXT \
            $$RAYTRACE_INS_DOC/NEWS$$EXT
        raytracer_ins_docs.path = $$RAYTRACE_INS_DOC_DIR

        raytracer_ins_resources_ini.files += \
            $$RAYTRACE_INS_RES/ini/allanim.ini \
            $$RAYTRACE_INS_RES/ini/allstill.ini \
            $$RAYTRACE_INS_RES/ini/low.ini \
            $$RAYTRACE_INS_RES/ini/pngflc.ini \
            $$RAYTRACE_INS_RES/ini/pngfli.ini \
            $$RAYTRACE_INS_RES/ini/povray.ini \
            $$RAYTRACE_INS_RES/ini/quickres.ini \
            $$RAYTRACE_INS_RES/ini/res120.ini \
            $$RAYTRACE_INS_RES/ini/res1k.ini \
            $$RAYTRACE_INS_RES/ini/res320.ini \
            $$RAYTRACE_INS_RES/ini/res640.ini \
            $$RAYTRACE_INS_RES/ini/res800.ini \
            $$RAYTRACE_INS_RES/ini/slow.ini \
            $$RAYTRACE_INS_RES/ini/tgaflc.ini \
            $$RAYTRACE_INS_RES/ini/tgafli.ini \
            $$RAYTRACE_INS_RES/ini/zipflc.ini \
            $$RAYTRACE_INS_RES/ini/zipfli.ini
        raytracer_ins_resources_ini.path = $$RAYTRACE_INS_RES_DIR/ini

        raytracer_ins_resources_include.files += \
            $$RAYTRACE_INS_RES/include/arrays.inc \
            $$RAYTRACE_INS_RES/include/ash.map \
            $$RAYTRACE_INS_RES/include/benediti.map \
            $$RAYTRACE_INS_RES/include/bubinga.map \
            $$RAYTRACE_INS_RES/include/bumpmap_.png \
            $$RAYTRACE_INS_RES/include/cedar.map \
            $$RAYTRACE_INS_RES/include/chars.inc \
            $$RAYTRACE_INS_RES/include/colors.inc \
            $$RAYTRACE_INS_RES/include/colors_ral.inc \
            $$RAYTRACE_INS_RES/include/consts.inc \
            $$RAYTRACE_INS_RES/include/crystal.ttf \
            $$RAYTRACE_INS_RES/include/cyrvetic.ttf \
            $$RAYTRACE_INS_RES/include/debug.inc \
            $$RAYTRACE_INS_RES/include/finish.inc \
            $$RAYTRACE_INS_RES/include/fract003.png \
            $$RAYTRACE_INS_RES/include/functions.inc \
            $$RAYTRACE_INS_RES/include/glass.inc \
            $$RAYTRACE_INS_RES/include/glass_old.inc \
            $$RAYTRACE_INS_RES/include/golds.inc \
            $$RAYTRACE_INS_RES/include/ior.inc \
            $$RAYTRACE_INS_RES/include/logo.inc \
            $$RAYTRACE_INS_RES/include/makegrass.inc \
            $$RAYTRACE_INS_RES/include/marbteal.map \
            $$RAYTRACE_INS_RES/include/math.inc \
            $$RAYTRACE_INS_RES/include/meshmaker.inc \
            $$RAYTRACE_INS_RES/include/metals.inc \
            $$RAYTRACE_INS_RES/include/Mount1.png \
            $$RAYTRACE_INS_RES/include/Mount2.png \
            $$RAYTRACE_INS_RES/include/mtmand.pot \
            $$RAYTRACE_INS_RES/include/mtmandj.png \
            $$RAYTRACE_INS_RES/include/orngwood.map \
            $$RAYTRACE_INS_RES/include/pinkmarb.map \
            $$RAYTRACE_INS_RES/include/plasma2.png \
            $$RAYTRACE_INS_RES/include/plasma3.png \
            $$RAYTRACE_INS_RES/include/povlogo.ttf \
            $$RAYTRACE_INS_RES/include/povmap.png \
            $$RAYTRACE_INS_RES/include/rad_def.inc \
            $$RAYTRACE_INS_RES/include/rand.inc \
            $$RAYTRACE_INS_RES/include/rdgranit.map \
            $$RAYTRACE_INS_RES/include/screen.inc \
            $$RAYTRACE_INS_RES/include/shapes.inc \
            $$RAYTRACE_INS_RES/include/shapes2.inc \
            $$RAYTRACE_INS_RES/include/shapes3.inc \
            $$RAYTRACE_INS_RES/include/shapesq.inc \
            $$RAYTRACE_INS_RES/include/shapes_old.inc \
            $$RAYTRACE_INS_RES/include/skies.inc \
            $$RAYTRACE_INS_RES/include/spiral.df3 \
            $$RAYTRACE_INS_RES/include/stage1.inc \
            $$RAYTRACE_INS_RES/include/stars.inc \
            $$RAYTRACE_INS_RES/include/stdcam.inc \
            $$RAYTRACE_INS_RES/include/stdinc.inc \
            $$RAYTRACE_INS_RES/include/stoneold.inc \
            $$RAYTRACE_INS_RES/include/stones.inc \
            $$RAYTRACE_INS_RES/include/stones1.inc \
            $$RAYTRACE_INS_RES/include/stones2.inc \
            $$RAYTRACE_INS_RES/include/strings.inc \
            $$RAYTRACE_INS_RES/include/sunpos.inc \
            $$RAYTRACE_INS_RES/include/teak.map \
            $$RAYTRACE_INS_RES/include/test.png \
            $$RAYTRACE_INS_RES/include/textures.inc \
            $$RAYTRACE_INS_RES/include/timrom.ttf \
            $$RAYTRACE_INS_RES/include/transforms.inc \
            $$RAYTRACE_INS_RES/include/ttffonts.cat \
            $$RAYTRACE_INS_RES/include/whiteash.map \
            $$RAYTRACE_INS_RES/include/woodmaps.inc \
            $$RAYTRACE_INS_RES/include/woods.inc
        raytracer_ins_resources_include.path = $$RAYTRACE_INS_RES_DIR/include

        macx: QMAKE_BUNDLE_DATA += \
            raytracer_ins_docs \
            raytracer_ins_resources_ini \
            raytracer_ins_resources_include
        else: INSTALLS += \
            raytracer_ins_docs \
            raytracer_ins_resources_ini \
            raytracer_ins_resources_include

        message("~~~ $${LPUB3D} 3 OF 3 $$upper($${VER_POVRAY}) DOCUMENTATION AND RESOURCES INSTALLED ~~~")
    }
}
