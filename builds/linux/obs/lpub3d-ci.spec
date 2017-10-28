%define icons_dir 2

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

Summary: An LDraw Building Instruction Editor
%if 0%{?scientificlinux_version}
%define dist scl
%endif

%if 0%{?rhel_version}
%define dist rhel
%endif

%if 0%{?centos_ver}
%define centos_version %{centos_ver}00
%define dist cos
%endif

# distro group settings
%if 0%{?suse_version} || 0%{?sles_version}
Group: Productivity/Graphics/Viewers
%endif

%if 0%{?mageia} || 0%{?rhel_version}
Group: Graphics
%endif

%if 0%{?suse_version} || 0%{?sles_version}
License: GPL-3.0+
BuildRequires: fdupes
%endif
%if 0%{?fedora} || 0%{?centos_version}
Group: Amusements/Graphics
%endif
%if 0%{?fedora} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?scientificlinux_version} || 0%{?mageia}
License: GPLv3+
%endif

# set packing platform
%define serviceprovider %(echo %{vendor})
%if %(if [[ "%{vendor}" == obs://* ]]; then echo 1; else echo 0; fi)
%define buildservice 1
%define packingplatform %(echo openSUSE BuildService)
%else
%define packingplatform %(echo $HOSTNAME [`uname`])
%endif

# set packer
%if 0%{?buildservice}==1
%define distpacker %(echo openSUSE BuildService [abuild])
%endif

# set custom dir paths
%define _3rdexedir /opt/lpub3d/3rdParty

# define git version string from source
Source10: lpub3d-ci.spec.git.version
%define gitversion 1.0.0

# preamble
Name: lpub3d-ci
Icon: lpub3d.xpm
Version: %{gitversion}
Release: %{?dist}
URL: https://trevorsandy.github.io/lpub3d
Vendor: Trevor SANDY
BuildRoot: %{_builddir}/%{name}
Requires: unzip
BuildRequires: freeglut-devel
Source0: lpub3d-ci-git.tar.gz
Source20: lpub3d-ci-rpmlintrc

# package requirements
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}
BuildRequires: qt5-qtbase-devel
%if 0%{?fedora}
BuildRequires: qt5-linguist
%endif
%if 0%{?buildservice}!=1
BuildRequires: git
%endif
BuildRequires: gcc-c++, make
%endif

%if 0%{?fedora} || 0%{?centos_version} || 0%{?scientificlinux_version}
BuildRequires: mesa-libOSMesa-devel
%endif

%if 0%{?mageia}
BuildRequires: qtbase5-devel
%ifarch x86_64
BuildRequires: lib64osmesa-devel
%if 0%{?buildservice}
BuildRequires: lib64sane1, lib64proxy-webkit
%endif
%else
BuildRequires: libosmesa-devel
%if 0%{?buildservice}
BuildRequires: libsane1, libproxy-webkit
%endif
%endif
%endif

%if 0%{?fedora}
%if 0%{?fedora_version}==22
BuildRequires: qca
%endif
%if 0%{?fedora_version}==23
BuildRequires: qca, gnu-free-sans-fonts
%endif
%endif

%if 0%{?buildservice}
%if 0%{?fedora_version}==25
BuildRequires: llvm-libs
%endif
%endif

%if 0%{?rhel_version} || 0%{?centos_version}
BuildRequires: libXext-devel
%endif

%if 0%{?suse_version}
BuildRequires: libqt5-qtbase-devel, zlib-devel
%if 0%{?buildservice}
BuildRequires: -post-build-checks
%endif
%endif

%if 0%{?suse_version} > 1300
BuildRequires: Mesa-devel
%endif

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
%setup -q -n %{name}-git

%build
export QT_SELECT=qt5

# use Qt5
%if 0%{?fedora}==23
%ifarch x86_64
export Q_CXXFLAGS="$Q_CXXFLAGS -fPIC"
%endif
%endif
if which qmake-qt5 >/dev/null 2>/dev/null ; then
	qmake-qt5 -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d
else
	qmake -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d
fi
make clean
make %{?_smp_mflags}

%install
make INSTALL_ROOT=%buildroot install
%if 0%{?suse_version} || 0%{?sles_version}
%fdupes %{buildroot}/%{icons_dir}
%endif
export NO_BRP_CHECK_RPATH=true

%clean
rm -rf $RPM_BUILD_ROOT

%files
%if 0%{?sles_version} || 0%{?suse_version}
%defattr(-,root,root)
%endif
%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%changelog
* Sat Oct 28 2017 - trevor.dot.sandy.at.gmail.dot.com 2.0.21.150
- LPub3D Linux package (rpm) release
