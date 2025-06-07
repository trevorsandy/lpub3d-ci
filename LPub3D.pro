# qmake Configuration settings
# CONFIG+=ldviewqt      # build LDVQt LDView libs using Qt OpenGL
# CONFIG+=ldviewwgl     # build LDVQt LDView libs using WGL OpenGL
# CONFIG+=ldviewosmesa  # build LDVQt LDView libs using OSMesa OpenGL

# LPub3D QMake directory and project file structre
# ------------------------------------------
# /LPub3D
#  |
#  |--- LPub3D.pro                 Subdirs project file
#  |--- gitversion.pri             Construct project version
#  |
#  `--- /lclib
#  |     |--- lclib.pro            Library project file - consumes lclib.pri
#  |     |--- lclib.pri            Library declarations and directives project include
#  |
#  `--- /ldrawini
#  |     |--- ldrawini.pri         Library declarations and directives project include
#  |     |--- Lldrawini.pro        Library project file - consumes ldrawini.pri
#  |
#  `--- /ldvlib
#  |     |
#  |     `--- /LDVQt
#  |     |     |--- LDVQtLibs.pri  Library declarations and directives project include
#  |     |     |--- LDVQt.pro      Library project file - consumes LDVQtLibs.pri
#  |     |
#  |     `--- /WPngImage
#  |           |--- WPngImage.pri  Library declarations and directives project include
#  |           |--- WPngImage.pro  Library project file - consumes WPngImage.pri
#  |
#  `--- /mainApp
#  |     |--- macosfiledistro.pri
#  |     |    |--- install3rdpartyassets.pri Project include
#  |     |
#  |     `--- posixfiledistro.pri
#  |     |    |--- install3rdpartyassets.pri Project include
#  |     |    |--- locallibsdistro.pri
#  |     |
#  |     |--- winfiledistro.pri
#  |     |    |--- stageassets.pri           Project include
#  |     |
#  |     |--- otherfiles.pri       Project include
#  |     |--- gitversion.pri       Project include
#  |     |--- mainApp.pri          Library declarations and directives project include
#  |     |--- LDVQtLibs.pri        Library declarations and directives project include
#  |     |--- QsLog.pri            Library declarations and directives project include
#  |     |--- QSimpleUpdater.pri   Library declarations and directives project include
#  |     |--- mainApp.pro          Library project file - consumes mainApp.pri, gitversion.pri and LDVQtLibs.pri
#  |
#  `--- /QSimpleUpdater
#  |     |--- QSimpleUpdater.pri   Library declarations and directives project include
#  |
#  `--- /qslog
#  |     |--- QsLog.pri            Library declarations and directives project include
#  |
#  `--- /quazip
#  |     |--- quazip.pri           Library declarations and directives project include
#  |     |--- quazip.pro           Library project file - consumes quazip.pri
#  |
#  `--- /waitingspinner
#  |     |--- WaitingSpinner.pri   Library declarations and directives project include
#  |     |--- waitingspinner.pro   Library project file - consumes waitingspinner.pri

TEMPLATE=subdirs
CONFIG+=ordered # This tells Qt to compile the following SUBDIRS in order

win32:HOST = $$system(systeminfo | findstr /B /C:\"OS Name\")
unix:!macx:HOST = $$system(. /etc/os-release 2>/dev/null; [ -n \"$PRETTY_NAME\" ] && echo \"$PRETTY_NAME\" || echo `uname`)
macx:HOST = $$system(echo `sw_vers -productName` `sw_vers -productVersion`)
isEmpty(HOST):HOST = UNKNOWN HOST

BUILD_ARCH = $$(TARGET_CPU)
!contains(QT_ARCH, unknown): \
BUILD_ARCH = $$QT_ARCH
else: isEmpty(BUILD_ARCH): \
BUILD_ARCH = UNKNOWN ARCH
CONFIG(debug, debug|release) {
    BUILD  = DEBUG BUILD
    LPUB3D = LPub3Dd
} else {
    BUILD  = RELEASE BUILD
    LPUB3D = LPub3D
}

ldviewqt: \
POSTFIX  = QT
else:msys:ldviewwgl: \
POSTFIX  = WGL
else:!win32-msvc*: \
POSTFIX  = OSMesa

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

SUBDIRS += ldvqt_$${POSTFIX}
ldvqt_$${POSTFIX}.subdir   = $$PWD/ldvlib/LDVQt
ldvqt_$${POSTFIX}.makefile = Makefile-ldvqt.$$lower($${POSTFIX})
ldvqt_$${POSTFIX}.target   = sub-ldvqt_$${POSTFIX}
ldvqt_$${POSTFIX}.depends  =

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
mainApp.depends  = ldvqt_$${POSTFIX}
mainApp.depends  = wpngimage
mainApp.depends  = waitingspinner

RESOURCES += \
    qsimpleupdater/etc/resources/qsimpleupdater.qrc \
    mainApp/lpub3d.qrc
