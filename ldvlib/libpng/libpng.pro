TEMPLATE = lib
CONFIG  += staticlib

unix: !macx: TARGET = png
else:        TARGET = PNG

# The ABI version.
VER_MAJ = 1
VER_MIN = 6
VER_PAT = 28
VER_BLD = 0
win32: VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT"."$$VER_BLD  # major.minor.patch.build
else: VERSION  = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT              # major.minor.patch

include(../ldvlib.pri)

message("~~~ lib$${TARGET} $$join(ARCH,,,bit) $$BUILD_ARCH $${BUILD} ~~~")

include(libpng.pri)

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
