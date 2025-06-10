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
    DEFINES += \
        _CRT_SECURE_NO_WARNINGS \
        _CRT_SECURE_NO_DEPRECATE=1 \
        _CRT_NONSTDC_NO_WARNINGS=1
    QMAKE_CXXFLAGS_RELEASE += \
        /FI winsock2.h /FI winsock.h \
        /wd4675
    QMAKE_LFLAGS += \
    -Wl,--allow-multiple-definition
}

OBJECTS_DIR   = $${DESTDIR}/.obj
win32: \
QMAKE_EXT_OBJ = .obj

# suppress warnings
unix|msys {
QMAKE_CFLAGS_WARN_ON = \
                     -Wall -W \
                     -Wno-unused-parameter \
                     -Wno-parentheses \
                     -Wno-unused-variable \
                     -Wno-deprecated-declarations \
                     -Wno-return-type \
                     -Wno-sign-compare \
                     -Wno-uninitialized \
                     -Wno-format \
                     -Wno-switch \
                     -Wno-comment \
                     -Wno-unused-result \
                     -Wno-unused-but-set-variable \
                     -Wno-unused-variable \
                     -Wno-implicit-fallthrough \
                     -Wno-misleading-indentation \
                     -Wno-format-security                     

QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}

QMAKE_CFLAGS_WARN_ON += \
                     -Wno-implicit-function-declaration \
                     -Wno-incompatible-pointer-types
macx {
QMAKE_CFLAGS_WARN_ON += \
                     -Wno-incompatible-pointer-types-discards-qualifiers \
                     -Wno-undefined-bool-conversion \
                     -Wno-invalid-source-encoding \
                     -Wno-mismatched-new-delete \
                     -Wno-for-loop-analysis \
                     -Wno-int-conversion \
                     -Wno-reorder
} else {
QMAKE_CFLAGS_WARN_ON += \
                     -Wno-calloc-transposed-args
}
} # unix|msys
