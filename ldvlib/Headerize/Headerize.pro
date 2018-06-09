TEMPLATE         = app
CONFIG          += qt
win32: CONFIG   += console

TARGET          = Headerize

include(../ldvlib.pri)

message("~~~ HEADERIZE EXECUTABLE $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

H_VER_MAJ = 4
H_VER_MIN = 3


unix: !macx: DEP_TARGET = tcfoundation
else:        DEP_TARGET = TCFoundation

CONFIG(debug, debug|release) {
    macx: DEP_TARGET = $$join(DEP_TARGET,,,_debug)
    win32: DEP_TARGET = $$join(DEP_TARGET,,,d$${H_VER_MAJ}$${H_VER_MIN})
    unix:!macx: DEP_TARGET = $$join(DEP_TARGET,,,d)
} else {
    ARCH_BLD = bit_release
    win32: DEP_TARGET = $$join(DEP_TARGET,,,$${H_VER_MAJ}$${H_VER_MIN})
}

DEP_DESTDIR = $$join(ARCH,,,$$ARCH_BLD)

exists($$system_path(../TCFoundation/$$DEP_DESTDIR/lib$${DEP_TARGET}.a)) {
    message("~~~ HEADERIZE DEPENDENCY lib$${DEP_TARGET}.a FOUND ~~~")
}

LIBDIRS         = -L$$system_path(../TCFoundation/$$DEP_DESTDIR)

LDLIBS          = $$system_path(../TCFoundation/$$DEP_DESTDIR/lib$${DEP_TARGET}.a)

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


