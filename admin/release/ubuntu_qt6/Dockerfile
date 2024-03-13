# dockerfile for us3 compiles
### Initial test for QT6 compile

### exposed ARGs override defaults with --build-arg <varname>=<value>

ARG image=ubuntu:23.10
## ARGs are scoped FROM
FROM $image

## compile defaults
ARG parallel_compile=80
ARG ultrascan_branch=master

## qt defaults
ARG qt_major_version=6.6
ARG qt_version=6.6.2
### N.B. qwt_version is hard coded in the local.pri.somo so we need to change this & in the codeql container & workflow/codeql-analysis.yml

# needed vor "opensource-" variants of qt source
ARG qt_opensource=

# needed for python apt variants, e.g. ubuntu22 needs python3
ARG apt_python_version=python3
    
# env vars (could be moved up)
ENV ULTRASCAN=/ultrascan3
ENV QTDIR=/qt-$qt_version
ENV QWTDIR=/qwt-$qwt_version

ARG DEBIAN_FRONTEND=noninteractive

# basic install stuff

RUN apt-get update
RUN apt-get install -y --no-install-recommends apt-utils 
RUN apt-get install -y build-essential
RUN apt-get install -y tzdata
RUN apt-get install -y software-properties-common
RUN apt-get install -y bind9-host
RUN apt-get install -y subversion git libssl-dev bc curl wget
RUN apt-get install -y zlib1g-dev
RUN apt-get install -y pkg-config
RUN apt-get install -y re2c
RUN apt-get install -y openssh-server
RUN apt-get install -y vim
RUN apt-get install -y emacs-nox
RUN apt-get install -y xorg
RUN apt-get install -y x11-apps
RUN apt-get install -y telnet
RUN apt-get install -y $apt_python_version
RUN apt-get install -y rsync

# bits to ease development/testing

RUN yes 'y' | ssh-keygen -t rsa -N '' -f ~/.ssh/id_rsa
RUN cat ~/.ssh/id_rsa.pub
RUN echo "Host host\nHostName 172.17.0.1\nUser ehb" > ~/.ssh/config
# RUN scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null host:.emacs ~/
RUN echo 'env TERM=vt100 emacs -nw $*' > /usr/bin/e && chmod +x /usr/bin/e
RUN touch ~/p
RUN echo 'ls -sxF $*' > /usr/bin/s && chmod +x /usr/bin/s
RUN echo 'ls -lth $* | head' > /usr/bin/lth && chmod +x /usr/bin/lth

# qt
RUN sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list
RUN apt-get update
RUN apt-get build-dep -y qtbase5-dev
#RUN apt-get build-dep -y qt5-default
RUN apt-get install -y libxcb*
RUN apt-get install -y libx11*
RUN apt-get install -y libxkbcommon*
RUN apt-get install -y cmake
RUN apt-get install -y ninja-build
RUN apt-get install -y libb2-dev
RUN apt-get install -y libzstd-dev
RUN apt-get install -y libsystemd-dev
RUN apt-get install -y libhunspell-dev
RUN apt-get install -y libmng-dev
RUN apt-get install -y libwebp-dev
RUN apt-get install -y libdouble-conversion-dev
RUN apt-get install -y libkrb5-dev
RUN apt-get install -y libdirectfb-dev
RUN apt-get install -y libts-dev
RUN apt-get install -y libproxy-dev
RUN apt-get install -y libsctp-dev
RUN apt-get install -y libbrotli-dev
RUN apt-get install -y libmd4c*
RUN apt-get install -y nodejs
RUN apt-get install -y python3-html5lib
RUN apt-get install -y gperf bison flex
RUN apt-get install -y libnss3*
RUN apt-get install -y libxshmfence-dev
RUN apt-get install -y libxkbfile-dev libsecret-1-dev
RUN apt-get install -y clang libclang-dev

# build qt
RUN wget https://download.qt.io/archive/qt/$qt_major_version/$qt_version/single/qt-everywhere-${qt_opensource}src-$qt_version.tar.xz
RUN XZ_DEFAULTS="-T 0" tar Jxf qt-everywhere-${qt_opensource}src-$qt_version.tar.xz

RUN mkdir /qt-build

## note need configure options, does support  configure -help & configure -list-features.
## -prefix
## -release
## also see https://doc.qt.io/qt-6/configure-options.html

RUN cd /qt-build && /qt-everywhere-src-$qt_version/configure 2>&1 > configure.log
RUN cd /qt-build && cmake --build . --parallel 2>&1 > build.log
RUN cd /qt-build && cmake --install .

