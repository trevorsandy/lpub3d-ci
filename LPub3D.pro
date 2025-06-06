TEMPLATE=subdirs
CONFIG+=ordered # This tells Qt to compile the following SUBDIRS in order

win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

BUILD_ARCH = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
CONFIG(debug, debug|release) {
                              BUILD  = DEBUG BUILD
                              LPUB3D = LPub3Dd
} else {
                              BUILD  = RELEASE BUILD
                              LPUB3D = LPub3D
}

message("~~~ $${LPUB3D} $$upper($$BUILD_ARCH) $${BUILD} ON $$upper($$HOST) ~~~")

SUBDIRS = ldrawini
ldrawini.subdir   = $$PWD/ldrawini
ldrawini.makefile = Makefile.ldrawini
ldrawini.target   = sub-ldrawini
ldrawini.depends  =

isEmpty(quazipnobuild) {
  SUBDIRS += quazip
  quazip.subdir   = $$PWD/quazip
  quazip.makefile = Makefile.quazip
  quazip.target   = sub-quazip
  quazip.depends  =
}

SUBDIRS += ldvqt
ldvqt.subdir   = $$PWD/ldvlib/LDVQt
ldvqt.makefile = Makefile.ldvqt
ldvqt.target   = sub-ldvqt
ldvqt.depends  = #ldlib

SUBDIRS += wpngimage
wpngimage.subdir   = $$PWD/ldvlib/WPngImage
wpngimage.makefile = Makefile.wpngimage
wpngimage.target   = sub-wpngimage
wpngimage.depends  =

SUBDIRS += lclib
lclib.subdir   = $$PWD/lclib
lclib.makefile = Makefile.lclib
lclib.target   = sub-lclib
lclib.depends  =

SUBDIRS += waitingspinner
waitingspinner.subdir   = $$PWD/waitingspinner
waitingspinner.makefile = Makefile.waitingspinner
waitingspinner.target   = sub-waitingspinner
waitingspinner.depends  =

SUBDIRS += mainApp
mainApp.subdir   = $$PWD/mainApp
mainApp.makefile = Makefile.mainapp
mainApp.target   = sub-mainapp
isEmpty(quazipnobuild): \
mainApp.depends  = quazip
mainApp.depends  = ldrawini
mainApp.depends  = lclib
mainApp.depends  = ldvqt
mainApp.depends  = wpngimage
mainApp.depends  = waitingspinner

RESOURCES += \
    qsimpleupdater/etc/resources/qsimpleupdater.qrc \
    mainApp/lpub3d.qrc
