# This means build Qt GUI - it does not mean 'using Qt!!'
contains(DEFINES, _OSMESA): \
DEFINES -= _OSMESA
DEFINES	+= _QT
include(LDLib.pri)
