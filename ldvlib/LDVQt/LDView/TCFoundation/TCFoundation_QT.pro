# This means build Qt GUI - it does not mean 'using Qt!!'
contains(DEFINES, _OSMESA): \
DEFINES -= _OSMESA
DEFINES	+= _QT
include(TCFoundation.pri)

MISC_HEADER = $$shell_path( $$absolute_path( $$_PRO_FILE_PWD_/../../LDVMisc.h ) )
UTILS_DIR = $$shell_path( $$absolute_path( $$_PRO_FILE_PWD_/../Utilities ) )
if (mingw:ide_qtcreator)|win32-msvc*: \
LINK_CMD = cd $${UTILS_DIR} & if not exist \"misc.h\" \( mklink misc.h $${MISC_HEADER} \)
else: \
LINK_CMD = cd $${UTILS_DIR}; if ! test -f misc.h; then ln -s $${MISC_HEADER} misc.h; fi
linkMiscHeader.target = linkMiscHeaderFile
linkMiscHeader.depends = $${MISC_HEADER}
linkMiscHeader.commands = $${LINK_CMD}
QMAKE_EXTRA_TARGETS += linkMiscHeader
PRE_TARGETDEPS += linkMiscHeaderFile

QMAKE_CLEAN += $$_PRO_FILE_PWD_/../Utilities/misc.h
