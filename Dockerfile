FROM ubuntu:xenial
LABEL maintainer="Trevor SANDY <trevor.sandy@gmail.com>"
RUN apt-get update ; apt-get install -y git wget lintian build-essential debhelper ccache lsb-release
RUN apt-get install -y fakeroot debhelper qtbase5-dev qt5-qmake libqt5opengl5-dev libpng-dev libjpeg-dev zlib1g-dev
COPY CreateDeb.sh /
VOLUME /buildpkg
CMD ./CreateDeb.sh; if test -d /vol/mediabuild; then cp -f /debbuild/*.deb /buildpkg/; fi
