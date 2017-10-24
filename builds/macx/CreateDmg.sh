#!/bin/bash
# Trevor SANDY
# Last Update October 25 2017
# To run:
# $ chmod 755 CreateDmg.sh
# $ ./CreateDmg.sh

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
BUILD_DATE=`date "+%Y%m%d"`

echo "Start $ME execution at $CWD..."

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d-ci}"
echo "   LPUB3D SOURCE DIR......${LPUB3D}"

# when running locally, use this block...
if [ "${TRAVIS}" != "true"  ]; then
  # logging stuff
  # increment log file name
  f="${CWD}/$ME"
  ext=".log"
  if [[ -e "$f$ext" ]] ; then
    i=1
    f="${f%.*}";
    while [[ -e "${f}_${i}${ext}" ]]; do
      let i++
    done
    f="${f}_${i}${ext}"
  else
    f="${f}${ext}"
  fi
  # output log file
  LOG="$f"
  exec > >(tee -a ${LOG} )
  exec 2> >(tee -a ${LOG} >&2)

  # use this instance of Qt
  export PATH=~/Qt/IDE/5.7/clang_64:~/Qt/IDE/5.7/clang_64/bin:$PATH

  echo
  echo "Enter d to download LPub3D source or any key to"
  echo "skip download and use existing source if available."
  read -n 1 -p "Do you want to continue with this option? : " getsource

# For Travis CI, use this block (script called from [pwd] lpub3d/)
else
  # copy downloaded source
  getsource=c
  # move outside lpub3d/
  cd ../
fi

echo "-  create DMG build working directory..."
if [ ! -d dmgbuild ]
then
  mkdir dmgbuild
  echo "   dmgbuild created."
fi

if [ ! -d lpub3d_macos_3rdparty ]
then
  echo "-  download lpub3d_macos_3rdparty repository..."
  git clone https://github.com/trevorsandy/lpub3d_macos_3rdparty.git
else
  echo "-  lpub3d_macos_3rdparty repository folder exist. skipping download..."
fi

cd dmgbuild

if [ -d ${LPUB3D} ] && [ "$getsource" = "d" ] || [ "$getsource" = "D" ]
then
  echo "-  processing download LPub3D source request..."
  rm -rf ${LPUB3D}
  git clone https://github.com/trevorsandy/${LPUB3D}.git
elif [ "$getsource" = "c" ]
then
  echo "-  copying LPub3D source..."
  echo "-  DEBUG COPY SOURCE ROOT DIR LIST: `ls ../`"
  cp -rf ../${LPUB3D}/ ./${LPUB3D}/
  echo "-  DEBUG PWD: $PWD"
  echo "-  DEBUG COPY DEST DIR LIST: `ls`"
elif [ ! -d ${LPUB3D} ]
then
  echo "-  download LPub3D source..."
  git clone https://github.com/trevorsandy/${LPUB3D}.git
else
  echo "-  LPub3D source exist. skipping download"
fi

echo "-  source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
echo "   DEBUG _PRO_FILE_PWD_ = ${_PRO_FILE_PWD_}"
source ${LPUB3D}/builds/utilities/update-config-files.sh
SOURCE_DIR=${LPUB3D}-${LP3D_APP_VERSION}

cd ${LPUB3D}

if [ ! -f "mainApp/extras/complete.zip" ]
then
  echo "-  get ldraw official library archive..."
  curl "http://www.ldraw.org/library/updates/complete.zip" -o "mainApp/extras/complete.zip"
else
  echo "-  ldraw official library exist. skipping download"
fi
if [ ! -f "mainApp/extras/lpub3dldrawunf.zip" ]
then
  echo "-  get ldraw unofficial library archive..."
  curl "http://www.ldraw.org/library/unofficial/ldrawunf.zip" -o "mainApp/extras/lpub3dldrawunf.zip"
else
  echo "-  ldraw unofficial library exist. skipping download"
fi

echo "-  configure and build LPub3D source..."
#qmake LPub3D.pro -spec macx-clang CONFIG+=x86_64 /usr/bin/make qmake_all
echo "-  DEBUG WHERE IS QMAKE: `whereis qmake`"
qmake -v
qmake -r
/usr/bin/make

# create dmg environment - begin #
#
cd builds/macx

echo "- copy LPub3D bundle components..."
cp -rf ../../mainApp/release/LPub3D.app .
cp -f ../utilities/icons/lpub3d.icns .
cp -f ../utilities/icons/lpub3dbkg.png .
cp -f ../utilities/COPYING_BRIEF .COPYING

echo "- install library links..."
/usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Libs/libQuaZIP.0.dylib

echo "- change mapping to LPub3D..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/MacOS/LPub3D

echo "- bundle LPub3D..."
macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo "- change library dependency mapping..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo "- setup dmg source dir..."
if [ ! -d DMGSRC ]
then
  mkdir DMGSRC
fi
mv -f LPub3D.app DMGSRC/LPub3D.app

echo "- setup dmg output dir..."
DMGDIR=../../../DMGS
if [ -d ${DMGDIR} ]
then
  rm -f -R ${DMGDIR}
fi
mkdir -p ${DMGDIR}

# pos: builds/macx
echo "- generate README file and dmg make script..."
cat <<EOF >README
Thank you for installing LPub3D v${APP_VERSION} for MacOS".

Drag the LPub3D Application icon to the Applications folder.

After installation, remove the mounted LPub3D disk image by dragging it to the Trash

Cheers,
EOF

cat <<EOF >makedmg
#!/bin/bash
../utilities/create-dmg \\
--volname "LPub3D Installer" \\
--volicon "lpub3d.icns" \\
--background "lpub3dbkg.png" \\
--icon-size 90 \\
--text-size 10 \\
--window-pos 200 120 \\
--window-size 640 480 \\
--icon LPub3D.app 192 344 \\
--hide-extension LPub3D.app \\
--custom-icon Readme README 512 128 \\
--app-drop-link 448 344 \\
--eula .COPYING \\
"${DMGDIR}/LPub3D_${APP_VERSION_LONG}_osx.dmg" \\
DMGSRC/
EOF

echo "- create dmg packages..."
chmod +x makedmg && ./makedmg

cp "${DMGDIR}/LPub3D_${APP_VERSION_LONG}_osx.dmg" "${DMGDIR}/LPub3D-UpdateMaster_${VERSION}_osx.dmg"
echo "      Download package..: LPub3D_${APP_VERSION_LONG}_osx.dmg"
echo "      Update package....: LPub3D-UpdateMaster_${VERSION}_osx.dmg"

echo "- cleanup..."
rm -f -R DMGSRC
rm -f lpub3d.icns lpub3dbkg.png README .COPYING makedmg

# if [ "${TRAVIS}" == "true"  ]; then
#   # export vars used by travis.yml so paths must be relative to project download dir
#   export LP3D_Download_DmgPackage=`ls ${DMGDIR}/LPub3D_*_osx.dmg`
#   export LP3D_Update_DmgPackage=`ls ${DMGDIR}/LPub3D-UpdateMaster_*_osx.dmg`
#   echo " Package files: `ls ${DMGDIR}/LPub3D*_osx.dmg`"
#   env | grep -P 'LP3D*'
#   env | grep -P 'TRAVIS*'
# fi

# create dmg - end #
echo "$ME Finished!"
# mv $LOG "${CWD}/dmgbuild/$ME.log"
