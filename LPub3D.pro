TEMPLATE=subdirs
CONFIG+=ordered # This tells Qt to compile the following SUBDIRS in order

win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

BUILD_ARCH = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
CONFIG(debug, debug|release): BUILD = DEBUG BUILD
else:	                      BUILD = RELEASE BUILD

message("~~~ LPUB3D $$upper($$BUILD_ARCH) $${BUILD} ON $$upper($$HOST) ~~~")

SUBDIRS = 3rdParty_ldrawini
3rdParty_ldrawini.subdir   = $$PWD/ldrawini
3rdParty_ldrawini.makefile = Makefile.ldrawini
3rdParty_ldrawini.target   = sub-ldrawini
3rdParty_ldrawini.depends  =

isEmpty(quazipnobuild) {
  SUBDIRS += 3rdParty_quazip
  3rdParty_quazip.subdir   = $$PWD/quazip
  3rdParty_quazip.makefile = Makefile.quazip
  3rdParty_quazip.target   = sub-quazip
  3rdParty_quazip.depends  =
}

if (unix:exists(/usr/include/tinyxml.h)|exists(/usr/local/include/tinyxml.h)) {
    message("~~~ Library tinyxml found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: library tinyxml not found, using local... ~~~")
}
if (unix:exists(/usr/include/gl2ps.h)|exists(/usr/local/include/gl2ps.h)) {
    message("~~~ Library gl2ps found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: library gl2ps not found, using local ~~~")
}
if (unix:exists(/usr/include/lib3ds.h)|exists(/usr/local/include/lib3ds.h)){
    message("~~~ Library 3ds found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: library 3ds not found, using local... ~~~")
}
if (unix:macx:exists(/usr/include/zip.h)|exists(/usr/local/include/minizip/zip.h)) {
    message("~~~ Library minizip found ~~~")
} else:macx:!win32-msvc* {
    message("~~~ ALERT: library minizip not found, using local... ~~~")
}
if (unix:exists(/usr/include/png.h)|exists(/usr/local/include/png.h)) {
    message("~~~ Library png found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: library png not found, using local... ~~~")
}
if (unix:exists(/usr/include/jpeglib.h)|exists(/usr/local/include/jpeglib.h)) {
    message("~~~ Library jpeglib found ~~~")
} else:!win32-msvc* {
    message("~~~ ALERT: library jpeg not found, using local... ~~~")
}

SUBDIRS += ldvqt
ldvqt.subdir   = $$PWD/ldvlib/LDVQt
ldvqt.makefile = Makefile.ldvqt
ldvqt.target   = sub-ldvqt
ldvqt.depends  = #ldlib

SUBDIRS += lclib
lclib.subdir   = $$PWD/lclib
lclib.makefile = Makefile.lc
lclib.target   = sub-lclib
lclib.depends  =

SUBDIRS += mainApp
mainApp.subdir   = $$PWD/mainApp
mainApp.makefile = Makefile.mainapp
mainApp.target   = sub-mainApp
mainApp.depends  = quazip
mainApp.depends  = ldrawini
mainApp.depends  = ldvqt

RESOURCES += \
    qsimpleupdater/etc/resources/qsimpleupdater.qrc \
    mainApp/lpub3d.qrc
