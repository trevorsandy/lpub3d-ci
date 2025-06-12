TEMPLATE = lib
TARGET   = WaitingSpinner
QT      += core
QT      += widgets
CONFIG  += qt warn_on
CONFIG  += staticlib
CONFIG  += skip_target_version_ext

# The ABI version.
VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

# common directives
include(../common.pri)

win32 {
    QMAKE_TARGET_COMPANY = "Alex Turkin"
    QMAKE_TARGET_DESCRIPTION = "Qt Waiting Spinner Widget"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2015 Alex Turkin"
    QMAKE_TARGET_PRODUCT = "$${TARGET} ($$join(ARCH,,,bit))"

    QMAKE_EXT_OBJ = .obj
}

if (unix|msys):!macx: TARGET = $$lower($$TARGET)

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

# Input files
include(WaitingSpinner.pri)
