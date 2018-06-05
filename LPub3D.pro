TEMPLATE=subdirs
CONFIG += ordered # This tells Qt to compile the following SUBDIRS in order
SUBDIRS += \
     $$PWD/quazip \
     $$PWD/ldrawini \
     $$PWD/lclib \
     $$PWD/ldvlib \
     $$PWD/ldvlib/Headerize \
     $$PWD/mainApp

quazip.subdir = $$PWD/quazip
quazip.target = sub-quazip
quazip.depends =

ldrawini.subdir = $$PWD/ldrawini
ldrawini.target = sub-ldrawini
ldrawini.depends =

lclib.subdir = $$PWD/lclib
lclib.target = sub-lclib
lclib.depends =

ldvlib.subdir = $$PWD/ldvlib
ldvlib.target = sub-ldvlib
ldvlib.depends =

headerize.subdir = $$PWD/ldvlib/Headerize
headerize.target = sub-headerize
headerize.depends = ldvlib

mainApp.subdir = $$PWD/mainApp
mainApp.target = sub-mainApp
mainApp.depends = quazip
mainApp.depends = ldrawini
mainApp.depends = lclib
mainApp.depends = ldvlib
mainApp.depends = headerize

RESOURCES += \
    qsimpleupdater/res/qsu_resources.qrc \
    mainApp/lpub3d.qrc

win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

BUILD_ARCH = $$(TARGET_CPU)
!contains(QT_ARCH, unknown):  BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH):    BUILD_ARCH = UNKNOWN ARCH
CONFIG(debug, debug|release): BUILD = DEBUG BUILD
else:	                        BUILD = RELEASE BUILD
message("~~~ LPUB3D $$upper($$BUILD_ARCH) $${BUILD} ON $$upper($$HOST) ~~~")
