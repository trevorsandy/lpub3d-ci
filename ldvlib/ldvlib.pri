INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
DEFINES += _TC_STATIC
win32:LIBS += -lws2_32  # Winsock2

include(libjpeg/libjpeg.pri)
include(tinyxml/TinyXml.pri)
include(libpng/libpng.pri)
include(gl2ps/gl2ps.pri)
include(stb/stb.pri)

include(TRE/TRE.pri)
include(LDLib/LDLib.pri)
include(LDLoader/LDLoader.pri)
include(LDExporter/LDExporter.pri)
include(TCFoundation/TCFoundation.pri)
