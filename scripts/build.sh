#!/usr/bin/env bash
set -euo pipefail

echo "=========================================="
echo "UltraScan3 Build Setup"
echo "=========================================="
echo "NOTE: First build takes 30-45 minutes."
echo "      Dependencies are cached after that."
echo ""

# Detect if running in CI
if [ "${CI:-false}" = "true" ]; then
  echo "Running in CI environment"
  NON_INTERACTIVE=true
else
  NON_INTERACTIVE=false
fi

# =============================================================================
# PARSE COMMAND LINE OPTIONS
# =============================================================================
REBUILD=false          # --rebuild: wipe build dir only (tier 1)
CLEAN=false            # --clean:   wipe build dir + vcpkg installed/ for triplet (tier 2)
PURGE_CACHE=false      # --purge-cache: additive to --clean, also wipes binary cache (tier 3)
BUILD_PKG=false        # --pkg: build platform-native package
PROFILE="APP"          # default profile
QT_VARIANT="qt6"       # qt6 | qt5-qwt616 | qt5-qwt630
ARCH=""
US3_VCPKG_ROOT="${US3_VCPKG_ROOT:-}"

# Parse options
while [[ $# -gt 0 ]]; do
  case $1 in
    --rebuild)      REBUILD=true;               shift ;;
    --clean)        CLEAN=true;                 shift ;;
    --purge-cache)  PURGE_CACHE=true;            shift ;;
    --qt6)          QT_VARIANT="qt6";           shift ;;
    --qt5-qwt616)   QT_VARIANT="qt5-qwt616";   shift ;;
    --qt5-qwt630)   QT_VARIANT="qt5-qwt630";   shift ;;
    --arch)
      ARCH="$2"; shift 2
      if [[ "$ARCH" != "x64" && "$ARCH" != "arm64" ]]; then
        echo "ERROR: --arch must be x64 or arm64"
        exit 1
      fi
      ;;
    --vcpkg-root)
      US3_VCPKG_ROOT="$2"; shift 2
      ;;
    --pkg)
      BUILD_PKG=true
      shift
      ;;
    --help)
      echo "Usage: $0 [OPTIONS] [PROFILE]"
      echo ""
      echo "OPTIONS:"
      echo "  --rebuild            Tier 1: removes the CMake build directory only."
      echo "                         Fast - vcpkg packages are untouched and restore"
      echo "                         from binary cache. Use when the build tree is"
      echo "                         corrupted or you want a clean UltraScan recompile."
      echo "  --clean              Tier 2: removes build dir + vcpkg buildtrees + the"
      echo "                         installed packages for the active triplet."
      echo "                         Forces vcpkg to reinstall all dependencies."
      echo "                         Required after vcpkg.json feature changes."
      echo "  --clean --purge-cache Tier 3: same as --clean plus wipes the binary cache"
      echo "                         (~/.vcpkg-cache). Forces full recompile from source."
      echo "                         Use when switching compilers or suspecting cache"
      echo "                         corruption."
      echo "  --pkg                Build platform-native package:"
      echo "                         macOS   -> PKG installer (installs to /Applications/UltraScan3)"
      echo "                                    Output: build/<preset>/UltraScan3-<version>-macOS.pkg"
      echo "                         Linux   -> portable tar.gz archive"
      echo "                                    Output: build/<preset>/UltraScan3-<version>-Linux-<arch>.tar.gz"
      echo "                         Windows -> NSIS installer"
      echo "                                    Output: build/<preset>/UltraScan3-<version>-Windows.exe"
      echo "  --qt6                Build with Qt6 + Qwt6.3.0 [default on macOS]"
      echo "  --qt5-qwt616         Build with Qt5 + Qwt6.1.6 [default on Linux/Windows]"
      echo "  --qt5-qwt630         Build with Qt5 + Qwt6.3.0"
      echo "  --arch x64           Target x64 architecture [default: auto-detect]"
      echo "  --arch arm64         Target ARM64 architecture"
      echo "  --vcpkg-root <path>  Path to vcpkg installation"
      echo "  --help               Show this help message"
      echo ""
      echo "PROFILE:"
      echo "  APP           Desktop/user build (GUI + programs + DB) [default]"
      echo "  TEST          Dev/CI build (programs + tests, prefer static libs)"
      echo "  HPC           Headless / no DB / no GUI"
      echo ""
      echo "VCPKG LOCATION (in order of priority):"
      echo "  1. --vcpkg-root <path> argument"
      echo "  2. US3_VCPKG_ROOT environment variable"
      echo "  3. vcpkg/ directory inside the source tree"
      echo "  4. \$HOME/vcpkg (default)"
      echo ""
      echo "EXAMPLES:"
      echo "  $0                        # Build only"
      echo "  $0 TEST                   # Build with TEST profile"
      echo "  $0 --qt5-qwt616           # Build Qt5+Qwt6.1.6"
      echo "  $0 --rebuild              # Wipe build dir, rebuild UltraScan only"
      echo "  $0 --clean                # Full dep reinstall (after vcpkg.json changes)"
      echo "  $0 --clean --purge-cache  # Nuke everything, recompile deps from source"
      echo "  $0 --clean TEST           # Clean build with TEST profile"
      echo "  $0 --pkg                  # build + platform-native package"
      echo "  $0 --clean --pkg          # clean build + platform-native package"
      echo ""
      echo "ENVIRONMENT VARIABLES:"
      echo "  US3_BUILD_JOBS      Override number of parallel build jobs"
      echo "  US3_VCPKG_ROOT      Override vcpkg location (default: \$HOME/vcpkg)"
      echo "  US3_VCPKG_CACHE     Override binary cache path (default: \$HOME/.vcpkg-cache)"
      echo "  US3_VCPKG_DOWNLOADS Override downloads cache path"
      echo "  US3_SCRATCH_ROOT    Override Linux CI scratch root"
      exit 0
      ;;
    [Aa][Pp][Pp]|[Tt][Ee][Ss][Tt]|[Hh][Pp][Cc])
      PROFILE="$(echo "$1" | tr '[:lower:]' '[:upper:]')"
      shift
      ;;
    *)
      echo "ERROR: Unknown option: $1"
      echo "Run '$0 --help' for usage information"
      exit 1
      ;;
  esac
done

# =============================================================================
# PLATFORM DETECTION
# =============================================================================
if [[ "$OSTYPE" == "darwin"* ]]; then
  PLATFORM="macOS"
  PLATFORM_PREFIX="macos"
elif [[ "$OSTYPE" == "linux-gnu"* || "$(uname -s)" == "Linux" ]]; then
  PLATFORM="Linux"
  PLATFORM_PREFIX="linux"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
  PLATFORM="Windows"
  PLATFORM_PREFIX="windows"
else
  _UNAME=$(uname -s 2>/dev/null || echo "unknown")
  echo "ERROR: Unsupported platform: OSTYPE=${OSTYPE:-unset}, uname=${_UNAME}"
  exit 1
fi

QT_VERSION_LABEL=""
case "$QT_VARIANT" in
  qt6)         QT_VERSION_LABEL="Qt6 (Qwt 6.3.0)" ;;
  qt5-qwt616)  QT_VERSION_LABEL="Qt5 (Qwt 6.1.6)" ;;
  qt5-qwt630)  QT_VERSION_LABEL="Qt5 (Qwt 6.3.0)" ;;
esac

echo "Selected build profile : ${PROFILE}"
echo "Selected Qt variant    : ${QT_VERSION_LABEL}"
if [ "$REBUILD" = true ]; then
  echo "Rebuild requested      : --rebuild (build dir only)"
fi
if [ "$CLEAN" = true ]; then
  echo "Clean requested        : --clean (build dir + vcpkg installed/ for triplet)"
fi
if [ "$PURGE_CACHE" = true ]; then
  echo "Purge cache requested  : --purge-cache (binary cache will also be wiped)"
fi
if [ "$BUILD_PKG" = true ]; then
  echo "Installer requested    : --pkg"
fi
echo ""

# =============================================================================
# ARCHITECTURE DETECTION
# =============================================================================
if [ -z "$ARCH" ]; then
  MACHINE=$(uname -m)
  if [ "$MACHINE" = "arm64" ] || [ "$MACHINE" = "aarch64" ]; then
    ARCH="arm64"
  else
    ARCH="x64"
  fi
  echo "Auto-detected architecture: $ARCH"
else
  echo "Architecture (specified)  : $ARCH"
fi

# =============================================================================
# DERIVE CONFIGURE AND BUILD PRESET NAMES
# =============================================================================
ARM64_SUFFIX=""
X64_SUFFIX=""
if [[ "$PLATFORM" == "Windows" && "$ARCH" == "arm64" ]]; then
  ARM64_SUFFIX="-arm64"
fi
if [[ "$PLATFORM" == "macOS" && "$ARCH" == "x64" ]]; then
  X64_SUFFIX="-x64"
fi

# HPC on Linux and macOS uses dedicated presets (separate binary dir, no GUI)
if [[ "$PROFILE" == "HPC" && ( "$PLATFORM" == "Linux" || "$PLATFORM" == "macOS" ) ]]; then
  CONFIGURE_PRESET="${PLATFORM_PREFIX}-hpc-release-${QT_VARIANT}${X64_SUFFIX}"
else
  CONFIGURE_PRESET="${PLATFORM_PREFIX}-release-${QT_VARIANT}${ARM64_SUFFIX}${X64_SUFFIX}"
fi
BUILD_PRESET="build-${CONFIGURE_PRESET}"

echo "Platform               : $PLATFORM ($ARCH)"
echo "Configure preset       : $CONFIGURE_PRESET"
echo "Build preset           : $BUILD_PRESET"
echo ""

# =============================================================================
# DETERMINE BUILD PARALLELISM
# =============================================================================
if [ "$PLATFORM" = "macOS" ]; then
  CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
elif [ "$PLATFORM" = "Linux" ]; then
  CORES=$(nproc 2>/dev/null || echo 4)
else
  CORES=${NUMBER_OF_PROCESSORS:-4}
fi

if [ -n "${US3_BUILD_JOBS:-}" ]; then
  # Explicit override always wins
  BUILD_JOBS="$US3_BUILD_JOBS"
elif [ "${CI:-false}" = "true" ]; then
  # Linux Qt6 builds on GitHub runners can run out of disk at higher parallelism
  if [ "$PLATFORM" = "Linux" ] && [ "$QT_VARIANT" = "qt6" ]; then
    BUILD_JOBS=2
  else
    BUILD_JOBS="$CORES"
  fi
else
  # Local builds: leave ~10% headroom to keep the machine usable
  BUILD_JOBS=$((CORES * 9 / 10))
  if [ "$BUILD_JOBS" -lt 1 ]; then
    BUILD_JOBS=1
  fi
fi

echo "Detected $CORES cores; using $BUILD_JOBS parallel build jobs."
echo ""

export VCPKG_MAX_CONCURRENCY="$BUILD_JOBS"

# =============================================================================
# SCRIPT_DIR / SOURCE_DIR
# Defined early so bootstrap scripts can be located relative to this script.
# =============================================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# =============================================================================
# LINUX CI SCRATCH / DISK MANAGEMENT
# On GitHub-hosted Ubuntu runners, root disk space is tight. Put large mutable
# build state on scratch storage in CI when possible.
# =============================================================================
US3_SCRATCH_ROOT="${US3_SCRATCH_ROOT:-}"

if [ "$PLATFORM" = "Linux" ]; then
  echo "=========================================="
  echo "Linux disk preflight"
  echo "=========================================="
  df -h

  if [ "${CI:-false}" = "true" ]; then
    echo "Freeing large preinstalled tool stacks not needed for UltraScan..."
    sudo rm -rf /usr/share/dotnet || true
    sudo rm -rf /opt/ghc || true
    sudo rm -rf /usr/local/lib/android || true
    sudo rm -rf /opt/hostedtoolcache/CodeQL || true
    echo ""
    echo "Disk after cleanup:"
    df -h

    # In CI, prefer /mnt only if it is actually a different filesystem.
    if [ -z "$US3_SCRATCH_ROOT" ]; then
      if [ -d /mnt ] && [ "$(stat -c '%d' / 2>/dev/null || echo x)" != "$(stat -c '%d' /mnt 2>/dev/null || echo x)" ]; then
        US3_SCRATCH_ROOT="/mnt/us3"
      else
        US3_SCRATCH_ROOT="$HOME/us3-scratch"
      fi
    fi
  fi

  # Only create scratch directories when a scratch root is actually in use.
  if [ -n "$US3_SCRATCH_ROOT" ]; then
    mkdir -p "$US3_SCRATCH_ROOT"/{vcpkg-cache,vcpkg-downloads,build}
    echo "Using Linux scratch root: $US3_SCRATCH_ROOT"
  else
    echo "Using standard Linux home-directory paths."
  fi
  echo ""
fi

# =============================================================================
# CHECK REQUIRED TOOLS
# =============================================================================
REQUIRED_TOOLS=(cmake git)

if [[ "$PLATFORM" == "macOS" ]]; then
  REQUIRED_TOOLS+=(xcodebuild xcrun)
elif [[ "$PLATFORM" == "Linux" ]]; then
  REQUIRED_TOOLS+=(g++)
fi

if [ "$BUILD_PKG" = true ] && [ "$PLATFORM" = "macOS" ] && [ "$PROFILE" != "HPC" ]; then
  REQUIRED_TOOLS+=(pkgbuild productbuild rsync)
fi

# HPC profile requires an MPI implementation
if [ "$PROFILE" = "HPC" ]; then
  REQUIRED_TOOLS+=(mpicxx)
fi

MISSING_TOOLS=()
for tool in "${REQUIRED_TOOLS[@]}"; do
  if ! command -v "$tool" &>/dev/null; then
    MISSING_TOOLS+=("$tool")
  fi
done

if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
  echo "ERROR: Missing required tools: ${MISSING_TOOLS[*]}"
  echo ""
  # Check if MPI is among the missing tools and give a targeted hint
  for _t in "${MISSING_TOOLS[@]}"; do
    if [ "$_t" = "mpicxx" ]; then
      echo "MPI is required for the HPC profile."
      if [ "$PLATFORM" == "macOS" ]; then
        echo "  Install Open MPI via Homebrew:"
        echo "    brew install open-mpi"
        echo ""
        echo "  NOTE: Open MPI's Homebrew bottle requires the Xcode Command Line Tools"
        echo "  installed at /Library/Developer/CommandLineTools (separate from Xcode.app)."
        echo "  If 'brew install open-mpi' fails with a GCC/CLT error, run first:"
        echo "    xcode-select --install"
        echo "  then re-run brew install open-mpi."
      elif [ "$PLATFORM" == "Linux" ]; then
        echo "  Install Open MPI (Debian/Ubuntu):"
        echo "    sudo apt-get install -y libopenmpi-dev openmpi-bin"
        echo "  Or MPICH:"
        echo "    sudo apt-get install -y libmpich-dev mpich"
        echo "  On HPC clusters, load the site MPI module instead:"
        echo "    module load openmpi  (or: module load mpich)"
      fi
      echo ""
    fi
  done
  if [ "$PLATFORM" == "macOS" ]; then
    # Generic CLT hint only when MPI wasn't already the issue (it includes its own CLT note)
    _mpi_missing=false
    for _t2 in "${MISSING_TOOLS[@]}"; do [ "$_t2" = "mpicxx" ] && _mpi_missing=true; done
    if [ "$_mpi_missing" = false ]; then
      echo "On macOS, install Xcode command line tools:"
      echo "  xcode-select --install"
    fi
  elif [ "$PLATFORM" == "Linux" ]; then
    echo "On Debian/Ubuntu, run:"
    echo "  sudo apt-get update && sudo apt-get install -y build-essential cmake git"
  elif [ "$PLATFORM" == "Windows" ]; then
    echo "On Windows (MSYS/MinGW), install cmake, git, and a compiler toolchain."
  fi
  exit 1
fi

echo "All required tools are available."
echo ""

# =============================================================================
# macOS BOOTSTRAP
# Delegates to bootstrap-macos.sh, which is the single authoritative source
# for Homebrew-level prerequisites (cmake, ninja, nasm, pkg-config, python3,
# and optionally open-mpi for HPC).  It is idempotent: exits immediately
# with no side effects when all packages are already installed.
# The HPC profile requires MPI, so --hpc is passed in that case.
# =============================================================================
if [ "$PLATFORM" = "macOS" ]; then
  _BOOTSTRAP="${SCRIPT_DIR}/bootstrap-macos.sh"
  if [ ! -f "$_BOOTSTRAP" ]; then
    echo "ERROR: bootstrap-macos.sh not found at $_BOOTSTRAP"
    echo "Please ensure scripts/bootstrap-macos.sh exists in the repository."
    exit 1
  fi

  _BOOTSTRAP_ARGS=()

  if [ "$PROFILE" = "HPC" ]; then
    _BOOTSTRAP_ARGS+=("--hpc")
  fi

  if [ "${#_BOOTSTRAP_ARGS[@]}" -gt 0 ]; then
    bash "$_BOOTSTRAP" "${_BOOTSTRAP_ARGS[@]}"
  else
    bash "$_BOOTSTRAP"
  fi
  echo ""
fi

# =============================================================================
# XCODE 15/16 SETUP ON macOS
# Runs after bootstrap so Homebrew tools are already on PATH.
# bootstrap-macos.sh verified that the CLT exists; here we select the right
# Xcode.app version for the build (requires sudo xcode-select --switch).
# =============================================================================
if [ "$PLATFORM" = "macOS" ]; then
  CURRENT_XCODE_PATH=$(xcode-select -p || echo "")
  XCODE_DEFAULT_PATH="/Applications/Xcode.app/Contents/Developer"

  echo "Checking Xcode configuration..."
  echo "Current Xcode path: ${CURRENT_XCODE_PATH:-<not set>}"
  echo ""

  DESIRED_XCODE_PATH=""
  XCODE_CANDIDATES=()
  for app in /Applications/Xcode-15*.app /Applications/Xcode-16*.app; do
    [ -d "$app" ] && XCODE_CANDIDATES+=("$app/Contents/Developer")
  done
  XCODE_CANDIDATES+=("$XCODE_DEFAULT_PATH")

  for XCODE_CANDIDATE in "${XCODE_CANDIDATES[@]}"; do
    if [ -d "$XCODE_CANDIDATE" ]; then
      XCODE_APP=$(dirname "$(dirname "$XCODE_CANDIDATE")")
      XCODE_PLIST="$XCODE_APP/Contents/Info.plist"
      if [ -f "$XCODE_PLIST" ]; then
        XCODE_VERSION=$(/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "$XCODE_PLIST" 2>/dev/null || echo "0")
        XCODE_MAJOR=$(echo "$XCODE_VERSION" | cut -d. -f1)
        if [ "$XCODE_MAJOR" -ge 15 ] 2>/dev/null; then
          DESIRED_XCODE_PATH="$XCODE_CANDIDATE"
          echo "Found Xcode $XCODE_VERSION at: $XCODE_CANDIDATE"
          break
        fi
      fi
    fi
  done

  if [ -n "$DESIRED_XCODE_PATH" ]; then
    echo "Compatible Xcode path: $DESIRED_XCODE_PATH"
    if [ "$CURRENT_XCODE_PATH" != "$DESIRED_XCODE_PATH" ]; then
      echo "Xcode 15/16 is installed but not active."
      if [ "$NON_INTERACTIVE" = false ]; then
        echo "About to switch Xcode to:"
        echo "  $DESIRED_XCODE_PATH"
        echo ""
        read -rp "Proceed with 'sudo xcode-select --switch ...'? [y/N] " answer
        if [[ "$answer" =~ ^[Yy]$ ]]; then
          echo "Switching Xcode..."
          sudo xcode-select --switch "$DESIRED_XCODE_PATH"
          echo "Xcode is now set to: $(xcode-select -p)"
        else
          echo "Skipping Xcode switch. Continuing with current Xcode."
        fi
      else
        echo "Non-interactive mode: switching Xcode automatically..."
        sudo xcode-select --switch "$DESIRED_XCODE_PATH"
        echo "Xcode is now set to: $(xcode-select -p)"
      fi
    else
      echo "Compatible Xcode is already active."
    fi
  else
    echo "Xcode 15 or 16 not found. Checked:"
    echo "  /Applications/Xcode-15*.app"
    echo "  /Applications/Xcode-16*.app"
    echo "  $XCODE_DEFAULT_PATH"
    echo ""
    echo "Please install Xcode 15 or 16 from the App Store or developer.apple.com."
    echo ""
    if [ "$NON_INTERACTIVE" = false ]; then
      exit 1
    else
      echo "WARNING: Continuing in CI mode, build may fail..."
    fi
  fi

  echo ""
fi

# =============================================================================
# VCPKG SETUP
# Priority: --vcpkg-root arg > US3_VCPKG_ROOT env > source-tree vcpkg > ~/vcpkg
# =============================================================================
if [ -n "$US3_VCPKG_ROOT" ]; then
  echo "Using vcpkg from --vcpkg-root argument: $US3_VCPKG_ROOT"
elif [ -n "${US3_VCPKG_ROOT:-}" ]; then
  US3_VCPKG_ROOT="${US3_VCPKG_ROOT}"
  echo "Using vcpkg from US3_VCPKG_ROOT environment variable: $US3_VCPKG_ROOT"
elif [ -f "${SOURCE_DIR}/vcpkg/bootstrap-vcpkg.sh" ]; then
  US3_VCPKG_ROOT="${SOURCE_DIR}/vcpkg"
  echo "Using vcpkg from source tree: $US3_VCPKG_ROOT"
else
  if [ "$PLATFORM" = "Linux" ] && [ "${CI:-false}" = "true" ] && [ -n "${US3_SCRATCH_ROOT:-}" ]; then
    US3_VCPKG_ROOT="$US3_SCRATCH_ROOT/vcpkg"
  else
    US3_VCPKG_ROOT="$HOME/vcpkg"
  fi
  echo "Using vcpkg from default location: $US3_VCPKG_ROOT"
fi

echo ""

if [ -d "$US3_VCPKG_ROOT" ] && [ ! -d "$US3_VCPKG_ROOT/.git" ]; then
  echo "ERROR: $US3_VCPKG_ROOT exists but is not a vcpkg git clone."
  echo "Set --vcpkg-root or US3_VCPKG_ROOT to a valid vcpkg path."
  exit 1
fi

if [ ! -d "$US3_VCPKG_ROOT/.git" ]; then
  echo "vcpkg not found at $US3_VCPKG_ROOT, cloning..."
  git clone https://github.com/microsoft/vcpkg.git "$US3_VCPKG_ROOT"
fi

if [ ! -x "$US3_VCPKG_ROOT/vcpkg" ]; then
  echo ""
  echo "Bootstrapping vcpkg at $US3_VCPKG_ROOT..."
  if [[ "$PLATFORM" == "Windows" ]]; then
    ( cd "$US3_VCPKG_ROOT" && ./bootstrap-vcpkg.bat -disableMetrics )
  else
    # -disableMetrics suppresses the telemetry consent prompt which can
    # stall non-interactive CI environments on first bootstrap.
    "$US3_VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
  fi
fi

export VCPKG_ROOT="$US3_VCPKG_ROOT"
export VCPKG_INSTALLED_DIR="$US3_VCPKG_ROOT/installed"

# Allow explicit override of the binary cache location via env var.
# In Linux CI, prefer the scratch root when available to reduce pressure on
# the runner's root filesystem. Otherwise default to ~/.vcpkg-cache.
if [ -n "${US3_VCPKG_CACHE:-}" ]; then
  US3_VCPKG_CACHE="$US3_VCPKG_CACHE"
elif [ "$PLATFORM" = "Linux" ] && [ "${CI:-false}" = "true" ] && [ -n "${US3_SCRATCH_ROOT:-}" ]; then
  US3_VCPKG_CACHE="$US3_SCRATCH_ROOT/vcpkg-cache"
else
  US3_VCPKG_CACHE="$HOME/.vcpkg-cache"
fi

mkdir -p "$US3_VCPKG_CACHE"
export VCPKG_BINARY_SOURCES="clear;files,$US3_VCPKG_CACHE,readwrite"

if [ -n "${US3_VCPKG_DOWNLOADS:-}" ]; then
  US3_VCPKG_DOWNLOADS="$US3_VCPKG_DOWNLOADS"
elif [ "$PLATFORM" = "Linux" ] && [ "${CI:-false}" = "true" ] && [ -n "${US3_SCRATCH_ROOT:-}" ]; then
  US3_VCPKG_DOWNLOADS="$US3_SCRATCH_ROOT/vcpkg-downloads"
else
  US3_VCPKG_DOWNLOADS="$HOME/vcpkg-downloads"
fi

mkdir -p "$US3_VCPKG_DOWNLOADS"
export US3_VCPKG_DOWNLOADS

VCPKG_TOOLCHAIN_FILE="$US3_VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "$VCPKG_TOOLCHAIN_FILE" ]; then
  echo "ERROR: vcpkg toolchain file not found at $VCPKG_TOOLCHAIN_FILE"
  exit 1
fi

echo "vcpkg ready."
echo ""

# =============================================================================
# CLEAN / REBUILD (tiered)
#
#  --rebuild       Tier 1 : build dir only
#  --clean         Tier 2 : build dir + vcpkg buildtrees + installed/ for triplet
#  --purge-cache   Tier 3 : additive to --clean; also wipes ~/.vcpkg-cache
# =============================================================================

# Helper: derive the vcpkg triplet for the current platform/arch combination.
# Must match what the CMake presets pass as VCPKG_TARGET_TRIPLET.
_derive_triplet() {
  # These names must exactly match what toolchain.cmake sets as VCPKG_TARGET_TRIPLET.
  # Linux arm64 uses a custom static triplet; Linux x64 uses vcpkg's built-in dynamic one.
  if [ "$PLATFORM" = "Linux" ]; then
    [ "$ARCH" = "arm64" ] && echo "arm64-linux" || echo "x64-linux-dynamic"
  elif [ "$PLATFORM" = "macOS" ]; then
    [ "$ARCH" = "arm64" ] && echo "arm64-osx-dynamic" || echo "x64-osx-dynamic"
  else
    echo ""
  fi
}

_remove_build_dir() {
  local build_dir="build/$CONFIGURE_PRESET"
  if [ -d "$build_dir" ]; then
    echo "Removing build directory: $build_dir"
    rm -rf "$build_dir"
  else
    echo "Build directory does not exist: $build_dir"
  fi
}

_remove_vcpkg_triplet() {
  local triplet
  triplet=$(_derive_triplet)
  if [ -z "$triplet" ]; then
    echo "(no triplet derived for $PLATFORM -- skipping vcpkg installed/ removal)"
    return
  fi

  if [ -d "$US3_VCPKG_ROOT/buildtrees" ]; then
    echo "Removing vcpkg buildtrees..."
    rm -rf "$US3_VCPKG_ROOT/buildtrees"
  fi

  # Always wipe the vcpkg bookkeeping directory (status file, .list files,
  # pending updates) when doing a clean. This is safe: it only tracks what is
  # in THIS installed/ tree and vcpkg regenerates it on the next install.
  # Must be done unconditionally -- stale 'half-installed' status entries
  # survive even when the triplet dir was removed by a prior clean, causing
  # vcpkg to fail reading pkgconfig files that no longer exist on the next run.
  if [ -d "$US3_VCPKG_ROOT/installed/vcpkg" ]; then
    echo "Removing vcpkg installed/vcpkg bookkeeping (will be regenerated)..."
    rm -rf "$US3_VCPKG_ROOT/installed/vcpkg"
  fi

  if [ -d "$US3_VCPKG_ROOT/installed/$triplet" ]; then
    echo "Removing vcpkg installed packages for triplet: $triplet"
    rm -rf "$US3_VCPKG_ROOT/installed/$triplet"
  else
    echo "vcpkg installed/$triplet does not exist -- nothing to remove"
  fi
}

_purge_binary_cache() {
  local cache_dir="${US3_VCPKG_CACHE:-$HOME/.vcpkg-cache}"
  if [ -d "$cache_dir" ]; then
    echo "Purging vcpkg binary cache: $cache_dir"
    rm -rf "$cache_dir"
    mkdir -p "$cache_dir"
  else
    echo "Binary cache does not exist: $cache_dir"
  fi
}

if [ "$REBUILD" = true ] && [ "$CLEAN" = false ]; then
  echo "=========================================="
  echo "Tier 1 rebuild: removing build directory"
  echo "=========================================="
  _remove_build_dir
  echo "Rebuild clean complete."
  echo ""
fi

if [ "$CLEAN" = true ]; then
  echo "=========================================="
  echo "Tier 2 clean: build dir + vcpkg installed/"
  echo "=========================================="
  _remove_build_dir
  _remove_vcpkg_triplet

  if [ "$PURGE_CACHE" = true ]; then
    echo "------------------------------------------"
    echo "Tier 3: purging binary cache"
    echo "------------------------------------------"
    _purge_binary_cache
  fi

  echo "Clean complete."
  echo ""
fi

echo "=========================================="
echo "UltraScan3 Bootstrap Steps"
echo "=========================================="
echo "  1. Ensure platform toolchain is ready"
echo "  2. Build Qt, Qwt, and other dependencies via vcpkg"
echo "  3. Configure and build UltraScan3"
echo "     Configure preset : $CONFIGURE_PRESET"
echo "     Build preset     : $BUILD_PRESET"
echo ""

# =============================================================================
# LINUX SYSTEM PACKAGE CHECK
# Delegates to bootstrap-deps.sh, which is the single authoritative source
# for the OS-level package list.  It is idempotent: exits immediately with no
# side effects when all packages are already installed (typical on repeat runs).
# The HPC profile requires MPI, so --hpc is passed in that case.
# =============================================================================
if [ "$PLATFORM" == "Linux" ]; then
  _BOOTSTRAP="${SCRIPT_DIR}/bootstrap-deps.sh"
  if [ ! -f "$_BOOTSTRAP" ]; then
    echo "ERROR: bootstrap-deps.sh not found at $_BOOTSTRAP"
    echo "Please ensure scripts/bootstrap-deps.sh exists in the repository."
    exit 1
  fi

  _BOOTSTRAP_ARGS=()
  [ "$PROFILE" = "HPC" ] && _BOOTSTRAP_ARGS+=("--hpc")

  bash "$_BOOTSTRAP" "${_BOOTSTRAP_ARGS[@]}"
  echo ""
fi

# =============================================================================
# SPHINX CHECK - ensure sphinx-build is on PATH and requirements are installed
# =============================================================================
SPHINX_REQUIREMENTS="${SOURCE_DIR}/doc/manual/source/requirements.txt"

# On macOS, pip installs user binaries to a Python-version-specific path
# that isn't on the default PATH. Search for it and add it if found.
if [ "$PLATFORM" = "macOS" ]; then
  if ! command -v sphinx-build &>/dev/null; then
    for pyver in 3.13 3.12 3.11 3.10 3.9 3.8; do
      candidate="$HOME/Library/Python/${pyver}/bin"
      if [ -x "${candidate}/sphinx-build" ]; then
        export PATH="${candidate}:$PATH"
        echo "Found sphinx-build in ${candidate}, added to PATH"
        break
      fi
    done
  fi
fi

# Detect whether pip supports --break-system-packages (pip >= 23.0, Ubuntu 23.04+).
# RHEL/Fedora don't use the PEP 668 externally-managed marker so the flag is
# not needed there and older pip versions will error on it.
_pip_break_flag=""
if command -v pip3 &>/dev/null; then
  if pip3 install --break-system-packages --dry-run pip &>/dev/null 2>&1; then
    _pip_break_flag="--break-system-packages"
  fi
fi

# Helper: run pip install with the right flags for this system.
# In CI, actions/setup-python provides a managed venv where --user is not
# only unnecessary but actively rejected. Outside CI, --user installs to
# ~/.local which keeps the system Python clean.
_pip_install() {
  if command -v pip3 &>/dev/null; then
    if [ "${CI:-false}" = "true" ]; then
      pip3 install ${_pip_break_flag:+$_pip_break_flag} -q "$@" 2>/dev/null || true
    else
      pip3 install ${_pip_break_flag:+$_pip_break_flag} --user -q "$@" 2>/dev/null || true
    fi
  fi
}

# After a pip --user install, binaries land in ~/.local/bin (Linux) or
# ~/Library/Python/X.Y/bin (macOS). Ensure those are on PATH so
# sphinx-build is findable in non-interactive CI environments.
_add_user_bin_to_path() {
  if [ "$PLATFORM" = "Linux" ] && [ -d "$HOME/.local/bin" ]; then
    case ":$PATH:" in
      *":$HOME/.local/bin:"*) ;;
      *) export PATH="$HOME/.local/bin:$PATH" ;;
    esac
  elif [ "$PLATFORM" = "macOS" ]; then
    for pyver in 3.13 3.12 3.11 3.10 3.9 3.8; do
      candidate="$HOME/Library/Python/${pyver}/bin"
      if [ -d "$candidate" ]; then
        case ":$PATH:" in
          *":${candidate}:"*) ;;
          *) export PATH="${candidate}:$PATH" ;;
        esac
      fi
    done
  fi
}

# Add user bin dirs to PATH now, before any sphinx check, so that a
# previously installed sphinx-build is found in CI without .bash_profile.
_add_user_bin_to_path

if ! command -v sphinx-build &>/dev/null; then
  echo "sphinx-build not found - attempting to install from requirements.txt..."
  if [ -f "$SPHINX_REQUIREMENTS" ] && command -v pip3 &>/dev/null; then
    _pip_install -r "$SPHINX_REQUIREMENTS"
    _add_user_bin_to_path
    if command -v sphinx-build &>/dev/null; then
      echo "sphinx-build installed successfully."
    else
      echo "WARNING: sphinx-build still not found after install - documentation will not be built."
      echo "  Install manually: pip3 install -r doc/manual/source/requirements.txt"
    fi
  else
    echo "WARNING: pip3 or requirements.txt not found - documentation will not be built."
    echo "  Install manually: pip3 install -r doc/manual/source/requirements.txt"
  fi
else
  # sphinx-build present - silently ensure all requirements are up to date
  if [ -f "$SPHINX_REQUIREMENTS" ]; then
    _pip_install -r "$SPHINX_REQUIREMENTS"
  fi
  echo "sphinx-build is available: $(command -v sphinx-build)"
fi

# =============================================================================
# FINAL BUILD SUMMARY
# =============================================================================
echo ""
echo "=========================================="
echo "Ready to build UltraScan3"
echo "=========================================="
echo "  Platform         : ${PLATFORM} (${ARCH})"
echo "  Preset           : ${CONFIGURE_PRESET}"
echo "  Profile          : ${PROFILE}"
echo "  Qt variant       : ${QT_VERSION_LABEL}"
echo "  Installer        : ${BUILD_PKG}"
echo "  Rebuild          : ${REBUILD}"
echo "  Clean            : ${CLEAN}"
echo "  Purge cache      : ${PURGE_CACHE}"
echo "  vcpkg root       : ${VCPKG_ROOT}"
echo "  Build jobs       : ${BUILD_JOBS}"
echo ""

if [ "$NON_INTERACTIVE" = false ]; then
  if [ "$REBUILD" = true ] && [ "$CLEAN" = false ]; then
    echo "Tier 1 rebuild - UltraScan recompiled, vcpkg packages restored from cache"
  elif [ "$CLEAN" = true ] && [ "$PURGE_CACHE" = false ]; then
    echo "Tier 2 clean - dependencies will be reinstalled (binary cache still warm)"
  elif [ "$CLEAN" = true ] && [ "$PURGE_CACHE" = true ]; then
    echo "Tier 3 clean - full recompile from source (binary cache purged)"
  else
    echo "Incremental build - only changed files will be rebuilt"
  fi
  echo "Grab a coffee if this is your first build!"
fi
echo ""

# =============================================================================
# CONFIGURE AND BUILD
# =============================================================================
echo "Configuring..."
cmake --preset "$CONFIGURE_PRESET" \
  -DUS3_PROFILE="${PROFILE}" \
  -DVCPKG_ROOT="$US3_VCPKG_ROOT" \
  -DVCPKG_INSTALLED_DIR="$VCPKG_INSTALLED_DIR"

echo ""
echo "Building..."

# On Linux with vcpkg Qt, build-time Qt tools (rcc, qhelpgenerator, lupdate)
# are dynamically linked against vcpkg-built libs (libdouble-conversion, libicuXX,
# etc.) that are NOT on the system LD_LIBRARY_PATH. Export the vcpkg lib dir so
# these tools can find their runtime libraries when CMake invokes them.
if [ "$PLATFORM" = "Linux" ]; then
  _vcpkg_lib_dir="${VCPKG_INSTALLED_DIR}/$(_derive_triplet)/lib"
  if [ -d "$_vcpkg_lib_dir" ]; then
    export LD_LIBRARY_PATH="${_vcpkg_lib_dir}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
  fi
fi

cmake --build --preset "$BUILD_PRESET" --parallel "$BUILD_JOBS"

echo ""
echo "=========================================="
echo "Build complete!"
echo "=========================================="
echo ""

# =============================================================================
# BUILD SUMMARY
# =============================================================================
BUILD_DIR="build/$CONFIGURE_PRESET"

QT_EXACT_VERSION="unknown"
QWT_EXACT_VERSION="unknown"
OPENSSL_VERSION_DISPLAY="unknown"
MARIADB_VERSION_DISPLAY="unknown"
US3_VERSION_DISPLAY="unknown"

_build_info="$BUILD_DIR/us3_build_info.sh"
if [ -f "$_build_info" ]; then
  # shellcheck disable=SC1090
  source "$_build_info"
  [ -n "${US3_QT_VERSION:-}" ]        && QT_EXACT_VERSION="$US3_QT_VERSION"
  [ -n "${US3_QWT_VERSION:-}" ]       && QWT_EXACT_VERSION="$US3_QWT_VERSION"
  [ -n "${US3_OPENSSL_VERSION:-}" ]   && OPENSSL_VERSION_DISPLAY="$US3_OPENSSL_VERSION"
  [ -n "${US3_MARIADB_VERSION:-}" ]   && MARIADB_VERSION_DISPLAY="$US3_MARIADB_VERSION"
  [ -n "${US3_CMAKE_VERSION:-}" ]     && US3_VERSION_DISPLAY="$US3_CMAKE_VERSION"
fi

if [ "$US3_VERSION_DISPLAY" = "unknown" ]; then
  US3_VERSION_DISPLAY=$(sed -n 's/^.*US_Version[^"]*"\([^"]*\)".*$/\1/p' utils/us_defines.h 2>/dev/null | head -1)
  if [ -z "$US3_VERSION_DISPLAY" ]; then
    US3_VERSION_DISPLAY="unknown"
  fi
fi

BIN_COUNT=0
if [ -d "$BUILD_DIR/bin" ]; then
  BIN_COUNT=$(find "$BUILD_DIR/bin" -maxdepth 1 \( -name '*.app' -o \( -type f -perm /111 \) \) 2>/dev/null | wc -l | tr -d ' ')
fi

echo "=========================================="
echo "Build Summary"
echo "=========================================="
echo "  UltraScan3 version : ${US3_VERSION_DISPLAY}"
echo "  Platform           : ${PLATFORM} (${ARCH})"
echo "  Profile            : ${PROFILE}"
echo "  Configure preset   : ${CONFIGURE_PRESET}"
echo "  Qt                 : ${QT_EXACT_VERSION} (${QT_VERSION_LABEL})"
echo "  Qwt                : ${QWT_EXACT_VERSION}"
echo "  Build directory    : ${BUILD_DIR}"
echo "  Binaries           : ${BIN_COUNT} in ${BUILD_DIR}/bin/"
echo "=========================================="
echo ""
if [ "$NON_INTERACTIVE" = false ]; then
  echo "Next time you build it will be much faster since dependencies are cached."
  echo ""
  echo "To wipe build dir only:  $0 --rebuild"
  echo "To reinstall vcpkg deps: $0 --clean"
  echo "To recompile everything: $0 --clean --purge-cache"
  echo ""
fi

# =============================================================================
# LINUX PACKAGE (--pkg on Linux)
#
#   APP profile -> package_linux_tarball   (desktop, includes GUI, help, etc.)
#   HPC profile -> package_linux_hpc_tarball (headless, no docs, no GUI)
#
# Output:
#   APP: build/<preset>/UltraScan3-<version>-Linux-<arch>.tar.gz
#   HPC: build/<preset>/UltraScan3-HPC-<version>-Linux-<arch>.tar.gz
# =============================================================================
if [ "$BUILD_PKG" = true ] && [ "$PLATFORM" = "Linux" ]; then
  BUILD_DIR="build/$CONFIGURE_PRESET"

  if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: Build directory not found: $BUILD_DIR"
    exit 1
  fi

  if [ "$PROFILE" = "HPC" ]; then
    _PKG_TARGET="package_linux_hpc_tarball"
    _PKG_LABEL="Linux HPC tarball"
    _PKG_GLOB="${BUILD_DIR}/UltraScan3-HPC-*-Linux-*.tar.gz"
    _PKG_STAGE="_hpc_stage/UltraScan3-HPC"
    _DEPLOY_TARGET="deploy_linux_hpc"
  else
    _PKG_TARGET="package_linux_tarball"
    _PKG_LABEL="Linux tarball"
    _PKG_GLOB="${BUILD_DIR}/UltraScan3-*-Linux-*.tar.gz"
    _PKG_STAGE="_stage/UltraScan3"
    _DEPLOY_TARGET="deploy_linux"
  fi

  echo ""
  echo "=========================================="
  echo "Building ${_PKG_LABEL}..."
  echo "=========================================="
  echo "  CMake target : ${_PKG_TARGET}"
  echo "  Build dir    : ${BUILD_DIR}"
  echo ""

  if ! cmake --build --preset "$BUILD_PRESET" \
             --target "${_PKG_TARGET}" \
             --parallel "$BUILD_JOBS"; then
    echo ""
    echo "ERROR: '${_PKG_TARGET}' target failed."
    echo ""
    echo "Common causes:"
    echo "  1. ${_DEPLOY_TARGET} failed (BIN_DIR empty or deploy script error)."
    echo "     Run: cmake --build --preset $BUILD_PRESET --target ${_DEPLOY_TARGET}"
    echo "  2. tar failed (check that ${_PKG_STAGE} was created)."
    echo ""
    exit 1
  fi

  echo ""
  echo "=========================================="
  echo "${_PKG_LABEL} complete!"
  echo "=========================================="
  echo ""

  for pkg in ${_PKG_GLOB}; do
    if [ -f "$pkg" ]; then
      PKG_SIZE=$(du -sh "$pkg" 2>/dev/null | cut -f1 || echo "unknown")
      echo "  Created : $pkg"
      echo "  Size    : ${PKG_SIZE}"
    fi
  done
  echo ""
fi

# =============================================================================
# macOS PKG INSTALLER (--pkg on macOS)
# Runs the package_macos_pkg CMake target which:
#   1. Calls deploy_macos to stage the app tree via macdeployqt
#   2. Builds pkgroot with the staged app + LaunchDaemon plist
#   3. Runs pkgbuild to create the component package
#   4. Runs productbuild to create the final installer with UI screens
# Output: build/<preset>/UltraScan3-<version>-macOS.pkg
# =============================================================================
if [ "$BUILD_PKG" = true ] && [ "$PLATFORM" = "macOS" ]; then
  BUILD_DIR="build/$CONFIGURE_PRESET"

  if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: Build directory not found: $BUILD_DIR"
    exit 1
  fi

  if [ "$PROFILE" = "HPC" ]; then
    # ------------------------------------------------------------------
    # macOS HPC: produce a portable tar.gz (no PKG installer, no GUI)
    # ------------------------------------------------------------------
    _MAC_PKG_TARGET="package_macos_hpc_tarball"
    _MAC_PKG_LABEL="macOS HPC tarball"
    _MAC_PKG_GLOB="${BUILD_DIR}/UltraScan3-HPC-*-macOS-*.tar.gz"
    _MAC_DEPLOY_TARGET="deploy_macos_hpc"

    echo ""
    echo "=========================================="
    echo "Building ${_MAC_PKG_LABEL}..."
    echo "=========================================="
    echo "  CMake target : ${_MAC_PKG_TARGET}"
    echo "  Build dir    : ${BUILD_DIR}"
    echo ""

    if ! cmake --build --preset "$BUILD_PRESET" \
               --target "${_MAC_PKG_TARGET}" \
               --parallel "$BUILD_JOBS"; then
      echo ""
      echo "ERROR: '${_MAC_PKG_TARGET}' target failed."
      echo ""
      echo "Common causes:"
      echo "  1. ${_MAC_DEPLOY_TARGET} failed (macdeployqt not found or no us_* targets built)."
      echo "     Run: cmake --build --preset $BUILD_PRESET --target ${_MAC_DEPLOY_TARGET}"
      echo "  2. tar failed (check that _hpc_stage/UltraScan3-HPC was created)."
      echo ""
      exit 1
    fi

    echo ""
    echo "=========================================="
    echo "${_MAC_PKG_LABEL} complete!"
    echo "=========================================="
    echo ""

    for pkg in ${_MAC_PKG_GLOB}; do
      if [ -f "$pkg" ]; then
        PKG_SIZE=$(du -sh "$pkg" 2>/dev/null | cut -f1 || echo "unknown")
        echo "  Created : $pkg"
        echo "  Size    : ${PKG_SIZE}"
      fi
    done
    echo ""

  else
    # ------------------------------------------------------------------
    # macOS APP: produce the PKG installer (pkgbuild + productbuild)
    # ------------------------------------------------------------------
    echo ""
    echo "=========================================="
    echo "Building macOS PKG installer..."
    echo "=========================================="
    echo "  CMake target : package_macos_pkg"
    echo "  Build dir    : ${BUILD_DIR}"
    echo ""

    if ! cmake --build --preset "$BUILD_PRESET" \
               --target package_macos_pkg \
               --parallel "$BUILD_JOBS"; then
      echo ""
      echo "ERROR: 'package_macos_pkg' target failed."
      echo ""
      echo "Common causes:"
      echo "  1. deploy_macos failed (macdeployqt not found or 'us' target missing)."
      echo "     Run: cmake --build --preset $BUILD_PRESET --target deploy_macos"
      echo "  2. pkgbuild or productbuild returned an error."
      echo "  3. pkg/macos/resources/Welcome.txt or LICENSE.txt is missing."
      echo ""
      exit 1
    fi

    echo ""
    echo "=========================================="
    echo "macOS PKG installer complete!"
    echo "=========================================="
    echo ""

    for pkg in "$BUILD_DIR"/UltraScan3-*-macOS.pkg; do
      if [ -f "$pkg" ]; then
        PKG_SIZE=$(du -sh "$pkg" 2>/dev/null | cut -f1 || echo "unknown")
        echo "  Created : $pkg"
        echo "  Size    : ${PKG_SIZE}"
      fi
    done
    echo ""
    echo "To install (requires admin password):"
    echo "  sudo installer -pkg <path>.pkg -target /"
    echo ""
    echo "To verify after install:"
    echo "  sudo launchctl list | grep ultrascan_sysctl"
    echo "  sysctl kern.sysv.shmmax kern.sysv.shmall"
    echo ""
  fi
fi
