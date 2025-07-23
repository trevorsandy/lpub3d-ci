TEMPLATE = lib
TARGET   = QuaZIP
CONFIG   += qt warn_on
QT       -= gui
CONFIG   += staticlib
CONFIG   += skip_target_version_ext

# 1.0.0 is the first stable ABI.
# The next binary incompatible change will be 2.0.0 and so on.
# The existing QuaZIP policy on changing ABI requires to bump the
# major version of QuaZIP itself as well. Note that there may be
# other reasons for chaging the major version of QuaZIP, so
# in case where there is a QuaZIP major version bump but no ABI change,
# the VERSION variable will stay the same.

# For example:

# QuaZIP 1.0 is released after some 0.x, keeping binary compatibility.
# VERSION stays 1.0.0.
# Then some binary incompatible change is introduced. QuaZIP goes up to
# 2.0, VERSION to 2.0.0.
# And so on.

# The ABI version.
VER_MAJ = 1
VER_MIN = 4
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

# common directives
include(../common.pri)


win32 {
    QMAKE_TARGET_COMPANY = "Sergey A. Tachenov"
    QMAKE_TARGET_DESCRIPTION = "C++ wrapper over Gilles Vollant's ZIP/UNZIP"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2005-2014 Sergey A. Tachenov"
    QMAKE_TARGET_PRODUCT = "$${TARGET} ($$join(ARCH,,,bit))"

    QMAKE_EXT_OBJ = .obj

    win32-arm64-msvc|win32-msvc* {
        INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
    } else {
        LIBS += -lz
    }
} else {
    LIBS += -lz
}

if (unix|msys):!macx: TARGET = $$lower($$TARGET)

# You'll need to define this one manually if using a build system other
# than qmake or using QuaZIP sources directly in your project.

# Indicate build type
staticlib {
    BUILD    = Static
    DEFINES += QUAZIP_STATIC
    QMAKE_LFLAGS += -static
} else {
    BUILD    = Shared
    DEFINES += QUAZIP_BUILD
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

# Input files
include(quazip.pri)
