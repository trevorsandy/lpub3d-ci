TEMPLATE = lib
CONFIG  += staticlib

unix: !macx: TARGET = gl2ps
else:        TARGET = GL2PS

# The ABI version.
VER_MAJ = 1
VER_MIN = 3
VER_PAT = 3
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

include(../ldvlib.pri)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

DEFINES += GL2PS_HAVE_ZLIB \
           GL2PS_HAVE_LIBPNG

INCLUDEPATH += $$_PRO_FILE_PWD_/../libpng
macx: INCLUDEPATH += $$_PRO_FILE_PWD_/../libpng/MacOSX

# use libpng14 or higher. Ubunu's default is libpng12
CONFIG(debug, debug|release) {
    macx:PNG_LIB       = PNG_debug
    win32:PNG_LIB      = PNGd16
    unix:!macx:PNG_LIB = pngd
} else {
    macx::PNG_LIB      = PNG
    win32:PNG_LIB      = PNG16
    unix:!macx:PNG_LIB = png
}

PNG_LIBDIR  = ../libpng/$$DESTDIR
win32: PNG_COPY_CMD = CD $${PNG_LIBDIR} && IF NOT EXIST \"libpng.a\" COPY lib$${PNG_LIB}.a libpng.a
else:  PNG_COPY_CMD = cd $${PNG_LIBDIR}; if ! test -f libpng.a; then ln -s lib$${PNG_LIB}.a libpng.a; fi
copypng.target = lib$${PNG_LIB}.a
copypng.depends = $${PNG_LIBDIR}/lib$${PNG_LIB}.a
copypng.commands = $${PNG_COPY_CMD}
QMAKE_EXTRA_TARGETS += copypng
PRE_TARGETDEPS += lib$${PNG_LIB}.a
LIBS += -L$${PNG_LIBDIR} $${PNG_LIBDIR}/lib$${PNG_LIB}.a

# Input
HEADERS += $$PWD/gl2ps.h
SOURCES += $$PWD/gl2ps.c

# suppress warnings
QMAKE_CFLAGS_WARN_ON =  \
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
    -Wno-unused-but-set-variable

QMAKE_CXXFLAGS_WARN_ON = $${QMAKE_CFLAGS_WARN_ON}

QMAKE_CFLAGS_WARN_ON +=  \
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
}
