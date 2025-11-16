# dockerfile for us3 compiles (DEV IMAGE, full toolchain) — Ubuntu 24.04 (Noble)

FROM ubuntu:24.04

# ---------- compile defaults ----------
ARG parallel_compile=64
ARG ultrascan_branch=main

# ---------- qt / qwt versions ----------
ARG qt_major_version=6.8
ARG qt_version=6.8.3
ARG qwt_version=6.3.0

# ---------- env ----------
ENV DEBIAN_FRONTEND=noninteractive
ENV ULTRASCAN=/ultrascan3
ENV QTDIR=/qt-${qt_version}
ENV QWTDIR=/qwt-${qwt_version}
ENV TERM=xterm-256color
ENV LC_ALL=C.UTF-8 LANG=C.UTF-8

# Base toolchain & dev deps (single layer)
RUN set -eux; \
  apt-get update; \
  apt-get install -y --no-install-recommends \
    # essentials
    build-essential git ca-certificates curl wget xz-utils binutils file \
    cmake ninja-build python3 python3-pip pkg-config bison flex gperf \
    mpich \
    # X11 / xcb / Wayland / GL stack
    libx11-dev libxext-dev libxi-dev libxrandr-dev libxrender-dev \
    libxcb1-dev libxkbcommon-dev libxkbcommon-x11-dev \
    libxcb-util-dev libxcb-image0-dev libxcb-icccm4-dev \
    libxcb-keysyms1-dev libxcb-render0-dev libxcb-render-util0-dev \
    libxcb-shm0-dev libxcb-xfixes0-dev \
    libwayland-dev wayland-protocols \
    libgl1-mesa-dev libegl1-mesa-dev libdrm-dev libgbm-dev libglu1-mesa-dev \
    # text/font/crypto/etc
    libfreetype-dev libharfbuzz-dev libjpeg-dev libpng-dev zlib1g-dev \
    libbrotli-dev libpcre2-dev libssl-dev \
    # desktop services & i18n
    dbus libdbus-1-dev libcups2-dev libglib2.0-dev libicu-dev \
    # audio/media
    libasound2-dev libpipewire-0.3-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    # accessibility
    at-spi2-core libatspi2.0-dev \
    # database client (MariaDB)
    libmariadb-dev libmariadb-dev-compat \
    libarchive-dev \
    libpq-dev \
    # editors & QoL
    emacs-nox; \
  rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Node.js 22 (Nodesource) — single layer
RUN set -eux; \
  apt-get update; apt-get install -y --no-install-recommends gpg; \
  rm -rf /var/lib/apt/lists/*; \
  curl -fsSL https://deb.nodesource.com/setup_22.x | bash -; \
  apt-get update; \
  apt-get install -y --no-install-recommends nodejs; \
  rm -rf /var/lib/apt/lists/*; \
  node -v && npm -v

# Qt build in ONE RUN (download → prune → build → install → strip → cleanup)
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

# Qwt build in ONE RUN (Qt6 qmake; lib only; no docs/examples/designer)
RUN set -eux; \
  QWT_URL="https://download.sourceforge.net/project/qwt/qwt/${qwt_version}/qwt-${qwt_version}.tar.bz2"; \
  echo "Downloading ${QWT_URL}"; \
  curl -fLo qwt-${qwt_version}.tar.bz2 "${QWT_URL}"; \
  tar -xf qwt-${qwt_version}.tar.bz2; \
  rm -f  qwt-${qwt_version}.tar.bz2; \
  cd qwt-${qwt_version}; \
  sed -i "/^unix {/,/^}/ s|QWT_INSTALL_PREFIX.*|QWT_INSTALL_PREFIX = /qwt-${qwt_version}|" qwtconfig.pri; \
  ${QTDIR}/bin/qmake; \
  make -j ${parallel_compile}; \
  make install; \
  # strip the built lib
  find "${QWTDIR}" -type f -name 'libqwt*.so*' -exec strip --strip-unneeded {} + || true; \
  cd /src; rm -rf "qwt-${qwt_version}"

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

# convenience repo—run, then remove its clone in the same layer
RUN set -eux; \
  git clone --depth 1 https://github.com/ehb54/convenience /tmp/convenience; \
  (cd /tmp/convenience && ./installcont.sh); \
  rm -rf /tmp/convenience

# Clone UltraScan (shallow). Keep .git for devs; drop if desired.
ARG ultrascan_branch
RUN set -eux; \
  git clone --branch "${ultrascan_branch}" https://github.com/ehb54/ultrascan3.git "${ULTRASCAN}"

# local.pri files
ADD local.pri.somo /ultrascan3/us_somo/develop/local.pri
ADD local.pri.gui  /ultrascan3/local.pri

# small QoL bits
RUN set -eux; \
  printf '%s\n' "export MAKEFLAGS='-j80'" > /root/qt6env; \
  git config --global core.pager ''

WORKDIR /ultrascan3

## somehow ubuntu is not honoring /etc/profile.d/qt.sh

ENV PATH="${QTDIR}/bin:${QTDIR}/libexec:${PATH}" 
#    LD_LIBRARY_PATH="${QTDIR}/lib:${LD_LIBRARY_PATH}" \
#    PKG_CONFIG_PATH="${QTDIR}/lib/pkgconfig:${PKG_CONFIG_PATH}" \
#    QT_PLUGIN_PATH="${QTDIR}/plugins:${QT_PLUGIN_PATH}" \
#    QML2_IMPORT_PATH="${QTDIR}/qml:${QML2_IMPORT_PATH}"


SHELL ["/bin/bash", "-lc"]
CMD ["/bin/bash", "-l"]
