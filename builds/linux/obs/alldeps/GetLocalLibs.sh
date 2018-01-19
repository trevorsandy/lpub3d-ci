#!/bin/bash
#
# This script is sourced on 2 occasions:
# 1. to extract local library assets,and
# 2. to deploy the local library assets
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: January 18, 2018
#  Copyright (c) 2017 - 2018 by Trevor SANDY
#
# sample command [call from root build directory - e.g. lpub3d/]:
# chmod a+x builds/linux/obs/alldeps/GetLocalLibs.sh && ./builds/linux/obs/alldeps/GetLocalLibs.sh

# Set working directory and qt dir path
LP3D_OBS_WD=$PWD && echo "Entering Local Libs working directory..."
LP3D_LL_WD=$(cd ../ && echo $PWD) && cd $LP3D_LL_WD

# Set user and etc paths
LP3D_LL_USR=$LP3D_LL_WD/usr
LP3D_LL_ETC=$LP3D_LL_WD/etc
LP3D_LL_BIN=$LP3D_LL_USR/bin

# Export library folders
if [ "$RPM_STAGE" = "build" ]; then
  # Extact tarball
  [ ! -f "mesa-libOSMesa-17.0.1-6.20170307.el7.x86_64.rpm" ] && \
  echo "Extracting locallibs.el.x86_64.tar.gz..." && \
  tar -xzvf locallibs.el.x86_64.tar.gz || true

  # Extract rpm libraries into library folder(s)
  [ -f "mesa-libOSMesa-17.0.1-6.20170307.el7.x86_64.rpm" ] && \
  for f in $(find . -type f -name '*.el7.x86_64.rpm'); do
  echo "Extracting rpm $f..." && rpm2cpio $f | cpio -idm
  done || echo "ERROR - locallibs.el.x86_64.tar.gz was not extracted properly"

  # Create symlink for llvm-config-64
  [ -f $LP3D_LL_USR/bin/llvm-config-64 ] && \
  ln -s $LP3D_LL_USR/bin/llvm-config-64 $LP3D_LL_USR/bin/llvm-config && \
  [ -f $LP3D_LL_USR/bin/llvm-config ] && export LP3D_LL_BIN &&\
  echo "Created symlink $LP3D_LL_USR/bin/llvm-config " || \
  echo "ERROR - unable to create symlink $LP3D_LL_USR/bin/llvm-config"
fi

# Export library base paths
[ -d $LP3D_LL_USR ] && export LP3D_LL_USR || \
echo "ERROR - $LP3D_LL_USR does not exist"
[ -d $LP3D_LL_ETC ] && export LP3D_LL_ETC || \
echo "ERROR - $LP3D_LL_ETC does not exist"

# Restore working directory
cd $LP3D_OBS_WD && echo "Return to working directory: $LP3D_OBS_WD"
