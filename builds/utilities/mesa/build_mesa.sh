#!/bin/bash
#
# Build Mesa, GLU and ZStandard libraries
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update June 4, 2025
#  Copyright (C) 2018 - 2025 by Trevor SANDY
#
# Useage: env WD=$PWD [COPY_CONFIG=1] ./lpub3d/builds/utilities/mesa/build_mesa.sh
# Note: If WD is not defined,
#

# capture elapsed time - reset BASH time counter
SECONDS=0

# configuration options:
# specify the mesa verion to build
mesa_version="${MESA_VERSION:-21.3.9}"
# specify the libglu version
glu_version="${GLU_VERSION:-9.0.1}"
# specify llvm not supported - deprecated (this is the old RHEL no_gallium flag)
llvm_not_supported=0
# specify the libzstd version
zstd_version="${ZSTD_VERSION:-1.5.7}"
# specify llvm is not used for default OSMesa configuration
llvm_not_used="${LLVM_NOT_USED:-1}"
# specify llvm will be built from source
llvm_build="${LLVM_BUILD:-0}"
# specify llvm-config path if different from system default
llvm_config="${LLVM_CONFIG:-/usr/bin/llvm-config}"
# specify the number of job processes
MK_JOBS="${MK_JOBS:-4}"
# specify the build output path
mesa_prefix="${MESA_PREFIX:-$WD/lpub3d_linux_3rdparty/mesa}"
# specify build OSMesa only - do not build full Mesa-amber (e.g. gles, dri, gbm etc...)
build_osmesa_only="${BUILD_OSMESA_ONLY:-1}"
# specify the osmesa bits - set empty for 8 bits (when not build_osmesa_only)
osmesa_bits="${OSMESA_BITS:-}"
# specify static/shared build
static_build="${STATIC:-1}"
# specify if to remove existing build and build new
clean_build="${CLEAN:-0}"
# specify if to overwrite the existing osmesa-config - this file is copied during the build process
config_copy="${COPY_CONFIG:-0}"
# tell turl to follow links and continue after abnormal end if supported
curl_opts="-L -C -"
# build log
log_path="${LOG_PATH:-$PWD}"
# building on Open Build Service
OBS="${OBS:-false}"
# compiler flags
[ -z "${CC:-}" ] && CC="gcc" || :
[ -z "${CXX:-}" ]&& CXX="g++" || :
CFLAGS="-O2"
CXXFLAGS="${CXXFLAGS:-${CFLAGS}} -std=c++11"
# lpub3d library repository
LP3D_GITHUB_URL="https://github.com/trevorsandy"
LP3D_LIBS_BASE="${LP3D_GITHUB_URL}/lpub3d_libs/releases/download/v1.0.1"
#LP3D_MESA_FTP=ftp://ftp.freedesktop.org/pub/mesa
#LP3D_MESA_URL=https://archive.mesa3d.org/

# grab the script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# get the script location
script_dir=$(dirname "$0")

# set Mesa packege name
mesa_package=Mesa
# default library extension
lib_ext="$([ "${static_build}" -eq 1 ] && echo a || echo so)"
# archive file suffix
arch_suffix=xz

# default decompress format
z_cat=xzcat
# initialize variables
use_autoconf=0
use_meson=0
# Get platform
os_name=`uname`
if [ "$os_name" = "Darwin" ]; then
  os_version=$(uname -r | awk -F . '{print $1}')
  platform=macos
else
  platform=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $OS_NAME | awk '{print tolower($0)}')   #'
fi
if [ "$OBS" != "true" ]; then
  # logging stuff
  # increment log file name
  f="${log_path}/${ME}_${platform}"
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
  LOG="${f}"
  exec > >(tee -a ${LOG} )
  exec 2> >(tee -a ${LOG} >&2)
fi

# Functions
# Args: [$1 = -n,] $2 = <message>
function Info()
{
  local i
  if [ "${NEW_LINE}" = 0 ]; then
    NEW_LINE=1
    echo "${*}" >&2
    return
  elif [ "$1" = "-n" ]; then
    NEW_LINE=0
    i=${*:2}
    echo -n "-mesa- ${i}" >&2
    return
  else
    i=${*}
  fi
  echo "-mesa- ${i}" >&2
}

# see https://stackoverflow.com/a/24067243
SORT=sort
if [ "$os_name" = Darwin ] && [ "$os_version" -le 10 ]; then
  SORT=gsort
fi
function version_gt() {
  test "$(printf '%s\n' "$@" | $SORT -V | head -n 1)" != "$1";
}

# $mesa_version is less than 19.0.0
if version_gt 19.0.0 "$mesa_version"; then
  mesa_build_sys="Autoconf"
  use_autoconf=1
  arch_suffix=gz
else
  mesa_build_sys="Meson"
  use_meson=1
fi
# $mesa_version is greater than 21.3.0
if version_gt "$mesa_version" 21.3.0; then
  mesa_package="${mesa_package}-amber"
else
  unset zstd_version
fi

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
  WD="$(cd ../../../../ && pwd)"
  Info "WARNING - 'WD' environment variable not specified. Using $WD"
fi
declare -r l=Log

# Set Mesa library file name
[[ -z "${osmesa_bits}" && "${build_osmesa_only}" -eq 1 ]] && osmesa_bits=32 || :
lib_file="OSMesa${osmesa_bits}"

Info
Info "Working Directory........[${WD}]"
Info "Script Directory.........[${script_dir}]"
Info "Install Prefix...........[${mesa_prefix}]"
Info "Mesa Version.............[${mesa_version}]"
Info "GLU Version..............[${glu_version}]"
[ -n "$zstd_version" ] && \
Info "ZStandard Version........[${zstd_version}]" || :
Info "Library File.............[lib${lib_file}.${lib_ext}]"
Info "Build System.............[${mesa_build_sys}]"
Info "Build Library............[$([ "${static_build}" -eq 1 ] && echo "Static" || echo "Shared")]"
Info "Build Option.............[$([ "${build_osmesa_only}" -eq 1 ] && echo "OSMesa" || echo "Mesa")]"
[ "$OBS" != "true" ] && \
Info "Build Log................[${LOG}]" || :

cd $WD
[ "${static_build}" -ne 1 ] && lib_ext=so || :
if [[ -d "mesa-${mesa_version}" && "${clean_build}" -eq 1 ]]; then
  Info "cleanup old mesa-$mesa_version..."
  rm -rf "mesa-$mesa_version"
  if [ -d "${mesa_prefix}" ]; then
    rm -rf "${mesa_prefix}"
  fi
fi

#check for llvm-config - and process OBS alternative config (e.g. no gallium)
if [ "${llvm_not_used}" -eq 1 ]; then
  Info "LLVM Not Used............[${mesa_package} ${mesa_version} using Gallium-swrast]"
elif [ "${llvm_build}" -eq 1 ]; then
  llvm_version="$($llvm_config --version | egrep -o '^[0-9.]+')"
  llmv_message="LLVM will be built from source"
  if [ -n "${llvm_version}" ]; then
    llmv_message="Installed LLVM ${llvm_version} not supported"
  fi
  Info "LLVM version.............[${llmv_message}]"
elif [ ! -f "${llvm_config}" ]; then
  if [ "$OBS" = "true" ]; then
    if [ "${platform}" != "arch" ]; then
      Info && Info "ERROR - llmv-config not found at ${llvm_config}. $ME will terminate"
      exit 1
    fi
  else
    if [ "$RPM_BUILD" = "true" ]; then
      Info "LLVM-Config..............[Not found at ${llvm_config}, (re)installing build dependencies]"
      sudo dnf builddep -y mesa
    else
      Info && Info "ERROR - llmv-config not found at ${llvm_config}. $ME will terminate"
      exit 1
    fi
  fi
else
  Info "LLVM-Config..............[${llvm_config}]"
  Info "LLVM Version.............[$($llvm_config --version | egrep -o '^[0-9.]+')]"
fi

if [ "${OBS}" = "true" ];then
  Info "Using OBS................[Yes]"
  [ "$RPM_BUILD" = "true" ] && \
  Info "OBS Build Family.........[RPM_BUILD]" || :
fi

Info && Info "----------------------------------------------------"
Info "Building Mesa..."

# sourcepath="${SOURCE_PATH:-projects/Working/Docker-output}"
if [ ! -f "mesa-${mesa_version}.tar.${arch_suffix}" ]; then
  if [ "$OBS" != "true" ]; then
    Info "downloading Mesa ${mesa_version}..."
    #curl $curl_opts -O "${LP3D_MESA_FTP}/mesa-${mesa_version}.tar.${arch_suffix}" || \
    #curl $curl_opts -O "${LP3D_MESA_FTP}/older-versions/${mesa_version/.*/.x}/mesa-${mesa_version}.tar.${arch_suffix}" || \
    #curl $curl_opts -O "${LP3D_MESA_FTP}/older-versions/${mesa_version/.*/.x}/${mesa_version}/mesa-${mesa_version}.tar.${arch_suffix}"
    #curl $curl_opts -O "${LP3D_MESA_URL}/older-versions/${mesa_version/.*/.x}/mesa-${mesa_version}.tar.${arch_suffix}"
    curl $curl_opts -O "${LP3D_LIBS_BASE}/mesa-${mesa_version}.tar.${arch_suffix}"
  else
    Info "ERROR - archive file mesa-${mesa_version}.tar.${arch_suffix} was not found. $ME will terminate."
    exit 1
  fi
fi

# Mesa source directory
if [ ! -d "mesa-${mesa_version}" ]; then
  Info -n "extracting Mesa (tar.${arch_suffix})..."
  [ "${arch_suffix}" = "gz" ] && z_cat="gzip -dc" || :
  ($z_cat "mesa-${mesa_version}.tar.${arch_suffix}" | tar xf -) >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
fi

# Mesa build directory
if [ ! -d "${mesa_prefix}" ]; then
  Info "create install prefix..."
  mkdir -p "${mesa_prefix}"
fi

if [ ! -f "$mesa_prefix/lib/lib${lib_file}.${lib_ext}" ]; then
  if [ "${use_autoconf}" -eq 1 ]; then
    cd mesa-${mesa_version}
    # Mesa configuration options
    configure_options="\
    --disable-dependency-tracking \
    --enable-static \
    --disable-shared \
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
    --disable-va \
    --disable-opencl \
    --disable-shared-glapi \
    --disable-driglx-direct \
    --with-dri-drivers= \
    --with-platforms= \
    --with-osmesa-bits=32 \
    "
    if [ "${mesa_version}" = "18.3.5" ]; then
      configure_options="\
      $configure_options \
      --disable-omx-bellagio \
      "
    else
      configure_options="\
      $configure_options \
      --enable-texture-float \
      --disable-omx \
      "
    fi
    if [ "$llvm_not_supported" -eq 1 ]; then
      configure_options="\
      $configure_options \
      --disable-gallium-osmesa \
      --disable-llvm \
      --enable-osmesa \
      "
    else
      configure_options="\
      $configure_options \
      --disable-osmesa \
      --enable-gallium-osmesa \
      --enable-llvm=yes \
      --disable-llvm-shared-libs \
      --with-gallium-drivers=swrast \
      "
    fi
    configure_options="\
    $configure_options \
    --prefix=${mesa_prefix} \
    "
    Info "Using confops: $configure_options" && Info

    # configure command
    env PKG_CONFIG_PATH="$mesa_prefix/lib/pkgconfig:$PKG_CONFIG_PATH" \
    ./configure ${configure_options} CC="$CC" CFLAGS="$CFLAGS" CXX="$CXX" CXXFLAGS="$CXXFLAGS"

    # build command
    Info && make -j${MK_JOBS}

    # install command [sudo is not needed with user install prefix]
    Info "installing ${lib_file}..."
    make install
  elif [ "${use_meson}" -eq 1 ]; then
    # Build only minimal debug info to reduce size
    CFLAGS+=' -g1'
    CXXFLAGS+=' -g1'

    Info "Patches..."
    PATCHES="\
    meson_change_mesa_shared_library_to_library_enable_static_builds.patch \
    meson_replace_distutils.version_with_packaging.version.patch \
    "
    for i in $PATCHES; do
      if [ -f "${script_dir}/patches/mesa-$mesa_version/$i" ]; then
        Info -n "Applying patch $i..."
        (patch -p1 -d "mesa-${mesa_version}" < "${script_dir}/patches/mesa-$mesa_version/$i") >$l.out 2>&1 && rm $l.out
        [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
      fi
    done

    if [ "${build_osmesa_only}" -eq 1 ]; then
      dri_drivers=
      gallium_drivers=swrast
      glx=disabled
      is_enabled=disabled
      platforms=
      vulcan_drivers=
    else
      dri_drivers=i915,i965,r100,r200,nouveau
      gallium_drivers=swrast
      glx=dri
      is_enabled=enabled
      osmesa_bits=8
      platforms=x11,wayland
      vulcan_drivers=auto
    fi
    if [ "${llvm_not_used}" -eq 1 ]; then
      llvm_enabled=disabled
    else
      llvm_enabled=enabled
    fi

    cd mesa-${mesa_version}

    # configure command
    meson_options=(
      -D prefix=$mesa_prefix
      -D amber=true
      -D b_ndebug=true
      -D buildtype=release
      -D dri-drivers=$dri_drivers
      -D dri3=$is_enabled
      -D egl=$is_enabled
      -D gallium-drivers=$gallium_drivers
      -D gbm=$is_enabled
      -D gles1=$is_enabled
      -D gles2=$is_enabled
      -D glvnd=$is_enabled
      -D glx=$glx
      -D libunwind=$is_enabled
      -D llvm=$llvm_enabled
      -D lmsensors=disabled
      -D microsoft-clc=disabled
      -D osmesa=true
      -D osmesa-bits=$osmesa_bits
      -D platforms=$platforms
      -D shared-glapi=$is_enabled
      -D valgrind=$is_enabled
      -D vulkan-drivers=$vulcan_drivers
    )

    [ "${static_build}" -eq 1 ] && library=static || library=shared

    Info "Using meson_options: ${meson_options[@]}" && Info

    meson setup --default-library=$library ${meson_options[@]} ../${library}_build/
    meson configure ../${library}_build/ # Print config
    # build command
    meson compile -C ../${library}_build/

    # install command [sudo is not needed with user install prefix]
    Info "installing ${lib_file}..."
    meson install -C ../${library}_build/

    # indirect rendering
    if [[ "${build_osmesa_only}" -ne 1 && "${static_build}" -ne 1 ]]; then
      ln -s $mesa_prefix/lib/libGLX_amber.so.0 "$mesa_prefix/lib/libGLX_indirect.so.0"
    fi
    install -m644 -Dt "${mesa_prefix}/share/licenses/mesa-amber" ${script_dir}/LICENSE
  fi
else
  Info "library ${lib_file} exist - build skipped."
fi

# copy config file
if [[ $config_copy -eq 1 || ! -f "${mesa_prefix}/osmesa-config" ]]; then
  cp -f "${script_dir}/osmesa-config" "${mesa_prefix}"
  if [ -f "${mesa_prefix}/osmesa-config" ]; then
    Info "osmesa-config copied to ${mesa_prefix}"
    Info "setting permissions..."
    chmod +x "${mesa_prefix}/osmesa-config"
  else
    Info "ERROR - osmesa-config was not copied to ${mesa_prefix}"
  fi
elif  [ -f "${mesa_prefix}/osmesa-config" ]; then
  Info "osmesa-config exist - copy skipped."
fi

# update version in config file
if  [ -f "${mesa_prefix}/osmesa-config" ]; then
  [ -z "$zstd_version" ] && \
  sed -e 's; -lzstd;;g' -e 's; \${libdir}/libzstd.a;;g' -i ${mesa_prefix}/osmesa-config || :
  sed '/    --version)/{n;s/.*/      echo '"${mesa_version}"'/}' -i ${mesa_prefix}/osmesa-config
  Info "osmesa-config version updated"
  sed -n '26,27p;28q' ${mesa_prefix}/osmesa-config
  sed "s/@mesa@/${lib_file}/g" -i ${mesa_prefix}/osmesa-config
  Info "osmesa-config library name updated"
  sed -n '50,51p;52q' ${mesa_prefix}/osmesa-config
else
  Info "osmesa-config was not found - version update failed."
  exit 1
fi

Info && Info "----------------------------------------------------"
Info "Building GLU..."

cd $WD
if version_gt "${glu_version}" 9.0.0; then arch_suffix=xz; else arch_suffix=bz2; fi
if [[ -d "glu-${glu_version}" && ${clean_build} -eq 1 ]]; then
  Info "cleanup old glu-${glu_version}..."
  rm -rf "glu-${glu_version}"
  if [ -d "${mesa_prefix}" ]; then
    rm -rf "${mesa_prefix}"
  fi
fi

if [ ! -f "glu-${glu_version}.tar.${arch_suffix}" ]; then
  if [ "$OBS" != "true" ]; then
    Info "* downloading GLU ${glu_version}..."
    #curl $curl_opts -O "${LP3D_MESA_FTP}/glu/glu-${glu_version}.tar.${arch_suffix}"
    #curl $curl_opts -O "${LP3D_MESA_URL}/glu/glu-${glu_version}.tar.${arch_suffix}"
    curl $curl_opts -O "${LP3D_LIBS_BASE}/glu-${glu_version}.tar.${arch_suffix}"
  else
    Info "ERROR - archive file glu-${glu_version}.tar.${arch_suffix} was not found."
  fi
fi

# GLU source directory
if [ ! -d "glu-${glu_version}" ]; then
  Info -n "extracting GLU (tar.${arch_suffix})..."
  [ "${arch_suffix}" = "xz" ] && z_cat=xzcat || z_cat=bzcat
  ($z_cat glu-${glu_version}.tar.${arch_suffix} | tar xf -) >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
fi

# GLU build directory
if [ ! -d "${mesa_prefix}" ]; then
  Info "create install prefix..."
  mkdir -p "${mesa_prefix}"
fi
lib_ext=a

cd glu-${glu_version}

# GLU configuration options
configure_options="\
--disable-dependency-tracking \
--enable-static \
--disable-shared \
--enable-osmesa \
--prefix=${mesa_prefix} \
"
if [ ! -f "${mesa_prefix}/lib/libGLU.${lib_ext}" ]; then
  # configure command
  env PKG_CONFIG_PATH="${mesa_prefix}/lib/pkgconfig:$PKG_CONFIG_PATH" \
  ./configure ${configure_options} CC="$CC" CFLAGS="$CFLAGS" CXX="$CXX" CXXFLAGS="$CXXFLAGS"

  # build command
  Info && make -j${MK_JOBS}

  # install command [sudo is not needed with user install prefix]
  Info "installing GLU..."
  make install
else
  Info "library GLU exist - build skipped."
fi

if [ -n "$zstd_version" ]; then
  Info && Info "----------------------------------------------------"
  Info "Building ZStandard..."

  cd $WD
  arch_suffix=gz
  if [[ -d "zstd-${zstd_version}" && ${clean_build} -eq 1 ]]; then
    Info "cleanup old zstd-${zstd_version}..."
    rm -rf "zstd-${zstd_version}"
    if [ -d "${mesa_prefix}" ]; then
      rm -rf "${mesa_prefix}"
    fi
  fi

  if [ ! -f "zstd-${zstd_version}.tar.${arch_suffix}" ]; then
    if [ "$OBS" != "true" ]; then
      Info "* downloading ZSTD ${zstd_version}..."
      curl $curl_opts -O "${LP3D_LIBS_BASE}/zstd-${zstd_version}.tar.${arch_suffix}"
    else
      Info "ERROR - archive file zstd-${zstd_version}.tar.${arch_suffix} was not found."
    fi
  fi

  # ZStandard source directory
  if [ ! -d "zstd-${zstd_version}" ]; then
    Info -n "extracting ZStandard (tar.${arch_suffix})..."
    [ "${arch_suffix}" = "gz" ] && z_cat="gzip -dc" || :
    ($z_cat zstd-${zstd_version}.tar.${arch_suffix} | tar xf -) >$l.out 2>&1 && rm $l.out
    [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
  fi

  # ZStandard build directory
  if [ ! -d "${mesa_prefix}" ]; then
    Info "create install prefix..."
    mkdir -p "${mesa_prefix}"
  fi

  cd zstd-${zstd_version}/lib

  if [ ! -f "${mesa_prefix}/lib/libzstd.${lib_ext}" ]; then
    Info "build and install ZStandard..."
    # use 'make install ...' to build and install both shared and static libs
    make install-pc install-includes install-static prefix=${mesa_prefix}
  else
    Info "library ZStandard exist - build skipped."
  fi
fi

# elapsed execution time
ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
Info
Info "$ME Finsihed!"
Info "$ELAPSED"
Info "----------------------------------------------------"
