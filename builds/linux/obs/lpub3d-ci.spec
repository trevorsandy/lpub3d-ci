#
# spec file for LPub3D package
#
# Last Update: July 01, 2025
# Copyright © 2018 - 2025 Trevor SANDY
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
%define packingplatform %(. /etc/os-release 2>/dev/null; [ -n "$PRETTY_NAME" ] && echo "$PRETTY_NAME" || echo $HOSTNAME [`uname`])
%endif

# set packer
%define serviceprovider %(echo Trevor SANDY trevor.sandy@gmail.com)
%if 0%{?buildservice}
%define distpacker %(echo openSUSE Build Service [`whoami`])
%define targetplatform %{_target}
%else
%define distpacker %(echo `whoami`)
%define targetplatform %{packingplatform}
%endif

# set target platform id
# distinguish between OpenSUSE and SLE
%if (0%{?sles_version} && 0%{?sle_version}>=120000)
%define dist .SUSE%(echo %{sles_version} | sed 's/0$//')
%define suse_dist_name %(echo SUSE Linux Enterprise Server)
%define suse_dist_label %(echo %{suse_dist_name}...%{sle_version})
%else
%if 0%{?suse_version}
%define dist .openSUSE%(echo %{suse_version} | sed 's/0$//')
%define suse_dist_name %(echo OpenSUSE)
%define suse_dist_label %(echo %{suse_dist_name}.......................%{suse_version})
%endif
%endif

%if 0%{?centos_ver}
%define centos_version %{centos_ver}00
%endif

%if 0%{?mageia}
%define mageia_version %{mageia}
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

%if 0%{?centos_version} || 0%{?fedora} || 0%{?mageia} || 0%{?rhel_version} || 0%{?scientificlinux_version}
License: GPLv2+
%endif

%if 0%{?suse_version} || 0%{?sles_version}
License: GPL-2.0+
BuildRequires: fdupes
%endif

# set directory paths
%define _lp3d_3rd_exec_dir /usr/bin/lpub3d/3rdParty
%define _iconsdir %{_datadir}/icons

# preamble
Summary: An LDraw Building Instruction Editor
Name: lpub3d-ci
Icon: lpub3d.xpm
Version: 2.4.9.4563
Release: 1%{?dist}
URL: https://trevorsandy.github.io/lpub3d
Vendor: Trevor SANDY
BuildRoot: %{_builddir}/%{name}
BuildRequires: unzip
Source0: lpub3d-ci-git.tar.gz
Source10: lpub3d-ci-rpmlintrc

# package requirements
%if 0%{?fedora} || 0%{?centos_version} || 0%{?rhel_version}
BuildRequires: qt6-qtbase-devel, qt6-qttools-devel
BuildRequires: hostname
BuildRequires: gcc-c++, make
BuildRequires: git
%if 0%{?fedora_version}>30
BuildRequires: libXext-devel
%endif
%endif

%if 0%{?suse_version}
BuildRequires: libqt6-qtbase-devel
BuildRequires: update-desktop-files hostname
BuildRequires: zlib-devel
%if 0%{?buildservice}
BuildRequires: -post-build-checks
%endif
%endif

%if 0%{?mageia}
BuildRequires: qtbase6-devel, qttools6
%endif

# configuration settings
%description
 LPub3D is an Open Source WYSIWYG editing application for creating
 LEGO® style digital building instructions. LPub3D is developed and
 maintained by Trevor SANDY. It uses the LDraw™ parts library, the
 most comprehensive library of digital Open Source LEGO® bricks
 available (www.ldraw.org/ ) and reads the LDraw LDR and MPD model
 file formats. LPub3D is available for free under the GNU Public License v3
 and runs on Windows, Linux and macOS Operating Systems.
 Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague,
 LeoCAD© 2022 Leonardo Zide.and additional third party components.
 LEGO® is a trademark of the LEGO Group of companies which does not
 sponsor, authorize or endorse this application.
 Copyright © 2015 - 2025 Trevor SANDY

%prep
set +x
%if 0%{?suse_version} || 0%{?sles_version}
echo "%{suse_dist_label}"
%endif
%if 0%{?centos_ver}
echo "CentOS.........................%{centos_version}"
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
echo "Mageia.........................%{mageia_version}"
%endif
%if 0%{?buildservice}
echo "Using OpenBuildService.........%{usingbuildservice}"
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
set +x
echo "Current working directory: $PWD"
[ -f "../../SOURCES/complete.zip" ] && SrcPath=../../SOURCES || SrcPath=../../../SOURCES
# copy ldraw archive libraries
for LDrawLibFile in \
  ${SrcPath}/complete.zip \
  ${SrcPath}/lpub3dldrawunf.zip \
  ${SrcPath}/tenteparts.zip \
  ${SrcPath}/vexiqparts.zip; do
  LibFile="$(basename ${LDrawLibFile})"
  if [ -f "${LDrawLibFile}" ]; then
    if [ "${LibFile}" = "complete.zip" ]; then
      cp -f ${LDrawLibFile} ../ || \
      echo "Error: ${LibFile} copy to $(readlink -e ../) failed."
    fi
    mv -f ${LDrawLibFile} mainApp/extras/ || \
	echo "Error: ${LibFile} move to $(readlink -e mainApp/extras) failed."
  else
    echo "Error: ${LDrawLibFile} not found."
  fi
done
# move 3rd party renderer source archives and build renderers
for TarballFile in \
  ${SrcPath}/ldglite.tar.gz \
  ${SrcPath}/ldview.tar.gz \
  ${SrcPath}/povray.tar.gz; do
  LibFile="$(basename ${TarballFile})"
  if [ -f "${TarballFile}" ]; then
    mv -f ${TarballFile} ../ || \
    echo "Error: ${LibFile} move to $(readlink -e ../) failed."
  else
    echo "Error: ${TarballFile} not found."
  fi
done
set -x
# indicate OBS status (should always be No for this spec file)
export OBS=%{usingbuildservice}
# RPM exported variables
export TARGET_VENDOR=%{_target_vendor}
export PLATFORM_CODE="fc"
export TARGET_CPU=%{_target_cpu}
export PLATFORM_VER=%{fedora}
export LPUB3D=%{name}
export RPM_BUILD=true
export LDRAWDIR=${HOME}/ldraw
# 3rd-party renderers build-from-source requirements
# build 3rd-party renderers
export WD=$(readlink -e ../)
%if 0%{?buildservice}
%define _lp3d_cpu_cores %(echo `nproc`)
%define _lp3d_log_path %(echo `pwd`)
%define _lp3d_3rd_dist_dir %(echo lpub3d_linux_3rdparty)
%endif
export LP3D_LOG_PATH="%{_lp3d_log_path}"; \
export LP3D_CPU_CORES="%{_lp3d_cpu_cores}"; \
export LP3D_3RD_DIST_DIR="%{_lp3d_3rd_dist_dir}"; \
chmod a+x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# Qt setup
if which qmake-qt6 >/dev/null 2>&1; then
  QMAKE_EXEC=qmake-qt6
elif which qmake >/dev/null 2>&1; then
  QMAKE_EXEC=qmake
fi
echo && ${QMAKE_EXEC} -v && echo
# configure and build LPub3d
${QMAKE_EXEC} -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=build_check CONFIG-=debug_and_release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d
make clean
make %{?_smp_mflags}
# check lpub3d dependencies
lp3drelease="32bit_release" && [[ "${TARGET_CPU}" = "x86_64" || "${TARGET_CPU}" = "aarch64" ]] && lp3drelease="64bit_release"
versuffix=$(cat builds/utilities/version.info | cut -d " " -f 1-2 | sed s/" "//g)
validexe="mainApp/${lp3drelease}/lpub3d${versuffix}"
[ -f "${validexe}" ] && echo "LDD check lpub3d${versuffix}..." && ldd ${validexe} 2>/dev/null || \
echo "ERROR - LDD check failed for $(realpath ${validexe})"

%install
make INSTALL_ROOT=%buildroot install
%if 0%{?suse_version}
%suse_update_desktop_file lpub3d Graphics 3DGraphics Publishing Viewer Education Engineering
%endif
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
%{_datadir}/lpub3d/*
%{_datadir}/icons/*
#  {_datadir}/metainfo/*
%{_datadir}/mime/packages/*
%{_datadir}/applications/*
%attr(755,-,-) %{_lp3d_3rd_exec_dir}/*
%attr(644,-,-) %{_mandir}/man1/*
%attr(644,-,-) %doc %{_docdir}/lpub3d/*
%attr(644,-,-) %{_iconsdir}/hicolor/scalable/mimetypes/*

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

%changelog
* Tue Jul 01 2025 - trevor.dot.sandy.at.gmail.dot.com 2.4.9.4563
- LPub3D 2.4.9 enhancements and fixes - see RELEASE_NOTES for details

* Tue Jan 07 2025 - trevor dot sandy at gmail dot com 2.4.9.4047
- LPub3D 2.4.9 enhancements and fixes - see RELEASE_NOTES for details

* Fri Oct 18 2024 - trevor dot sandy at gmail dot com 2.4.8.3953
- LPub3D 2.4.8 enhancements and fixes - see RELEASE_NOTES for details

* Wed Jun 21 2023 - trevor dot sandy at gmail dot com 2.4.7.3476
- LPub3D 2.4.7 enhancements and fixes - see RELEASE_NOTES for details

* Wed Nov 30 2022 - trevor dot sandy at gmail dot com 2.4.6.3001
- LPub3D 2.4.6 enhancements and fixes - see RELEASE_NOTES for details

* Tue Nov 22 2022 - trevor dot sandy at gmail dot com 2.4.5.2977
- LPub3D 2.4.5 enhancements and fixes - see RELEASE_NOTES for details

* Tue May 31 2022 - trevor dot sandy at gmail dot com 2.4.4.2670
- LPub3D 2.4.4 enhancements and fixes - see RELEASE_NOTES for details

* Fri May 14 2021 - trevor dot sandy at gmail dot com 2.4.3.2557
- LPub3D 2.4.3 enhancements and fixes - see RELEASE_NOTES for details

* Wed Mar 31 2021 - trevor dot sandy at gmail dot com 2.4.2.2432
- LPub3D 2.4.2 enhancements and fixes - see RELEASE_NOTES for details

* Mon Mar 15 2021 - trevor dot sandy at gmail dot com 2.4.1.2389
- LPub3D 2.4.1 enhancements and fixes - see RELEASE_NOTES for details

* Tue Mar 10 2020 - trevor dot sandy at gmail dot com 2.4.0.1990
- LPub3D 2.4.0 enhancements and fixes - see RELEASE_NOTES for details

* Tue Sep 10 2019 - trevor dot sandy at gmail dot com 2.3.13.1511
- LPub3D 2.3.13 enhancements and fixes - see RELEASE_NOTES for details

* Thu Jun 20 2019 - trevor dot sandy at gmail dot com 2.3.12.1356
- LPub3D 2.3.12 enhancements and fixes - see RELEASE_NOTES for details

* Fri May 17 2019 - trevor dot sandy at gmail dot com 2.3.11.1269
- LPub3D 2.3.11 enhancements and fixes - see RELEASE_NOTES for details

* Tue Apr 23 2019 - trevor dot sandy at gmail dot com 2.3.10.1240
- LPub3D 2.3.10 enhancements and fixes - see RELEASE_NOTES for details

* Thu Apr 18 2019 - trevor dot sandy at gmail dot com 2.3.9.1228
- LPub3D 2.3.9 enhancements and fixes - see RELEASE_NOTES for details

* Mon Apr 08 2019 - trevor dot sandy at gmail dot com 2.3.8.1210
- LPub3D 2.3.8 enhancements and fixes - see RELEASE_NOTES for details

* Wed Mar 27 2019 - trevor dot sandy at gmail dot com 2.3.7.1195
- LPub3D 2.3.7 enhancements and fixes - see RELEASE_NOTES for details

* Wed Dec 19 2018 - trevor dot sandy at gmail dot com 2.3.6.1101
- LPub3D 2.3.6 enhancements and fixes - see RELEASE_NOTES for details

* Mon Dec 17 2018 - trevor dot sandy at gmail dot com 2.3.5.1092
- LPub3D 2.3.5 enhancements and fixes - see RELEASE_NOTES for details

* Mon Dec 10 2018 - trevor dot sandy at gmail dot com 2.3.4.1084
- LPub3D 2.3.4 enhancements and fixes - see RELEASE_NOTES for details

* Sat Oct 27 2018 - trevor dot sandy at gmail dot com 2.3.3.1034
- LPub3D 2.3.3 enhancements and fixes - see RELEASE_NOTES for details

* Fri Oct 05 2018 - trevor dot sandy at gmail dot com 2.3.2.998
- LPub3D 2.3.2 enhancements and fixes - see RELEASE_NOTES for details

* Wed Oct 03 2018 - trevor dot sandy at gmail dot com 2.3.1.977
- LPub3D 2.3.1 enhancements and fixes - see RELEASE_NOTES for details

* Tue Sep 04 2018 - trevor dot sandy at gmail dot com 2.3.0.887
- LPub3D 2.3.0 enhancements and fixes - see RELEASE_NOTES for details

* Thu Apr 12 2018 - trevor dot sandy at gmail dot com 2.2.2.848
- LPub3D 2.2.2 enhancements and fixes - see RELEASE_NOTES for details

* Sat Mar 31 2018 - trevor dot sandy at gmail dot com 2.2.1.824
- LPub3D 2.2.1 enhancements and fixes - see RELEASE_NOTES for details

* Fri Mar 16 2018 - trevor dot sandy at gmail dot com 2.2.0.795
- LPub3D 2.2.0 enhancements and fixes - see RELEASE_NOTES for details

* Mon Feb 12 2018 - trevor dot sandy at gmail dot com 2.1.0.775
- LPub3D 2.1.0 enhancements and fixes - see RELEASE_NOTES for details

* Fri Feb 10 2017 - trevor dot sandy at gmail dot com 2.0.20.645
- LPub3D 2.0.20 enhancements and fixes - see RELEASE_NOTES for details
