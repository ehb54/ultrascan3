FROM ubuntu:22.04

# dockerfile for us3 compiles

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
RUN apt-get install -y cmake
RUN apt-get install -y libarchive-dev

# qt
RUN apt-get install -y qtbase5-dev
RUN apt-get install -y libqt5svg5-dev
RUN apt-get install -y libqt5sql5-mysql
RUN apt-get install -y libqt5sql5-psql
RUN apt-get install -y qtmultimedia5-dev
RUN apt-get install -y libqt5datavisualization5
RUN apt-get install -y libqt5datavisualization5-dev

# qt support

# RUN apt-get install -y libmariadbclient-dev
RUN apt-get install -y libmysqlclient-dev
RUN apt-get install -y mpich

# install qwt
RUN wget https://versaweb.dl.sourceforge.net/project/qwt/qwt/6.1.6/qwt-6.1.6.tar.bz2
RUN tar jxf qwt-6.1.6.tar.bz2
RUN cd qwt-6.1.6 && qmake && make -j12 && make install

# get source
RUN git clone https://github.com/ehb54/ultrascan3.git

# setup initial local.pri's

RUN cp /ultrascan3/admin/codeql/docker/local.pri.somo /ultrascan3/us_somo/develop/local.pri
RUN cp /ultrascan3/admin/codeql/docker/local.pri.gui /ultrascan3/local.pri

# somo version setup
RUN cd /ultrascan3/us_somo/develop && env ULTRASCAN=/ultrascan3 ./revision.sh && env us3=/ultrascan3 ./version.sh
