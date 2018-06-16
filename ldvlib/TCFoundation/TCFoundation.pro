TEMPLATE = lib
QT      += core
QT 	+= opengl
CONFIG  += staticlib

unix: !macx: TARGET = tcfoundation
else:        TARGET = TCFoundation

# The ABI version.
VER_MAJ = 4
VER_MIN = 3
VER_PAT = 0
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

include(../ldvlib.pri)

if (macx:|win32:) {
DEFINES += HAVE_MINIZIP
} else {
DEFINES += UNZIP_CMD
}

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

include(TCFoundation.pri)

# suppress warnings
QMAKE_CFLAGS_WARN_ON += \
     -Wall -W \
     -Wno-unknown-pragmas \
     -Wno-unused-parameter \
     -Wno-parentheses \
     -Wno-unused-variable \
     -Wno-deprecated-declarations \
     -Wno-return-type \
     -Wno-sign-compare \
     -Wno-uninitialized \
     -Wno-unused-result \
     -Wno-implicit-fallthrough
macx {
QMAKE_CFLAGS_WARN_ON += \
     -Wno-implicit-function-declaration \
     -Wno-incompatible-pointer-types-discards-qualifiers \
     -Wno-incompatible-pointer-types \
     -Wno-nullability-completeness \
     -Wno-undefined-bool-conversion \
     -Wno-invalid-source-encoding \
     -Wno-mismatched-new-delete \
     -Wno-for-loop-analysis \
     -Wno-int-conversion \
     -Wno-reorder
} else {
QMAKE_CFLAGS_WARN_ON += \
     -Wno-clobbered
}
QMAKE_CXXFLAGS_WARN_ON += $${QMAKE_CFLAGS_WARN_ON}
