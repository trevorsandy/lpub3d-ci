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
#  Last Update: January 18, 2018
#  Copyright (c) 2017 - 2018 by Trevor SANDY
#
# sample command [call from root build directory - e.g. lpub3d/]:
# chmod a+x builds/linux/obs/alldeps/GetQt5Libs.sh && ./builds/linux/obs/alldeps/GetQt5Libs.sh

# Set working directory and qt dir path
LP3D_OBS_WD=$PWD && echo "Entering Qt5 working directory..."
LP3D_QT_WD=$(cd ../ && echo $PWD) && cd $LP3D_QT_WD
LP3D_QT5_DIR="qt5/5.9.3/gcc_64"

# Extract Qt library
[ ! -d "$LP3D_QT5_DIR" ] && \
echo "Extracting tarball qt5-5.9.3-gcc_64-el.tar.gz..." && \
tar -xzf qt5-5.9.3-gcc_64-el.tar.gz || true

# Set QtBinPatcher command options
[ "$RPM_STAGE" = "build" ] && patchopts="--verbose --nobackup" || \
patchopts="--verbose --nobackup --qt-dir=$PWD/$LP3D_QT5_DIR/bin --new-dir=$RPM_LIBDIR"

# Run QtBinPatcher - export QT5_BIN path
[ -f "$PWD/$LP3D_QT5_DIR/bin/qmake" ] && \
cd $PWD/$LP3D_QT5_DIR/bin && ./qtbinpatcher $patchopts &&
export LP3D_QT5_BIN=$PWD || \
echo "ERROR - Could not run QtBinPatcher"

# Generate a qt.conf and connect it to lpub3d.qrc - set before the install call
if [ "$RPM_STAGE" = "build" ]; then
  LP3D_QTCONF_FILE="$LP3D_OBS_WD/mainApp/qt.conf"
  LP3D_QTQRC_FILE="$LP3D_OBS_WD/mainApp/lpub3d.qrc"
  cat <<EOF >$LP3D_QTCONF_FILE
[Paths]
Prefix=$RPM_LIBDIR/qt5
Libraries=lib
Plugins=plugins
EOF
  sed -i '/<\/qresource>/a <qresource prefix="\/qt\/etc"><file alias="qt.conf">qt.conf<\/file><\/qresource>' "$LP3D_QTQRC_FILE"

  # DEBUG
  echo "DEBUG-DEBUG-DEBUG"
  cat $LP3D_QTQRC_FILE
  cat $LP3D_QTCONF_FILE
fi

# Install Qt libs if DEPLOY_QT5=1 specified
if [ "$RPM_STAGE" = "install" ]; then
  cd $LP3D_QT_WD && mkdir qt5deploy
  LP3D_QT5_LIB=$LP3D_QT5_DIR/lib
  LP3D_QT5_PLUGINS=$LP3D_QT5_DIR/plugins
  # bearer
  cp -f --parents ${LP3D_QT5_PLUGINS}/bearer/libqgenericbearer.so qt5deploy
  # iconengines
  cp -f --parents ${LP3D_QT5_PLUGINS}/iconengines/libqsvgicon.so qt5deploy
  # imageformats
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqgif.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqicns.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqico.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqjpeg.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqsvg.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqtga.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqtiff.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqwbmp.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/imageformats/libqwebp.so qt5deploy
  # platforms
  cp -f --parents ${LP3D_QT5_PLUGINS}/platforms/libqlinuxfb.so qt5deploy
  cp -f --parents ${LP3D_QT5_PLUGINS}/platforms/libqxcb.so qt5deploy
  # qtlibs
  cp -f --parents ${LP3D_QT5_LIB}/libQt5Core.so qt5deploy
  cp -f --parents ${LP3D_QT5_LIB}/libQt5Gui.so qt5deploy
  cp -f --parents ${LP3D_QT5_LIB}/libQt5Network.so qt5deploy
  cp -f --parents ${LP3D_QT5_LIB}/libQt5OpenGL.so qt5deploy
  cp -f --parents ${LP3D_QT5_LIB}/libQt5PrintSupport.so qt5deploy
  cp -f --parents ${LP3D_QT5_LIB}/libQt5Widgets.so qt5deploy
  export LP3D_QT5_LIB=$LP3D_QT_WD/qt5deploy/$LP3D_QT5_DIR/lib
  export LP3D_QT5_PLUGINS=$LP3D_QT_WD/qt5deploy/$LP3D_QT5_DIR/plugins
fi

# Restore working directory
cd $LP3D_OBS_WD && echo "Return to working directory: $LP3D_OBS_WD"
