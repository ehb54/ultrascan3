# dockerfile for us3 compiles

### exposed ARGs override defaults with --build-arg <varname>=<value>

ARG image=docker.io/library/rockylinux:8.10
## ARGs are scoped FROM
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

# needed for python apt variants, e.g. ubuntu22 needs python3
ARG apt_python_version=python

# env vars (could be moved up)
ENV ULTRASCAN=/ultrascan3
ENV QTDIR=/qt-$qt_version
ENV QWTDIR=/qwt-$qwt_version

# basic install stuff

RUN dnf update -y
RUN dnf install -y yum-utils dnf-plugins-core && dnf config-manager --set-enabled powertools && dnf groupinstall -y "Development Tools" && dnf install -y tzdata git openssl-devel bc curl wget zlib-devel pkg-config mpich mpich-devel re2c vim emacs-nox xorg-x11* $apt_python_version glib2-devel postgresql-devel at-spi2-core-devel libjpeg-devel fontconfig-devel libarchive-devel && alternatives --set python /usr/bin/python3 && yum-builddep -y qt5-devel && dnf install -y libxcb* xcb-util* libX11* libxkbcommon* mesa-lib* mariadb-devel rsync && dnf -y clean all && rm -rf /var/cache/dnf

## required for qt to find xcb-xlib deps
# RUN ln -s /lib64/libXext.so.[0-9] /lib64/libXext.so
RUN ls -l /lib64/libXext*
RUN mkdir /usr/X11R6 && ln -s /lib64 /usr/X11R6/lib64

# build qt
RUN wget https://download.qt.io/archive/qt/$qt_major_version/$qt_version/single/qt-everywhere-${qt_opensource}src-$qt_version.tar.xz && \
    XZ_DEFAULTS="-T 0" tar Jxf qt-everywhere-${qt_opensource}src-$qt_version.tar.xz && rm qt-everywhere-${qt_opensource}src-$qt_version.tar.xz && \
    cd qt-everywhere-src-$qt_version && env MAKEFLAGS=-j$parallel_compile ./configure -prefix /qt-$qt_version -release -opensource -confirm-license -platform linux-g++-64 -nomake tests -nomake examples -xcb -xcb-xlib -D QT_SHAREDMEMORY -D QT_SYSTEMSEMAPHORE -no-icu && \
    make -j$parallel_compile 2>&1 > make.out && \
     make install -j1 2>&1 > makeinstall.out && \
     rm -fr /qt-everywhere-src-$qt_version

# RUN ls -l /qt-$qt_version/bin /qt-$qt_version/lib

# install qwt
RUN wget https://versaweb.dl.sourceforge.net/project/qwt/qwt/$qwt_version/qwt-$qwt_version.tar.bz2 && tar jxf qwt-$qwt_version.tar.bz2 && cd qwt-$qwt_version && /qt-$qt_version/bin/qmake && make -j$parallel_compile

# get source
## invalidate docker cache
## BRANCH could be pulled to top
ADD https://api.github.com/repos/ehb54/ultrascan3/git/refs/heads/$ultrascan_branch version.json
RUN git clone --branch "${ultrascan_branch}" https://github.com/ehb54/ultrascan3.git
RUN cd $ULTRASCAN && git fetch --all
RUN cd $ULTRASCAN && git checkout -B $ultrascan_branch
RUN cd $ULTRASCAN && git reset --hard origin/$ultrascan_branch

# setup initial local.pri's

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
  ln -sf ${QTDIR}/bin/qmake        /usr/bin/qmake5 || true; \
  ln -sf ${QTDIR}/bin/qtpaths      /usr/bin/qtpaths5 || true; \
  ln -sf ${QTDIR}/bin/moc          /usr/bin/moc5 || true; \
  ln -sf ${QTDIR}/bin/uic          /usr/bin/uic5 || true; \
  ln -sf ${QTDIR}/bin/rcc          /usr/bin/rcc5 || true; \
  ln -sf ${QTDIR}/bin/lrelease     /usr/bin/lrelease5 || true; \
  ln -sf ${QTDIR}/bin/lupdate      /usr/bin/lupdate5 || true; \
  ln -sf ${QTDIR}/bin/qml          /usr/bin/qml5 || true; \
  ln -sf ${QTDIR}/bin/qmlcachegen  /usr/bin/qmlcachegen5 || true; \
  ln -sf ${QTDIR}/bin/qmlimportscanner /usr/bin/qmlimportscanner5 || true; \
  printf '%s\n' \
    "# Qt6 environment (Docker)" \
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

## add node.js 22

ARG nodejs_stream=22
RUN set -eux; \
  dnf -y module reset nodejs || true; \
  dnf -y module enable nodejs:${nodejs_stream}; \
  dnf -y install nodejs npm; \
  dnf -y clean all; rm -rf /var/cache/dnf; \
  node -v && npm -v

WORKDIR /ultrascan3

ENV PATH="${QTDIR}/bin:${QTDIR}/libexec:${PATH}" 

# Set MPI-specific environment variables
ENV MPI_BIN "/usr/lib64/mpich/bin"
ENV MPI_SYSCONFIG "/etc/mpich-x86_64"
ENV MPI_INCLUDE "/usr/include/mpich-x86_64"
ENV MPI_LIB "/usr/lib64/mpich/lib"
ENV MPI_MAN "/usr/share/man/mpich-x86_64"
ENV MPI_COMPILER "mpich-x86_64"
ENV MPI_HOME "/usr/lib64/mpich"
ENV PATH "$MPI_BIN:$PATH"
ENV LD_LIBRARY_PATH "$MPI_LIB:$LD_LIBRARY_PATH"
ENV PKG_CONFIG_PATH "$MPI_LIB/pkgconfig:$PKG_CONFIG_PATH"

SHELL ["/bin/bash", "-lc"]
CMD ["/bin/bash", "-l"]
