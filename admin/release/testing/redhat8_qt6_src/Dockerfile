# dockerfile for us3 compiles (DEV IMAGE, full toolchain)

ARG image=docker.io/library/rockylinux:8.10
FROM $image

# ---------- compile defaults ----------
ARG parallel_compile=64
ARG ultrascan_branch=main

# ---------- qt / qwt versions ----------
ARG qt_major_version=6.8
ARG qt_version=6.8.3
ARG qwt_version=6.3.0

# ---------- env ----------
ENV ULTRASCAN=/ultrascan3
ENV QTDIR=/qt-${qt_version}
ENV QWTDIR=/qwt-${qwt_version}
ENV TERM=xterm-256color
ENV LC_ALL=C.UTF-8 LANG=C.UTF-8

# CHANGED: lean DNF + toolchain & dev deps IN ONE LAYER (no wildcards)
RUN set -eux; \
  printf '\ntsflags=nodocs\ninstall_weak_deps=False\n' >> /etc/dnf/dnf.conf; \
  dnf -y update; \
  dnf -y install dnf-plugins-core epel-release; \
  dnf config-manager --set-enabled powertools; \
  dnf -y groupinstall "Development Tools"; \
  dnf -y install \
    git ca-certificates curl \
    cmake python3 python3-pip ninja-build \
    bison flex gperf pkgconf-pkg-config \
    mpich mpich-devel \
    libX11-devel libXext-devel libXi-devel libXrandr-devel libXrender-devel \
    libxcb-devel xcb-util*-devel libxkbcommon-devel libxkbcommon-x11-devel \
    wayland-devel wayland-protocols-devel \
    mesa-libGL-devel mesa-libEGL-devel libdrm-devel mesa-libgbm-devel \
    freetype-devel harfbuzz-devel libjpeg-turbo-devel libpng-devel \
    zlib-devel brotli-devel pcre2-devel openssl-devel \
    dbus dbus-devel cups-devel glib2-devel libicu-devel \
    alsa-lib-devel pipewire-devel \
    gstreamer1-devel gstreamer1-plugins-base-devel gstreamer1-plugins-bad-free-devel \
    at-spi2-core at-spi2-atk \
    mariadb-devel \
    libarchive-devel \
    postgresql-devel; \
  dnf -y clean all; rm -rf /var/cache/dnf

WORKDIR /src

# CHANGED: Node 22 (one layer)
ARG nodejs_stream=22
RUN set -eux; \
  dnf -y module reset nodejs || true; \
  dnf -y module enable nodejs:${nodejs_stream}; \
  dnf -y install nodejs npm; \
  dnf -y clean all; rm -rf /var/cache/dnf; \
  node -v && npm -v

RUN dnf -y install gcc-toolset-13 gcc-toolset-13 && dnf clean all

SHELL ["/bin/bash", "-lc"]

# Make the newer GCC the default toolchain for all subsequent layers
ENV GCC_TOOLSET_ROOT=/opt/rh/gcc-toolset-13/root
ENV PATH="${GCC_TOOLSET_ROOT}/usr/bin:${PATH}"
ENV LD_LIBRARY_PATH="${GCC_TOOLSET_ROOT}/usr/lib64:${GCC_TOOLSET_ROOT}/usr/lib"
ENV CC="${GCC_TOOLSET_ROOT}/usr/bin/gcc" \
    CXX="${GCC_TOOLSET_ROOT}/usr/bin/g++" \
    AR="${GCC_TOOLSET_ROOT}/usr/bin/gcc-ar" \
    NM="${GCC_TOOLSET_ROOT}/usr/bin/gcc-nm" \
    RANLIB="${GCC_TOOLSET_ROOT}/usr/bin/gcc-ranlib"

ENV GCC_TOOLSET_DIR=/opt/rh/gcc-toolset-13/
RUN source ${GCC_TOOLSET_DIR}/enable && gcc --version && g++ --version

# CHANGED: Qt build in ONE RUN (download → prune → build → install → strip → cleanup)
RUN set -eux; \
  QT_SINGLE_URL="https://download.qt.io/archive/qt/${qt_major_version}/${qt_version}/single/qt-everywhere-src-${qt_version}.tar.xz"; \
  echo "Downloading ${QT_SINGLE_URL}"; \
  curl -fLO "${QT_SINGLE_URL}"; \
  tar -xf "qt-everywhere-src-${qt_version}.tar.xz"; \
  rm -f  "qt-everywhere-src-${qt_version}.tar.xz"; \
  cd "qt-everywhere-src-${qt_version}"; \
  rm -rf qtwebengine* qtpdf* qtvirtualkeyboard* qtwebview* || true; \
  cmake -G Ninja -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${QTDIR}" \
    -DQT_BUILD_TESTS=OFF \
    -DQT_BUILD_EXAMPLES=OFF \
    -DQT_FEATURE_optimize_size=ON \
    -DFEATURE_clang=OFF -DFEATURE_clangcpp=OFF \
    -DCMAKE_DISABLE_FIND_PACKAGE_Clang=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_LLVM=ON \
    -DQT_BUILD_TOOLS=ON \
    -DQT_BUILD_QTHELP=ON \
    -DQT_FEATURE_sql_psql=ON; \
  ninja -C build -j ${parallel_compile}; \
  ninja -C build install; \
  # prune static/dev detritus we don't need for the toolchain itself
  find "${QTDIR}" -type f \( -name '*.a' -o -name '*.la' -o -name '*.prl' \) -delete; \
  # strip bins & DSOs (best-effort)
  find "${QTDIR}" -type f -perm -0100 -exec strip --strip-unneeded {} + || true; \
  find "${QTDIR}" -type f -name '*.so*'  -exec strip --strip-unneeded {} + || true; \
  cd /src; rm -rf "qt-everywhere-src-${qt_version}"

# CHANGED: Qwt build in ONE RUN (lib only; Svg mandatory; no docs/examples/designer)
# --- build & install Qwt 6.3.0 with qmake (Qt6) ---
RUN set -eux; \
  QWT_URL="https://download.sourceforge.net/project/qwt/qwt/${qwt_version}/qwt-${qwt_version}.tar.bz2" && \
  echo "Downloading ${QWT_URL}" && \
  curl -fLo qwt-${qwt_version}.tar.bz2 "${QWT_URL}" && \
  tar -xf qwt-${qwt_version}.tar.bz2 && \
  rm -f  qwt-${qwt_version}.tar.bz2 && \
  cd qwt-${qwt_version} && \
  sed -i "/^unix {/,/^}/ s|QWT_INSTALL_PREFIX.*|QWT_INSTALL_PREFIX = /qwt-${qwt_version}|" qwtconfig.pri && \
  ${QTDIR}/bin/qmake && \
  make -j ${parallel_compile} && \
  make install && \
  # strip the built lib
  find "${QWTDIR}" -type f -name 'libqwt*.so*' -exec strip --strip-unneeded {} + || true && \
  cd /src && rm -rf "qwt-${qwt_version}"

# symlinks + env (one small layer)
RUN set -eux; \
  ln -sf ${QTDIR}/bin/qmake        /usr/bin/qmake6 || true; \
  ln -sf ${QTDIR}/bin/qtpaths      /usr/bin/qtpaths6 || true; \
  ln -sf ${QTDIR}/bin/moc          /usr/bin/moc6 || true; \
  ln -sf ${QTDIR}/bin/uic          /usr/bin/uic6 || true; \
  ln -sf ${QTDIR}/bin/rcc          /usr/bin/rcc6 || true; \
  ln -sf ${QTDIR}/bin/lrelease     /usr/bin/lrelease6 || true; \
  ln -sf ${QTDIR}/bin/lupdate      /usr/bin/lupdate6 || true; \
  ln -sf ${QTDIR}/bin/qml          /usr/bin/qml6 || true; \
  ln -sf ${QTDIR}/bin/qmlcachegen  /usr/bin/qmlcachegen6 || true; \
  ln -sf ${QTDIR}/bin/qmlimportscanner /usr/bin/qmlimportscanner6 || true; \
  printf '%s\n' \
    "# Qt6 environment (Docker)" \
    "export QTDIR=${QTDIR}" \
    'export PATH=${QTDIR}/bin:${QTDIR}/libexec:$PATH' \
    'export LD_LIBRARY_PATH=${QTDIR}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}' \
    'export PKG_CONFIG_PATH=${QTDIR}/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}' \
    'export QT_PLUGIN_PATH=${QTDIR}/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}' \
    'export QML2_IMPORT_PATH=${QTDIR}/qml${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}' \
    "export ULTRASCAN=${ULTRASCAN}" \
    "export QWTDIR=${QWTDIR}" \
    > /etc/profile.d/qt6.sh; \
  chmod 0755 /etc/profile.d/qt6.sh

# CHANGED: convenience repo—run, then remove its clone in the same layer
RUN set -eux; \
  git clone --depth 1 https://github.com/ehb54/convenience /tmp/convenience; \
  (cd /tmp/convenience && ./installcont.sh); \
  rm -rf /tmp/convenience

RUN set -eux; \
  dnf -y install \
    emacs-nox \
    which; \
  dnf -y clean all; rm -rf /var/cache/dnf

# CHANGED: remove the extra wildcard RPM install (it bloats images)
# (You already installed needed dev headers above, including mesa and mariadb-devel.)

# CHANGED: clone UltraScan in one layer (shallow). Keep .git for devs; drop if you want.
ARG ultrascan_branch
RUN set -eux; \
  git clone --branch "${ultrascan_branch}" https://github.com/ehb54/ultrascan3.git "${ULTRASCAN}"

# local.pri files
ADD local.pri.somo /ultrascan3/us_somo/develop/local.pri
ADD local.pri.gui  /ultrascan3/local.pri
ADD local.pri.mpi  /ultrascan3/local.pri.mpi

# small QoL bits
RUN set -eux; \
  printf '%s\n' "export MAKEFLAGS='-j80'" > /root/qt6env; \
  git config --global core.pager ''

# patches
# ADD qwt_compat.h /qwt-${qwt_version}/src/qwt_compat.h
# RUN sed -i '3i#include <QtCore5Compat>' /ultrascan3/us_somo/develop/include/us_extern.h

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

WORKDIR /ultrascan3

SHELL ["/bin/bash", "-lc"]
CMD ["/bin/bash", "-l"]

  
