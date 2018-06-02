TEMPLATE = lib
CONFIG += qt warn_on
QT -= gui

# The ABI version.
!win32:VERSION = 2.6.2

win32 {

    QMAKE_EXT_OBJ = .obj
    CONFIG += windows
    CONFIG += debug_and_release	
}

TARGET = tinyxml

CONFIG += shared
CONFIG(debug, debug|release) {
        DESTDIR = build/debug
} else {
        DESTDIR = build/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc

# Input
include(tinyxml.pri)
include(../LPub3DPlatformSpecific.pri)



