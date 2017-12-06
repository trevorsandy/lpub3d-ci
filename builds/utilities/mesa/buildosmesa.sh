#!/bin/bash
#
# Build all libOSMesa and libGLU libraries - short
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: December 05, 2017
#  Copyright (c) 2017 by Trevor SANDY
#
# Useage: env WD=$PWD [COPY_CONFIG=1] ./lpub3d/builds/utilities/mesa/buildosmesa.sh
# Note: If WD is not defined,
#

# capture elapsed time - reset BASH time counter
SECONDS=0

# configuration options:
# specify the osmesa verion to build
mesaversion="${OSMESA_VERSION:-17.2.6}"
# specify llvm-config path if different from system default
llvm_config="${LLVM_CONFIG:-/usr/bin/llvm-config}"
# specify the libglu version
gluversion="${GLU_VERSION:-9.0.0}"
# specify the number of job processes
mkjobs="${MKJOBS:-4}"
# tell turl to follow links and continue after abnormal end if supported
curlopts="-L -C -"
# specify the build output path
osmesaprefix="${OSMESA_PREFIX:-$WD/lpub3d_linux_3rdparty/mesa}"
# specify if to remove existing build and build new
cleanbuild="${CLEAN:-0}"
# specify if to overwrite the existing osmesa-config - this file is copied during the build process
config_copy="${COPY_CONFIG:-0}"
# compiler flags
CC="gcc"
CXX="g++"
CFLAGS="-O2"
CXXFLAGS="-O2 -std=c++11"

# grab te script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# get the script location
ScriptDir=$(dirname "$0")

# Get platform
OS_NAME=`uname`
if [ "$OS_NAME" = "Darwin" ]; then
  PLATFORM=$(echo `sw_vers -productName`_`sw_vers -productVersion`)
else
  PLATFORM=$(. /etc/os-release && if test "${NAME}" != "" && test "${VERSION_ID}" != ""; then echo "${NAME}_${VERSION_ID}"; else echo `uname`; fi)
fi

# logging stuff
# increment log file name
f="$PWD/${ME}_${PLATFORM}"
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

# Functions
Info () {
  echo "-osmesa- ${*}" >&2
}

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
	WD="$(cd ../ && pwd)"
  Info "WARNING - 'WD' environment varialbe not specified. Using $WD"
fi

Info
Info "Working Directory....[${WD}]"
Info "Script Directory.....[${ScriptDir}]"
Info "LLVM-Config Path.....[${llvm_config}]"
Info "Install Prefix.......[${osmesaprefix}]"
Info "OSMesa Version.......[${mesaversion}]"
Info "GLU Version..........[${gluversion}]"

# build OSMesa
Info && Info "building OSMesa..."

cd $WD
if [[ -d "mesa-${mesaversion}" && ${cleanbuild} = 1 ]]; then
	Info "cleanup old mesa-$mesaversion..."
	rm -rf "mesa-$mesaversion"
	if [ -d "${osmesaprefix}" ]; then
		rm -rf "${osmesaprefix}"
	fi
fi

#check for llvm-config
if [ ! -f "${llvm_config}" ]; then
	Info "llmv-config not found, (re)installing Mesa build dependencies..."
	sudo dnf builddep -y mesa
fi

# sourcepath="${SOURCE_PATH:-projects/Working/Docker-output}"
if [ ! -f "mesa-${mesaversion}.tar.gz" ]; then
  #cp -rf "${sourcepath}/mesa-${mesaversion}.tar.gz" . && Info "mesa-${mesaversion}.tar.gz copied to ~/"
	Info "downloading Mesa ${mesaversion}..."
	curl $curlopts -O "ftp://ftp.freedesktop.org/pub/mesa/mesa-${mesaversion}.tar.gz" || curl $curlopts -O "ftp://ftp.freedesktop.org/pub/mesa/${mesaversion}/mesa-${mesaversion}.tar.gz"
fi

if [ ! -d "mesa-${mesaversion}" ]; then
	Info "extracting Mesa..."
	tar zxf mesa-${mesaversion}.tar.gz
fi

if [ ! -d "${osmesaprefix}" ]; then
	Info "create install prefix..."
    mkdir -p "${osmesaprefix}"
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
	Info && make -j${mkjobs}

	# install command [sudo is not needed with user install prefix]
	Info "installing OSMesa..."
	make install
else
	Info "library OSMesa32 exist - build skipped."
fi

# copy config file
if [[ $config_copy -eq 1 || ! -f "${osmesaprefix}/osmesa-config" ]]; then
	echo "DEBUG - WE ARE HERE: $PWD"
	echo "DEBUG - KO COPY PATH: ${ScriptDir}/osmesa-config"
	cp -f "${ScriptDir}/osmesa-config" "${osmesaprefix}"
	if [ -f "${osmesaprefix}/osmesa-config" ]; then
		Info "osmesa-config copied to ${osmesaprefix}"
	else
		Info "ERROR - osmesa-config was not copied to ${osmesaprefix}"
  fi
elif  [ -f "${osmesaprefix}/osmesa-config" ]; then
	Info "osmesa-config exist - copy skipped."
fi

# build GLU
Info && Info "building GLU..."

cd $WD
if [[ -d glu-$gluversion && ${cleanbuild} = 1 ]]; then
	Info "cleanup old glu-$gluversion..."
	rm -rf "glu-$gluversion"
	if [ -d "${osmesaprefix}" ]; then
		rm -rf "${osmesaprefix}"
	fi
fi

if [ ! -f glu-${gluversion}.tar.bz2 ]; then
	Info "* downloading GLU ${gluversion}..."
	curl $curlopts -O "ftp://ftp.freedesktop.org/pub/mesa/glu/glu-${gluversion}.tar.bz2"
fi

if [ ! -d glu-${gluversion} ]; then
	Info "extracting GLU..."
	tar jxf glu-${gluversion}.tar.bz2
fi

if [ ! -d "${osmesaprefix}" ]; then
	Info "create install prefix..."
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
	Info && make -j${mkjobs}

	# install command [sudo is not needed with user install prefix]
	Info "installing GLU..."
	make install
else
	Info "library GLU exist - build skipped."
fi
cd $WD

# elapsed execution time
ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
Info
Info "$ME Finsihed!"
Info "$ELAPSED"
