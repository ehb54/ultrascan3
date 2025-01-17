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

# temporary bits to ease development/testing

RUN ssh-keygen -t rsa -N '' -f ~/.ssh/id_rsa
RUN cat ~/.ssh/id_rsa.pub
RUN echo "Host host\nHostName 172.17.0.1\nUser ehb" > ~/.ssh/config
# RUN scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null host:.emacs ~/
RUN echo 'env TERM=vt100 emacs -nw $*' > /usr/bin/e && chmod +x /usr/bin/e
RUN touch ~/p
RUN echo 'ls -sxF $*' > /usr/bin/s && chmod +x /usr/bin/s
RUN echo 'ls -lth $* | head' > /usr/bin/lth && chmod +x /usr/bin/lth

RUN apt-get install -y acl \
aria2 \
binutils \
bison \
brotli \
build-essential \
bzip2 \
coreutils \
curl \
dbus \
dnsutils \
dpkg \
fakeroot \
file \
flex \
fonts-noto-color-emoji \
ftp \
gnupg2 \
haveged \
imagemagick \
iproute2 \
iputils-ping \
jq \
lib32z1 \
libc++-dev \
libc++abi-dev \
libcurl4 \
libgbm-dev \
libgconf-2-4 \
libgsl-dev \
libgtk-3-0 \
libmagic-dev \
libmagickcore-dev \
libmagickwand-dev \
libsecret-1-dev \
libsqlite3-dev \
libunwind8 \
libxkbfile-dev \
libxss1 \
locales \
m4 \
mediainfo \
mercurial \
net-tools \
netcat \
openssh-client \
p7zip-full \
p7zip-rar \
parallel \
pass \
patchelf \
pkg-config \
pollinate \
python-is-python3 \
rpm \
rsync \
shellcheck \
sphinxsearch \
sqlite3 \
ssh \
sshpass \
subversion \
sudo \
swig \
telnet \
texinfo \
time \
tk \
tzdata \
unzip \
upx \
wget \
xorriso \
xvfb \
xz-utils \
zip \
zsync

ENV LIB lib64
ENV PLATFORM x86_64

RUN wget https://github.com/github/codeql-cli-binaries/releases/download/v2.7.2/codeql-linux64.zip
RUN git clone https://github.com/github/codeql.git codeql
RUN cd codeql && unzip ../codeql-linux64.zip

# get test source
RUN git clone https://github.com/ehb54t/ultrascan3.git ultrascan3t

# setup initial local.pri's

RUN cp /ultrascan3t/admin/codeql/docker/local.pri.somo /ultrascan3t/us_somo/develop/local.pri
RUN cp /ultrascan3t/admin/codeql/docker/local.pri.gui /ultrascan3t/local.pri

# somo version setup
RUN cd /ultrascan3t/us_somo/develop && env ULTRASCAN=/ultrascan3t ./revision.sh && env us3=/ultrascan3t ./version.sh && qmake libus_somo.pro
RUN apt-get install -y php
RUN git clone https://github.com/ehb54/us3lims_dbutils
