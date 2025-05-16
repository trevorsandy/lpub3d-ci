contains(DEFINES, _QT): \
DEFINES       -= _QT
contains(DEFINES, _OSMESA): \
DEFINES       -= _OSMESA
include(LDExporter.pri)
