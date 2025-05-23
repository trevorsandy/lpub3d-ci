######################################################################
# Automatically generated by qmake (2.01a) Sat 3. Jun 19:47:13 2017
######################################################################

TEMPLATE = lib
TARGET   = TCFoundation
QT      -= core
QT      -= opengl
QT      -= gui
CONFIG  -= qt
CONFIG  -= opengl
CONFIG  += thread
CONFIG  += warn_on
CONFIG  += staticlib
contains(DEFINES, _QT) {
QT      += core
QT      += opengl
CONFIG  += qt
CONFIG  += opengl
}

include(../LDViewGlobal.pri)

TARGET   = TCFoundation$$POSTFIX

message("~~~ lib$${TARGET}.$${EXT_S} LIBRARY - $$BUILD ~~~")

DEFINES     += HAVE_MINIZIP

INCLUDEPATH  = . .. $${LIBS_INC}

# Input
HEADERS += $$PWD/ConvertUTF.h \
           $$PWD/mystring.h \
           $$PWD/TCAlert.h \
           $$PWD/TCAlertManager.h \
           $$PWD/TCAlertSender.h \
           $$PWD/TCArray.h \
           $$PWD/TCAutoreleasePool.h \
           $$PWD/TCBmpImageFormat.h \
           $$PWD/TCDefines.h \
           $$PWD/TCDictionary.h \
           $$PWD/TCImage.h \
           $$PWD/TCImageFormat.h \
           $$PWD/TCImageOptions.h \
           $$PWD/TCJpegImageFormat.h \
           $$PWD/TCJpegOptions.h \
           $$PWD/TCLocalStrings.h \
           $$PWD/TCMacros.h \
           $$PWD/TCNetwork.h \
           $$PWD/TCNetworkClient.h \
           $$PWD/TCObject.h \
           $$PWD/TCObjectArray.h \
           $$PWD/TCPngImageFormat.h \
           $$PWD/TCProgressAlert.h \
           $$PWD/TCSortedStringArray.h \
           $$PWD/TCStlIncludes.h \
           $$PWD/TCStringArray.h \
           $$PWD/TCTypedDictionary.h \
           $$PWD/TCTypedObjectArray.h \
           $$PWD/TCTypedPointerArray.h \
           $$PWD/TCTypedValueArray.h \
           $$PWD/TCUnzip.h \
           $$PWD/TCUnzipStream.h \
           $$PWD/TCUserDefaults.h \
           $$PWD/TCVector.h \
           $$PWD/TCWebClient.h
SOURCES += $$PWD/ConvertUTF.c \
           $$PWD/mystring.cpp \
           $$PWD/STB.cpp \
           $$PWD/TCAlert.cpp \
           $$PWD/TCAlertManager.cpp \
           $$PWD/TCArray.cpp \
           $$PWD/TCAutoreleasePool.cpp \
           $$PWD/TCBmpImageFormat.cpp \
           $$PWD/TCDictionary.cpp \
           $$PWD/TCImage.cpp \
           $$PWD/TCImageFormat.cpp \
           $$PWD/TCImageOptions.cpp \
           $$PWD/TCJpegImageFormat.cpp \
           $$PWD/TCJpegOptions.cpp \
           $$PWD/TCLocalStrings.cpp \
           $$PWD/TCNetwork.cpp \
           $$PWD/TCNetworkClient.cpp \
           $$PWD/TCObject.cpp \
           $$PWD/TCObjectArray.cpp \
           $$PWD/TCPngImageFormat.cpp \
           $$PWD/TCProgressAlert.cpp \
           $$PWD/TCSortedStringArray.cpp \
           $$PWD/TCStringArray.cpp \
           $$PWD/TCUnzip.cpp \
           $$PWD/TCUnzipStream.cpp \
           $$PWD/TCUserDefaults.cpp \
           $$PWD/TCVector.cpp \
           $$PWD/TCWebClient.cpp
