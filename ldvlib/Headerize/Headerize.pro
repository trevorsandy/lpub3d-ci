TEMPLATE         = app
QT              += core
QT              -= gui
CONFIG          += qt
CONFIG          += warn_on
win32: CONFIG   += console
macx:  CONFIG   -= app_bundle

TARGET +=
DEPENDPATH += .
INCLUDEPATH += .

message("~~~ HEADERIZE EXECUTABLE ~~~")

V_MAJ = 4
V_MIN = 3

BUILD_ARCH = $$(TARGET_CPU)
if (contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64)|contains(BUILD_ARCH, aarch64)) {
    ARCH  = 64
} else {
    ARCH  = 32
}

unix: !macx: DEP_TARGET = ldv
else:        DEP_TARGET = Ldv

CONFIG(debug, debug|release) {
    BUILD += Debug Build
    ARCH_BLD = bit_debug
    macx: DEP_TARGET = $$join(DEP_TARGET,,,_debug)
    win32: DEP_TARGET = $$join(DEP_TARGET,,,d$${V_MAJ}$${V_MIN})
    unix:!macx: DEP_TARGET = $$join(DEP_TARGET,,,d)
} else {
    BUILD += Release Build
    ARCH_BLD = bit_release
    win32: DEP_TARGET = $$join(DEP_TARGET,,,$${V_MAJ}$${V_MIN})
}

DEP_DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

exists($$system_path(../$$DEP_DESTDIR/lib$${DEP_TARGET}.a)) {
    message("~~~ HEADERIZE DEPENDENCY lib$${DEP_TARGET}.a FOUND ~~~")
}

DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

PRECOMPILED_DIR = $$DESTDIR/.pch
OBJECTS_DIR     = $$DESTDIR/.obj
MOC_DIR         = $$DESTDIR/.moc
RCC_DIR         = $$DESTDIR/.qrc
UI_DIR          = $$DESTDIR/.ui


INCLUDEPATH     += $$_PRO_FILE_PWD_/../

QMAKE_CXXFLAGS  += $(Q_CXXFLAGS)
QMAKE_LFLAGS    += $(Q_LDFLAGS)
QMAKE_CFLAGS    += $(Q_CFLAGS)

win32 {
    DEFINES      += _TC_STATIC
    QMAKE_EXT_OBJ = .obj
}

DEFINES        += _QT

TARGET          = Headerize

LIBDIRS         = -L$$system_path(../$$DEP_DESTDIR)

LDLIBS          = $$system_path(../$$DEP_DESTDIR/lib$${DEP_TARGET}.a)

LIBS           += $${LIBDIRS} $${LDLIBS}

PRE_TARGETDEPS += $${LDLIBS}

HEADERS += \
    $$PWD/../TCFoundation/mystring.h

SOURCES += \
    $$PWD/Headerize.cpp

# suppress warnings
QMAKE_CFLAGS_WARN_ON   = -Wall -W \
                         -Wno-unknown-pragmas \
                         -Wno-implicit-fallthrough
QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}


