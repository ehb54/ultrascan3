# dockerfile for us3 compiles

### exposed ARGs override defaults with --build-arg <varname>=<value>

ARG image=docker.io/library/rockylinux:9.6
## ARGs are scoped FROM
FROM $image

## compile defaults
ARG parallel_compile=64
ARG ultrascan_branch=main

## qt defaults
ARG qt_major_version=5.15
ARG qt_version=5.15.17
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

# basic install stuff

RUN dnf update -y
RUN dnf install -y yum-utils
RUN dnf install -y dnf-plugins-core

# Works on Rocky 8 and 9;
RUN set -eux; \
    . /etc/os-release; \
    dnf -y install dnf-plugins-core; \
    # Enable the correct source repo for build deps
    case "$VERSION_ID" in \
      8*) dnf config-manager --set-enabled powertools ;; \
      9*|10*) dnf config-manager --set-enabled crb ;; \
      *)  echo "Unsupported Rocky version: $VERSION_ID" >&2; exit 1 ;; \
    esac; \
    # Enable EPEL (try the packaged one first; fall back to the EPEL URL)
    if ! rpm -q epel-release >/dev/null 2>&1; then \
      if ! dnf -qy install epel-release; then \
        major="${VERSION_ID%%.*}"; \
        dnf -y install "https://dl.fedoraproject.org/pub/epel/epel-release-latest-${major}.noarch.rpm"; \
      fi; \
    fi; \
    # Install curl variant and re2c
    case "$VERSION_ID" in \
      8*) dnf -y install curl re2c ;; \
      9*|10*) dnf -y install curl-minimal re2c ;; \
    esac

RUN dnf groupinstall -y "Development Tools"
RUN dnf install -y tzdata
# RUN dnf install -y software-properties-common
# RUN dnf install -y bind9-host
RUN dnf install -y subversion
RUN dnf install -y git
RUN dnf install -y openssl-devel
RUN dnf install -y bc
RUN dnf install -y wget
RUN dnf install -y zlib-devel
RUN dnf install -y pkg-config
RUN dnf install -y re2c
RUN dnf install -y openssh-server
RUN dnf install -y vim
RUN dnf install -y emacs-nox
RUN dnf install -y 'xorg-x11*' --exclude=xorg-x11-server-x2gokdrive
RUN dnf install -y telnet
RUN dnf install -y $apt_python_version
RUN dnf install -y rsync
RUN dnf install -y glib2-devel postgresql-devel at-spi2-core-devel libjpeg-devel fontconfig-devel 
RUN dnf install -y libarchive-devel

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

# RUN dnf update
RUN yum-builddep -y qt5-devel
RUN dnf install -y libxcb*
RUN dnf install -y xcb-util*
RUN dnf install -y libX11*
RUN dnf install -y libxkbcommon*
# build qt
RUN wget https://download.qt.io/archive/qt/$qt_major_version/$qt_version/single/qt-everywhere-${qt_opensource}src-$qt_version.tar.xz
RUN XZ_DEFAULTS="-T 0" tar Jxf qt-everywhere-${qt_opensource}src-$qt_version.tar.xz

## required for qt to find xcb-xlib deps
## install to get libXext.so
RUN dnf -y install libXext-devel || true
RUN ls -l /lib64/libXext*
RUN mkdir /usr/X11R6 && ln -s /lib64 /usr/X11R6/lib64
RUN dnf install -y mesa-lib* mariadb-devel
# RUN dnf groupinstall -y --allowerasing workstation

## 2025.09.30 - build openssl & mariadb connector (mysqlclient) - needed until we have new certs mechanism for db connection

ARG OPENSSL_V=1.1.1w
ARG MARIADB_CC_V=3.3.8

# Build deps
RUN dnf -y install gcc gcc-c++ make cmake bzip2 xz tar gzip bison ncurses-devel zlib-devel libaio-devel libarchive-devel openssl-devel perl

# install dependencies for building manual
# 2025.10.03 - moved up in dockerfile as it takes awhile and everything upto qt build is relatively stable
RUN dnf install -y doxygen texlive-*
RUN dnf install -y perl-CPAN && cpan Template
RUN dnf install -y graphviz

# ---- Build OpenSSL 1.1.1 (shared) ----
WORKDIR /src
RUN curl -fsSLO https://www.openssl.org/source/openssl-${OPENSSL_V}.tar.gz
RUN tar xzf openssl-${OPENSSL_V}.tar.gz
RUN cd openssl-${OPENSSL_V} && \
    ./config --prefix=/openssl-1.1.1 --openssldir=/openssl-1.1.1 shared && \
    make -j"$(parallel_compile)" && make install_sw

# ---- Build MariaDB Connector/C against our OpenSSL 1.1.1 ----
# (client-only lib: libmariadb.so + headers)
RUN curl -fsSLO https://downloads.mariadb.com/Connectors/c/connector-c-${MARIADB_CC_V}/mariadb-connector-c-${MARIADB_CC_V}-src.tar.gz && \
    tar xzf mariadb-connector-c-${MARIADB_CC_V}-src.tar.gz && \
    mkdir -p mariadb-connector-c-${MARIADB_CC_V}-src/build && \
    cd mariadb-connector-c-${MARIADB_CC_V}-src/build && \
    cmake .. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/mariadb-client-openssl111 \
      -DOPENSSL_INCLUDE_DIR=/openssl-1.1.1/include \
      -DOPENSSL_SSL_LIBRARY=/openssl-1.1.1/lib/libssl.so \
      -DOPENSSL_CRYPTO_LIBRARY=/openssl-1.1.1/lib/libcrypto.so \
      -DWITH_UNIT_TESTS=OFF \
      -DBUILD_TESTING=OFF && \
    make -j"$(parallel_compile)" && make install

WORKDIR /

## qt need llvm/libclang (only if qdoc, qtwebengine or qtpdf is used - keeping if needed in future)
#RUN set -eux; \
#    . /etc/os-release; \
#    # ensure config-manager is present (minimal images)
#    dnf -y install dnf-plugins-core; \
#    # enable the right repo (Powertools on 8, CRB on 9)
#    case "$VERSION_ID" in \
#      8*) dnf config-manager --set-enabled powertools ;; \
#      9*|10*) dnf config-manager --set-enabled crb ;; \
#    esac; \
#    # common build tools
#    dnf -y install gperf bison flex; \
#    # LLVM/Clang by major version
#    case "$VERSION_ID" in \
#      8*) dnf -y install clang clang-devel llvm llvm-devel ;; \
#      9*|10*) dnf -y install clang-devel llvm llvm-devel llvm-static ;; \
#    esac; \
#    # sanity check
#    which llvm-config || echo "Note: llvm-config not found (on RL8 it comes via llvm-devel; on RL9 via llvm-static)."

# qt needs python (only if qtwebengine or qtpdf is used - keeping if needed in future)
#RUN set -eux; \
#    . /etc/os-release; \
#    case "$VERSION_ID" in \
#      8*) \
#        dnf -y install python3 && ln -sf /usr/bin/python3 /usr/bin/python; \
#        ;; \
#      9*|10*) \
#        dnf -y install python3 python-unversioned-command; \
#        ;; \
#    esac
#RUN python --version

## 2025.09.29 - add -skip webengine -skip location -skip wayland ; they are not needed to build or run ultrascan and create dependancy issues
## 2025.09.30 - force openssl linkage, extend configure to multi-line
RUN cd qt-everywhere-src-$qt_version && \
    env MAKEFLAGS=-j$parallel_compile \
        OPENSSL_PREFIX=/openssl-1.1.1 \
        OPENSSL_INCDIR=/openssl-1.1.1/include \
        OPENSSL_LIBDIR=/openssl-1.1.1/lib \
        OPENSSL_LIBS="/openssl-1.1.1/lib/libssl.a /openssl-1.1.1/lib/libcrypto.a -lz -ldl -lpthread" \
        PKG_CONFIG_PATH=/openssl-1.1.1/lib/pkgconfig:$PKG_CONFIG_PATH \
    ./configure -prefix /qt-$qt_version \
                -release \
                -opensource \
                -confirm-license \
                -platform linux-g++-64 \
                -nomake tests \
                -nomake examples \
                -xcb -xcb-xlib \
                -no-icu \
                -skip qtwebengine \
                -skip qtlocation \
                -skip qtwayland \
                -openssl-linked \
                -I /openssl-1.1.1/include \
                -L /openssl-1.1.1/lib \
                QMAKE_CFLAGS+="-I/openssl-1.1.1/include -DQT_SHAREDMEMORY -DQT_SYSTEMSEMAPHORE" \
                QMAKE_CXXFLAGS+="-I/openssl-1.1.1/include -DQT_SHAREDMEMORY -DQT_SYSTEMSEMAPHORE" \
                QMAKE_LFLAGS+="-Wl,-rpath,/qt-$qt_version/lib -Wl,--enable-new-dtags"

RUN cd qt-everywhere-src-$qt_version && nohup make -j$parallel_compile 2>&1 > make.out
RUN cd qt-everywhere-src-$qt_version && nohup make install -j1 > makeinstall.out
## 2025.10.03 make static libqsqlmysql.a
RUN cd /qt-everywhere-src-$qt_version/qtbase/src/plugins/sqldrivers/mysql && \
    /qt-$qt_version/bin/qmake CONFIG+=staticlib \
       "QMAKE_MYSQL_CONFIG=/mariadb-client-openssl111/bin/mariadb_config" \
       "QMAKE_INCDIR_MYSQL=/mariadb-client-openssl111/include/mariadb" \
       "QMAKE_LIBDIR_MYSQL=/mariadb-client-openssl111/lib/mariadb" \
       "QMAKE_LIBS_MYSQL=/mariadb-client-openssl111/lib/mariadb/libmariadb.a /openssl-1.1.1/lib/libssl.a /openssl-1.1.1/lib/libcrypto.a -lz -lzstd" \
       "CONFIG+=no_link_prl" \
       mysql.pro && \
       make clean && make -j$parallel_compile

# Neutralize qsqlmysql PRL so it doesn't inject -lmariadb into your apps
RUN set -euo pipefail; \
    prl="/qt-everywhere-src-$qt_version/qtbase/plugins/sqldrivers/libqsqlmysql.prl"; \
    if [ ! -f "$prl" ]; then \
      echo "ERROR: $prl not found. Build the static qsqlmysql first."; \
      ls -l /qt-everywhere-src-$qt_version/qtbase/plugins/sqldrivers || true; \
      exit 1; \
    fi; \
    sed -i -e 's/^QMAKE_PRL_LIBS.*/QMAKE_PRL_LIBS =/' \
           -e 's/^QMAKE_PRL_LIBS_FOR_CMAKE.*/QMAKE_PRL_LIBS_FOR_CMAKE =/' "$prl"; \
    echo "After neutralize:"; \
    grep -n 'QMAKE_PRL_LIBS' "$prl" || true

RUN rm -f /qt-everywhere-src-$qt_version/qtbase/plugins/sqldrivers/libqsqlmysql.prl

## 2025.10.03 remove shared library libqsqlmysql.so so nothing can link against
RUN rm -f /qt-$qt_version/plugins/sqldrivers/libqsqlmysql.so 

RUN ls -l /qt-$qt_version/bin /qt-$qt_version/lib
# verify docs tools installed
RUN ls -l /qt-$qt_version/bin/{qhelpgenerator,qcollectiongenerator}
# 2025.09.30 verify right ssl 
RUN grep -i 'OpenSSL' /qt-everywhere-src-$qt_version/config.summary
# RUN ldd /qt-$qt_version/lib/libQt5Network.so | egrep 'ssl|crypto'

# install qwt
RUN wget https://versaweb.dl.sourceforge.net/project/qwt/qwt/$qwt_version/qwt-$qwt_version.tar.bz2 && tar jxf qwt-$qwt_version.tar.bz2 && cd qwt-$qwt_version && /qt-$qt_version/bin/qmake && make -j$parallel_compile

## 2025.10.03 libarchive static
# deps for a clean, feature-minimal static build
RUN dnf -y install gcc gcc-c++ make cmake tar xz
RUN dnf -y install zlib-devel zlib-static   # keep zlib; turn off the rest to simplify

# fetch & unpack (pin the version you prefer)
RUN curl -fsSLO https://www.libarchive.org/downloads/libarchive-3.7.4.tar.xz
RUN tar -xf libarchive-3.7.4.tar.xz

# configure: static + PIC, zlib only, NO crypto/SSL, NO libxml2/xar, NO extras
RUN cmake -S libarchive-3.7.4 -B /tmp/la-build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DENABLE_ZLIB=ON \
  -DENABLE_BZip2=OFF \
  -DENABLE_LZMA=OFF \
  -DENABLE_LZ4=OFF \
  -DENABLE_ZSTD=OFF \
  -DENABLE_EXPAT=OFF \
  -DENABLE_LIBXML2=OFF \
  -DENABLE_OPENSSL=OFF \
  -DENABLE_GNUTLS=OFF \
  -DENABLE_MBEDTLS=OFF \
  -DENABLE_NETTLE=OFF \
  -DENABLE_LIBB2=OFF \
  -DENABLE_CNG=OFF

RUN cmake --build /tmp/la-build -j$parallel_compile
RUN cmake --install /tmp/la-build

# sanity: ensure it’s a static archive and PIC-friendly
RUN file /usr/lib/libarchive.a && \
    nm -g --defined-only /usr/lib/libarchive.a | head -n 25

# get source
## invalidate docker cache
## BRANCH could be pulled to top
ADD https://api.github.com/repos/ehb54/ultrascan3/git/refs/heads/$ultrascan_branch version.json
RUN git clone https://github.com/ehb54/ultrascan3.git
RUN cd $ULTRASCAN && git fetch --all
RUN cd $ULTRASCAN && git checkout -B $ultrascan_branch
RUN cd $ULTRASCAN && git reset --hard origin/$ultrascan_branch

# setup initial local.pri's

RUN cp /ultrascan3/admin/codeql/docker/local.pri.somo /ultrascan3/us_somo/develop/local.pri
RUN cp /ultrascan3/admin/codeql/docker/local.pri.gui /ultrascan3/local.pri

# us version setup
RUN cd /ultrascan3/programs/us && env ULTRASCAN=/ultrascan3 ./revision.sh

# somo version setup
RUN cd /ultrascan3/us_somo/develop && env ULTRASCAN=/ultrascan3 ./revision.sh && env us3=/ultrascan3 ./version.sh

## upgrade doxygen as the file is often not current in repo
RUN cd $ULTRASCAN && doxygen -u

## 2025.09.30 - patches to link against mariadb openssl 1.1.1
WORKDIR /ultrascan3
RUN dnf -y install patch
RUN patch -p1 <<'PATCH'
diff --git a/gui.pri b/gui.pri
index 1c646d68d..f81b656f1 100644
--- a/gui.pri
+++ b/gui.pri
@@ -15,11 +15,15 @@ CONFIG       += $$DEBUGORRELEASE qt thread warn widgets

 unix {
   LIBS       += -L../../lib -lus_gui -lus_utils
-  LIBS       += -lcrypto
+#  LIBS       += -lcrypto
   LIBS       += $$QWTLIB
   ##LIBS       += -lqwtplot3d-qt4 -lGLU
   LIBS       += -lqwtplot3d -lGLU
-  LIBS       += -L$$MYSQLDIR -lmysqlclient
+  LIBS       += -L$$MYSQLDIR/libmariadb.a
+  LIBS       += /openssl-1.1.1/lib/libssl.a
+  LIBS       += /openssl-1.1.1/lib/libcrypto.a
+  LIBS         += -Wl,-Bstatic /usr/lib/libarchive.a -Wl,-Bdynamic -lz
+
   DEFINES    += INTEL LINUX

   DEPENDPATH   += ../../gui ../../utils $$QWTPATH/include ..

PATCH

RUN patch -p1 <<'PATCH'
diff --git a/library.pri b/library.pri
index 352d72950..dfb1686c2 100644
--- a/library.pri
+++ b/library.pri
@@ -26,8 +26,8 @@ unix {
     INCLUDEPATH  += $$MYSQLPATH
   }
   DEFINES      += INTEL LINUX
-  LIBS         += -larchive
-  
+  LIBS         += -Wl,-Bstatic /usr/lib/libarchive.a -Wl,-Bdynamic -lz -lxml2  
+
 }
 
 win32 {

PATCH

RUN patch -p1 <<'PATCH'
diff --git a/utils/libus_utils.pro b/utils/libus_utils.pro
index 63d8c2a6f..41becfc3f 100644
--- a/utils/libus_utils.pro
+++ b/utils/libus_utils.pro
@@ -1,3 +1,5 @@
+BUILD_LIBRARY = yes
+
 include ( ../library.pri )
 
 QT          -= gui
@@ -5,9 +7,53 @@ QT          += network
 QT          += sql
 unix: TARGET = us_utils
 
+CONFIG  += dll shared no_link_prl
+DEFINES += BUILD_US_UTILS
+
+# --- Keep system libs out (and avoid PRL transitive deps) ---
+QMAKE_LIBDIR  -= /usr/lib64 /lib64
+LIBS          -= -L/usr/lib64 -L/lib64 -lssl -lcrypto -lmariadb -lmysqlclient -larchive
+QMAKE_LFLAGS  += -Wl,--as-needed
+
+# --- Visibility: export only your API ; keep everything else local ---
+QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
+
+# If you ever call into your own (now hidden) SSL symbols from inside this .so,
+# keep them bound locally (helps avoid PLT interposition surprises):
+QMAKE_LFLAGS += -Wl,-Bsymbolic-functions
+
+# --- Headers for your bundled deps ---
+INCLUDEPATH += /mariadb-client-openssl111/include/mariadb
+INCLUDEPATH += /openssl-1.1.1/include
+# (Add your libarchive headers if you need them)
+
+# --- Link your static deps (order matters ; group helps circular refs) ---
+# /openssl-1.1.1/lib/libssl.a \
+# /openssl-1.1.1/lib/libcrypto.a \
+
+LIBS += -Wl,--start-group \
+/mariadb-client-openssl111/lib/mariadb/libmariadb.a \
+/usr/lib/libarchive.a \
+-lz -lzstd \
+-Wl,--end-group
+
+# Common libc bits many of these need:
+LIBS += -ldl -lpthread -lresolv -lm
+
+# --- CRITICAL: hide all those static libs' symbols from the runtime ---
+# (choose ONE of the two lines below)
+
+# A) Precise: list the archives you want hidden
+# QMAKE_LFLAGS += -Wl,--exclude-libs,libssl.a,libcrypto.a,libmariadb.a,libarchive.a
+QMAKE_LFLAGS += -Wl,--exclude-libs,libssl.a
+
+# B) Nuclear: hide ALL symbols from ALL static archives linked into this .so
+#    (use this if you keep adding more .a's and don’t want to maintain the list)
+# QMAKE_LFLAGS += -Wl,--exclude-libs,ALL
+
 macx {
       TARGET = us_utils
-      LIBS  += -L$${MYSQLDIR} -lmysqlclient
+#      LIBS  += -L$${MYSQLDIR} -lmysqlclient
       LIBS  += -L../lib
 }
 
PATCH

RUN patch -p1 <<'PATCH'
diff --git a/gui/libus_gui.pro b/gui/libus_gui.pro
index f6c5ddf6a..30f746390 100644
--- a/gui/libus_gui.pro
+++ b/gui/libus_gui.pro
@@ -1,3 +1,5 @@
+BUILD_LIBRARY = yes
+
 include ( ../library.pri )
 
 QT            += opengl
@@ -7,6 +9,52 @@ QT            += printsupport
 
 unix:   TARGET = us_gui
 
+QMAKE_USE -= mysql
+
+CONFIG  += dll shared no_link_prl
+DEFINES += BUILD_US_GUI
+
+# --- Keep system libs out (and avoid PRL transitive deps) ---
+QMAKE_LIBDIR  -= /usr/lib64 /lib64
+LIBS          -= -L/usr/lib64 -L/lib64 -lssl -lcrypto -lmariadb -lmysqlclient -larchive
+QMAKE_LFLAGS  += -Wl,--as-needed
+
+# --- Visibility: export only your API ; keep everything else local ---
+QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
+
+# If you ever call into your own (now hidden) SSL symbols from inside this .so,
+# keep them bound locally (helps avoid PLT interposition surprises):
+QMAKE_LFLAGS += -Wl,-Bsymbolic-functions
+
+# --- Headers for your bundled deps ---
+INCLUDEPATH += /mariadb-client-openssl111/include/mariadb
+INCLUDEPATH += /openssl-1.1.1/include
+# (Add your libarchive headers if you need them)
+
+# --- Link your static deps (order matters ; group helps circular refs) ---
+# /openssl-1.1.1/lib/libssl.a \
+# /openssl-1.1.1/lib/libcrypto.a \
+
+# LIBS += -Wl,--start-group \
+# /mariadb-client-openssl111/lib/mariadb/libmariadb.a \
+# /usr/lib/libarchive.a \
+# -lz -lzstd \
+# -Wl,--end-group
+
+# Common libc bits many of these need:
+LIBS += -ldl -lpthread -lresolv -lm
+
+# --- CRITICAL: hide all those static libs' symbols from the runtime ---
+# (choose ONE of the two lines below)
+
+# A) Precise: list the archives you want hidden
+# QMAKE_LFLAGS += -Wl,--exclude-libs,libssl.a,libcrypto.a,libmariadb.a,libarchive.a
+QMAKE_LFLAGS += -Wl,--exclude-libs,libssl.a
+
+# B) Nuclear: hide ALL symbols from ALL static archives linked into this .so
+#    (use this if you keep adding more .a's and don’t want to maintain the list)
+# QMAKE_LFLAGS += -Wl,--exclude-libs,ALL
+
 win32 {
         DEFINES += QWT_DLL US_MAKE_GUI_DLL
         TARGET   = us_gui
PATCH

RUN patch -p1 <<'PATCH'
diff --git a/utils/us_extern.h b/utils/us_extern.h
index b57a682a8..d8cbe2ae2 100644
--- a/utils/us_extern.h
+++ b/utils/us_extern.h
diff --git a/utils/us_extern.h b/utils/us_extern.h
index b57a682a8..ff7b82eb6 100644
--- a/utils/us_extern.h
+++ b/utils/us_extern.h
@@ -105,4 +105,19 @@
   #define US_GUI_EXTERN
 #endif

+#pragma once
+#include <QtCore/qglobal.h>
+
+#ifdef BUILD_US_UTILS
+#  define US_UTIL_EXTERN Q_DECL_EXPORT
+#else
+#  define US_UTIL_EXTERN Q_DECL_IMPORT
+#endif
+
+#ifdef BUILD_US_GUI
+#  define US_GUI_EXTERN Q_DECL_EXPORT
+#else
+#  define US_GUI_EXTERN Q_DECL_IMPORT
+#endif
+
 #endif // US_EXTERN_H
PATCH

RUN dnf install -y ed

RUN ed -s local.pri <<'ED'
17,18c
  MYSQLDIR    = /mariadb-client-openssl111/lib/mariadb
  MYSQLPATH   = /mariadb-client-openssl111/include/mariadb
.
$a
# --- DB stack is ON by default. Set `NO_DB = yes` in a .pro to disable. ---
!contains(NO_DB, yes) {

  QT += sql
  QTPLUGIN += qsqlmysql          # auto-register the static plugin (no Q_IMPORT_PLUGIN needed)

  # Paths
  QMYSQL_STATIC_PLUGIN = /qt-everywhere-src-$qt_version/qtbase/plugins/sqldrivers/libqsqlmysql.a
  DBCLIENT_ARCHIVE = /mariadb-client-openssl111/lib/mariadb/libmariadb.a
  OPENSSL_SSL_A    = /openssl-1.1.1/lib/libssl.a
  OPENSSL_CRYPTO_A = /openssl-1.1.1/lib/libcrypto.a

  # If your connector produced libmariadbclient.a instead, fall back to it:
  !exists($$DBCLIENT_ARCHIVE) {
    DBCLIENT_ARCHIVE = /mariadb-client-openssl111/lib/mariadb/libmariadbclient.a
  }

  # Hard fail if the static plugin isn't built yet (saves head-scratching)
  !exists($$QMYSQL_STATIC_PLUGIN) {
    error("Static QMYSQL plugin not found at $$QMYSQL_STATIC_PLUGIN. Build it first.")
  }

  # Headers
  INCLUDEPATH += /mariadb-client-openssl111/include/mariadb
  INCLUDEPATH += /openssl-1.1.1/include

  # Link the static plugin + MariaDB + OpenSSL (no runtime search needed)
  LIBS += $$QMYSQL_STATIC_PLUGIN
  LIBS += $$DBCLIENT_ARCHIVE
  contains(BUILD_LIBRARY, yes) {
     message( "libssl/crypto disabled for $$TARGET" )
   } else {
     LIBS += $$OPENSSL_SSL_A $$OPENSSL_CRYPTO_A
     message( "libssl/crypto enabled for $$TARGET" )
  }
  LIBS += -lz -lzstd -ldl -lpthread -lresolv -lm

  # Keep Qt shared; make sure it's found at runtime
  QMAKE_LFLAGS += -Wl,-rpath,/qt-$qt_version/lib -Wl,--enable-new-dtags

  # Your global defines
  DEFINES += QT_SHAREDMEMORY QT_SYSTEMSEMAPHORE

  message( "DB enabled for $$TARGET (static QMYSQL + MariaDB + OpenSSL 1.1.1)" )
} else {
  message( "DB disabled for $$TARGET (NO_DB = yes)" )
}
.
wq
ED

## $qt_version is not replaced due to the heredoc needing to keep qmake's $ variable fixed, so just patch it after
RUN sed -i "s|\$qt_version|${qt_version}|g" local.pri

WORKDIR /

# make ultrascan
RUN dnf install -y libxml2 libxml2-devel
## faster qmake, make first
RUN cd $ULTRASCAN && export PATH=$QTDIR/bin:$PATH && qmake && make -j$parallel_compile

RUN cd $ULTRASCAN && export PATH=$QTDIR/bin:$PATH && ./makeall.sh -j$parallel_compile

## old versions of makeall.sh fail to build manual as it is doing in parallel, make sure we build the manual
RUN cd $ULTRASCAN/doc/manual && export PATH=$QTDIR/bin:$PATH && make -j1

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

# get centos rasmol
ADD https://api.github.com/repos/ehb54/rasmol-centos/git/refs/heads/main ras_version.json
RUN git clone https://github.com/ehb54/rasmol-centos.git && cp rasmol-centos/bin/rasmol /ultrascan3/bin/ && cp rasmol-centos/lib/* /ultrascan3/lib/

# copy assistant

RUN cp $QTDIR/bin/assistant /ultrascan3/bin

# dependency needed on RH 10
RUN dnf install -y libgtk-x11-2*; true

# validations
#RUN grep -i 'OpenSSL' /qt-everywhere-src-$qt_version/config.summary
#RUN ldd /qt-$qt_version/lib/libQt5Network.so | egrep 'ssl|crypto'
#RUN LD_LIBRARY_PATH=/ultrascan3/lib ldd /mariadb-client-openssl111/lib/mariadb/libmariadb.so | egrep 'ssl|crypto'

# make package
ARG image
RUN IMAGE=`echo $image | sed 's/rockylinux\/rockylinux/redhat/' | sed s/:/-/` && echo $IMAGE && LD_LIBRARY_PATH=/ultrascan3/lib:$LD_LIBRARY_PATH /ultrascan3/admin/release/linux.pl $IMAGE && ls -1 us3-*.tar.xz > /lastpkgname

# 2025.10.06 - extra bits for testing - doesn't hurt as we are only using the container to build ultrascan - not distributing it

RUN git clone https://github.com/ehb54/convenience && ./convenience/installcont.sh

RUN cat <<EOF > /root/qt5env
export PATH=/qt-$qt_version/bin:/root/.local/bin:/root/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
export LD_LIBRARY_PATH=/ultrascan3/lib:/qwt-$qwt_version/lib:/qt-$qt_version/lib
export MAKEFLAGS='-j80'
EOF

RUN cat <<EOF > /root/prodenv
export PATH=/root/.local/bin:/root/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
export LD_LIBRARY_PATH=/us3-*/lib
export MAKEFLAGS='-j80'
EOF

RUN git config --global core.pager ''
RUN git config --global user.email brookes@uthscsa.edu
RUN git config --global user.name ehb54
