#!/bin/bash
#
# Build all libOSMesa and libGLU libraries - short
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: November 30, 2017
#  Copyright (c) 2017 by Trevor SANDY
#

# capture elapsed time - reset BASH time counter
SECONDS=0

# grab te script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# get the script location
ScriptDir=$(dirname "$0")

# logging stuff
# increment log file name
f="$PWD/$ME"
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

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
	WD="$(cd ../ && pwd)"
  echo "WARNING - 'WD' environment varialbe not specified. Using $WD"
fi

# static configuration options
mesaversion="${OSMESA_VERSION:-17.2.6}"
gluversion="${GLU_VERSION:-9.0.0}"
mkjobs="${MKJOBS:-4}"
curlopts="-L -C -"
CC="gcc"
CXX="g++"
CFLAGS="-O2"
CXXFLAGS="-O2 -std=c++11"
osmesaprefix="${OSMESA_PREFIX:-$WD/lpub3d_linux_3rdparty/mesa}"
cleanbuild="${CLEAN:-0}"

echo
echo "Working Directory....[${WD}]"
#echo "Script Directory.....[${ScriptDir}]"
echo "Install Prefix.......[${osmesaprefix}]"
echo "OSMesa version.......[${mesaversion}]"
echo "GLU version..........[${gluversion}]"

# build OSMesa
echo && echo "building OSMesa..."

cd $WD
if [[ -d "mesa-${mesaversion}" && ${cleanbuild} = 1 ]]; then
	echo "cleanup old mesa-$mesaversion..."
	rm -rf "mesa-$mesaversion"
	if [ -d "${osmesaprefix}" ]; then
		rm -rf "${osmesaprefix}"
	fi
fi

# sourcepath="${SOURCE_PATH:-projects/Working/Docker-output}"
if [ ! -f "mesa-${mesaversion}.tar.gz" ]; then
    #cp -rf "${sourcepath}/mesa-${mesaversion}.tar.gz" . && echo "mesa-${mesaversion}.tar.gz copied to ~/"
	echo "downloading Mesa ${mesaversion}..."
	curl $curlopts -O "ftp://ftp.freedesktop.org/pub/mesa/mesa-${mesaversion}.tar.gz" || curl $curlopts -O "ftp://ftp.freedesktop.org/pub/mesa/${mesaversion}/mesa-${mesaversion}.tar.gz"
fi

if [ ! -d "mesa-${mesaversion}" ]; then
	echo "extracting Mesa..."
	tar zxf mesa-${mesaversion}.tar.gz
fi

if [ ! -d "${osmesaprefix}" ]; then
	echo "create install prefix..."
    mkdir -p "${osmesaprefix}"
fi

# copy config file to
cp -f "$ScriptDir/osmesa-config" "${osmesaprefix}"
if  [ -f "${osmesaprefix}/osmesa-config" ]; then
	echo "osmesa-config copied to ${osmesaprefix}"
else
	echo "Error - osmesa-config was not copied to ${osmesaprefix}"
fi

cd mesa-${mesaversion}

# OSMesa configuration options
confopts="\
--disable-dependency-tracking \
--enable-static \
--disable-shared \
--enable-texture-float \
--disable-gles1 \
--disable-gles2 \
--disable-dri \
--disable-dri3 \
--disable-glx \
--disable-glx-tls \
--disable-egl \
--disable-gbm \
--disable-xvmc \
--disable-vdpau \
--disable-omx \
--disable-va \
--disable-opencl \
--disable-shared-glapi \
--disable-driglx-direct \
--with-dri-drivers= \
--with-osmesa-bits=32 \
--with-platforms= \
--disable-osmesa \
--enable-gallium-osmesa \
--enable-llvm=yes \
--disable-llvm-shared-libs \
--with-gallium-drivers=swrast \
--prefix=${osmesaprefix} \
"
if [ ! -f "$osmesaprefix/lib/libOSMesa32.a" ]; then
	# configure command
	env PKG_CONFIG_PATH="$osmesaprefix/lib/pkgconfig:$PKG_CONFIG_PATH" \
	./configure ${confopts} CC="$CC" CFLAGS="$CFLAGS" CXX="$CXX" CXXFLAGS="$CXXFLAGS"

	# build command
	echo && make -j${mkjobs}

	# install command [sudo is not needed with user install prefix]
	echo "installing OSMesa..."
	make install
else
	echo "library OSMesa32 exist - build skipped."
fi

# build GLU
echo && echo "building GLU..."

cd $WD
if [[ -d glu-$gluversion && ${cleanbuild} = 1 ]]; then
	echo "cleanup old glu-$gluversion..."
	rm -rf "glu-$gluversion"
	if [ -d "${osmesaprefix}" ]; then
		rm -rf "${osmesaprefix}"
	fi
fi

if [ ! -f glu-${gluversion}.tar.bz2 ]; then
	echo "* downloading GLU ${gluversion}..."
	curl $curlopts -O "ftp://ftp.freedesktop.org/pub/mesa/glu/glu-${gluversion}.tar.bz2"
fi

if [ ! -d glu-${gluversion} ]; then
	echo "extracting GLU..."
	tar jxf glu-${gluversion}.tar.bz2
fi

if [ ! -d "${osmesaprefix}" ]; then
	echo "create install prefix..."
    mkdir -p "${osmesaprefix}"
fi

cd glu-${gluversion}

# GLU configuration options
confopts="\
--disable-dependency-tracking \
--enable-static \
--disable-shared \
--enable-osmesa \
--prefix=$osmesaprefix \
"
if [ ! -f "$osmesaprefix/lib/libGLU.a" ]; then
	# configure command
	env PKG_CONFIG_PATH="$osmesaprefix/lib/pkgconfig:$PKG_CONFIG_PATH" \
	./configure ${confopts} CC="$CC" CFLAGS="$CFLAGS" CXX="$CXX" CXXFLAGS="$CXXFLAGS"

	# build command
	echo && make -j${mkjobs}

	# install command [sudo is not needed with user install prefix]
	echo "installing GLU..."
	make install
else
	echo "library GLU exist - build skipped."
fi
cd $WD

# elapsed execution time
ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
echo
echo "$ME Finsihed!"
echo "$ELAPSED"
