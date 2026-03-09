#!/usr/bin/env bash
# =============================================================================
# bootstrap-deps.sh -- UltraScan3 OS-level prerequisite installer
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
# what bootstrap-deps.sh installs.
#
# USAGE
# -----
#   ./scripts/bootstrap-deps.sh [OPTIONS]
#
# OPTIONS
#   --hpc      Also install MPI (required for the HPC build profile)
#   --dry-run  Print what would be installed without actually installing
#   --help     Show this message and exit
#
# SUPPORTED PLATFORMS
# -------------------
#   Ubuntu 22.04 LTS (Jammy)
#   Ubuntu 24.04 LTS (Noble)  <-- primary GitHub Actions runner target
#   Debian 12 (Bookworm)
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
DRY_RUN=false

for arg in "$@"; do
  case "$arg" in
    --hpc)      INSTALL_HPC=true  ;;
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

case "$OS_ID" in
  ubuntu|debian) ;;  # supported
  "")
    die "Cannot determine OS. /etc/os-release not found or ID is missing."
    ;;
  *)
    die "Unsupported OS: '$OS_ID'. This script supports Ubuntu and Debian only.
On macOS, build.sh handles Xcode selection and brew-based MPI hints directly.
On RHEL/Fedora, adapt the package names listed in this script for dnf."
    ;;
esac

log "Detected: ${OS_ID} ${OS_VERSION_ID} (${OS_CODENAME})"
log ""

# =============================================================================
# PACKAGE LISTS
# =============================================================================
# Each group is documented with WHY it is needed, not just what it is.
# Keep this as the canonical reference for the Ubuntu/Debian bootstrap surface.

# --- Core build toolchain ---------------------------------------------------
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

# --- Autotools chain --------------------------------------------------------
# vcpkg builds several ports via ./configure (ICU, libmariadb internals, etc.).
# Without the full autotools suite those configure scripts fail to run.
# autoconf-archive: provides m4 macros used by some port configure.ac files
# libtool: required by ports that build shared libraries via libtoolize
PKGS_AUTOTOOLS=(
  autoconf
  autoconf-archive
  automake
  libtool
)

# --- Archive and download utilities -----------------------------------------
# curl: vcpkg uses curl to download port source archives
# zip / unzip: vcpkg decompresses source archives during port builds
# tar: used by vcpkg and by build.sh --pkg (Linux tarball packaging)
PKGS_ARCHIVE=(
  curl
  zip
  unzip
  tar
)

# --- Binary patching --------------------------------------------------------
# patchelf: vcpkg adjusts RPATH of installed .so files on Linux so that
#           libraries are relocatable (sets $ORIGIN-relative RPATHs).
#           Without it the "Adjusted RPATH of ..." steps in vcpkg output fail.
PKGS_PATCHELF=(
  patchelf
)

# --- Assembly and code generation -------------------------------------------
# nasm: vcpkg Qt and several image format ports (libjpeg-turbo, openssl)
#       require NASM for optimised assembly routines
# gperf: required by the Qt5/Qt6 qtbase vcpkg port for keyword hash tables
# bison: required by libmariadb connector port (SQL parser grammar)
# flex: required by libmariadb connector port (SQL lexer)
PKGS_CODEGEN=(
  nasm
  gperf
  bison
  flex
)

# --- Python (Sphinx documentation) ------------------------------------------
# python3 / pip: Sphinx is installed via pip from doc/manual/source/requirements.txt
#                (build.sh handles the actual pip install; we just ensure python3
#                and pip are available as prerequisites)
PKGS_PYTHON=(
  python3
  python3-pip
)

# --- OpenGL / graphics headers ----------------------------------------------
# vcpkg Qt requires OpenGL and GLU headers at configure time even when
# the final binary uses software rendering (e.g. offscreen in CI).
# libgl-dev: provides GL/gl.h (Mesa implementation header)
# libglu1-mesa-dev: provides GL/glu.h
PKGS_OPENGL=(
  libgl-dev
  libglu1-mesa-dev
)

# --- X11 / display system headers -------------------------------------------
# Qt requires X11 headers to build the xcb platform plugin.
# libx11-dev: core X11 (Xlib.h) — direct dependency of Qt xcb plugin
# libxext-dev: X11 extensions, required by Qt XCB
# libxrender-dev: XRender extension, used by Qt font rendering
# libxi-dev: XInput extension, required by Qt input handling
# libxrandr-dev: RandR extension, used by Qt multi-monitor support
# libxcursor-dev: Xcursor, used by Qt cursor support
# libxinerama-dev: Xinerama extension, used by Qt screen geometry
# libxkbcommon-dev: keyboard handling — required by Qt xcb and Wayland
# libxkbcommon-x11-dev: xkb-x11 integration for Qt xcb
PKGS_X11=(
  libx11-dev
  libxext-dev
  libxrender-dev
  libxi-dev
  libxrandr-dev
  libxcursor-dev
  libxinerama-dev
  libxkbcommon-dev
  libxkbcommon-x11-dev
)

# --- Font and graphics library headers --------------------------------------
# libfontconfig1-dev: font configuration — required by Qt text rendering
# libfreetype-dev: font rasterizer — required by Qt and several Qt modules
# libharfbuzz-dev: text shaping — pulled in by Qt font stack
PKGS_FONTS=(
  libfontconfig1-dev
  libfreetype-dev
  libharfbuzz-dev
)

# --- D-Bus ------------------------------------------------------------------
# libdbus-1-dev: inter-process communication — required by Qt D-Bus module
#               and the Qt xcb platform plugin
PKGS_DBUS=(
  libdbus-1-dev
)

# --- EGL / Wayland (for Qt Wayland/EGL backend) ----------------------------
# libegl-dev: EGL headers — required by Qt EGL and EGLFS plugins
# libwayland-dev: Wayland protocol — required by Qt Wayland platform plugin
# wayland-protocols: extra Wayland protocol definitions used during Qt build
PKGS_EGL=(
  libegl-dev
  libwayland-dev
  wayland-protocols
)

# --- Input / multimedia device headers --------------------------------------
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

# --- MPI (optional, --hpc flag) ---------------------------------------------
# Required only for the HPC build profile (mpicxx must be on PATH for build.sh).
# Using openmpi as the default; mpich is an acceptable substitute.
PKGS_HPC=(
  libopenmpi-dev
  openmpi-bin
)

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
  "${PKGS_FONTS[@]}"
  "${PKGS_DBUS[@]}"
  "${PKGS_EGL[@]}"
  "${PKGS_INPUT[@]}"
)

if [ "$INSTALL_HPC" = true ]; then
  ALL_PKGS+=("${PKGS_HPC[@]}")
  log "HPC mode: MPI packages will be included."
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
  log "Command that would run:"
  log "  ${SUDO:+$SUDO }apt-get update -qq"
  log "  ${SUDO:+$SUDO }apt-get install -y ${ALL_PKGS[*]}"
  exit 0
fi

# =============================================================================
# FILTER: ONLY INSTALL WHAT IS MISSING
# =============================================================================
# Using dpkg-query rather than command -v so that dev/header packages
# (which have no binary) are checked correctly.
log "Checking which packages are already installed..."

PKGS_TO_INSTALL=()
for pkg in "${ALL_PKGS[@]}"; do
  if ! dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null | grep -q "install ok installed"; then
    PKGS_TO_INSTALL+=("$pkg")
  fi
done

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

# =============================================================================
# POST-INSTALL VERIFICATION
# =============================================================================
log ""
log "Verifying key tools..."

VERIFY_TOOLS=(cmake ninja g++ git autoconf automake nasm gperf bison flex python3)
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
