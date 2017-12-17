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
Version: 2.0.21.262
Release: %{?dist}
URL: https://trevorsandy.github.io/lpub3d
Vendor: Trevor SANDY
BuildRoot: %{_builddir}/%{name}
BuildRequires: unzip, sudo
Source0: lpub3d-ci-git.tar.gz
Source10: lpub3d-ci-rpmlintrc

# package requirements
%if 0%{?fedora} || 0%{?centos_version}
BuildRequires: qt5-qtbase-devel, qt5-qttools-devel
BuildRequires: gcc-c++, make
%if 0%{?fedora}
BuildRequires: qt5-linguist
%endif
%if 0%{?buildservice}!=1
BuildRequires: git
%endif
%endif

%if 0%{?suse_version}
BuildRequires: libqt5-qtbase-devel
BuildRequires: zlib-devel
%if 0%{?suse_version} > 1300
BuildRequires: Mesa-devel
%endif
%if 0%{?buildservice}
BuildRequires: -post-build-checks
%endif
%endif

%if 0%{?mageia}
BuildRequires: qtbase5-devel, qttools5
%ifarch x86_64
%if 0%{?buildservice}
BuildRequires: lib64sane1, lib64proxy-webkit
%endif
%else
%if 0%{?buildservice}
BuildRequires: libsane1, libproxy-webkit
%endif
%endif
%endif

%if 0%{?fedora}
%if 0%{?fedora_version}==23
BuildRequires: qca, gnu-free-sans-fonts
%endif
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
%if 0%{?buildservice}==1
echo "OpenBuildService Target........%{_target_vendor}"
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
* Sun Dec 17 2017 - trevor.dot.sandy.at.gmail.dot.com 2.0.21.262

* Wed Dec 13 2017 - trevor.dot.sandy.at.gmail.dot.com 2.0.21.237
- LPub3D Linux package (rpm) release
