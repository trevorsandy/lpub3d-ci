#!/bin/bash
#
# Download, extract and patch Qt5 library
# This script is used exclusively on Open Build Service
# for platforms that do not have Qt5 - e.g. CentOS 6, RHEL 6/7...
#
# This script is sourced on 2 occasions:
# 1. to extract and bin-patch the Qt assets,and
# 2. to deploy the Qt assets
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: January 19, 2018
#  Copyright (c) 2017 - 2018 by Trevor SANDY
#
# sample command [call from root build directory - e.g. lpub3d/]:
# chmod a+x builds/linux/obs/alldeps/GetQt5Libs.sh && ./builds/linux/obs/alldeps/GetQt5Libs.sh

# Set working directory and qt dir path
LP3D_OBS_WD=$PWD && echo "Entering Qt5 working directory..."
LP3D_QT_WD=$(cd ../ && echo $PWD)
LP3D_QT5_DIR="qt5/5.9.3/gcc_64"
LP3D_QT5_DIR_PATH=$LP3D_QT_WD/$LP3D_QT5_DIR

# Extract Qt library
cd $LP3D_QT_WD && [ ! -d "$LP3D_QT5_DIR" ] && \
echo "Extracting tarball qt5-5.9.3-gcc_64-el.tar.gz..." && \
tar -xzf qt5-5.9.3-gcc_64-el.tar.gz || true

# Set QtBinPatcher command options
[ "$RPM_STAGE" = "build" ] && patchopts="--verbose --nobackup" || \
patchopts="--verbose --nobackup --qt-dir=$PWD/$LP3D_QT5_DIR/bin --new-dir=$RPM_LIBDIR"

# Run QtBinPatcher - export QT5_BIN path
[ -f "$PWD/$LP3D_QT5_DIR/bin/qmake" ] && \
cd $LP3D_QT5_DIR_PATH/bin && ./qtbinpatcher $patchopts &&
export LP3D_QT5_BIN=$PWD && export LP3D_QT5_DIR_PATH || \
echo "ERROR - Could not run QtBinPatcher"

# Generate .conf files - set during build stage
if [ "$RPM_STAGE" = "build" ]; then
  LP3D_QTCONF_FILE="$LP3D_OBS_WD/mainApp/qt.conf"
  LP3D_QTQRC_FILE="$LP3D_OBS_WD/mainApp/lpub3d.qrc"
  LP3D_QTLDCONF_FILE="$LP3D_OBS_WD/mainApp/lpub3d-qtlibs.conf"

  # create qt.conf and connect it to lpub3d.qrc
  cat <<EOF >$LP3D_QTCONF_FILE
[Paths]
Prefix=$RPM_LIBDIR/lpub3dlib/qt5
Libraries=lib
Plugins=plugins
EOF
  sed -i '/<\/qresource>/a <qresource prefix="\/qt\/etc"><file alias="qt.conf">qt.conf<\/file><\/qresource>' "$LP3D_QTQRC_FILE"

  # create lpub3d-qtlibs.conf
  cat <<EOF >$LP3D_QTLDCONF_FILE
$RPM_LIBDIR/lpub3dlib/qt5/lib
EOF
  echo "DEBUG-DEBUG..." && cat $LP3D_QTLDCONF_FILE
fi

# Update ld_library_path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LP3D_QT5_DIR_PATH/bin

# Restore working directory
cd $LP3D_OBS_WD && echo "Return to working directory: $LP3D_OBS_WD"
