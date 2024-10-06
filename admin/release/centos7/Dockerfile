# dockerfile for us3 compiles

### exposed ARGs override defaults with --build-arg <varname>=<value>

ARG image=docker.io/library/centos:7
## ARGs are scoped FROM
FROM $image

## compile defaults
ARG parallel_compile=64
ARG ultrascan_branch=master

## qt defaults
ARG qt_major_version=5.14
ARG qt_version=5.14.2
ARG qwt_version=6.1.6
### N.B. qwt_version is hard coded in the local.pri.somo so we need to change this & in the codeql container & workflow/codeql-analysis.yml

# needed for "opensource-" variants of qt source
ARG qt_opensource=

# needed for python apt variants, e.g. ubuntu22 needs python3
ARG apt_python_version=python

# env vars (could be moved up)
ENV ULTRASCAN=/ultrascan3
ENV QTDIR=/qt-$qt_version
ENV QWTDIR=/qwt-$qwt_version

# basic install stuff

RUN yum -y update
RUN yum install -y yum-utils
# RUN yum config-manager --set-enabled powertools
RUN yum groupinstall -y "Development Tools"
RUN yum install -y tzdata
# RUN yum install -y software-properties-common
# RUN yum install -y bind9-host
RUN yum install -y subversion git openssl-devel bc curl wget
RUN yum install -y zlib-devel
RUN yum install -y pkgconfig
RUN yum install -y epel-release
RUN yum install -y re2c
RUN yum install -y openssh-server
RUN yum install -y vim
RUN yum install -y emacs-nox
RUN yum install -y xorg-x11*
RUN yum install -y telnet
RUN yum install -y $apt_python_version
RUN yum install -y rsync
RUN yum install -y glib2-devel postgresql-devel at-spi2-core-devel libjpeg-devel fontconfig-devel 
RUN yum install -y libarchive-devel

# bits to ease development/testing

RUN ssh-keygen -t rsa -N '' -f ~/.ssh/id_rsa
RUN cat ~/.ssh/id_rsa.pub
RUN echo "Host host\nHostName 172.17.0.1\nUser ehb" > ~/.ssh/config
# RUN scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null host:.emacs ~/
RUN echo 'env TERM=vt100 emacs -nw $*' > /usr/bin/e && chmod +x /usr/bin/e
RUN touch ~/p
RUN echo 'ls -sxF $*' > /usr/bin/s && chmod +x /usr/bin/s
RUN echo 'ls -lth $* | head' > /usr/bin/lth && chmod +x /usr/bin/lth

# qt
# RUN sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list
# RUN yum update
#RUN yum install -y --skip-broken gbm* glesv2* libpcre2-posix libproxy-1.0 xcb-* xkeyboard-config
#RUN yum-builddep -y qt5-qtbase
RUN yum install -y libxcb*
RUN yum install -y --skip-broken xcb-util*
RUN yum install -y libX11*
RUN yum install -y libxkbcommon*
# build qt
RUN wget https://download.qt.io/archive/qt/$qt_major_version/$qt_version/single/qt-everywhere-${qt_opensource}src-$qt_version.tar.xz
RUN XZ_DEFAULTS="-T 0" tar Jxf qt-everywhere-${qt_opensource}src-$qt_version.tar.xz

## required for qt to find xcb-xlib deps
# RUN ls -l /lib64/libXext*
# RUN ln -s /lib64/libXext.so.[0-9] /lib64/libXext.so
# 
RUN yum install -y mesa-lib* mariadb-devel libzstd* libpcre2* harfbuzz*

RUN yum install -y centos-release-scl && yum install -y devtoolset-8*

RUN yum install -y openssl11*
RUN mkdir /usr/X11R6 && ln -s /lib64 /usr/X11R6/lib64
RUN cd qt-everywhere-src-$qt_version && . /opt/rh/devtoolset-8/enable && env MAKEFLAGS=-j$parallel_compile ./configure -prefix /qt-$qt_version -release -opensource -confirm-license -platform linux-g++-64 -nomake tests -nomake examples -xcb -xcb-xlib -D QT_SHAREDMEMORY -D QT_SYSTEMSEMAPHORE -no-icu

RUN cd qt-everywhere-src-$qt_version && . /opt/rh/devtoolset-8/enable && nohup make -j$parallel_compile 2>&1 > make.out
RUN cd qt-everywhere-src-$qt_version && . /opt/rh/devtoolset-8/enable && nohup make install -j1 2>&1 > makeinstall.out
# RUN ls -l /qt-$qt_version/bin /qt-$qt_version/lib

# install qwt
RUN wget https://versaweb.dl.sourceforge.net/project/qwt/qwt/$qwt_version/qwt-$qwt_version.tar.bz2 && tar jxf qwt-$qwt_version.tar.bz2 && cd qwt-$qwt_version && . /opt/rh/devtoolset-8/enable && /qt-$qt_version/bin/qmake && make -j$parallel_compile

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
RUN dnf install -y doxygen texlive-*
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

# get ubuntu rasmol
ADD https://api.github.com/repos/ehb54/rasmol-ubuntu/git/refs/heads/main ras_version.json
RUN git clone https://github.com/ehb54/rasmol-ubuntu.git && cp rasmol-ubuntu/bin/rasmol /ultrascan3/bin/ && cp rasmol-ubuntu/lib/* /ultrascan3/lib/

# copy assistant

RUN cp $QTDIR/bin/assistant /ultrascan3/bin

# make package
ARG image
# RUN LD_LIBRARY_PATH=/ultrascan3/lib:$LD_LIBRARY_PATH /ultrascan3/admin/release/linux.pl `echo $image | sed s/:/-/` && ls -1 us3-ubuntu*.tar.xz > /lastpkgname
