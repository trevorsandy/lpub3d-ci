#include <qglobal.h>
/****************************************************************************
**
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef VERSION_H
#define VERSION_H

// ~~~~~ predefines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define STR(x)   #x
#define STRING(x)  STR(x)

// ~~~~~ Generic Version info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define VER_COMPILE_DATE_STR                DATE_YY " " DATE_MM " " DATE_DD " " BUILD_TIME
#define VER_PRODUCTVERSION_STR              VER_MAJOR "." VER_MINOR "." VER_PATCH
#define VER_PRODUCTNAME_STR                 "LPub3D"
#define VER_COMPANYDOMAIN_STR               "io.github.trevorsandy.LPub3D"
#define VER_COMPANYNAME_BLD_STR             "LPub3D Software"
#define VER_FILEDESCRIPTION_STR             "LPub3D - An LDraw Building Instruction Editor"
#define VER_LEGALCOPYRIGHT_STR              "Copyright &copy; 2015 - 2025 by Trevor SANDY"
#define VER_LEGALTRADEMARKS1_STR            "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR            VER_LEGALTRADEMARKS1_STR
#define VER_PUBLISHER_STR                   "Trevor SANDY"
#define VER_GITHUB_USER_STR                 "https://github.com/trevorsandy"
#define VER_GITHUB_API_STR                  "https://api.github.com/repos/trevorsandy"
#define VER_FILEVERSION_STR                 STRING(VER_MAJOR) "." STRING(VER_MINOR) "." STRING(VER_PATCH) "." VER_REVISION_STR "." VER_COMMIT_STR
#define VER_PUBLISHER_SUPPORT_STR           "https://github.com/trevorsandy/lpub3d/issues/new/choose"
#define VER_LICENSE_INFO_STR                "http://www.gnu.org/licenses/gpl-3.0.html"
#define VER_UNOFFICIAL_MODEL_STR            "Unofficial_Model"
#define VER_UNOFFICIAL_PART_STR             "Unofficial_Part"
#define VER_LDRAW_FILE_LICENSE_STR          "Redistributable under CCAL version 2.0 : see CAreadme.txt"
#ifdef INSTALL_PREFIX
  #define VER_INSTALL_PREFIX_STR            INSTALL_PREFIX
#else
  #define VER_INSTALL_PREFIX_STR            ""
#endif
#ifdef QT_DEBUG_MODE
  #define COMPANYNAME_STR                   "LPub3D Software Maint"
#else
  #define COMPANYNAME_STR                   "LPub3D Software"
#endif
#define VER_COMPANYNAME_STR                 COMPANYNAME_STR                  // "LPub3D Software Maint"
#define VER_CHIPSET_STR                     VER_CHIPSET
#define VER_BUILD_ARCH_STR                  VER_ARCH

#define VER_APPIMAGE_BUILD_STR              "Appimage"
#define VER_FLATPAK_BUILD_STR               "Flatpak"
#define VER_CONDA_BUILD_STR                 "Conda"
#define VER_SNAP_BUILD_STR                  "Snap"
#define VER_MSYS2_BUILD_STR                 "MSYS2 MinGW"

#ifdef LP3D_CONTINUOUS_BUILD
  #ifdef LP3D_DEVOPS_BUILD
     #ifdef QT_DEBUG_MODE
         #define    VER_BUILD_TYPE_STR      "Dev-debug"
     #else
         #define    VER_BUILD_TYPE_STR      "Dev-release"
     #endif
  #elif defined LP3D_NEXT_BUILD
     #ifdef QT_DEBUG_MODE
         #define    VER_BUILD_TYPE_STR      "Next-debug"
     #else
         #define    VER_BUILD_TYPE_STR      "Next-release"
     #endif
  #else
     #define    VER_BUILD_TYPE_STR          "Release"
  #endif
#elif defined LP3D_DEVOPS_BUILD
  #define    VER_BUILD_TYPE_STR             "Dev-release"
#elif defined LP3D_NEXT_BUILD
  #define    VER_BUILD_TYPE_STR             "Next-release"
#else
  #define    VER_BUILD_TYPE_STR             "Version"
#endif

// ~~~~~~ 3rdParty Applications ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define VER_LDVIEW_STR                      VER_LDVIEW
#define VER_LDGLITE_STR                     VER_LDGLITE
#define VER_POVRAY_STR                      VER_POVRAY

#define VER_LDVIEW_WIN_X86                 "LDView-win-x86.zip"
#define VER_LDGLITE_WIN_X86                "LDGLite-win-x86.zip"
#define VER_POVRAY_WIN_X86                 "lpub3d_trace_cui-win-x86.zip"

#define VER_LDVIEW_INI_FILE                 "ldview.ini"      // LDView renderer config settings
#define VER_LDVIEW_POV_INI_FILE             "ldviewPOV.ini"   // LDView POV file generation config settings
#define VER_POVRAY_CONF_FILE                "povray.conf"     // POV-Ray access control config settings
#define VER_POVRAY_INI_FILE                 "povray.ini"      // POV-Ray renderer config settings
#define VER_LDGLITE_INI_FILE                "ldglite.ini"     // LDGLite renderer config settings
#define VER_NATIVE_EXPORT_INI_FILE          "ldvExport.ini"   // Native LDV Export (POV file gen, 3ds, STL) config settings
#define VER_POVRAY_INC_FILE                 "consts.inc"      // Only used to validate directory
#define VER_POVRAY_SCENE_FILE               "index.htm"       // Only used to validate directory
#define VER_LGEO_XML_FILE                   "LGEO.xml"        // LGEO xml file

// ~~~~~~ App Parameters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define VER_LOGGING_LEVELS_STR              "STATUS,INFO,TRACE,DEBUG,NOTICE,WARNING,ERROR,FATAL,OFF"

#define VER_EXTRAS_LDCONFIG_FILE            "LDConfig.ldr"
#define VER_PDFPRINT_IMAGE_FILE             "PDFPrint.jpg"
#define VER_LDVMESSAGESINI_FILE             "ldvMessages.ini"
#define VER_ROTATE_ICON_FILE                "rotate-icon.png"

#define VER_TITLE_ANNOTATIONS_FILE          "titleAnnotations.lst"
#define VER_LEGO_TITLE_ANNOTATIONS_FILE     "LEGOTitleAnnotations.lst"
#define VER_TENTE_TITLE_ANNOTATIONS_FILE    "TENTETitleAnnotations.lst"
#define VER_VEXIQ_TITLE_ANNOTATIONS_FILE    "VEXIQTitleAnnotations.lst"

#define VER_FREEFOM_ANNOTATIONS_FILE        "freeformAnnotations.lst"
#define VER_LEGO_FREEFROM_ANNOTATIONS_FILE  "LEGOFreeFormAnnotations.lst"
#define VER_TENTE_FREEFROM_ANNOTATIONS_FILE "TENTEFreeFormAnnotations.lst"
#define VER_VEXIQ_FREEFROM_ANNOTATIONS_FILE "VEXIQFreeFormAnnotations.lst"

#define VER_ANNOTATION_STYLE_FILE           "annotationStyle.lst"
#define VER_LEGO_ANNOTATION_STYLE_FILE      "LEGOAnnotationStyle.lst"
#define VER_TENTE_ANNOTATION_STYLE_FILE     "TENTEAnnotationStyle.lst"
#define VER_VEXIQ_ANNOTATION_STYLE_FILE     "VEXIQAnnotationStyle.lst"

#define VER_PLI_SUBSTITUTE_PARTS_FILE       "pliSubstituteParts.lst"
#define VER_LEGO_PLI_SUBSTITUTE_FILE        "LEGOPliSubstituteParts.lst"
#define VER_TENTE_PLI_SUBSTITUTE_FILE       "TENTEPliSubstituteParts.lst"
#define VER_VEXIQ_PLI_SUBSTITUTE_FILE       "VEXIQPliSubstituteParts.lst"

#define VER_EXCLUDED_PARTS_FILE             "excludedParts.lst"
#define VER_LEGO_PLI_EXCLUDED_FILE          "LEGOExcludedParts.lst"
#define VER_TENTE_PLI_EXCLUDED_FILE         "TENTEExcludedParts.lst"
#define VER_VEXIQ_PLI_EXCLUDED_FILE         "VEXIQExcludedParts.lst"

#define VER_STICKER_PARTS_FILE             "stickerParts.lst"
#define VER_LEGO_PLI_STICKER_FILE          "LEGOStickerParts.lst"
#define VER_TENTE_PLI_STICKER_FILE         "TENTEStickerParts.lst"
#define VER_VEXIQ_PLI_STICKER_FILE         "VEXIQStickerParts.lst"

#define VER_PLI_CONTROL_FILE                "pli.mpd"
#define VER_LEGO_PLI_CONTROL_FILE           "LEGOPliControl.ldr"
#define VER_TENTE_PLI_CONTROL_FILE          "TENTEPliControl.ldr"
#define VER_VEXIQ_PLI_CONTROL_FILE          "VEXIQPliControl.ldr"

#define VER_LPUB3D_COLOR_PARTS              "fadeStepColorParts.lst"
#define VER_LPUB3D_LEGO_COLOR_PARTS         "LEGOFadeStepColorParts.lst"
#define VER_LPUB3D_TENTE_COLOR_PARTS        "TENTEFadeStepColorParts.lst"
#define VER_LPUB3D_VEXIQ_COLOR_PARTS        "VEXIQFadeStepColorParts.lst"

#define VER_LDRAW_OFFICIAL_ARCHIVE          "complete.zip"
#define VER_LPUB3D_TENTE_ARCHIVE            "tenteparts.zip"
#define VER_LPUB3D_VEXIQ_ARCHIVE            "vexiqparts.zip"

#define VER_LDRAW_UNOFFICIAL_ARCHIVE        "ldrawunf.zip"
#define VER_LPUB3D_UNOFFICIAL_ARCHIVE       "lpub3dldrawunf.zip"
#define VER_LPUB3D_TENTE_CUSTOM_ARCHIVE     "tentecustomparts.zip"
#define VER_LPUB3D_VEXIQ_CUSTOM_ARCHIVE     "vexiqcustomparts.zip"

#if defined Q_OS_WIN || defined Q_OS_MACOS
#define VER_LEGO_LDRAWDIR_STR               "LDraw"
#define VER_TENTE_LDRAWDIR_STR              "TENTEParts"
#define VER_VEXIQ_LDRAWDIR_STR              "VEXIQParts"
#else
#define VER_LEGO_LDRAWDIR_STR               "ldraw"
#define VER_TENTE_LDRAWDIR_STR              "tenteparts"
#define VER_VEXIQ_LDRAWDIR_STR              "vexiqparts"
#endif

#define VER_LPUB3D_THEME_CHECK_SCRIPT       "themecheck.sh"

#define VER_LEGO_LDRAWPART_STR              "/parts/1.dat"
#define VER_TENTE_LDRAWPART_STR             "/parts/4circulo.dat"
#define VER_VEXIQ_LDRAWPART_STR             "/parts/228-2500-001.dat"

#define VER_FADE_PART_COLOURS_FILE          "fadePartColours.lst"

#define VER_LPUB3D_BLCOLORS_FILE            "colors.txt"
#define VER_LPUB3D_BLCODES_FILE             "codes.txt"
#define VER_LPUB3D_USERELEMENTS_FILE        "userelements.lst"
#define VER_LPUB3D_LEGOELEMENTS_FILE        "legoelements.lst"

#define VER_LPUB3D_LD2BLCOLORSXREF_FILE     "ld2blcolorsxref.lst"
#define VER_LPUB3D_LD2BLCODESXREF_FILE      "ld2blcodesxref.lst"
#define VER_LPUB3D_LD2RBCOLORSXREF_FILE     "ld2rbcolorsxref.lst"
#define VER_LPUB3D_LD2RBCODESXREF_FILE      "ld2rbcodesxref.lst"

#define VER_BLENDER_DEFAULT_BLEND_FILE      "lpub3d.blend"
#define VER_BLENDER_ADDON_FILE              "LDrawBlenderRenderAddons.zip"
#define VER_BLENDER_ADDON_INSTALL_FILE      "install_blender_ldraw_addons.py"
#define VER_BLENDER_ADDON_CONFIG_FILE       "LDrawRendererPreferences.ini"
#define VER_BLENDER_DOCUMENT_CONFIG_FILE    "PreferredRendererPreferences.ini"
#define VER_BLENDER_LDRAW_PARAMS_FILE       "LDrawParameters.lst"


// ~~~~~~ Resource Locators ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define VER_HOMEPAGE_GITHUB_STR             "https://trevorsandy.github.io/lpub3d/"
#define VER_SOURCE_GITHUB_STR               VER_GITHUB_USER_STR "/lpub3d/"
#define VER_BLENDER_ADDON_STR               VER_GITHUB_USER_STR "/blenderldrawrender/"
#define VER_BLENDER_ADDON_API_STR           VER_GITHUB_API_STR "/blenderldrawrender/"
#define VER_LPUB3D_LIBRARIES_STR            VER_GITHUB_USER_STR "/lpub3d_libs/releases/download/v1.0.1/"
#define VER_SOURCEFORGE_STR                 "https://lpub3d.sourceforge.io/"

#define VER_QT_URL                          "https://www.qt.io/product"
#define VER_LPUB3D_ONLINE_MANUAL_URL        "https://sites.google.com/view/workingwithlpub3d/"
#define VER_REBRICKABLE_API_URL             "https://rebrickable.com/api/v3/lego"
#define VER_LDVIEW_ARCHIVE_JSON_URL         VER_SOURCEFORGE_STR "ldview_renderers.json"
#define VER_LDGLITE_ARCHIVE_JSON_URL        VER_SOURCEFORGE_STR "ldglite_renderers.json"
#define VER_POVRAY_ARCHIVE_JSON_URL         VER_SOURCEFORGE_STR "povray_renderers.json"
#define VER_OFFICIAL_LIBRARY_JSON_URL       VER_SOURCEFORGE_STR "complete.json"
#define VER_UNOFFICIAL_LIBRARY_JSON_URL     VER_SOURCEFORGE_STR "lpub3dldrawunf.json"
#define VER_CHANGE_LOG_URL                  VER_SOURCEFORGE_STR "change_log.txt"
#ifdef QT_DEBUG_MODE
  #define VER_UPDATE_CHECK_JSON_URL         VER_SOURCEFORGE_STR "lpub3dupdates-test.json"
#else
  #define VER_UPDATE_CHECK_JSON_URL         VER_SOURCEFORGE_STR "lpub3dupdates.json"
#endif
#define VER_REBRICKABLE_KEYS_JSON_URL       VER_HOMEPAGE_GITHUB_STR "assets/resources/rebrickablekeys.json"
#define VER_LPUB3D_BLCODES_DOWNLOAD_URL     VER_HOMEPAGE_GITHUB_STR "assets/resources/codes.txt"
#define VER_LPUB3D_LIBRARY_BLCODES_URL      VER_LPUB3D_LIBRARIES_STR "codes.txt"
#define VER_LPUB3D_IMAGE_NOT_FOUND_URL      VER_HOMEPAGE_GITHUB_STR "assets/resources/missingimage.png"
#define VER_BLENDER_ADDON_URL               VER_BLENDER_ADDON_STR "releases/latest/download/" VER_BLENDER_ADDON_FILE
#define VER_BLENDER_ADDON_LATEST_URL        VER_BLENDER_ADDON_API_STR "releases/latest"
#define VER_BLENDER_ADDON_SHA_HASH_URL      VER_BLENDER_ADDON_URL ".sha256"

#ifdef Q_OS_MACOS
#define VER_HOMEBREW_PATH_PREFIX            "/usr/local/opt"
#define VER_HOMEBREW_PATH_INSERT            "PATH=/usr/local/Homebrew/bin:/opt/local/bin:/usr/local/bin:"
#endif

// ~~~~~~ LeoCAD Version ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define VER_LC_VERSION_TEXT                 "23.03"
#define VER_LC_VERSION_SHA                  "edad749"

// ~~~~~~ Build Parameters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#ifdef __GNUC__
  #ifdef __MINGW64__
    #define VER_COMPILED_ON             "MinGW (i686-7.2.0 64bit) Windows"
    #define VER_COMPILED_FOR            "MS Windows " VER_CHIPSET_STR " " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (MinGW " VER_BUILD_ARCH_STR "bit)"
    #define VER_IDE                     "Qt Creator 17.0.1 on Qt 6.9.1 (MSVC 2022, " VER_BUILD_ARCH_STR "bit)"
  #elif __MINGW32__
    #define VER_COMPILED_ON             "MinGW (i686-5.3.0 32bit) Windows"
    #define VER_COMPILED_FOR            "MS Windows " VER_CHIPSET_STR " " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (MinGW " VER_BUILD_ARCH_STR "bit)"
    #define VER_IDE                     "Qt Creator 17.0.1 on Qt 6.9.1 (MSVC 2022, " VER_BUILD_ARCH_STR "bit)"
  #elif __APPLE__
    #define VER_COMPILED_ON             "Clang (x86" VER_BUILD_ARCH_STR "bit) Apple"
    #define VER_COMPILED_FOR            "macOS " VER_CHIPSET_STR " " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (Clang " VER_BUILD_ARCH_STR "bit)"
    #define VER_IDE                     "QtQt Creator 14.0.1 on Qt 6.7.2 (Clang 13.0, x64)"
  #elif __linux__
    #define VER_COMPILED_ON             "GCC (9.1 20220914)"
    #define VER_COMPILED_FOR            "Linux " VER_CHIPSET_STR " " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (GCC)"
    #define VER_IDE                     "Qt Creator 9.0.0 on Qt 6.3.1 (GCC 9.1, x64)"
  #elif defined(_POSIX_VERSION)
    #define VER_COMPILED_ON             "GCC (9.1 20220914)"
    #define VER_COMPILED_FOR            "POSIX " VER_CHIPSET_STR " " VER_BUILD_ARCH_STR "bit"
    #define VER_COMPILED_WITH           "Qt qtver (GCC)"
    #define VER_IDE                     "Qt Creator 9.0.0 on Qt 6.3.1 (GCC 9.1, x64)"
  #else
    #error "Unknown compiler"
  #endif
#elif _MSC_VER
  #define VER_COMPILED_ON                "Micorsoft Visual C++ 2022"
  #define VER_COMPILED_FOR               "MS Windows " VER_CHIPSET_STR " " VER_BUILD_ARCH_STR "bit"
  #define VER_COMPILED_WITH              "Qt qtver (MSVC2022 " VER_BUILD_ARCH_STR "bit)"
  #define VER_IDE                        "Qt Creator 17.0.1 on Qt 6.9.1 (MSVC 2022, " VER_BUILD_ARCH_STR "bit)"
#endif

#endif // VERSION_H
