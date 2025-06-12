# 3rd party library directives

#requires(USE_THIRD_PARTY_LIBS)

TEMPLATE = lib
QT      -= core
QT      -= gui
CONFIG  -= qt
CONFIG  += warn_on
CONFIG  += staticlib

# common directives
COMMON = LDView
include($${PWD}/../../../common.pri)

# build type
CONFIG(debug, debug|release) {
    BUILD = DEBUG
    DESTDIR = $$join(ARCH,,,bit_debug)
} else {
    BUILD = RELEASE
    DESTDIR = $$join(ARCH,,,bit_release)
}
BUILD += BUILD ON $$upper($$HOST)

INCLUDEPATH += $${PWD}
!equals(TARGET, z) {
    !USE_SYSTEM_ZLIB {
        INCLUDEPATH  += $${_PRO_FILE_PWD_}/../zlib
    } else {
        INCLUDEPATH  += $${_PRO_FILE_PWD_}/../../include   # for zlib.h and zconf.h
    }
}

# USE GNU_SOURCE
unix|msys:!macx: \
DEFINES += _GNU_SOURCE

win32-msvc* {
    QMAKE_CXXFLAGS_RELEASE += \
        /FI winsock2.h /FI winsock.h \
        /wd4675
    QMAKE_LFLAGS += \
    -Wl,--allow-multiple-definition
}

OBJECTS_DIR   = $${DESTDIR}/.obj
win32: \
QMAKE_EXT_OBJ = .obj
