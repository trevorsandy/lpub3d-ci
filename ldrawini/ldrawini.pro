TEMPLATE = lib
CONFIG += qt warn_on
QT -= gui

# The ABI version.
# Version format is year.month.day.patch
win32: VERSION = 16.1.8.0  # major.minor.patch.build
else: VERSION = 16.1.8     # major.minor.patch

contains(QT_ARCH, x86_64) {
    ARCH = 64
    STG_ARCH = x86_64
} else {
    ARCH = 32
    STG_ARCH = x86
}

win32 {

    QMAKE_EXT_OBJ = .obj
    CONFIG += windows

    QMAKE_TARGET_COMPANY = "Lars C. Hassing"
    QMAKE_TARGET_DESCRIPTION = "LDrawDir and SearchDirs API"
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2004-2008  Lars C. Hassing"
    QMAKE_TARGET_PRODUCT = "LDrawIni ($$join(ARCH,,,bit))"
}

macx {
    QMAKE_CXXFLAGS += -F/System/Library/Frameworks
    LIBS += -framework CoreFoundation
}

CONFIG += skip_target_version_ext
unix:!macx: TARGET = ldrawini
else: TARGET = LDrawIni

# Indicate build type,
# be sure to add CONFIG+=staticlib in Additional Arguments of qmake build steps
staticlib: BUILD = Static
else:      BUILD = Shared

CONFIG(debug, debug|release) {
    DESTDIR = $$join(ARCH,,,bit_debug)
    BUILD += Debug Build
    macx: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,d161)
    unix:!macx: TARGET = $$join(TARGET,,,d)
} else {
    DESTDIR = $$join(ARCH,,,bit_release)
    BUILD += Release Build
    win32: TARGET = $$join(TARGET,,,161)
}
message("~~~ LDRAWINI $$join(ARCH,,,bit) $${BUILD} ~~~")

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
QMAKE_EXT_CPP = .c

# Input
include(ldrawini.pri)
include(../LPub3DPlatformSpecific.pri)

unix {
    isEmpty(PREFIX):PREFIX = /usr
    headers.path=$$PREFIX/include
    headers.files=$$HEADERS
    deb {
        target.path=$$PREFIX/lib/$$QT_ARCH-linux-gnu
        message("~~~ LDRAWINI DEB $$join(ARCH,,,bit) LIB ~~~")
    }
    rpm {
        equals (ARCH, 64) {
            target.path=$$PREFIX/lib$$ARCH
            message("~~~ LDRAWINI RPM $$join(ARCH,,,bit) LIB ~~~")
        } else {
            target.path=$$PREFIX/lib
            message("~~~ LDRAWINI RPM 32bit LIB ~~~")
        }
    }
    !deb:!rpm {
        target.path=$$PREFIX/lib
        message("~~~ LDRAWINI $$join(ARCH,,,bit) LIB ~~~")
    }
    INSTALLS += target
    libheaders: INSTALLS += headers
    libheaders: message("~~~ INSTALL LDRAWINI LIB HEADERS ~~~")

}

# Suppress warnings
QMAKE_CFLAGS_WARN_ON += -Wall -W \
    -Wno-sign-compare
QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}
