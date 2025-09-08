# These settings are used for the MacOS package distribution

# Update Info.plist
INFO_PLIST_FILE = $$_PRO_FILE_PWD_/Info.plist
PLIST_COMMAND = /usr/libexec/PlistBuddy -c
system( $$PLIST_COMMAND \"Set :CFBundleShortVersionString $${VERSION}\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :CFBundleVersion $${VERSION}.$${VER_COMMIT}-$${VER_GIT_SHA}\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :CFBundleExecutable $${TARGET}\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :CFBundleName $${TARGET}\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :CFBundleDisplayName $${TARGET}\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :CFBundleIdentifier com.trevorsandy.$$lower($$TARGET)\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :CFBundleGetInfoString $${TARGET} $${VERSION} https://trevorsandy.github.io/lpub3d\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :UTExportedTypeDeclarations:0:UTTypeIdentifier com.trevorsandy.$$lower($$TARGET)\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :UTExportedTypeDeclarations:1:UTTypeIdentifier com.trevorsandy.$$lower($$TARGET)\" \"$${INFO_PLIST_FILE}\" )
system( $$PLIST_COMMAND \"Set :UTExportedTypeDeclarations:2:UTTypeIdentifier com.trevorsandy.$$lower($$TARGET)\" \"$${INFO_PLIST_FILE}\" )

BUILD_CODE = $${option}
isEmpty(BUILD_CODE): BUILD_CODE = dmg
isEmpty(HOST_VERSION): HOST_VERSION = macOS
isEmpty(BUILD_ARCH): BUILD_ARCH = $${QT_ARCH}
DISTRO_PACKAGE = $${BUILD_CODE}-$${HOST_VERSION}-$${BUILD_ARCH}
message("~~~ $${LPUB3D} DISTRO_PACKAGE_CODE: $${BUILD_ARCH}-macos-dmg ~~~")
message("~~~ $${LPUB3D} DISTRO_PACKAGE_ID: $$DISTRO_PACKAGE ~~~")
DEFINES += VER_DISTRO_PACKAGE=\\\"$$DISTRO_PACKAGE\\\"

#ICON = lpub3d.icns
ICON = $$_PRO_FILE_PWD_/$$lower($$join(DIST_TARGET,,,.icns))
QMAKE_INFO_PLIST = $$_PRO_FILE_PWD_/Info.plist

# documents and resources
document_icon.files += $$_PRO_FILE_PWD_/ldraw_document.icns
document_icon.path = Contents/Resources

document_readme.files += $$_PRO_FILE_PWD_/docs/README.txt
document_readme.path = Contents/Resources

document_release_notes.files += $$_PRO_FILE_PWD_/docs/RELEASE_NOTES.html
document_release_notes.path = Contents/Resources

document_credits.files += $$_PRO_FILE_PWD_/docs/CREDITS.txt
document_credits.path = Contents/Resources

document_copying.files += $$_PRO_FILE_PWD_/docs/COPYING.txt
document_copying.path = Contents/Resources

excluded_count_parts.files += $$_PRO_FILE_PWD_/extras/excludedParts.lst
excluded_count_parts.path = Contents/Resources

fadestep_color_parts.files += $$_PRO_FILE_PWD_/extras/fadeStepColorParts.lst
fadestep_color_parts.path = Contents/Resources

pli_freeform_annotations.files += $$_PRO_FILE_PWD_/extras/freeformAnnotations.lst
pli_freeform_annotations.path = Contents/Resources

pli_title_annotations.files += $$_PRO_FILE_PWD_/extras/titleAnnotations.lst
pli_title_annotations.path = Contents/Resources

pli_orientation.files += $$_PRO_FILE_PWD_/extras/pli.mpd
pli_orientation.path = Contents/Resources

pli_substitution_parts.files += $$_PRO_FILE_PWD_/extras/pliSubstituteParts.lst
pli_substitution_parts.path = Contents/Resources

ldraw_unofficial_library.files += $$_PRO_FILE_PWD_/extras/lpub3dldrawunf.zip
ldraw_unofficial_library.path = Contents/Resources

ldraw_official_library.files += $$_PRO_FILE_PWD_/extras/complete.zip
ldraw_official_library.path = Contents/Resources

ldraw_tente_library.files += $$_PRO_FILE_PWD_/extras/tenteparts.zip
ldraw_tente_library.path = Contents/Resources

ldraw_vexiq_library.files += $$_PRO_FILE_PWD_/extras/vexiqparts.zip
ldraw_vexiq_library.path = Contents/Resources

ldglite_ini.files += $$_PRO_FILE_PWD_/extras/ldglite.ini
ldglite_ini.path = Contents/Resources

nativexport_ini.files += $$_PRO_FILE_PWD_/extras/ldvExport.ini
nativexport_ini.path = Contents/Resources

ldv_messages_ini.files += $$_PRO_FILE_PWD_/extras/$$LDV_MESSAGES_INI
ldv_messages_ini.path = Contents/Resources

QMAKE_BUNDLE_DATA += \
    document_icon \
    document_readme \
    document_release_notes \
    document_credits \
    document_copying \
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

# The package distribution settings below requires a specific dev env configuration.
# Basically, if you create a projects folder e.g. c:\Users\<user>\Projects and
# clone the listed GitHub repos under the Projects folder, your dev env will
# conform to these settings.
# GitHub clone list:
# LPub3D           = https://github.com/trevorsandy/lpub3d [Branch: master]
# 3rdParty Content = https://github.com/trevorsandy/lpub3d_macos_3rdparty [Branch: master]
#
# At the end, your Projects folder should look like this:
# C:\Users\<user>\Projects\
#                   |
#                   ---\lpub3d_macos_3rdparty\
#                   |
#                   ---\LPub3D\
#
# Additionally, when using QtCreator be sure 'Shadow Build' is enabled.
#
# source path
isEmpty(THIRD_PARTY_SRC):THIRD_PARTY_SRC = $$THIRD_PARTY_DIST_DIR_PATH

# POVRay configure uses aarch64 architecture value on arm64 Mac build
macx:contains(QT_ARCH, arm64) { LP3D_RAY_ARCH = aarch64 } else { LP3D_RAY_ARCH = $$QT_ARCH }

# source executables - 3rd party components
isEmpty(LDGLITE_INS_EXE):LDGLITE_INS_EXE   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/bin/$$QT_ARCH/ldglite
isEmpty(LDVIEW_INS_EXE):LDVIEW_INS_EXE     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/bin/$$QT_ARCH/ldview
isEmpty(RAYTRACE_INS_EXE):RAYTRACE_INS_EXE = $$THIRD_PARTY_SRC/$$VER_POVRAY/bin/$$LP3D_RAY_ARCH/lpub3d_trace_cui

# source directories - 3rd party components
isEmpty(LDGLITE_INS_DOC):LDGLITE_INS_DOC   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/docs
isEmpty(LDVIEW_INS_DOC):LDVIEW_INS_DOC     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/docs
isEmpty(RAYTRACE_INS_DOC):RAYTRACE_INS_DOC = $$THIRD_PARTY_SRC/$$VER_POVRAY/docs

isEmpty(LDGLITE_INS_RES):LDGLITE_INS_RES   = $$THIRD_PARTY_SRC/$$VER_LDGLITE/resources
isEmpty(LDVIEW_INS_RES):LDVIEW_INS_RES     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/resources
isEmpty(RAYTRACE_INS_RES):RAYTRACE_INS_RES = $$THIRD_PARTY_SRC/$$VER_POVRAY/resources

# installed data directories - 3rd party components
isEmpty(LDGLITE_INS_DIR):LDGLITE_INS_DIR           = Contents/3rdParty/$$VER_LDGLITE/bin
isEmpty(LDGLITE_INS_DOC_DIR):LDGLITE_INS_DOC_DIR   = Contents/3rdParty/$$VER_LDGLITE/docs
isEmpty(LDGLITE_INS_RES_DIR):LDGLITE_INS_RES_DIR   = Contents/3rdParty/$$VER_LDGLITE/resources
isEmpty(LDVIEW_INS_DIR):LDVIEW_INS_DIR             = Contents/3rdParty/$$VER_LDVIEW/bin
isEmpty(LDVIEW_INS_DOC_DIR):LDVIEW_INS_DOC_DIR     = Contents/3rdParty/$$VER_LDVIEW/docs
isEmpty(LDVIEW_INS_RES_DIR):LDVIEW_INS_RES_DIR     = Contents/3rdParty/$$VER_LDVIEW/resources
isEmpty(RAYTRACE_INS_DIR):RAYTRACE_INS_DIR         = Contents/3rdParty/$$VER_POVRAY/bin
isEmpty(RAYTRACE_INS_DOC_DIR):RAYTRACE_INS_DOC_DIR = Contents/3rdParty/$$VER_POVRAY/docs
isEmpty(RAYTRACE_INS_RES_DIR):RAYTRACE_INS_RES_DIR = Contents/3rdParty/$$VER_POVRAY/resources

# install 3rd party content
include(install3rdpartyassets.pri)
