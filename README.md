![Icon][icon]
# LPub3D  (CI Testing)
An LDraw™ editor for LEGO® style digital building instructions.
  
| [![GitHub release][gh-rel-badge]][gh-rel-url]| [![Travis-CI build status][travis-badge]][travis-url] | [![AppVeyor build status][appveyor-badge]][appveyor-url] | [![GitHub top language][gh-top-lang-badge]][gh-top-lang-url] |
| :---        |    :----:   |    :----:   |          ---: |
| [![SourceForge weekly downloads][sf-dw-badge]][sf-dw-badge-url] | [![SourceForge total downloads][sf-dt-badge]][sf-dt-badge-url] | [![GitHub total downloads][gh-dl-badge]][gh-dl-url] | [![Maintained status][maintained-badge]](README.md "Last edited 07-01-2018") |
| [![GitHub commits (since latest release)][gh-comm-since-badge]][gh-comm-since-url] | [![GitHub issues][gh-issue-badge]][gh-issue-url] | [![GitHub pull requests][gh-pull-req-badge]][gh-pull-req-url] | [![GitHub last commit][gh-lst-commit-badge]][gh-lst-commit-url] |

**LPub3D** is an Open Source WYSIWYG editing application for creating LEGO® style digital building instructions.
LPub3D is developed and maintained by Trevor SANDY. It uses the LDraw parts library, the most comprehensive
library of digital Open Source LEGO® bricks available (www.ldraw.org/ ) and reads the LDraw LDR and MPD model file formats.

LPub3D provides three popular LDraw image renderers - custom adapted for tight integration and robust performance.
 - [LDGLite][ldglite] - imbedded console, fast image rendering
 - [LDView][ldview] - imbedded console, feature rich rendering and pov file generation
 - [LPub3D-Trace (POV-Ray)][povray] - imbedded console, high quality image and feature rich image rendering

Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague, LeoCAD© 2015 Leonardo Zide and additional third party components. See [Credits][credits] for full details on contributing content.

### Compatibility
LPub3D runs on Debian, Red Hat and Arch derived Linux distros, macOS and Windows Operating Systems.
 - [GitHub Releases][githubreleases]
 - [Sourceforge Releases][sfreleases]
 - [Open Build Service Releases][obsreleases]
 
|[![Appimage-Icon][appimage-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/AppImage/)|[![Arch-Icon][arch-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/Arch_Extra/)|[![Centos-Icon][centos-icon]]( https://download.opensuse.org/repositories/home:/trevorsandy/CentOS_7/)|[![Debian-Icon][debian-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/Debian_9.0/)|[![Fedora-Icon][fedora-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/Fedora_27/)|[![Mageia-Icon][mageia-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/Mageia_6/)|[![Macos-Icon][macos-icon]](https://github.com/trevorsandy/lpub3d-ci/releases/download/v2.1.0/LPub3D-2.1.0.0.363_20180103_macos.dmg)|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|[![Opensuse-Icon][opensuse-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/openSUSE_Factory/)|[![Rhel-Icon][rhel-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/RHEL_7/)|[![Sl-Icon][sl-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/ScientificLinux_7/)|[![Sle-Icon][sle-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/SLE_12_SP3/)|[![Ubuntu-Icon][ubuntu-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/xUbuntu_17.10/)|[![Univention-Icon][univention-icon]](https://download.opensuse.org/repositories/home:/trevorsandy/Univention_4.2/)|[![Windows-Icon][windows-icon]](https://github.com/trevorsandy/lpub3d-ci/releases/download/v2.1.0/LPub3D-2.1.0.0.363_20180103.exe)|

### Quick Links
 - [Build LPub3D package from source][buildfromsource]
 - [Change log][changelog]
 - [Credits][credits]

### License
LPub3D is available for free under the [GNU Public License version 3.0 (GPLv3)][copying].

### Disclaimer
LEGO® is a trademark of the LEGO Group of companies which does not sponsor, authorize or endorse this application.

### Features
 - Integrated 3D Viewer powered by LeoCAD
 - Set ROTSTEP (step rotation) meta using 3D Viewer
 - Fade previous step's parts (Fadestep)
 - Unlimited part list (PLI) annotations
 - Split the BOM (divide parts over number of BOM occurrences)
 - Sort PLI/BOM by part size, colour and category
 - "Go To" page navigation and submodel display via drop-down dialogue
 - Automatically generate formatted cover pages
 - Multiple renderers (LDGlite, LDView and POV-Ray)
 - POV-Ray high-quality rendered instructions
 - LGEO high-quality rendering library support
 - Customizable content page header and footer
 - Text and picture (e.g. logo) front and back page attributes
 - Print/Export preview
 - PDF, PNG, JPG and BMP export formats
 - Standard and custom page size options
 - On demand, software and LDraw archive library updates
 - All features from legacy LPub - enhanced and stabilized
 - Linux, Mac OSX and Microsoft Windows distributions
 - Automated installer and portable distributions available
 - Full capabilities to build from source or Build Service

[icon]:                https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/mainApp/images/lpub3d128.png
[changelog]:           https://github.com/trevorsandy/lpub3d-ci/blob/master/mainApp/docs/README.txt
[credits]:             https://github.com/trevorsandy/lpub3d-ci/blob/master/mainApp/docs/CREDITS.txt
[copying]:             https://github.com/trevorsandy/lpub3d-ci/blob/master/mainApp/docs/COPYING.txt
[buildfromsource]:     https://github.com/trevorsandy/lpub3d-ci/blob/master/builds/utilities/README.md

[ldglite]:             https://github.com/trevorsandy/ldglite
[ldview]:              https://github.com/trevorsandy/ldview
[povray]:              https://github.com/trevorsandy/povray

[sfreleases]:          https://sourceforge.net/projects/lpub3d/files/2.1.0/
[githubreleases]:      https://github.com/trevorsandy/lpub3d/releases
[obsreleases]:         https://software.opensuse.org//download.html?project=home:trevorsandy&package=lpub3d-ci
[travis-badge]:        https://img.shields.io/travis/trevorsandy/lpub3d.svg?label=travis
[travis-url]:          https://travis-ci.org/trevorsandy/lpub3d-ci

[appveyor-badge]:      https://img.shields.io/appveyor/ci/trevorsandy/lpub3d-ci.svg?label=appveyor
[appveyor-url]:        https://ci.appveyor.com/project/trevorsandy/lpub3d-ci

[gh-rel-badge]:        https://img.shields.io/github/release/trevorsandy/lpub3d-ci.svg
[gh-rel-url]:          https://github.com/trevorsandy/lpub3d-ci/releases/latest

[gh-dl-badge]:         https://img.shields.io/github/downloads/trevorsandy/lpub3d-ci/total.svg
[gh-dl-url]:           https://github.com/trevorsandy/lpub3d-ci/releases/latest

[gh-issue-badge]:      https://img.shields.io/github/issues/trevorsandy/lpub3d-ci.svg
[gh-issue-url]:        https://github.com/trevorsandy/lpub3d-ci/issues

[gh-pull-req-badge]:   https://img.shields.io/github/issues-pr/trevorsandy/lpub3d-ci.svg
[gh-pull-req-url]:     https://github.com/trevorsandy/lpub3d-ci/pulls

[gh-lst-commit-badge]: https://img.shields.io/github/last-commit/trevorsandy/lpub3d-ci.svg
[gh-lst-commit-url]:   https://github.com/trevorsandy/lpub3d-ci/commits/master

[gh-top-lang-badge]:   https://img.shields.io/github/languages/top/trevorsandy/lpub3d-ci.svg
[gh-top-lang-url]:     https://github.com/trevorsandy/lpub3d-ci

[gh-comm-since-badge]: https://img.shields.io/github/commits-since/trevorsandy/lpub3d-ci/latest.svg
[gh-comm-since-url]:   https://github.com/trevorsandy/lpub3d-ci/commits/master

[sf-dw-badge]:         https://img.shields.io/sourceforge/dw/lpub3d.svg
[sf-dw-badge-url]:     https://sourceforge.net/projects/lpub3d

[sf-dt-badge]:         https://img.shields.io/sourceforge/dt/lpub3d.svg
[sf-dt-badge-url]:     https://sourceforge.net/projects/lpub3d

[maintained-badge]:    https://img.shields.io/maintenance/yes/2018.svg

[appimage-icon]:     https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/appimage.png
[arch-icon]:         https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/arch.png
[centos-icon]:       https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/centos.png
[debian-icon]:       https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/debian.png
[fedora-icon]:       https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/fedora.png
[macos-icon]:        https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/macos.png
[mageia-icon]:       https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/mageia.png
[opensuse-icon]:     https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/opensuse.png
[rhel-icon]:         https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/rhel.png
[sl-icon]:           https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/sl.png
[sle-icon]:          https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/sle.png
[ubuntu-icon]:       https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/ubuntu.png
[univention-icon]:   https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/univention.png
[windows-icon]:      https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/utilities/icons/windows.png
Copyright (c) 2015 - 2018 by Trevor SANDY
