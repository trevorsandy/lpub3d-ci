#!/bin/bash
#
# Download, extract and patch Qt5 library
# This script is used exclusively on Open Build Service
# for platforms that do not have Qt5 - e.g. CentOS 6, RHEL 6/7...
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: January 16, 2018
#  Copyright (c) 2017 - 2018 by Trevor SANDY
#
# sample command [call from root build directory - e.g. lpub3d/]:
# chmod a+x builds/utilities/GetQt5Libs.sh && ./builds/utilities/GetQt5Libs.sh

# Extract QtBinPatcher
LP3D_WD=$(cd ../ && echo $PWD) && \
LP3D_BINPATCHER="QtBinPatcher-2.2.1" && cd $LP3D_WD && \
tar -xzf ${LP3D_BINPATCHER}.tar.gz && cd $LP3D_BINPATCHER && \
mkdir -p build; cd build || echo "ERROR - Could not extract QtBinPatcher!"

# Build QtBinPatcher
cmake -G"Unix Makefiles" \
-DCMAKE_INSTALL_PREFIX=$(cd ../ && echo $PWD) \
-DCMAKE_BUILD_TYPE=Release $(cd ../ && echo $PWD) && \
make && make install && cd $LP3D_WD || echo "ERROR - Could not build QtBinPatcher!"

# Extract Qt library
LP3D_QTLIBS=qt5-5.9.3-gcc_64-rhel && \
[ ! -d "Qt/5.9.3/gcc_64" ] && tar -xzf ${LP3D_QTLIBS}.tar.gz || \
echo "ERROR - Could not extract Qt library!"

# Copy and Run QtBinPatcher - export QT5_BIN path
[ -f "Qt/5.9.3/gcc_64/bin/qmake" ] && \
cp $LP3D_BINPATCHER/bin/qtbinpatcher Qt/5.9.3/gcc_64/bin && \
cd Qt/5.9.3/gcc_64/bin && ./qtbinpatcher --verbose --nobackup &&
export QT5_BIN=$PWD || \
echo "ERROR - Could not copy and run QtBinPatcher!"
