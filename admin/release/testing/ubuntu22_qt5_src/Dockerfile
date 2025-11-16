# dockerfile for us3 compiles on Ubuntu 24.04

### exposed ARGs override defaults with --build-arg <varname>=<value>

ARG image=docker.io/library/ubuntu:24.04
FROM $image

## compile defaults
ARG parallel_compile=64
ARG ultrascan_branch=main

## qt defaults
ARG qt_major_version=5.15
ARG qt_version=5.15.18
ARG qwt_version=6.3.0

# needed vor "opensource-" variants of qt source
ARG qt_opensource=opensource-

# env vars
ENV ULTRASCAN=/ultrascan3
ENV QTDIR=/qt-$qt_version
ENV QWTDIR=/qwt-$qwt_version

# basic install stuff
RUN set -eux; \
  apt-get update; \
  DEBIAN_FRONTEND=noninteractive apt-get dist-upgrade -y; \
  DEBIAN_FRONTEND=noninteractive apt-get install -y \
    tzdata \
    git \
    ca-certificates \
    openssl libssl-dev \
    bc \
    curl \
    wget \
    xz-utils \
    bzip2 \
    build-essential \
    pkg-config \
    mpich libmpich-dev \
    re2c \
    vim \
    emacs-nox \
    python3 python-is-python3 \
    zlib1g-dev \
    libglib2.0-dev \
    libpq-dev \
    libatspi2.0-dev \
    libjpeg-dev \
    libfontconfig1-dev \
    libarchive-dev \
    mesa-common-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libxext-dev \
    libxrender-dev \
    libxi-dev \
    xorg-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libxcb1-dev \
    libxcb-glx0-dev \
    libxcb-keysyms1-dev \
    libxcb-image0-dev \
    libxcb-shm0-dev \
    libxcb-icccm4-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-shape0-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-xinerama0-dev \
    libxcb-util-dev \
    libxcb-cursor-dev \
    default-libmysqlclient-dev \
    rsync; \
  rm -rf /var/lib/apt/lists/*

## required for qt to find xcb-xlib deps
RUN ls -l /usr/lib/x86_64-linux-gnu/libXext* || true
RUN mkdir -p /usr/X11R6 && ln -s /usr/lib/x86_64-linux-gnu /usr/X11R6/lib64 || true

# build qt
RUN set -eux; \
  wget https://download.qt.io/archive/qt/$qt_major_version/$qt_version/single/qt-everywhere-${qt_opensource}src-$qt_version.tar.xz; \
  XZ_DEFAULTS="-T 0" tar Jxf qt-everywhere-${qt_opensource}src-$qt_version.tar.xz; \
  rm qt-everywhere-${qt_opensource}src-$qt_version.tar.xz; \
  cd qt-everywhere-src-$qt_version; \
  env MAKEFLAGS=-j$parallel_compile ./configure \
    -prefix /qt-$qt_version \
    -release \
    -opensource -confirm-license \
    -platform linux-g++-64 \
    -nomake tests -nomake examples \
    -xcb \
    -D QT_SHAREDMEMORY -D QT_SYSTEMSEMAPHORE \
    -no-icu; \
  make -j$parallel_compile 2>&1 > make.out; \
  make install -j1 2>&1 > makeinstall.out; \
  cd /; \
  rm -rf /qt-everywhere-src-$qt_version

# install qwt
RUN set -eux; \
  wget https://versaweb.dl.sourceforge.net/project/qwt/qwt/$qwt_version/qwt-$qwt_version.tar.bz2; \
  tar jxf qwt-$qwt_version.tar.bz2; \
  cd qwt-$qwt_version; \
  /qt-$qt_version/bin/qmake; \
  make -j$parallel_compile

# get source
## invalidate docker cache
ADD https://api.github.com/repos/ehb54/ultrascan3/git/refs/heads/$ultrascan_branch version.json
RUN git clone --branch "${ultrascan_branch}" https://github.com/ehb54/ultrascan3.git
RUN cd $ULTRASCAN && git fetch --all
RUN cd $ULTRASCAN && git checkout -B $ultrascan_branch
RUN cd $ULTRASCAN && git reset --hard origin/$ultrascan_branch

# local.pri files
ADD local.pri.somo /ultrascan3/us_somo/develop/local.pri
ADD local.pri.gui  /ultrascan3/local.pri
ADD local.pri.mpi  /ultrascan3/local.pri.mpi

# us version setup
RUN cd /ultrascan3/programs/us && env ULTRASCAN=/ultrascan3 ./revision.sh

# somo version setup
RUN cd /ultrascan3/us_somo/develop && env ULTRASCAN=/ultrascan3 ./revision.sh && env us3=/ultrascan3 ./version.sh

# convenience repo—run
RUN set -eux; \
  git clone --depth 1 https://github.com/ehb54/convenience /tmp/convenience; \
  (cd /tmp/convenience && ./installcont.sh); \
  rm -rf /tmp/convenience

RUN git config --global core.pager ''
#RUN git config --global user.email brookes@uthscsa.edu
#RUN git config --global user.name ehb54

RUN cat <<EOF > /root/qt5env
export PATH=/qt-$qt_version/bin:/root/.local/bin:/root/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
export LD_LIBRARY_PATH=/ultrascan3/lib:/qwt-$qwt_version/lib:/qt-$qt_version/lib
export MAKEFLAGS='-j80'
EOF

# symlinks + env (one small layer)
RUN set -eux; \
  ln -sf ${QTDIR}/bin/qmake        /usr/bin/qmake5       || true; \
  ln -sf ${QTDIR}/bin/qtpaths      /usr/bin/qtpaths5     || true; \
  ln -sf ${QTDIR}/bin/moc          /usr/bin/moc5         || true; \
  ln -sf ${QTDIR}/bin/uic          /usr/bin/uic5         || true; \
  ln -sf ${QTDIR}/bin/rcc          /usr/bin/rcc5         || true; \
  ln -sf ${QTDIR}/bin/lrelease     /usr/bin/lrelease5    || true; \
  ln -sf ${QTDIR}/bin/lupdate      /usr/bin/lupdate5     || true; \
  ln -sf ${QTDIR}/bin/qml          /usr/bin/qml5         || true; \
  ln -sf ${QTDIR}/bin/qmlcachegen  /usr/bin/qmlcachegen5 || true; \
  ln -sf ${QTDIR}/bin/qmlimportscanner /usr/bin/qmlimportscanner5 || true; \
  printf '%s\n' \
    "# Qt5 environment (Docker)" \
    "export QTDIR=${QTDIR}" \
    'export PATH=${QTDIR}/bin:$PATH' \
    'export LD_LIBRARY_PATH=${QTDIR}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}' \
    'export PKG_CONFIG_PATH=${QTDIR}/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}' \
    'export QT_PLUGIN_PATH=${QTDIR}/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}' \
    'export QML2_IMPORT_PATH=${QTDIR}/qml${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}' \
    "export ULTRASCAN=${ULTRASCAN}" \
    "export QWTDIR=${QWTDIR}" \
    > /etc/profile.d/qt5.sh; \
  chmod 0755 /etc/profile.d/qt5.sh

## add node.js 22 (via NodeSource)
ARG nodejs_stream=22
RUN set -eux; \
  apt-get update; \
  curl -fsSL https://deb.nodesource.com/setup_${nodejs_stream}.x | bash -; \
  DEBIAN_FRONTEND=noninteractive apt-get install -y nodejs; \
  rm -rf /var/lib/apt/lists/*; \
  node -v && npm -v

WORKDIR /ultrascan3

ENV PATH="${QTDIR}/bin:${QTDIR}/libexec:${PATH}"

SHELL ["/bin/bash", "-lc"]
CMD ["/bin/bash", "-l"]
