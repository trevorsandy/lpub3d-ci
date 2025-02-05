#!/bin/bash
# Trevor SANDY
# Last Update January 17, 2025
#
# This script is called from .github/workflows/devops_ci_build.yml
#
# Run command:
# bash -ex builds/utilities/ci/github/linux-miltiarch-build.sh

if [[ "$LP3D_ARCH" == "" ]]; then
    echo "Usage: env LP3D_ARCH=... bash $0"
    exit 2
fi

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
    # Elapsed execution time
    set +x
    ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
    echo "----------------------------------------------------"
    ME="${ME} for (${docker_base}-${docker_dist}-${docker_arch})"
    [ "${LP3D_APPIMAGE}" = "true" ] && \
    ME="${ME} for (${docker_base}-${docker_dist}-${docker_arch}-appimage)" || :
    if [ "$BUILD_OPT" = "verify" ]; then
        echo "$ME Verification Finished!"
    else
        echo "$ME Finished!"
    fi
    echo "$ELAPSED"
    echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# resolve docker image options
docker_tag=
docker_image=
docker_dist="jammy"
docker_arch="${LP3D_ARCH:-amd64}"
docker_base="${LP3D_BASE:-ubuntu}"
docker_qemu="${LP3D_QEMU:-false}"
docker_platform="--platform linux/${LP3D_ARCH}"

# Format name and set WRITE_LOG - SOURCED if $1 is empty
ME="linux-multiarch-build"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced

# prepare output directory
[ -z "${LP3D_BUILDPKG_PATH}" ] && LP3D_BUILDPKG_PATH=$(cd ../ && echo $PWD/buildpkg)
[ ! -d "${LP3D_BUILDPKG_PATH}" ] && mkdir -p ${LP3D_BUILDPKG_PATH} || :
out_path="${LP3D_BUILDPKG_PATH}"

# automatic logging
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$out_path || :
if [ "${WRITE_LOG}" = "true" ]; then
    f="${0##*/}"; f="${f%.*}"; f="${f}-${docker_base}-${docker_dist}-${docker_arch}"
    [ "${LP3D_APPIMAGE}" = "true" ] && f="${f}-appimage"
    f="${out_path}/${f}"
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
fi

case "${docker_base}" in
    "ubuntu")
        if [ "${LP3D_APPIMAGE}" = "true" ]; then
            docker_dist="bionic"
        else
            docker_dist="jammy"
        fi 
        ;;
    "fedora")
        docker_dist="36" ;;
    "archlinux")
        docker_dist="latest" ;;
    *)
        echo "Unknown build base: ${docker_base}"
        exit 3
        ;;
esac
case "$LP3D_ARCH" in
    "amd64"|"x86_64")
        export LP3D_ARCH="amd64"
        aik_arch="x86_64"
        docker_tag="amd64/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        docker_platform="--platform linux/amd64"
        ;;
    "amd32"|"i386"|"i686")
        export LP3D_ARCH="i686"
        aik_arch="i686"
        docker_arch="i386"
        docker_tag="i386/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        ;;
    "arm64"|"aarch64")
        aik_arch="aarch64"
        docker_tag="arm64v8/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        docker_platform="--platform linux/arm64"
        ;;
    "arm32"|"armhf")
        aik_arch="armhf"
        docker_tag="arm32v7/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        docker_platform="--platform linux/arm/v7"
        ;;
    *)
        echo "Unknown architecture: $LP3D_ARCH"
        exit 3
        ;;
esac

# make sure we're in the repository root directory
cd ${GITHUB_WORKSPACE}

# prepare output directory
[ ! -d "${LP3D_BUILDPKG_PATH}" ] && \
mkdir -p ${LP3D_BUILDPKG_PATH} || :
out_path="${LP3D_BUILDPKG_PATH}"

# prepare third-party distribution directory
[ ! -d "${LP3D_3RD_PARTY_PATH}/${LP3D_BASE}_${LP3D_ARCH}" ] && \
mkdir -p ${LP3D_3RD_PARTY_PATH}/${LP3D_BASE}_${LP3D_ARCH} || :
base_path="${LP3D_3RD_PARTY_PATH}/${LP3D_BASE}_${LP3D_ARCH}"

# prepare ldraw directory
dist_path="${LP3D_3RD_PARTY_PATH}"
ldraw_path="${dist_path}/ldraw"
[ ! -f "${dist_path}/lpub3dldrawunf.zip" ] && \
wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/lpub3dldrawunf.zip -O ${dist_path}/lpub3dldrawunf.zip || :
[ ! -f "${dist_path}/complete.zip" ] && \
wget -q https://library.ldraw.org/library/updates/complete.zip -O ${dist_path}/complete.zip || :
[ ! -f "${dist_path}/tenteparts.zip" ] && \
wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -O ${dist_path}/tenteparts.zip || :
[ ! -f "${dist_path}/vexiqparts.zip" ] && \
wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip -O ${dist_path}/vexiqparts.zip || :
if [ ! -d "${ldraw_path}/parts" ]; then
    [ ! -d "${ldraw_path}" ] && mkdir -p ${ldraw_path} || :
    (cd ${dist_path} && unzip -od ./ -q complete.zip)
    if [ -d "${ldraw_path}/parts" ]; then
        echo "LDraw library extracted to ${ldraw_path}"
    else
        echo "ERROR - LDraw library was not extracted."
    fi
fi

# Link LDraw libraries to base path
[ ! -f "${base_path}/lpub3dldrawunf.zip" ] && \
(cd ${base_path} && ln -sf ${dist_path}/lpub3dldrawunf.zip lpub3dldrawunf.zip) || \
echo "${base_path}/lpub3dldrawunf.zip exists. Nothing to do."
[ ! -f "${base_path}/complete.zip" ] && \
(cd ${base_path} && ln -sf ${dist_path}/complete.zip complete.zip) || \
echo "${base_path}/complete.zip exists. Nothing to do."
[ ! -f "${base_path}/tenteparts.zip" ] && \
(cd ${base_path} && ln -sf ${dist_path}/tenteparts.zip tenteparts.zip) || \
echo "${base_path}/tenteparts.zip exists. Nothing to do."
[ ! -f "${base_path}/vexiqparts.zip" ] && \
(cd ${base_path} && ln -sf ${dist_path}/vexiqparts.zip vexiqparts.zip) || \
echo "${base_path}/vexiqparts.zip exists. Nothing to do."

# Trigger rebuild renderers if specified
ldglite_path=${base_path}/ldglite-1.3
ldview_path=${base_path}/ldview-4.5
povray_path=${base_path}/lpub3d_trace_cui-3.8
[[ "${LP3D_COMMIT_MSG}" == *"ALL_RENDERERS"* ]] && \
echo "'Build LDGLite, LDView and POV-Ray' detected." && \
LP3D_COMMIT_MSG="${LP3D_COMMIT_MSG} BUILD_LDGLITE BUILD_LDVIEW BUILD_POVRAY" || :
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_LDGLITE"* ]] && \
echo "'Build LDGLite' detected." && [ -d "${ldglite_path}" ] && \
rm -rf "${ldglite_path}" && echo "Cached ${ldglite_path} deleted" || :
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_LDVIEW"* ]] && \
echo "'Build LDView' detected." && [ -d "${ldview_path}" ] && \
rm -rf "${ldview_path}" && echo "Cached ${ldview_path} deleted" || :
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_POVRAY"* ]] && \
echo "'Build POV-Ray' detected." && [ -d "${povray_path}" ] && \
rm -rf "${povray_path}" && echo "Cached ${povray_path} deleted" || :

# run builds with privileged user account required to load library dependency
gid="$(id -g)"
uid="$(id -u)"
name="$(id -un)"

# generate Dockerfile and build image
cat << pbEOF >${out_path}/Dockerfile
FROM ${docker_tag}
pbEOF
case "${docker_base}" in
    "ubuntu")
        cp -f builds/linux/obs/alldeps/debian/control .
cat << pbEOF >>${out_path}/Dockerfile
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y apt-utils git wget cmake rsync unzip lintian build-essential debhelper fakeroot ccache lsb-release
RUN apt-get install -y libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev
RUN apt-get install -y autotools-dev autoconf pkg-config automake libtool curl zip fuse libfuse-dev
RUN apt-get install -y xvfb desktop-file-utils
RUN apt-get install -y $(grep Build-Depends control | cut -d: -f2| sed 's/(.*)//g' | tr -d ,)
pbEOF
        ;;
    "fedora")
        cp -f builds/linux/obs/alldeps/lpub3d-ci.spec .
        sed -e 's/Icon: lpub3d.xpm/# Icon: lpub3d.xpm remarked - fedora does not like/' \
            -e 's/<B_CNT>/1/' -i lpub3d-ci.spec
cat << pbEOF >>${out_path}/Dockerfile
ADD lpub3d-ci.spec /
RUN dnf install -y git wget unzip rsync which rpmlint ccache dnf-plugins-core rpm-build fuse fuse-devel
RUN dnf install -y glib2-devel libfdt-devel pixman-devel zlib-devel bzip2 ninja-build python3
RUN dnf install -y xorg-x11-server-Xvfb desktop-file-utils
RUN dnf builddep -y /lpub3d-ci.spec
pbEOF
        ;;
    "archlinux")
        cp -f builds/linux/obs/alldeps/PKGBUILD .
cat << pbEOF >>${out_path}/Dockerfile
RUN pacman -Suy --noconfirm
RUN pacman -Sy --noconfirm git wget unzip rsync ccache base-devel binutils fakeroot tinyxml awk file inetutils
RUN pacman -S --noconfirm --needed xorg-server-xvfb desktop-file-utils
RUN pacman -S --noconfirm --needed $(grep depends PKGBUILD | cut -f2 -d=|tr -d \'\(\) | tr '\n' ' ')
pbEOF
        ;;
    *)
        echo "Unknown distribution base: ${docker_base}"
        exit 3
        ;;
esac

cat << pbEOF >>${out_path}/Dockerfile
RUN groupadd -r ${name} -g ${gid} \\
    && useradd -u ${uid} -r -g ${name} -m -d /${name} -s /sbin/nologin -c "Build pkg user" ${name} \\
    && chmod 755 /${name}
WORKDIR /${name}
USER ${name}
VOLUME ["/in", "/out", "/dist", "/${name}/ldraw"]
pbEOF

# If QEMU and not AppImage, set package build script and pre-package check
if [[ "${LP3D_APPIMAGE}" = "false" && "${LP3D_QEMU}" = "true" ]]; then
    case "${docker_base}" in
        "ubuntu")
            cp -f builds/linux/CreateDeb.sh .
cat << pbEOF >>${out_path}/Dockerfile
RUN mkdir -p /${name}/debbuild/SOURCES
ADD CreateDeb.sh /${name}
pbEOF
            ;;
        "fedora")
            cp -f builds/linux/obs/lpub3d-ci.rpmlintrc .
            cp -f builds/linux/CreateRpm.sh .
cat << pbEOF >>${out_path}/Dockerfile
RUN mkdir -p /${name}/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
ADD CreateRpm.sh /${name}
ADD lpub3d-ci.spec /${name}
ADD lpub3d-ci.rpmlintrc /${name}/.config/rpmlint/
pbEOF
            ;;
       "archlinux")
            cp -f builds/linux/CreatePkg.sh .
cat << pbEOF >>${out_path}/Dockerfile
RUN mkdir -p /${name}/pkgbuild/{src,upstream}
ADD CreatePkg.sh /${name}
pbEOF
            ;;
    esac
    LP3D_PRE_PACKAGE_CHECK=1
fi

cp -f builds/linux/CreateLinuxMulitArch.sh .
cat << pbEOF >>${out_path}/Dockerfile
ADD --chown=${name}:${name} docker-run-CMD.sh /${name}
ADD --chown=${name}:${name} CreateLinuxMulitArch.sh /${name}
CMD /bin/bash
pbEOF

cat << pbEOF >${out_path}/docker-run-CMD.sh
#!/bin/bash
cd ~/ \\
&& chmod a+x CreateLinuxMulitArch.sh \\
&& ./CreateLinuxMulitArch.sh \\
&& if test -d /out; then \\
  cd ~/; \\
  ls -al ./ 2>/dev/null || :; \\
  ls -al /out/ 2>/dev/null || :; \\
  ls -al /dist/ 2>/dev/null || :; \\
fi; \\
echo "----------------------------------------------------"; \\
echo "docker-run-CMD Finished!"; \\
echo "----------------------------------------------------"
pbEOF

# add Dockerfile to context
cp -f ${out_path}/Dockerfile .

# add run CMD script to context and set executable
cp -f ${out_path}/docker-run-CMD.sh . && chmod a+x docker-run-CMD.sh

# Docker Hub login
echo "Login to Docker Hub..."
echo ${DOCKER_HUB_TOKEN} | docker login --username ${DOCKER_USERNAME} --password-stdin

# enable QEMU
if [ "${LP3D_QEMU}" = "true" ]; then
    echo "Enable QEMU multiarch environment..."
    if [ "${CI}" = "true" ]; then
        echo "Install gcc-${aik_arch}-linux-gnu and zlib1g-dev and export QEMU_LD_PREFIX..."
        sudo apt-get install -y gcc-${aik_arch}-linux-gnu zlib1g-dev
        export QEMU_LD_PREFIX=/usr/${aik_arch}-linux-gnu
    fi
    docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
    if [ "${LP3D_APPIMAGE}" = "true" ]; then
        LP3D_AI_BUILD_TOOLS=1
        # extract appimage payload is OFF
        if [ -z "${LP3D_AI_EXTRACT_PAYLOAD}" ]; then
            [[ "${LP3D_COMMIT_MSG}" == *"EXTRACT_PAYLOAD"* ]] && LP3D_AI_EXTRACT_PAYLOAD=1 || :
        fi
    fi
    # replace magic bytes default is ON
    if [ -z "${LP3D_AI_MAGIC_BYTES}" ]; then
        [[ "${LP3D_COMMIT_MSG}" == *"AI_MAGIC_BYTES"* ]] && LP3D_AI_MAGIC_BYTES=1 || :
    fi
fi

# download appimagetool and appimage runtime
if [ -n "${LP3D_AI_BUILD_TOOLS}" ]; then
    p=appimagetool
    k=AppImageToolKit
    rt=runtime-${aik_arch}
    ait=${p}-${aik_arch}.AppImage
    tools_path=${base_path}/AppDir/tools
    sd=${GITHUB_WORKSPACE}/builds/utilities
    if [ ! -f "${tools_path}/${p}" ]; then
        cd ../
        mkdir -p ${tools_path}
        echo && echo Downloading $k for ${aik_arch} at $PWD...
        [ ! -d $k ] && mkdir -p $k/ || :
        ( cd $k && export ARCH=${aik_arch} && \
            [ ! -f "${tools_path}/${p}" ] && \
            wget https://github.com/AppImage/appimagetool/releases/download/continuous/${ait} -P out/ && \
            if [ -f "out/${ait}" ]; then \
                mv -f out/${ait} ${tools_path}/${p} && \
                wget https://github.com/AppImage/type2-runtime/releases/download/continuous/${rt} && \
                [ -f "${rt}" ] && mv -f ${rt} ${tools_path}/runtime || \
                (echo "Download ${rt} failed." && exit 1); \
            else echo "Download ${ait} failed." && exit 1; \
            fi || echo "Skipped ${p} download" \
        ) >$k.foo 2>&1 && mv $k.foo $k.ok
        if [ -f $k.ok ]; then
            mb="41 49 02 00"
            echo "Download and rename $k succeeded"
            echo -n "Check runtime magic bytes... " && \
            hd=$(hexdump -Cv ${tools_path}/runtime | head -n 1 | (grep -oE "41 49 02 00")) && \
            [ "$mb" = "$hd" ] && echo "$mb Ok" || echo "magic bytes $mb not found"
            cat <<EOF >${tools_path}/AppRun
#! /bin/sh

set -e

this_dir="\$(readlink -f "\$(dirname "\$0")")"

# make appimagetool prefer the bundled mksquashfs
export PATH="\$this_dir"/usr/bin:"\$PATH"

exec "\$this_dir"/usr/bin/appimagetool "\$@"
EOF
            rm -rf $k
            mv -f $k.ok ${out_path}
        else
            echo Download and rename $k FAILED
            tail -80 $k.foo
            exit 9
        fi
    fi

    [ -f "${out_path}/$k.ok" ] && mv ${out_path}/$k.ok ${out_path}/$k.ok.log || :

    cd ${GITHUB_WORKSPACE}
fi

# pre-package build check default is OFF
if [ -z "${LP3D_PRE_PACKAGE_CHECK}" ]; then
    [[ "${LP3D_COMMIT_MSG}" == *"PRE_PACKAGE_CHECK"* ]] && LP3D_PRE_PACKAGE_CHECK=1 || :
fi

# reporitory
IFS='/' read -ra LP3D_SLUGS <<< "${GITHUB_REPOSITORY}"; unset IFS;
LPUB3D=${LP3D_SLUGS[1]}

# build docker image
docker_build_opts=(${docker_platform})
if [ "${DOCKER_CACHE}" = "false" ]; then
    docker_build_opts+=(
        --pull
        --no-cache
    )
fi
docker build --rm \
    "${docker_build_opts[@]}" \
    -f Dockerfile \
    -t ${docker_image} .

# marshall docker run options
common_docker_opts=(
    -e CI="${CI}"
    -e GITHUB="${GITHUB}"
    -e DOCKER="true"
    -e TERM="${TERM}"
    -e LPUB3D="${LPUB3D}"
    -e GITHUB_SHA="${GITHUB_SHA}"
    -e WRITE_LOG="${WRITE_LOG}"
    -e BUILD_OPT="${BUILD_OPT}"
    -e BUILD_CPUs="${BUILD_CPUs}"
    -e LP3D_QEMU="${LP3D_QEMU}"
    -e LP3D_BASE="${LP3D_BASE}"
    -e LP3D_ARCH="${LP3D_ARCH}"
    -e LP3D_LOG_PATH="/out"
    -e LP3D_APPIMAGE="${LP3D_APPIMAGE}"
    -e LP3D_NO_CLEANUP="${LP3D_NO_CLEANUP}"
    -e LP3D_AI_MAGIC_BYTES="${LP3D_AI_MAGIC_BYTES}"
    -e LP3D_AI_BUILD_TOOLS="${LP3D_AI_BUILD_TOOLS}"
    -e LP3D_PRE_PACKAGE_CHECK="${LP3D_PRE_PACKAGE_CHECK}"
    -e LP3D_AI_EXTRACT_PAYLOAD="${LP3D_AI_EXTRACT_PAYLOAD}"
    -v "${PWD}":/in
    -v "${out_path}":/out
    -v "${dist_path}":/dist
    -v "${ldraw_path}":/${name}/ldraw
)

# make ctrl-c work
if [[ "${CI}" != "true" ]] && [[ "${TERM}" != "" ]]; then
    common_docker_opts+=("-t")
fi

# enforce using the same UID in the container as outside, so that the created files are owned by the caller
docker run \
    ${docker_platform} \
    --rm \
    --user "${uid}" \
    --privileged \
    --security-opt seccomp=unconfined \
    "${common_docker_opts[@]}" \
    "${docker_image}" \
    /bin/bash -ex "./docker-run-CMD.sh"

# list the built files
if [ -d "${out_path}" ]; then
  # cleanup large files if only verification
  if [ "$BUILD_OPT" = "verify" ]; then
    pushd "${out_path}"
    rm -f ./*.AppImage* 2>/dev/null || :
    rm -f ./*.pkg.tar.zst 2>/dev/null || :
    rm -f ./*.rpm* 2>/dev/null || :
    rm -f ./*.deb* 2>/dev/null || :
    rm -f ./*.xz 2>/dev/null || :
    rm -f ./*.zip 2>/dev/null || :
    popd
  fi
  ls -al ${out_path}
fi

exit 0
