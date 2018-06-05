TEMPLATE = lib
QT      -= core
QT      -= gui
CONFIG  += qt
CONFIG  += staticlib
CONFIG  += warn_on

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += concurrent
}

TARGET +=
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../mainApp

# fine-grained host identification
win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

# The ABI version.
VER_MAJ = 4
VER_MIN = 3
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

# platform switch
BUILD_ARCH   = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH  = 64
    win32:!isEmpty($$(LP3D_QT64_MINGW_LIB)):!isEmpty($$(LP3D_QT64_MINGW_INC)) {
        LIBS += -L$$system_path($$(LP3D_QT64_MINGW_LIB))
        HEADERS += $$system_path($$(LP3D_QT64_MINGW_INC)/winsock2.h)
    }
} else {
    ARCH  = 32
    win32:!isEmpty($$(LP3D_QT32_MINGW_LIB)):!isEmpty($$(LP3D_QT32_MINGW_INC)) {
        LIBS += -L$$system_path($$(LP3D_QT32_MINGW_LIB))
        HEADERS += $$system_path($$(LP3D_QT32_MINGW_INC)/winsock2.h)
    }
}

# USE GNU_SOURCE
unix:!macx: DEFINES += _GNU_SOURCE

QMAKE_CXXFLAGS  += $(Q_CXXFLAGS)
QMAKE_LFLAGS    += $(Q_LDFLAGS)
QMAKE_CFLAGS    += $(Q_CFLAGS)

# Library defines
DEFINES += _QT
DEFINES += TIXML_USE_STL
DEFINES += USE_CPP11 _NO_BOOST
DEFINES += GL2PS_HAVE_ZLIB GL2PS_HAVE_LIBPNG

# USE CPP 11
contains(DEFINES, USE_CPP11) {
    unix:!freebsd:!macx {
        GCC_VERSION = $$system(g++ -dumpversion)
        greaterThan(GCC_VERSION, 4.6) {
            QMAKE_CXXFLAGS += -std=c++11
        } else {
            QMAKE_CXXFLAGS += -std=c++0x
        }
    } else {
        CONFIG += c++11
    }
}

# Merge ini files
merge_ini.target = LDViewMessages.ini
merge_ini.depends = $$_PRO_FILE_PWD_/LDLib/LDViewMessages.ini $$_PRO_FILE_PWD_/LDExporter/LDExportMessages.ini

win32 {

    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
    DEFINES += _TC_STATIC
    CONFIG  += windows
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
    QMAKE_EXT_OBJ = .obj

    merge_ini.commands = COPY /y /a /b \
                        $$system_path($$_PRO_FILE_PWD_/LDLib/LDViewMessages.ini) + \
                        $$system_path($$_PRO_FILE_PWD_/LDExporter/LDExportMessages.ini) \
                        $$system_path($$_PRO_FILE_PWD_/LDViewMessages.ini)

    LIBS += -ladvapi32 -lshell32 -lopengl32 -luser32
    LIBS += -lglu32 -lgdi32 -lws2_32

} else {

    merge_ini.commands = cat $$_PRO_FILE_PWD_/LDLib/LDViewMessages.ini \
                             $$_PRO_FILE_PWD_/LDExporter/LDExportMessages.ini > \
                             $$_PRO_FILE_PWD_/LDViewMessages.ini

    macx {                                                             # OSX
            INCLUDEPATH += /usr/X11/include
            LIBS        += -framework CoreFoundation -framework CoreServices
            LIBS        += -L/usr/X11/lib
    }
    LIBS  += -lGL -lGLU
}

QMAKE_EXTRA_TARGETS += merge_ini
PRE_TARGETDEPS += LDViewMessages.ini
QMAKE_CLEAN += LDViewMessages.ini

CONFIG += skip_target_version_ext

unix: !macx: TARGET = ldv
else:        TARGET = LDV

# Indicate build type
staticlib {
    BUILD    = Static
} else {
    BUILD    = Shared
}

CONFIG(debug, debug|release) {
    BUILD += Debug Build
    ARCH_BLD = bit_debug
    macx: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d$${VER_MAJ}$${VER_MIN})
    unix:!macx: TARGET = $$join(TARGET,,,d)
} else {
    BUILD += Release Build
    ARCH_BLD = bit_release
    win32: TARGET = $$join(TARGET,,,$${VER_MAJ}$${VER_MIN})
}
DESTDIR = $$join(ARCH,,,$$ARCH_BLD)
message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui

unix:INCLUDEPATH += /usr/include /usr/local/include

unix:exists(/usr/include/png.h)|exists(/usr/local/include/png.h){
    if (contains(HOST, Ubuntu):contains(HOST, 14.04.5)) {
        message("~~~ $$HOST detected, building in libpng version 1.6.28... ~~~")
        include(libpng/libpng.pri)
    } else {
        message("~~~ Library libpng found ~~~")
        BLD_LIBS += -lpng
    }
} else {
    !win32:message("~~~ ALERT: library libpng not found, building it in... ~~~")
    include(libpng/libpng.pri)
}
unix:exists(/usr/include/jpeglib.h)|exists(/usr/local/include/jpeglib.h){
    message("~~~ Library jpeglib found ~~~")
    BLD_LIBS += -ljpeg
} else {
    !win32:message("~~~ ALERT: library jpeglib not found, building it in... ~~~")
    include(libjpeg/libjpeg.pri)
}
unix:exists(/usr/include/gl2ps.h)|exists(/usr/local/include/gl2ps.h){
    message("~~~ Library gl2ps found ~~~")
    BLD_LIBS += -lgl2ps
} else {
    !win32:message("~~~ ALERT: library gl2ps not found, building it in... ~~~")
    include(gl2ps/gl2ps.pri)
}
unix:exists(/usr/include/tinyxml.h)|exists(/usr/local/include/tinyxml.h){
    message("~~~ Library tinyxml found ~~~")
    BLD_LIBS += -ltinyxml
} else {
    !win32:message("~~~ ALERT: library tinyxml not found, building it in... ~~~")
    include(tinyxml/TinyXml.pri)
}

!win32:BLD_LIBS += -lz

LIBS  += $${BLD_LIBS}

include(TRE/TRE.pri)
include(LDLib/LDLib.pri)
include(LDLoader/LDLoader.pri)
include(LDExporter/LDExporter.pri)
include(TCFoundation/TCFoundation.pri)
include(../LPub3DPlatformSpecific.pri)

# suppress warnings
QMAKE_CFLAGS_WARN_ON += \
     -Wall -W \
     -Wno-unknown-pragmas \
     -Wno-unused-parameter \
     -Wno-parentheses \
     -Wno-unused-variable \
     -Wno-deprecated-declarations \
     -Wno-return-type \
     -Wno-sign-compare \
     -Wno-uninitialized \
     -Wno-unused-result \
     -Wno-implicit-fallthrough
macx {
QMAKE_CFLAGS_WARN_ON += \
     -Wno-implicit-function-declaration \
     -Wno-incompatible-pointer-types-discards-qualifiers \
     -Wno-incompatible-pointer-types \
     -Wno-nullability-completeness \
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
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}

