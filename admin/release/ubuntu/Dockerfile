# dockerfile for us3 compiles

### exposed ARGs override defaults with --build-arg <varname>=<value>

ARG image=ubuntu:18.04
## ARGs are scoped FROM
FROM $image

## compile defaults
ARG parallel_compile=64
ARG ultrascan_branch=master

## qt defaults
ARG qt_major_version=5.15
ARG qt_version=5.15.12
ARG qwt_version=6.1.6
### N.B. qwt_version is hard coded in the local.pri.somo so we need to change this & in the codeql container & workflow/codeql-analysis.yml

# needed vor "opensource-" variants of qt source
ARG qt_opensource=opensource-

# needed for python apt variants, e.g. ubuntu22 needs python3
ARG apt_python_version=python

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
RUN apt-get install -y libarchive-dev

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
# build qt
RUN wget https://download.qt.io/archive/qt/$qt_major_version/$qt_version/single/qt-everywhere-${qt_opensource}src-$qt_version.tar.xz
RUN XZ_DEFAULTS="-T 0" tar Jxf qt-everywhere-${qt_opensource}src-$qt_version.tar.xz
RUN mkdir /usr/X11R6

## required for qt to find xcb-xlib deps
RUN ln -s /usr/lib/x86_64-linux-gnu /usr/X11R6/lib64
RUN cd qt-everywhere-src-$qt_version && env MAKEFLAGS=-j$parallel_compile ./configure -prefix /qt-$qt_version -release -opensource -confirm-license -platform linux-g++-64 -nomake tests -nomake examples -xcb -xcb-xlib -D QT_SHAREDMEMORY -D QT_SYSTEMSEMAPHORE -no-icu
RUN cd qt-everywhere-src-$qt_version && nohup make -j$parallel_compile 2>&1 > make.out
RUN cd qt-everywhere-src-$qt_version && nohup make install -j1 2>&1 > makeinstall.out
# RUN ls -l /qt-$qt_version/bin /qt-$qt_version/lib

# install qwt
RUN wget https://versaweb.dl.sourceforge.net/project/qwt/qwt/$qwt_version/qwt-$qwt_version.tar.bz2 && tar jxf qwt-$qwt_version.tar.bz2 && cd qwt-$qwt_version && /qt-$qt_version/bin/qmake && make -j$parallel_compile

# get source
## invalidate docker cache
## BRANCH could be pulled to top
ADD https://api.github.com/repos/ehb54/ultrascan3/git/refs/heads/$ultrascan_branch version.json
RUN git clone https://github.com/ehb54/ultrascan3.git

# setup initial local.pri's

RUN cp /ultrascan3/admin/codeql/docker/local.pri.somo /ultrascan3/us_somo/develop/local.pri
RUN cp /ultrascan3/admin/codeql/docker/local.pri.gui /ultrascan3/local.pri

# us version setup
RUN cd /ultrascan3/programs/us && env ULTRASCAN=/ultrascan3 ./revision.sh

# somo version setup
RUN cd /ultrascan3/us_somo/develop && env ULTRASCAN=/ultrascan3 ./revision.sh && env us3=/ultrascan3 ./version.sh

# install dependencies for building manual
RUN apt-get install -y doxygen texlive-full
RUN cpan Template

# make ultrascan
RUN cd $ULTRASCAN && export PATH=$QTDIR/bin:$PATH && ./makeall.sh -j$parallel_compile

# make sure manual exists
RUN cd $ULTRASCAN && ls -l bin/manual*

# make somo
RUN cd $ULTRASCAN && export PATH=$QTDIR/bin:$PATH && nohup ./makesomo.sh -j$parallel_compile 2>&1 > makesomo.out

# copy demo data (should really be in makesomo.sh)
RUN cp -rp $ULTRASCAN/us_somo/somo/demo/* $ULTRASCAN/somo/demo/

# copy manual data (should really be in makesomo.sh)
RUN cp -rp $ULTRASCAN/us_somo/somo/doc $ULTRASCAN/somo/

# get ssl confs
ADD https://api.github.com/repos/ehb54/openssl.cnf.d/git/refs/heads/main openssl_version.json
RUN cd /ultrascan3/etc && git clone https://github.com/ehb54/openssl.cnf.d

# get ubuntu rasmol
ADD https://api.github.com/repos/ehb54/rasmol-ubuntu/git/refs/heads/main ras_version.json
RUN git clone https://github.com/ehb54/rasmol-ubuntu.git && cp rasmol-ubuntu/bin/rasmol /ultrascan3/bin/ && cp rasmol-ubuntu/lib/* /ultrascan3/lib/

# copy assistant

RUN cp $QTDIR/bin/assistant /ultrascan3/bin

# make package
ARG image
RUN LD_LIBRARY_PATH=/ultrascan3/lib:$LD_LIBRARY_PATH /ultrascan3/admin/release/linux.pl `echo $image | sed s/:/-/` && ls -1 us3-ubuntu*.tar.xz > /lastpkgname
