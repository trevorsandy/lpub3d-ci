### ___________ BUILDING LPUB3D PACKAGE FROM SOURCE ___________

If you prefer to create your own LPub3D install package, these notes might be helpful.

Starting with version 2.1.0, LPub3D build scripts builds the main applicaiton (LPub3D) and
the three integrated image renderers - LDGlite, LDView and LPub3D-Trace (POV-Ray).
Furthermore, the LPub3D build process is now fully cloud-enabled and optimized for
containerized builds. This means the build process is completely automated and reqires
minimal intervention. However, as the build process is specifically designed to exploit
cloud-based CI services and tools like Docker, Travis-CI and AppVeyor, building LPub3D locally
may require some presets and dependencies likely not availabe on your build environment. The details
below hopefully address the necessary steps to complete the build process but, it also goes
without saying, that your local environment may present uinque configuration challenges not
addressed in the steps below.

*Additional dependencies include:*
* git
* tar
* wget
* curl

#### ___________ MAC: BUILD OSX ‘DMG’ PACKAGE ___________

**Platform-specific Dependencies:**
*  Download and install [Qt][qtwebsite] ([CreateDmg.sh][createdmg] assumes Qt 5.10.0 installed at ~/Qt/IDE/5.10.0/clang_64)
*  [Xcode][xcode]:
*  [Homebrew][homebrew]:         `/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
*  [XQuartz (X11)][x11]:         `brew cask list && brew cask install xquartz`

**Steps:**
- [ ] 1. create and enter working directory
`$ cd ~/`
- [ ] 2. export Qt location and bin
`export PATH=~/Qt/IDE/5.10.0/clang_64:~/Qt/IDE/5.10.0/clang_64/bin:$PATH`
- [ ] 2. get [CreateDmg.sh][createdmg] build and packaging script
`$ curl "https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/macx/CreateDmg.sh" -o "CreateDmg.sh"`
- [ ] 3. enable execute permissions and execute package script
`$ chmod 755 CreateDmg.sh && ./CreateDmg.sh`
- [ ] 4. mount and install package ./dmgbuild/DMGS/LPub3D_2.1.0.0.700_20180102_osx.dmg
- [ ] 5. have a :beer:

#### ___________ LINUX: BUILD ARCH 'PKG.TAR.XZ' PACKAGE ___________

**Steps:**
- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. get [CreatePkg.sh][createpkg] packaging script
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreatePkg.sh`
- [ ] 3. enable execute permissions and execute package script
`$ chmod 755 CreatePkg.sh && ./CreatePkg.sh`
- [ ] 4. install package availavble in ./pkgbuild
`$ sudo pacman -U pkgbuild/lpub3d-2.1.0.0.700_20180102_1_x86_64.pkg.tar.xz`
- [ ] 5. have a :beer:

#### ___________ LINUX: BUILD UBUNTU/DEBIAN 'DEB' PACKAGE ___________

**Steps:**
- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. get [CreateDeb.sh][createdeb] packaging script
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateDeb.sh`
- [ ] 3. enable execute permissions and execute package script
`$ chmod 755 CreateDeb.sh && ./CreateDeb.sh`
- [ ] 4. install package availavble in ./debbuild
`$ sudo dpkg --install debbuild/lpub3d_2.1.0.0.700_20180102_0ubuntu1_amd64.deb`
- [ ] 5. have a :beer:

#### ___________ LINUX: BUILD REDHAT/FEDORA/SUSE 'RPM' PACKAGE ___________

**Steps:**
- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. get [CreateRpm.sh][createrpm] packaging script
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateRpm.sh`
- [ ] 3. enable execute permissions and execute package script
`$ chmod 755 CreateRpm.sh && ./CreateRpm.sh`
- [ ] 4. install either availavble in ./rpmbuild/RPMS/x86_64
`$ sudo rpm -U rpmbuild/RPMS/x86_64/lpub3d-2.1.0.0.700_20180102_1fedora.x86_64.rpm`
- [ ] 5. have a :beer:

#### ___________ MICROSOFT WINDOWS PORTABLE (UNPACKED) PACKAGE ___________

**Platform-specific Dependencies:**
*  Note the Windows build script expects [MSYS2][msys2]/[Mingw32][mingw64] and [Mingw64][mingw64] [Qt][qtwebsite] at the following locations:
- C:\Msys2\Msys64\mingw32\bin
- C:\Msys2\Msys64\mingw64\bin
- However, you may choose to download and install [Qt][qtwebsite], in which case you will have to change the [AutoBuild.cmd][autobuild] script paths above to the Qt install path.
*  Download and install [Qt][qtwebsite] ([AutoBuild.cmd][autobuild] assumes Qt 5.9.2 installed at ~/Qt/IDE/5.10.0/clang_64)

**Steps:**
- [ ] 1. enter directory
`cd %USERPROFILE%`
- [ ] 2. get lpub3d source
`> git clone https:\\github.com\trevorsandy\lpub3d-ci.git`
- [ ] 3. enter source directory and execute package script
`> cd lpub3d-ci & call builds\windows\AutoBuild.cmd -x86 -3rd -ins -chk`
- [ ] 4. copy builds\windows\release\LPub3D-Any-2.1.0.0.700_20180102\LPub3D_x86\* to final destination
- [ ] 5. have a :beer:

#### ___________ ALL LINUX DISTROS - WINDOWS DOCKER BUILD ___________
**Platform-specific Dependencies:**
* [Docker][docker]                  Note: Be sure to set C as a Shared Drive available to your containers
* [Docker Compose][dockercompose]   Note: Compose instructions below are quite likely the same for Docker on Linux or macOS
* [WSL - ​Windows Subsystem for Linux][wsl]

**Steps:**
- [ ] 1. make and enter build direcories
`PS...> md dockerbuilds; cd dockerbuilds; md dockerfiles; md lpub3d_linux_3rdparty`
- [ ] 2. download docker config files
`PS...> bash curl -sL https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/linux/docker-compose/docker-compose-cibuild-linux.yml -o docker-compose-cibuild-linux.yml`
`PS...> bash curl -sL https://raw.githubusercontent.com/trevorsandy/lpub3d-ci/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-fedora_25 -o dockerfiles/Dockerfile-cibuild-fedora_25`
- [ ] 3. execute Docker-compose script
`PS...> docker-compose -f docker-compose-cibuild-linux.yml run fedora_25`
- [ ] 4. have a :beer:

[xcode]:           https://developer.apple.com/xcode/downloads/
[x11]:             https://www.xquartz.org/
[homebrew]:        http://brew.sh
[appdmg]: 		     https://github.com/LinusU/node-appdmg
[qtwebsite]:       https://www.qt.io/download/
[createdmg]:       https://github.com/trevorsandy/lpub3d/blob/master/builds/macx/CreateDmg.sh
[createrpm]:	     https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreateRpm.sh
[createpkg]:	     https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreatePkg.sh
[createdeb]:	     https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreateDeb.sh
[autobuild]:       https://github.com/trevorsandy/lpub3d/blob/master/builds/windows/AutoBuild.cmd
[msys2]:           http://www.msys2.org/
[mingw64]:         https://github.com/Alexpux/mingw-w64
[docker]:          https://www.docker.com/get-docker
[dockercompose]:   https://docs.docker.com/compose/install/
[wsl]:             https://docs.microsoft.com/en-us/windows/wsl/install-win10

