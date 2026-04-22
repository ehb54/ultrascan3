#!/usr/bin/env bash
# =============================================================================
# bootstrap-linux.sh -- UltraScan3 Linux OS-level prerequisite installer
#
# PURPOSE
# -------
# Installs the system packages and tools that must exist on a machine BEFORE
# build.sh is run.  This script does NOT build UltraScan3, does NOT invoke
# vcpkg, and does NOT duplicate any logic that belongs in build.sh.
#
# vcpkg builds almost everything from source, but it still requires a set of
# system-level tools (compilers, autotools, ninja, pkg-config, GL/X11 headers,
# etc.) that are not part of the compiler toolchain itself.  This is exactly
# what bootstrap-linux.sh installs.
#
# USAGE
# -----
#   ./scripts/bootstrap-linux.sh [OPTIONS]
#
# OPTIONS
#   --hpc      Also install MPI (required for the HPC build profile)
#   --pkg      Enable Linux packaging prerequisite handling for --pkg builds
#   --dry-run  Print what would be installed without actually installing
#   --help     Show this message and exit
#
# SUPPORTED PLATFORMS
# -------------------
#   Ubuntu 22.04 LTS (Jammy)
#   Ubuntu 24.04 LTS (Noble)  <-- primary GitHub Actions runner target
#   Debian 12 (Bookworm)
#   Rocky Linux 8
#   AlmaLinux 8
#   Oracle Linux 8
#   RHEL 8
#
# macOS is intentionally out of scope: Homebrew and Xcode are developer
# choices that build.sh already handles (Xcode version switching, MPI hint).
#
# DESIGN PRINCIPLES
# -----------------
#   - Idempotent: safe to run on a machine where packages are already present
#   - Non-interactive by default in CI (CI=true), prompts locally
#   - Uses sudo only when not already root
#   - Exits clearly with a message when the platform is unsupported
# =============================================================================

set -euo pipefail

# =============================================================================
# ARGUMENT PARSING
# =============================================================================
INSTALL_HPC=false
INSTALL_PKG=false
DRY_RUN=false

for arg in "$@"; do
  case "$arg" in
    --hpc)      INSTALL_HPC=true  ;;
    --pkg)      INSTALL_PKG=true  ;;
    --dry-run)  DRY_RUN=true      ;;
    --help)
      sed -n '/^# USAGE/,/^# ====/p' "$0" | grep -v '^# ====' | sed 's/^# \{0,1\}//'
      exit 0
      ;;
    *)
      echo "ERROR: Unknown option: $arg"
      echo "Run '$0 --help' for usage information."
      exit 1
      ;;
  esac
done

# =============================================================================
# HELPERS
# =============================================================================
log()  { echo "[bootstrap] $*"; }
warn() { echo "[bootstrap] WARNING: $*" >&2; }
die()  { echo "[bootstrap] ERROR: $*" >&2; exit 1; }

# Use sudo only when not already root (CI Docker containers run as root)
SUDO=""
if [ "$(id -u)" != "0" ]; then
  SUDO="sudo"
fi

# Detect CI mode the same way build.sh does
NON_INTERACTIVE=false
if [ "${CI:-false}" = "true" ]; then
  NON_INTERACTIVE=true
fi

# =============================================================================
# PLATFORM DETECTION
# =============================================================================
log "Detecting platform..."

OS_ID=""
OS_VERSION_ID=""
OS_CODENAME=""

if [ -f /etc/os-release ]; then
  # shellcheck disable=SC1091
  . /etc/os-release
  OS_ID="${ID:-}"
  OS_VERSION_ID="${VERSION_ID:-}"
  OS_CODENAME="${VERSION_CODENAME:-}"
fi

# Determine distro family — all subsequent logic branches on DISTRO_FAMILY,
# not on OS_ID directly, so adding a new distro only requires updating here.
DISTRO_FAMILY=""
case "$OS_ID" in
  ubuntu|debian)
    DISTRO_FAMILY="debian"
    ;;
  rhel|centos|rocky|almalinux|ol)
    DISTRO_FAMILY="rhel"
    ;;
  "")
    die "Cannot determine OS. /etc/os-release not found or ID is missing."
    ;;
  *)
    die "Unsupported OS: '$OS_ID'. This script supports Ubuntu, Debian, Rocky Linux, AlmaLinux, and RHEL.
On macOS, build.sh handles Xcode selection and brew-based MPI hints directly."
    ;;
esac

log "Detected: ${OS_ID} ${OS_VERSION_ID} (${OS_CODENAME:-n/a}) [family: ${DISTRO_FAMILY}]"
log ""

# =============================================================================
# PRECHECK: BASE TOOLS ALREADY ON PATH
# Diagnostic only. Missing tools will be installed below.
# =============================================================================
REQUIRED_TOOLS=(cmake git g++)
MISSING_TOOLS=()

for tool in "${REQUIRED_TOOLS[@]}"; do
  if ! command -v "$tool" &>/dev/null; then
    MISSING_TOOLS+=("$tool")
  fi
done

if [ ${#MISSING_TOOLS[@]} -eq 0 ]; then
  log "Base Linux build tools already present on PATH."
else
  log "Base Linux build tools missing from PATH: ${MISSING_TOOLS[*]}"
  log "bootstrap will install the required packages below."
fi
log ""

# =============================================================================
# PACKAGE LISTS
# =============================================================================
# Each group is documented with WHY it is needed, not just what it is.
# Packages are defined per distro family so the comments stay close to the
# code.  The logical groups (TOOLCHAIN, AUTOTOOLS, etc.) are identical across
# families — only the package names differ.
#
# RHEL/Rocky/OL8 notes:
#   - ninja-build, patchelf, nasm, autoconf-archive are in EPEL (enabled below).
#   - On Oracle Linux 8, EPEL is enabled via oracle-epel-release-el8.
#   - On Oracle Linux 8, CRB is named ol8_codeready_builder.
#   - Many -devel packages live in the CRB/PowerTools repo (enabled below).
#   - The RHEL xcb packages are split differently from Debian but cover the
#     same XCB headers that vcpkg qtbase requires.
#   - openmpi-devel provides both the headers and the mpicxx wrapper binary.
#   - MPI module load is NOT required here because we install from dnf and
#     the wrappers land directly in /usr/bin.

if [ "$DISTRO_FAMILY" = "debian" ]; then

  # --- Core build toolchain -------------------------------------------------
  # cmake: project build system (requires >= 3.21; Ubuntu 24.04 ships 3.28)
  # ninja-build: the generator used by every CMake preset (see CMakePresets.json)
  # build-essential: gcc, g++, make, libc-dev — the baseline compiler suite
  # g++: explicit because vcpkg checks for it specifically on Linux
  # git: vcpkg clones itself and its ports from GitHub
  # pkg-config: used by vcpkg during port configure steps and by the CMake presets
  #             (PKG_CONFIG_PATH is set in the linux-base preset)
  PKGS_TOOLCHAIN=(
    cmake
    ninja-build
    build-essential
    g++
    git
    pkg-config
  )

  # --- Autotools chain ------------------------------------------------------
  # vcpkg builds several ports via ./configure (ICU, libmariadb internals, etc.).
  # Without the full autotools suite those configure scripts fail to run.
  # autoconf-archive: provides m4 macros used by some port configure.ac files
  # libtool: required by ports that build shared libraries via libtoolize
  # libltdl-dev: provides ltdl.h and m4/ltdl.m4 — a SEPARATE package from libtool.
  #              Required by ports whose configure.ac calls LT_LIB_DLLOAD or
  #              LT_INIT with dlopen support (e.g. libxcrypt): autoreconf -vfi
  #              fails with "possibly undefined macro: LT_LIB_DLLOAD" if this
  #              package is absent even when libtool itself is installed.
  PKGS_AUTOTOOLS=(
    autoconf
    autoconf-archive
    automake
    libtool
    libltdl-dev
  )

  # --- Archive and download utilities ---------------------------------------
  # curl: vcpkg uses curl to download port source archives
  # zip / unzip: vcpkg decompresses source archives during port builds
  # tar: used by vcpkg and by build.sh --pkg (Linux tarball packaging)
  PKGS_ARCHIVE=(
    curl
    zip
    unzip
    tar
  )

  # --- Binary patching ------------------------------------------------------
  # patchelf: vcpkg adjusts RPATH of installed .so files on Linux so that
  #           libraries are relocatable (sets $ORIGIN-relative RPATHs).
  #           Without it the "Adjusted RPATH of ..." steps in vcpkg output fail.
  PKGS_PATCHELF=(
    patchelf
  )

  # --- Assembly and code generation -----------------------------------------
  # nasm: vcpkg Qt and several image format ports (libjpeg-turbo, openssl)
  #       require NASM for optimised assembly routines
  # gperf: required by the Qt5/Qt6 qtbase vcpkg port for keyword hash tables
  # bison: required by libmariadb connector port (SQL parser grammar)
  # flex: required by libmariadb connector port (SQL lexer)
  # libtime-piece-perl: provides Time::Piece, required by the OpenSSL Configure
  #   script invoked during vcpkg's openssl port build.  It is a core Perl module
  #   but is split into a separate package on Ubuntu/Debian and is not installed
  #   by default on GitHub-hosted Ubuntu 24.04 runners.
  PKGS_CODEGEN=(
    nasm
    gperf
    bison
    flex
    libtime-piece-perl
  )

  # --- Python (Sphinx documentation) ----------------------------------------
  # python3 / pip: Sphinx is installed via pip from doc/manual/source/requirements.txt
  #                (build.sh handles the actual pip install; we just ensure python3
  #                and pip are available as prerequisites)
  PKGS_PYTHON=(
    python3
    python3-pip
  )

  # --- OpenGL / graphics headers --------------------------------------------
  # vcpkg Qt requires OpenGL and GLU headers at configure time even when
  # the final binary uses software rendering (e.g. offscreen in CI).
  # libgl-dev: provides GL/gl.h (Mesa implementation header)
  # libglu1-mesa-dev: provides GL/glu.h
  PKGS_OPENGL=(
    libgl-dev
    libglu1-mesa-dev
  )

  # --- X11 / display system headers -----------------------------------------
  PKGS_X11=(
    libx11-dev
    libx11-xcb-dev
    libxext-dev
    libxrender-dev
    libxi-dev
    libxrandr-dev
    libxcursor-dev
    libxinerama-dev
    libxkbcommon-dev
    libxkbcommon-x11-dev
    libsm-dev
    libice-dev
  )

  # --- XCB development headers ----------------------------------------------
  # vcpkg qtbase on Ubuntu requires the XCB development family from the system
  # package manager. The vcpkg port itself recommends:
  #   '^libxcb.*-dev' libx11-xcb-dev ...
  #
  # We install the core XCB family explicitly so local Ubuntu and GitHub Ubuntu
  # runners behave the same.
  PKGS_XCB=(
    libxcb1-dev
    libxcb-cursor-dev
    libxcb-glx0-dev
    libxcb-icccm4-dev
    libxcb-image0-dev
    libxcb-keysyms1-dev
    libxcb-randr0-dev
    libxcb-render-util0-dev
    libxcb-shape0-dev
    libxcb-shm0-dev
    libxcb-sync-dev
    libxcb-util-dev
    libxcb-xfixes0-dev
    libxcb-xinerama0-dev
    libxcb-xinput-dev
    libxcb-xkb-dev
  )

  # --- Font and graphics library headers ------------------------------------
  # libfontconfig1-dev: font configuration — required by Qt text rendering
  # libfreetype-dev: font rasterizer — required by Qt and several Qt modules
  # libharfbuzz-dev: text shaping — pulled in by Qt font stack
  PKGS_FONTS=(
    libfontconfig1-dev
    libfreetype-dev
    libharfbuzz-dev
  )

  # --- D-Bus ----------------------------------------------------------------
  # libdbus-1-dev: inter-process communication — required by Qt D-Bus module
  #               and the Qt xcb platform plugin
  PKGS_DBUS=(
    libdbus-1-dev
  )

  # --- EGL / Wayland (for Qt Wayland/EGL backend) ---------------------------
  # libegl-dev: EGL headers — required by Qt EGL and EGLFS plugins
  # libwayland-dev: Wayland protocol — required by Qt Wayland platform plugin
  # wayland-protocols: extra Wayland protocol definitions used during Qt build
  PKGS_EGL=(
    libegl1-mesa-dev
    libwayland-dev
    wayland-protocols
  )

  # --- Input / multimedia device headers ------------------------------------
  # libinput-dev: input device abstraction — required by Qt libinput plugin
  # libudev-dev: udev device enumeration — required by Qt udev plugin
  # libasound2-dev: ALSA audio — required by Qt Multimedia on Linux
  # libpulse-dev: PulseAudio — required by Qt Multimedia on Linux
  PKGS_INPUT=(
    libinput-dev
    libudev-dev
    libasound2-dev
    libpulse-dev
  )

  # --- MPI (optional, --hpc flag) -------------------------------------------
  # Required only for the HPC build profile (mpicxx must be on PATH).
  # Using openmpi as the default; mpich is an acceptable substitute.
  PKGS_HPC=(
    libopenmpi-dev
    openmpi-bin
  )

elif [ "$DISTRO_FAMILY" = "rhel" ]; then

  # --- Core build toolchain -------------------------------------------------
  # gcc-c++: the g++ equivalent on RHEL/Rocky — vcpkg checks for c++ compiler
  # gcc-toolset-13: provides GCC 13 on Rocky/RHEL 8 via SCL.
  #   Rocky 8's default GCC 8.5 is incompatible with Qt 6.10's C++17 noexcept
  #   correctness checks (static_assert failures in qcomparehelpers.h).
  #   GCC 13 matches the compiler used on Ubuntu 24.04 and builds Qt 6.10 cleanly.
  #   Activation: warm-cache.sh and build.sh source the toolset environment
  #   before invoking vcpkg.
  # ninja-build: from EPEL — the generator used by every CMake preset
  # cmake: RHEL 8 BaseOS ships cmake 3.26+ which satisfies our >= 3.21 requirement
  # git: vcpkg clones itself and its ports from GitHub
  # pkgconf-pkg-config: provides pkg-config on RHEL 8 (pkg-config is an alias)
  # make: required by some vcpkg port build systems that fall back from ninja
  PKGS_TOOLCHAIN=(
    cmake
    ninja-build
    gcc
    gcc-c++
    gcc-toolset-13
    make
    git
    pkgconf-pkg-config
  )

  # --- Autotools chain ------------------------------------------------------
  # Same logical role as on Debian. autoconf-archive is in EPEL on Rocky/RHEL
  # and also in Oracle Linux EPEL (oracle-epel-release-el8); it is available
  # on all supported EL8 variants once EPEL is enabled.
  # libtool-ltdl-devel: the RHEL equivalent of Debian's libltdl-dev —
  #   provides ltdl.h required by ports that call LT_LIB_DLLOAD in configure.ac
  PKGS_AUTOTOOLS=(
    autoconf
    autoconf-archive
    automake
    libtool
    libtool-ltdl-devel
  )

  # --- Archive and download utilities ---------------------------------------
  # Same role as on Debian. All available in BaseOS/AppStream.
  PKGS_ARCHIVE=(
    curl
    zip
    unzip
    tar
  )

  # --- Binary patching ------------------------------------------------------
  # patchelf: from EPEL — same role as on Debian (vcpkg RPATH adjustment)
  PKGS_PATCHELF=(
    patchelf
  )

  # --- Assembly and code generation -----------------------------------------
  # nasm: from EPEL on Rocky 8 — same role as on Debian
  # gperf: in AppStream — same role as on Debian
  # bison: in AppStream — same role as on Debian
  # flex: in AppStream — same role as on Debian
  # file: provides /usr/bin/file — used by autoconf-generated configure scripts
  #   to detect file types; absent from the minimal Rocky 8 container image
  #   and causes configure to fail with "No such file or directory".
  # perl-IPC-Cmd: required by vcpkg's openssl portfile (unix/portfile.cmake uses
  #   IPC::Cmd to invoke subprocesses during the OpenSSL Configure/build steps).
  #   Present on Ubuntu as part of perl-base but NOT installed by default on
  #   Rocky/RHEL 8 — its absence causes a hard BUILD_FAILED for openssl.
  # perl-Time-Piece: provides Time::Piece, required by the OpenSSL Configure
  #   script.  A core Perl module but split into a separate package on Rocky/RHEL
  #   8 and not installed by default — its absence causes BUILD_FAILED for openssl.
  # perl-open: provides the 'open' pragma used by vcpkg port build-aux scripts
  #   (e.g. expand-selected-hashes in libxcrypt).  Not installed by default in
  #   the Rocky 8 container — its absence causes "Can't locate open.pm" failures.
  PKGS_CODEGEN=(
    nasm
    gperf
    bison
    flex
    file
    perl-IPC-Cmd
    perl-Time-Piece
    perl-open
  )

  # --- Python (Sphinx documentation) ----------------------------------------
  # python3 / python3-pip: same role as on Debian.
  # python39 / python39-pip: Rocky/RHEL 8 ships Python 3.6 as the system default,
  #   which is too old for vcpkg's meson port (requires >= 3.7).  python39 installs
  #   as a parallel alternative; the post-install section below registers it via
  #   update-alternatives so that `python3` resolves to 3.9 for vcpkg and Sphinx.
  PKGS_PYTHON=(
    python3
    python3-pip
    python39
    python39-pip
  )

  # --- OpenGL / graphics headers --------------------------------------------
  # mesa-libGL-devel: provides GL/gl.h — RHEL equivalent of libgl-dev
  # mesa-libGLU-devel: provides GL/glu.h — RHEL equivalent of libglu1-mesa-dev
  # mesa-libEGL-devel: provides EGL/egl.h — RHEL equivalent of libegl1-mesa-dev
  # These are in CRB (CodeReady Builder / PowerTools) on Rocky 8.
  PKGS_OPENGL=(
    mesa-libGL-devel
    mesa-libGLU-devel
    mesa-libEGL-devel
  )

  # --- X11 / display system headers -----------------------------------------
  # libX11-devel: core X11 (Xlib.h)
  # libXext-devel: X11 extensions
  # libXrender-devel: XRender extension
  # libXi-devel: XInput extension
  # libXrandr-devel: RandR extension
  # libXcursor-devel: Xcursor
  # libXinerama-devel: Xinerama extension
  # libxkbcommon-devel: keyboard handling (in CRB on Rocky 8)
  # libxkbcommon-x11-devel: xkb-x11 integration (in CRB on Rocky 8)
  # libSM-devel: session management
  # libICE-devel: inter-client exchange
  # Note: there is no separate libx11-xcb package on RHEL — the xcb bridge
  # is included in libX11-devel on this platform.
  PKGS_X11=(
    libX11-devel
    libXext-devel
    libXrender-devel
    libXi-devel
    libXrandr-devel
    libXcursor-devel
    libXinerama-devel
    libxkbcommon-devel
    libxkbcommon-x11-devel
    libSM-devel
    libICE-devel
  )

# --- XCB development headers ----------------------------------------------
  # libxcb-devel: core XCB (xcb/xcb.h) — base protocol headers
  # xcb-util-devel: xcb-util helpers metapackage (CRB)
  # xcb-util-cursor-devel: xcb-cursor (CRB)
  # xcb-util-image-devel: xcb-image — required by Qt xcb syslibs test (CRB)
  # xcb-util-keysyms-devel: xcb-keysyms — required by Qt xcb syslibs test (CRB)
  # xcb-util-renderutil-devel: xcb-renderutil — required by Qt xcb syslibs test (CRB)
  # xcb-util-wm-devel: xcb-wm — required by Qt xcb syslibs test (CRB)
  # Qt 6.10 hard-errors if FEATURE_xcb_syslibs=ON but TEST_xcb_syslibs fails;
  # all packages above must be present for the test to pass.
 PKGS_XCB=(
     libxcb-devel
     xcb-util-devel
     xcb-util-cursor-devel
     xcb-util-image-devel
     xcb-util-keysyms-devel
     xcb-util-renderutil-devel
     xcb-util-wm-devel
   )

  # --- Font and graphics library headers ------------------------------------
  # fontconfig-devel: font configuration — RHEL equivalent of libfontconfig1-dev
  # freetype-devel: font rasterizer — RHEL equivalent of libfreetype-dev
  # harfbuzz-devel: text shaping — RHEL equivalent of libharfbuzz-dev (in CRB)
  PKGS_FONTS=(
    fontconfig-devel
    freetype-devel
    harfbuzz-devel
  )

  # --- D-Bus ----------------------------------------------------------------
  # dbus-devel: RHEL equivalent of libdbus-1-dev
  PKGS_DBUS=(
    dbus-devel
  )

  # --- EGL / Wayland (for Qt Wayland/EGL backend) ---------------------------
  # EGL headers are in mesa-libEGL-devel (already in PKGS_OPENGL above).
  # wayland-devel: RHEL equivalent of libwayland-dev (in CRB)
  # wayland-protocols-devel: RHEL equivalent of wayland-protocols (in CRB)
  PKGS_EGL=(
    wayland-devel
    wayland-protocols-devel
  )

  # --- Input / multimedia device headers ------------------------------------
  # libinput-devel: RHEL equivalent of libinput-dev
  #   NOTE: not available on Oracle Linux 8 (absent from BaseOS/AppStream/EPEL/CRB).
  #   Omitted on OL8; Qt builds without it by falling back to evdev input.
  # systemd-devel: provides libudev.h — RHEL equivalent of libudev-dev
  # alsa-lib-devel: ALSA audio — RHEL equivalent of libasound2-dev
  # pulseaudio-libs-devel: PulseAudio — RHEL equivalent of libpulse-dev
  PKGS_INPUT=()
  if [ "$OS_ID" != "ol" ]; then
    PKGS_INPUT+=(libinput-devel)
  fi
  PKGS_INPUT+=(
    systemd-devel
    alsa-lib-devel
    pulseaudio-libs-devel
  )

  # --- MPI (optional, --hpc flag) -------------------------------------------
  # openmpi-devel: provides mpicxx wrapper and MPI headers.
  #   On Rocky 8 the wrapper lands in /usr/lib64/openmpi/bin/mpicxx and
  #   /usr/lib64/openmpi/bin is NOT on PATH by default.  We add it explicitly
  #   in the post-install section when --hpc is requested.
  PKGS_HPC=(
    openmpi-devel
  )

fi  # end distro-family package list definitions

# =============================================================================
# BUILD PACKAGE LIST
# =============================================================================
ALL_PKGS=(
  "${PKGS_TOOLCHAIN[@]}"
  "${PKGS_AUTOTOOLS[@]}"
  "${PKGS_ARCHIVE[@]}"
  "${PKGS_PATCHELF[@]}"
  "${PKGS_CODEGEN[@]}"
  "${PKGS_PYTHON[@]}"
  "${PKGS_OPENGL[@]}"
  "${PKGS_X11[@]}"
  "${PKGS_XCB[@]}"
  "${PKGS_FONTS[@]}"
  "${PKGS_DBUS[@]}"
  "${PKGS_EGL[@]}"
  "${PKGS_INPUT[@]}"
)

if [ "$INSTALL_HPC" = true ]; then
  ALL_PKGS+=("${PKGS_HPC[@]}")
  log "HPC mode: MPI packages will be included."
fi

if [ "$INSTALL_PKG" = true ]; then
  log "Packaging mode: no extra Linux OS packages are currently required beyond the standard set."
fi

# =============================================================================
# DRY-RUN MODE
# =============================================================================
if [ "$DRY_RUN" = true ]; then
  log "Dry-run mode -- nothing will be installed."
  log ""
  log "Packages that would be installed:"
  for pkg in "${ALL_PKGS[@]}"; do
    echo "  $pkg"
  done
  log ""
  if [ "$DISTRO_FAMILY" = "debian" ]; then
    log "Commands that would run:"
    log "  ${SUDO:+$SUDO }apt-get update -qq"
    log "  ${SUDO:+$SUDO }apt-get install -y ${ALL_PKGS[*]}"
  elif [ "$DISTRO_FAMILY" = "rhel" ]; then
    log "Commands that would run:"
    log "  ${SUDO:+$SUDO }dnf install -y epel-release"
    log "  ${SUDO:+$SUDO }dnf config-manager --set-enabled crb  # or powertools on older Rocky 8"
    log "  ${SUDO:+$SUDO }dnf install -y ${ALL_PKGS[*]}"
    log "  update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 39"
  fi
  exit 0
fi

# =============================================================================
# FILTER: ONLY INSTALL WHAT IS MISSING
# =============================================================================
# Use the correct package query tool for each distro family so that dev/header
# packages (which have no binary) are checked correctly.
log "Checking which packages are already installed..."

PKGS_TO_INSTALL=()

if [ "$DISTRO_FAMILY" = "debian" ]; then
  for pkg in "${ALL_PKGS[@]}"; do
    if ! dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null | grep -q "install ok installed"; then
      PKGS_TO_INSTALL+=("$pkg")
    fi
  done
elif [ "$DISTRO_FAMILY" = "rhel" ]; then
  for pkg in "${ALL_PKGS[@]}"; do
    if ! rpm -q "$pkg" &>/dev/null; then
      PKGS_TO_INSTALL+=("$pkg")
    fi
  done
fi

if [ ${#PKGS_TO_INSTALL[@]} -eq 0 ]; then
  log "All required packages are already installed. Nothing to do."
  log ""
  log "Run 'bash scripts/build.sh --help' to build UltraScan3."
  exit 0
fi

log ""
log "Packages to install (${#PKGS_TO_INSTALL[@]}):"
for pkg in "${PKGS_TO_INSTALL[@]}"; do
  echo "  $pkg"
done
log ""

# =============================================================================
# CONFIRMATION (interactive mode only)
# =============================================================================
if [ "$NON_INTERACTIVE" = false ]; then
  read -rp "[bootstrap] Proceed with installation? [y/N] " answer
  if [[ ! "$answer" =~ ^[Yy]$ ]]; then
    log "Aborted. No packages were installed."
    exit 0
  fi
fi

# =============================================================================
# INSTALL
# =============================================================================
if [ "$DISTRO_FAMILY" = "debian" ]; then

  log "Updating apt package index..."
  if [ "$NON_INTERACTIVE" = true ]; then
    DEBIAN_FRONTEND=noninteractive ${SUDO:+$SUDO} apt-get update -qq
  else
    ${SUDO:+$SUDO} apt-get update -qq
  fi

  log "Installing packages..."
  if [ "$NON_INTERACTIVE" = true ]; then
    DEBIAN_FRONTEND=noninteractive ${SUDO:+$SUDO} apt-get install -y \
      --no-install-recommends \
      "${PKGS_TO_INSTALL[@]}"
  else
    ${SUDO:+$SUDO} apt-get install -y "${PKGS_TO_INSTALL[@]}"
  fi

elif [ "$DISTRO_FAMILY" = "rhel" ]; then

  # Enable EPEL — provides ninja-build, patchelf, nasm, autoconf-archive
  # On Oracle Linux 8 the EPEL release package has a different name.
  log "Enabling EPEL repository..."
  if [ "$OS_ID" = "ol" ]; then
    ${SUDO:+$SUDO} dnf install -y oracle-epel-release-el8
  else
    ${SUDO:+$SUDO} dnf install -y epel-release
  fi

  # Enable CRB (CodeReady Builder) — provides mesa-*-devel, xcb-util-*,
  # libxkbcommon-*-devel, harfbuzz-devel, wayland-*-devel, and others.
  # Repo name varies by distro and version:
  #   Rocky 8.5 and earlier:  powertools
  #   Rocky 8.6 and later:    crb
  #   Oracle Linux 8:         ol8_codeready_builder
  # Try each name in turn; warn but continue if none succeed.
  log "Enabling CRB/PowerTools repository..."
  CRB_ENABLED=false
  for crb_name in crb ol8_codeready_builder powertools; do
    if ${SUDO:+$SUDO} dnf config-manager --set-enabled "$crb_name" 2>/dev/null; then
      log "Enabled repo: $crb_name"
      CRB_ENABLED=true
      break
    fi
  done
  if [ "$CRB_ENABLED" = false ]; then
    warn "Could not enable CRB/PowerTools/ol8_codeready_builder repo. Some packages may not be available."
  fi

  log "Installing packages..."
  ${SUDO:+$SUDO} dnf install -y "${PKGS_TO_INSTALL[@]}"

fi

# =============================================================================
# POST-INSTALL: RHEL GCC TOOLSET 13 ACTIVATION
# =============================================================================
# gcc-toolset-13 installs GCC 13 under /opt/rh/gcc-toolset-13/root/usr/bin/.
# It is NOT on PATH by default — it must be activated via scl or by sourcing
# the enable script.  We write a profile.d snippet so that CC/CXX are set
# for all subsequent shells and subprocesses, and also activate it for the
# current shell session so warm-cache.sh and build.sh pick it up immediately.
if [ "$DISTRO_FAMILY" = "rhel" ] && [ -f /opt/rh/gcc-toolset-13/enable ]; then
  GCC13_PROFILE="/etc/profile.d/gcc-toolset-13.sh"
  if [ ! -f "$GCC13_PROFILE" ]; then
    log "Writing gcc-toolset-13 activation to ${GCC13_PROFILE}..."
    ${SUDO:+$SUDO} bash -c "cat > ${GCC13_PROFILE}" <<'EOF'
# Added by UltraScan3 bootstrap-linux.sh
# Activates GCC 13 from gcc-toolset-13 for all shells on Rocky/RHEL 8.
# Rocky 8's default GCC 8.5 cannot build Qt 6.10 (C++17 noexcept failures).
source /opt/rh/gcc-toolset-13/enable
EOF
  fi
  # Activate for the current shell session
  # shellcheck disable=SC1091
  source /opt/rh/gcc-toolset-13/enable
  log "GCC toolset 13 activated: $(g++ --version | head -1)"
fi

# =============================================================================
# POST-INSTALL: RHEL PYTHON 3.9 ALTERNATIVES
# =============================================================================
# Rocky/RHEL 8 ships Python 3.6 as the system default, which is too old for
# vcpkg's meson port (requires >= 3.7).  python39 installs as a parallel
# alternative at /usr/bin/python3.9.  We register it via update-alternatives
# with priority 100 (higher than any typical system alternative) and then
# explicitly set it as the current default so the change takes effect
# immediately in this shell session without relying on priority ordering.
if [ "$DISTRO_FAMILY" = "rhel" ] && [ -x /usr/bin/python3.9 ]; then
  log "Registering python3.9 as the default python3 via update-alternatives..."
  ${SUDO:+$SUDO} update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 100
  ${SUDO:+$SUDO} update-alternatives --set python3 /usr/bin/python3.9
  log "python3 now resolves to: $(python3 --version 2>&1) at $(command -v python3)"
fi

# =============================================================================
# POST-INSTALL: RHEL MPI PATH FIX
# =============================================================================
# On Rocky/RHEL, openmpi-devel installs mpicxx to /usr/lib64/openmpi/bin/
# which is NOT on PATH by default (unlike Debian where it lands in /usr/bin).
# Add a profile.d snippet so mpicxx is findable in both interactive and
# non-interactive (cron/CI) shells without requiring 'module load mpi'.
if [ "$DISTRO_FAMILY" = "rhel" ] && [ "$INSTALL_HPC" = true ]; then
  MPI_BIN="/usr/lib64/openmpi/bin"
  PROFILE_SNIPPET="/etc/profile.d/openmpi.sh"
  if [ -d "$MPI_BIN" ] && [ ! -f "$PROFILE_SNIPPET" ]; then
    log "Adding OpenMPI to PATH via ${PROFILE_SNIPPET}..."
    ${SUDO:+$SUDO} bash -c "cat > ${PROFILE_SNIPPET}" <<'EOF'
# Added by UltraScan3 bootstrap-linux.sh
# Makes mpicxx/mpirun available without 'module load mpi'
export PATH="/usr/lib64/openmpi/bin:$PATH"
export LD_LIBRARY_PATH="/usr/lib64/openmpi/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
EOF
    # Also export for the current shell session so the verification below works
    export PATH="${MPI_BIN}:$PATH"
    export LD_LIBRARY_PATH="/usr/lib64/openmpi/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    log "OpenMPI PATH configured. Future shells will pick this up automatically."
  elif [ -d "$MPI_BIN" ]; then
    # Snippet already exists — ensure current shell can still find mpicxx
    export PATH="${MPI_BIN}:$PATH"
  fi
fi

# =============================================================================
# POST-INSTALL VERIFICATION
# =============================================================================
log ""
log "Verifying key tools..."

VERIFY_TOOLS=(cmake ninja g++ git autoconf autoreconf aclocal automake libtoolize nasm gperf bison flex python3)
if [ "$INSTALL_HPC" = true ]; then
  VERIFY_TOOLS+=(mpicxx)
fi

MISSING_AFTER=()
for tool in "${VERIFY_TOOLS[@]}"; do
  if ! command -v "$tool" &>/dev/null; then
    MISSING_AFTER+=("$tool")
  fi
done

if [ ${#MISSING_AFTER[@]} -ne 0 ]; then
  warn "The following tools are still not on PATH after installation:"
  for tool in "${MISSING_AFTER[@]}"; do
    warn "  $tool"
  done
  warn "This may indicate a PATH issue or a package name mismatch on this distro."
  warn "Build may fail. Please investigate before running build.sh."
else
  log "All key tools verified on PATH."
fi

# =============================================================================
# DONE
# =============================================================================
log ""
log "Bootstrap complete."
log ""
log "Next steps:"
log "  1. Run 'bash scripts/build.sh --help' to see build options."
log "  2. First build: 'bash scripts/build.sh --qt5-qwt616'  (or --qt6)"
log "     (vcpkg will build all library dependencies; allow 30-45 minutes)"
