#
# spec file for package lpub3d
#
# Copyright © 2017 Trevor SANDY
# Using RPM Spec file examples by Thomas Baumgart, Peter Bartfai and others
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to Trevor SANDY <trevor.sandy@gmail.com>
#

# set packing platform
%if %(if [[ "%{vendor}" == obs://* ]]; then echo 1; else echo 0; fi)
%define buildservice 1
%define usingbuildservice true
%define packingplatform %(echo openSUSE Build Service)
%else
%define usingbuildservice false
%define packingplatform %(source /etc/os-release && if [ "$PRETTY_NAME" != "" ]; then echo $HOSTNAME ["$PRETTY_NAME"]; else echo $HOSTNAME [`uname`]; fi)
%endif

# set packer
%define serviceprovider %(echo Trevor SANDY trevor.sandy@gmail.com)
%if 0%{?buildservice}==1
%define distpacker %(echo openSUSE Build Service [`whoami`])
%define targetplatform %{_target}
%else
%define distpacker %(echo `whoami`)
%define targetplatform %{packingplatform}
%endif

# set target platform id
%if 0%{?suse_version}
%define dist .openSUSE%(echo %{suse_version} | sed 's/0$//')
%endif

%if 0%{?sles_version}
%define dist .SUSE%(echo %{sles_version} | sed 's/0$//')
%define build_sdl2 1
%endif

%if 0%{?fedora}
%define dist fc
%endif

%if 0%{?mageia}
%define dist .mga%{mgaversion}
%define distsuffix .mga%{mgaversion}
%endif

%if 0%{?centos_ver}
%define centos_version %{centos_ver}00
%define dist cos
%define build_sdl2 1
%endif

%if 0%{?rhel_version}
%define dist rhel
%endif

%if 0%{?scientificlinux_version}
%define dist scl
%endif

# distro group settings
%if 0%{?suse_version} || 0%{?sles_version}
Group: Productivity/Graphics/Viewers
%endif

%if 0%{?mageia} || 0%{?rhel_version}
Group: Graphics
%endif

%if 0%{?centos_version} || 0%{?fedora}
Group: Amusements/Graphics
%endif

%if 0%{?centos_version} || 0%{?fedora} || 0%{?mageia}
License: GPLv3+
%endif

%if 0%{?suse_version} || 0%{?sles_version}
License: GPL-2.0+
BuildRequires: fdupes
%endif

# set custom directory paths
%define _3rdexedir /opt/lpub3d/3rdParty
%define _iconsdir %{_datadir}/icons

# preamble
Summary: An LDraw Building Instruction Editor
Name: lpub3d-ci
Icon: lpub3d.xpm
Version: 2.0.20.335
Release: %{?dist}
URL: https://trevorsandy.github.io/lpub3d
Vendor: Trevor SANDY
BuildRoot: %{_builddir}/%{name}
BuildRequires: unzip
Source0: lpub3d-ci-git.tar.gz
Source10: lpub3d-ci-rpmlintrc

# package requirements
%if 0%{?fedora} || 0%{?centos_version}
BuildRequires: qt5-qtbase-devel, qt5-qttools-devel
BuildRequires: mesa-libOSMesa-devel, mesa-libGLU-devel, OpenEXR-devel
BuildRequires: gcc-c++, make, libpng-devel
%if 0%{?fedora}
BuildRequires: qt5-linguist, SDL2-devel
%endif
%if 0%{?buildservice}!=1
BuildRequires: git
%endif
%endif

%if 0%{?fedora} || 0%{?centos_version} || 0%{?suse_version}
BuildRequires: freeglut-devel, boost-devel, libtiff-devel
%endif

%if (0%{?rhel_version}>=700 && 0%{?centos_version}>=700 && 0%{?fedora}>=26)
BuildRequires: libjpeg-turbo-devel
%endif

%if 0%{?fedora}
%define build_osmesa 1
BuildRequires: tinyxml-devel, gl2ps-devel
%if 0%{?buildservice}
BuildRequires: samba4-libs
%if 0%{?fedora_version}==23
BuildRequires: qca, gnu-free-sans-fonts
%endif
%if 0%{?fedora_version}==25
BuildRequires: llvm-libs
%endif
%if 0%{?fedora_version}==26
BuildRequires: openssl-devel, storaged
%endif
%endif
%endif

%if 0%{?suse_version}
BuildRequires: libqt5-qtbase-devel
BuildRequires: libOSMesa-devel, glu-devel, openexr-devel
BuildRequires: cmake, update-desktop-files
BuildRequires: zlib-devel
%if 0%{?suse_version}!=1315
BuildRequires: libpng16-compat-devel, libjpeg8-devel, libqt5-linguist
%endif
Requires(pre): gconf2
%if (0%{?suse_version} > 1210 && 0%{?suse_version}!=1315)
BuildRequires: gl2ps-devel
%endif
%if 0%{?suse_version} > 1220
BuildRequires: glu-devel
%endif
%if 0%{?suse_version} > 1300
BuildRequires: Mesa-devel
%endif
%if 0%{?buildservice}
BuildRequires: -post-build-checks
%endif
%endif
%if (0%{?suse_version} && !0%{?sles_version})
BuildRequires: libSDL2-devel
%endif

%if 0%{?mageia}
BuildRequires: qttools5
%ifarch x86_64
BuildRequires: lib64qt5base5-devel, lib64sdl2.0-devel, lib64osmesa-devel, lib64mesaglu1-devel, lib64freeglut-devel, lib64boost-devel, lib64tinyxml-devel, lib64gl2ps-devel, lib64tiff-devel
%if 0%{?mgaversion} > 5
BuildRequires: lib64openexr-devel
%endif
%if 0%{?buildservice}
BuildRequires: lib64sane1, lib64proxy-webkit
%endif
%else
BuildRequires: libqt5base5-devel, libsdl2.0-devel, libosmesa-devel, libmesaglu1-devel, freeglut-devel, libboost-devel, libtinyxml-devel, libgl2ps-devel, libtiff-devel
%if 0%{?mgaversion} > 5
BuildRequires: libopenexr-devel
%endif
%if 0%{?buildservice}
BuildRequires: libsane1, libproxy-webkit
%endif
%endif
%endif

# build OSMesa and libGLU from source
%if 0%{?build_osmesa}
%define buildosmesa yes
# libGLU build-from-source dependencies
BuildRequires:  gcc-c++
BuildRequires:  libtool
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(gl)
# libMesa build-from-source dependencies
%define libglvnd 0
%if 0%{?suse_version} >= 1330
%define libglvnd 1
%endif
%define glamor 1
%define _name_archive mesa
%define _version 17.2.6
%define with_opencl 0
%define with_vulkan 0
%ifarch %ix86 x86_64
%define gallium_loader 1
%else
%define gallium_loader 0
%endif
%ifarch %ix86 x86_64
%define xvmc_support 1
%define vdpau_nouveau 1
%define vdpau_radeon 1
%else
%define xvmc_support 0
%define vdpau_nouveau 0
%define vdpau_radeon 0
%endif
%ifarch %ix86 x86_64
%define with_nine 1
%endif
%if 0%{gallium_loader} && 0%{?suse_version} >= 1330
%ifarch %ix86 x86_64
%define with_vulkan 1
%endif
%endif
BuildRequires:  autoconf >= 2.60
BuildRequires:  automake
BuildRequires:  bison
BuildRequires:  fdupes
BuildRequires:  flex
BuildRequires:  gcc-c++
BuildRequires:  imake
BuildRequires:  libtool
BuildRequires:  pkgconfig
%if 0%{?fedora}
BuildRequires:  python
BuildRequires:  python-libs
%else
BuildRequires:  python-xml
BuildRequires:  python-base
BuildRequires:  pkgconfig(libdrm) >= 2.4.75
BuildRequires:  pkgconfig(libdrm_amdgpu) >= 2.4.79
BuildRequires:  pkgconfig(libdrm_nouveau) >= 2.4.66
BuildRequires:  pkgconfig(libdrm_radeon) >= 2.4.71
%endif
BuildRequires:  python-mako
BuildRequires:  pkgconfig(dri2proto)
BuildRequires:  pkgconfig(dri3proto)
BuildRequires:  pkgconfig(expat)
BuildRequires:  pkgconfig(glproto)
%if 0%{?libglvnd}
BuildRequires:  pkgconfig(libglvnd) >= 0.1.0
%endif
BuildRequires:  pkgconfig(libkms) >= 1.0.0
BuildRequires:  pkgconfig(libudev) > 151
BuildRequires:  pkgconfig(libva)
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(presentproto)
BuildRequires:  pkgconfig(vdpau) >= 1.1
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xcb-dri2)
BuildRequires:  pkgconfig(xcb-dri3)
BuildRequires:  pkgconfig(xcb-glx)
BuildRequires:  pkgconfig(xcb-present)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xshmfence)
BuildRequires:  pkgconfig(xvmc)
BuildRequires:  pkgconfig(xxf86vm)
BuildRequires:  pkgconfig(zlib)
%ifarch x86_64 %ix86
%if 0%{?fedora}
BuildRequires:  elfutils
BuildRequires:  elfutils-libelf-devel
BuildRequires:  libdrm-devel
%else
BuildRequires:  libelf-devel
BuildRequires:  pkgconfig(libdrm_intel) >= 2.4.75
%endif
%else
%if 0%{with_opencl}
BuildRequires:  libelf-devel
%endif
%endif
%if 0%{?suse_version} > 1320 || (0%{?sle_version} >= 120300 && 0%{?is_opensuse})
BuildRequires:  pkgconfig(wayland-client) >= 1.11
BuildRequires:  pkgconfig(wayland-protocols) >= 1.8
BuildRequires:  pkgconfig(wayland-server) >= 1.11
%endif
%ifarch %ix86 x86_64
BuildRequires:  llvm-devel
BuildRequires:  ncurses-devel
%endif
%if 0%{with_opencl}
BuildRequires:  clang-devel
BuildRequires:  clang-devel-static
BuildRequires:  libclc
%endif
%if 0%{?libglvnd}
Requires:       Mesa-libEGL1  = %{version}
Requires:       Mesa-libGL1  = %{version}
Requires:       libglvnd >= 0.1.0
%endif
%endif

# build SDL2 from source
%if 0%{?build_sdl2}
%define builsdl2 yes
BuildRequires:  cmake
BuildRequires:  dos2unix
BuildRequires:  gcc-c++
BuildRequires:  nasm
BuildRequires:  pkg-config
BuildRequires:  pkgconfig(alsa) >= 0.9.0
BuildRequires:  pkgconfig(dbus-1)
%if !0%{?sle_version}
BuildRequires:  pkgconfig(fcitx)
%endif
BuildRequires:  pkgconfig(egl)
BuildRequires:  pkgconfig(gl)
BuildRequires:  pkgconfig(glesv1_cm)
BuildRequires:  pkgconfig(glesv2)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(glu)
BuildRequires:  pkgconfig(ibus-1.0)
BuildRequires:  pkgconfig(ice)
# KMS/DRM driver needs libdrm and libgbm
BuildRequires:  pkgconfig(gbm) >= 9.0.0
BuildRequires:  pkgconfig(libdrm) >= 2.4.46
%if 0%{?suse_version} > 1220
BuildRequires:  pkgconfig(tslib)
%endif
BuildRequires:  pkgconfig(libpulse-simple) >= 0.9
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(udev)
BuildRequires:  pkgconfig(wayland-server)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xinerama)
BuildRequires:  pkgconfig(xrandr)
BuildRequires:  pkgconfig(xscrnsaver)
BuildRequires:  pkgconfig(xxf86vm)
%endif

# configuration settings
%description
 LPub3D is an Open Source WYSIWYG editing application for creating
 LEGO® style digital building instructions. LPub3D is developed and
 maintained by Trevor SANDY. It uses the LDraw™ parts library, the
 most comprehensive library of digital Open Source LEGO® bricks
 available (www.ldraw.org/ ) and reads the LDraw LDR and MPD model
 file formats. LPub3D is available for free under the GNU Public License v3
 and runs on Windows, Linux and OSX Operating Systems.
 Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague,
 LeoCAD© 2015 Leonardo Zide.and additional third party components.
 LEGO® is a trademark of the LEGO Group of companies which does not
 sponsor, authorize or endorse this application.
 © 2015-2017 Trevor SANDY

%prep
set +x
%if 0%{?suse_version}
echo "OpenSUSE.......................%{suse_version}"
%endif
%if 0%{?sles_version}
echo "SUSE Linux Enterprise Server...%{sles_version}"
%endif
%if 0%{?centos_ver}
echo "CentOS.........................%{centos_ver}"
%endif
%if 0%{?fedora}
echo "Fedora.........................%{fedora}"
%endif
%if 0%{?rhel_version}
echo "RedHat Enterprise Linux........%{rhel_version}"
%endif
%if 0%{?scientificlinux_version}
echo "Scientific Linux...............%{scientificlinux_version}"
%endif
%if 0%{?mageia}
echo "Mageia.........................%{mageia}"
%endif
echo "Using OpenBuildService.........%{usingbuildservice}"
%if 0%{?buildservice}
echo "OpenBuildService Target........%{_target_vendor}"
%endif
%if 0%{?build_osmesa}
echo "Build OSMesa from source.......%{buildosmesa}"
%endif
%if 0%{?build_sdl2}
echo "Build SDL2 from source.........%{builsdl2}"
%endif
echo "Target.........................%{_target}"
echo "Target Vendor..................%{_target_vendor}"
echo "Target CPU.....................%{_target_cpu}"
echo "Name...........................%{name}"
echo "Summary........................%{summary}"
echo "Version........................%{version}"
echo "Vendor.........................%{vendor}"
echo "Release........................%{release}"
echo "Distribution packer............%{distpacker}"
echo "Source0........................%{SOURCE0}"
echo "Source20.......................%{SOURCE10}"
echo "Service Provider...............%{serviceprovider}"
echo "Packing Platform...............%{packingplatform}"
echo "Build Package..................%{name}-%{version}-%{release}-%{_arch}.rpm"
set -x
%setup -q -n %{name}-git

%build
export OBS=%{usingbuildservice}
export TARGET_VENDOR=%{_target_vendor}
export QT_SELECT=qt5
# instruct qmake to install 3rd-party renderers
export LP3D_BUILD_PKG=yes
#set +x
echo "Current working directory: $PWD"
# download ldraw archive libraries
LDrawLibOffical=../../SOURCES/complete.zip
LDrawLibUnofficial=../../SOURCES/lpub3dldrawunf.zip
if [ -f ${LDrawLibOffical} ] ; then
  cp ${LDrawLibOffical} mainApp/extras && echo "LDraw archive library complete.zip copied to $(readlink -e mainApp/extras)"
  cp ${LDrawLibOffical} ../ && echo "LDraw archive library complete.zip copied to $(readlink -e ../)"
else
  echo "LDraw archive library complete.zip not found at $(readlink -e ../SOURCES)!"
fi
if [ -f ${LDrawLibUnofficial} ] ; then
  cp ${LDrawLibUnofficial} mainApp/extras && echo "LDraw archive library complete.zip copied to $(readlink -e mainApp/extras)"
else
  echo "LDraw archive library lpub3dldrawunf.zip not found at $(readlink -e ../SOURCES)!"
fi
# Copy 3rd party renderer source archives and build renderers
for ArchiveSourceFile in \
  ../../SOURCES/ldglite.tar.gz \
  ../../SOURCES/ldview.tar.gz \
  ../../SOURCES/povray.tar.gz \
  ../../SOURCES/mesa-17.2.6.tar.gz \
  ../../SOURCES/glu-9.0.0.tar.bz2; do
  if [ -f "${ArchiveSourceFile}" ]; then
    mv -f ${ArchiveSourceFile} ../ && echo "$(basename ${ArchiveSourceFile}) moved to $(readlink -e ../)"
  fi
done
#set -x
# 3rd-party renderers build-from-source requirements
%if 0%{?centos_version}
export build_sdl2=1
export build_tinyxml=1
export build_gl2ps=1
%endif
%if 0%{?suse_version}
export build_tinyxml=1
%endif
%if (0%{?suse_version} < 1210 || 0%{?suse_version}==1315)
export build_gl2ps=1
%endif
%if 0%{?sles_version}
%define osmesa_found %(test -f /usr/lib/libOSMesa.so -o -f /usr/lib64/libOSMesa.so && echo 1 || echo 0)
%if "%{osmesa_found}" != "1"
%define build_osmesa 1
export build_osmesa=%{?build_osmesa}
%endif
export build_sdl2=1
export build_tinyxml=1
%endif
# build 3rd-party renderers
export WD=$(readlink -e ../); \
chmod +x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# use Qt5
%if 0%{?fedora}==23
%ifarch x86_64
export Q_CXXFLAGS="$Q_CXXFLAGS -fPIC"
%endif
%endif
if which qmake-qt5 >/dev/null 2>/dev/null ; then
  qmake-qt5 -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG-=debug_and_release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d
else
  qmake -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG-=debug_and_release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d
fi
make clean
make %{?_smp_mflags}

%install
make INSTALL_ROOT=%buildroot install
%if 0%{?suse_version} || 0%{?sles_version}
%fdupes %{buildroot}/%{_iconsdir}
%endif
# skip rpath check on 3rd-party binaries
export NO_BRP_CHECK_RPATH=true

%clean
rm -rf $RPM_BUILD_ROOT

%files
%if 0%{?sles_version} || 0%{?suse_version}
%defattr(-,root,root)
%endif
%{_bindir}/*
%{_libdir}/*
%{_datadir}/pixmaps/*
%{_datadir}/mime/packages/*
%{_datadir}/applications/*
%{_datadir}/lpub3d
%{_3rdexedir}/*
%dir %{_iconsdir}/hicolor/
%dir %{_iconsdir}/hicolor/scalable/
%dir %{_iconsdir}/hicolor/scalable/mimetypes/
%attr(644,-,-) %{_iconsdir}/hicolor/scalable/mimetypes/*
%attr(644,-,-) %doc %{_docdir}/lpub3d
%attr(644,-,-) %{_mandir}/man1/*
%attr(755,-,-) %{_3rdexedir}/*
%post -p /sbin/ldconfig
%if 0%{?buildservice}!=1
update-mime-database  /usr/share/mime >/dev/null || true
update-desktop-database || true
%endif

%postun -p /sbin/ldconfig
%if 0%{?buildservice}!=1
update-mime-database /usr/share/mime >/dev/null || true
update-desktop-database || true
%endif

* Sun Dec 31 2017 - trevor.dot.sandy.at.gmail.dot.com 2.0.20.335
- LPub3D Linux package (rpm) release
