TEMPLATE = lib
TARGET   = LDrawIni
QT      -= gui
CONFIG   += qt warn_on
CONFIG   += staticlib
CONFIG   += skip_target_version_ext

# The ABI version.
VER_MAJ = 16
VER_MIN = 1
VER_PAT = 8
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

# common directives
include(../common.pri)

win32-arm64-msvc|win32-msvc*: \
DEFINES += _WIN_UTF8_PATHS


win32 {
    QMAKE_TARGET_COMPANY = "Lars C. Hassing"
    QMAKE_TARGET_DESCRIPTION = "LDrawDir and SearchDirs API"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2004-2008  Lars C. Hassing"
    QMAKE_TARGET_PRODUCT = "$${TEMPLATE} ($$join(ARCH,,,bit))"

    QMAKE_EXT_OBJ = .obj
}

macx {
    QMAKE_CXXFLAGS += -F/System/Library/Frameworks
    LIBS += -framework CoreFoundation
}

if(unix|msys):!macx: TARGET = $$lower($$TARGET)

# Indicate build type
staticlib {
    BUILD    = Static
    QMAKE_LFLAGS += -static
} else {
    BUILD    = Shared
    msys: \
    CONFIG  -= staticlib
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

QMAKE_EXT_CPP = .c

# Input files
include(ldrawini.pri)
