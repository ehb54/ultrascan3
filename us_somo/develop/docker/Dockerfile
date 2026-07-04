# =============================================================================
# Minimal Linux build ENVIRONMENT for the SOMO CMake/vcpkg build.
# =============================================================================
# This image is deliberately minimal: it does NOT build UltraScan and does NOT
# install the distro's Qt/qwt. vcpkg builds Qt5/Qt6 + qwt 6.3.0 + qwtplot3d from
# source (via the repo manifest) at configure time. We only install:
#   - a C++ toolchain + cmake + ninja + git
#   - vcpkg bootstrap prereqs (curl/zip/unzip/tar/pkg-config, autotools)
#   - the SYSTEM -dev libraries that vcpkg's Qt build links against (X11/xcb,
#     GL/EGL/mesa, fontconfig, dbus, glib) and qtmultimedia (alsa/pulse).
#
# The source tree is bind-mounted at run time (see build.sh), not COPYed in, so
# iteration is fast and the vcpkg binary cache (a mounted volume) persists.
#
# Build:  docker build -t somo-cmake-env us_somo/develop/docker
# Use:    us_somo/develop/docker/build.sh   (drives configure+build in a container)
# =============================================================================
ARG UBUNTU=22.04
FROM ubuntu:${UBUNTU}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential g++ cmake ninja-build git \
      curl zip unzip tar ca-certificates pkg-config \
      autoconf automake autoconf-archive libtool libtool-bin libltdl-dev \
      gettext autopoint perl bison flex gperf \
      python3 python3-venv python3-pip \
      libgl1-mesa-dev libglu1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev \
      libx11-dev libx11-xcb-dev libxext-dev libxrender-dev libxi-dev \
      libxrandr-dev libxfixes-dev libxdamage-dev libxcomposite-dev \
      libxcursor-dev libxtst-dev libxkbcommon-dev libxkbcommon-x11-dev \
      libxcb1-dev libxcb-glx0-dev libxcb-keysyms1-dev libxcb-image0-dev \
      libxcb-shm0-dev libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev \
      libxcb-shape0-dev libxcb-randr0-dev libxcb-render-util0-dev \
      libxcb-util-dev libxcb-xinerama0-dev libxcb-xkb-dev libxcb-cursor-dev \
      libxcb-xinput-dev libsm-dev libice-dev \
      libfontconfig1-dev libfreetype6-dev libdbus-1-dev libglib2.0-dev \
      libasound2-dev libpulse-dev \
    && rm -rf /var/lib/apt/lists/*

# vcpkg — full clone so the manifest's pinned builtin-baseline resolves.
ENV VCPKG_ROOT=/opt/vcpkg
RUN git clone https://github.com/microsoft/vcpkg "$VCPKG_ROOT" \
    && "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
ENV PATH="$VCPKG_ROOT:$PATH"

# git runs as root over a bind-mounted, differently-owned tree
RUN git config --global --add safe.directory '*'

WORKDIR /src
