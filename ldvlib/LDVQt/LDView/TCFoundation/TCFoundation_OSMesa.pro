# Build OSMesa CUI
contains(DEFINES, _QT): \
DEFINES -= _QT
DEFINES += _OSMESA
include(TCFoundation.pri)
