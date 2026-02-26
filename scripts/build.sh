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
CLEAN=false
PROFILE="APP"
QT_VERSION="qt6"
QWT_VERSION=""
ARCH=""
US3_VCPKG_ROOT=""

while [[ $# -gt 0 ]]; do
  case $1 in
    --clean)        CLEAN=true;               shift ;;
    --qt6)          QT_VERSION="qt6";         QWT_VERSION="";        shift ;;
    --qt5-qwt616)   QT_VERSION="qt5";         QWT_VERSION="-qwt616"; shift ;;
    --qt5-qwt630)   QT_VERSION="qt5";         QWT_VERSION="-qwt630"; shift ;;
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
    --help)
      echo "Usage: $0 [OPTIONS] [PROFILE]"
      echo ""
      echo "OPTIONS:"
      echo "  --clean                Clean build artifacts before building"
      echo "  --qt6                  Build with Qt6 + Qwt6.3.0 [default]"
      echo "  --qt5-qwt616           Build with Qt5 + Qwt6.1.6"
      echo "  --qt5-qwt630           Build with Qt5 + Qwt6.3.0"
      echo "  --arch x64             Target x64 architecture [default: auto-detect]"
      echo "  --arch arm64           Target ARM64 architecture"
      echo "  --vcpkg-root <path>    Path to vcpkg installation"
      echo "  --help                 Show this help message"
      echo ""
      echo "PROFILE:"
      echo "  APP                    Desktop/user build (GUI + programs + DB) [default]"
      echo "  TEST                   Dev/CI build (programs + tests, prefer static libs)"
      echo "  HPC                    Headless / no DB / no GUI"
      echo ""
      echo "VCPKG LOCATION (in order of priority):"
      echo "  1. --vcpkg-root <path> argument"
      echo "  2. US3_VCPKG_ROOT environment variable"
      echo "  3. vcpkg/ directory inside the source tree"
      echo "  4. \$HOME/vcpkg (default)"
      echo ""
      echo "EXAMPLES:"
      echo "  $0                                    # Qt6, auto-detect arch, APP profile"
      echo "  $0 --arch arm64                       # Qt6, ARM64, APP profile"
      echo "  $0 --qt5-qwt616                       # Qt5 + Qwt6.1.6, APP profile"
      echo "  $0 --qt6 TEST                         # Qt6, TEST profile"
      echo "  $0 --clean                            # Clean then build Qt6, APP profile"
      echo "  $0 --clean --qt5-qwt616               # Clean then build Qt5 + Qwt6.1.6"
      echo "  $0 --clean --arch arm64 TEST          # Clean ARM64 Qt6 TEST build"
      echo "  $0 --vcpkg-root /path/to/vcpkg        # Use specific vcpkg installation"
      echo "  $0 --vcpkg-root \$(pwd)/vcpkg          # Use source-tree vcpkg"
      echo ""
      echo "ENVIRONMENT VARIABLES:"
      echo "  US3_BUILD_JOBS         Override number of parallel build jobs"
      echo "  US3_VCPKG_ROOT         Override vcpkg location (see priority above)"
      exit 0
      ;;
    APP|TEST|HPC)
      PROFILE="$1"; shift
      ;;
    *)
      echo "ERROR: Unknown option: $1"
      echo "Run '$0 --help' for usage information"
      exit 1
      ;;
  esac
done

echo "Selected build profile : ${PROFILE}"
echo "Selected Qt version    : ${QT_VERSION}${QWT_VERSION}"
if [ "$CLEAN" = true ]; then
  echo "Clean build requested"
fi
echo ""

# =============================================================================
# PLATFORM DETECTION
# =============================================================================
if [[ "$OSTYPE" == "darwin"* ]]; then
  PLATFORM="macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  PLATFORM="Linux"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
  PLATFORM="Windows"
else
  echo "ERROR: Unsupported platform: $OSTYPE"
  exit 1
fi

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
# BUILD PRESET
# =============================================================================
case "$PLATFORM" in
  macOS)
    PRESET="macos-release-${QT_VERSION}${QWT_VERSION}"
    ;;
  Linux)
    PRESET="linux-release-${QT_VERSION}${QWT_VERSION}"
    ;;
  Windows)
    if [ "$ARCH" = "arm64" ]; then
      PRESET="windows-release-${QT_VERSION}${QWT_VERSION}-arm64"
    else
      PRESET="windows-release-${QT_VERSION}${QWT_VERSION}"
    fi
    ;;
esac

echo "Platform               : $PLATFORM"
echo "Preset                 : $PRESET"
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
  BUILD_JOBS="$US3_BUILD_JOBS"
else
  BUILD_JOBS=$((CORES * 9 / 10))
  if [ "$BUILD_JOBS" -lt 1 ]; then
    BUILD_JOBS=1
  fi
fi

echo "Detected $CORES cores; using $BUILD_JOBS parallel build jobs."
echo ""

export VCPKG_MAX_CONCURRENCY="$BUILD_JOBS"

# =============================================================================
# CHECK REQUIRED TOOLS
# =============================================================================
REQUIRED_TOOLS=(cmake git)

if [[ "$PLATFORM" == "macOS" ]]; then
  REQUIRED_TOOLS+=(xcodebuild xcrun)
elif [[ "$PLATFORM" == "Linux" ]]; then
  REQUIRED_TOOLS+=(g++)
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
  if [ "$PLATFORM" == "macOS" ]; then
    echo "On macOS, install Xcode command line tools:"
    echo "  xcode-select --install"
  elif [ "$PLATFORM" == "Linux" ]; then
    echo "On Debian/Ubuntu, run:"
    echo "  sudo apt-get update && sudo apt-get install -y build-essential cmake git"
  fi
  exit 1
fi

echo "All required tools are available."
echo ""

# =============================================================================
# XCODE SETUP ON macOS (requires Xcode 15 or later)
# =============================================================================
if [ "$PLATFORM" = "macOS" ]; then
  CURRENT_XCODE_PATH=$(xcode-select -p 2>/dev/null || echo "")

  XCODE_VERSIONED_PATH="/Applications/Xcode-15.app/Contents/Developer"
  XCODE_DEFAULT_PATH="/Applications/Xcode.app/Contents/Developer"

  echo "Checking Xcode configuration..."
  echo "Current Xcode path: ${CURRENT_XCODE_PATH:-<not set>}"
  echo ""

  check_xcode_path() {
    local xcode_developer_path="$1"
    if [ ! -d "$xcode_developer_path" ]; then return 1; fi
    local xcode_app
    xcode_app=$(dirname "$(dirname "$xcode_developer_path")")
    local plist="$xcode_app/Contents/Info.plist"
    if [ ! -f "$plist" ]; then return 1; fi
    local version
    version=$(/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "$plist" 2>/dev/null || echo "0")
    local major
    major=$(echo "$version" | cut -d. -f1)
    if [ "$major" -ge 15 ] 2>/dev/null; then
      echo "$version"
      return 0
    fi
    return 1
  }

  DESIRED_XCODE_PATH=""
  XCODE_VERSION=""

  if XCODE_VERSION=$(check_xcode_path "$XCODE_VERSIONED_PATH"); then
    DESIRED_XCODE_PATH="$XCODE_VERSIONED_PATH"
    echo "Found Xcode $XCODE_VERSION at Xcode-15.app location"
  elif XCODE_VERSION=$(check_xcode_path "$XCODE_DEFAULT_PATH"); then
    DESIRED_XCODE_PATH="$XCODE_DEFAULT_PATH"
    echo "Found Xcode $XCODE_VERSION at default location"
  fi

  if [ -n "$DESIRED_XCODE_PATH" ]; then
    echo "Compatible Xcode path: $DESIRED_XCODE_PATH"
    if [ "$CURRENT_XCODE_PATH" != "$DESIRED_XCODE_PATH" ]; then
      echo "Xcode $XCODE_VERSION is installed but not active."
      if [ "$NON_INTERACTIVE" = false ]; then
        read -rp "Switch to it with 'sudo xcode-select --switch'? [y/N] " answer
        if [[ "$answer" =~ ^[Yy]$ ]]; then
          sudo xcode-select --switch "$DESIRED_XCODE_PATH"
          echo "Xcode is now set to: $(xcode-select -p)"
        else
          echo "Skipping Xcode switch. Continuing with current Xcode."
        fi
      else
        sudo xcode-select --switch "$DESIRED_XCODE_PATH"
        echo "Xcode is now set to: $(xcode-select -p)"
      fi
    else
      echo "Compatible Xcode is already active."
    fi
  else
    echo "ERROR: Xcode 15 or later not found."
    echo "Please install Xcode 15 or later from the App Store or developer.apple.com."
    if [ "$NON_INTERACTIVE" = false ]; then exit 1; else echo "WARNING: Continuing in CI mode, build may fail..."; fi
  fi

  echo ""
fi

# =============================================================================
# VCPKG SETUP
# Priority: --vcpkg-root arg > US3_VCPKG_ROOT env > source-tree vcpkg > ~/vcpkg
# =============================================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

if [ -n "$US3_VCPKG_ROOT" ]; then
  echo "Using vcpkg from --vcpkg-root argument: $US3_VCPKG_ROOT"
elif [ -n "${US3_VCPKG_ROOT:-}" ]; then
  echo "Using vcpkg from US3_VCPKG_ROOT environment variable: $US3_VCPKG_ROOT"
elif [ -f "${SOURCE_DIR}/vcpkg/bootstrap-vcpkg.sh" ]; then
  US3_VCPKG_ROOT="${SOURCE_DIR}/vcpkg"
  echo "Using vcpkg from source tree: $US3_VCPKG_ROOT"
else
  US3_VCPKG_ROOT="$HOME/vcpkg"
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
  ( cd "$US3_VCPKG_ROOT" && ./bootstrap-vcpkg.sh )
fi

export VCPKG_ROOT="$US3_VCPKG_ROOT"
export VCPKG_BINARY_SOURCES="clear;files,$HOME/.vcpkg-cache,readwrite"
export VCPKG_INSTALLED_DIR="$US3_VCPKG_ROOT/installed"
mkdir -p "$HOME/.vcpkg-cache"

if [ ! -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
  echo "ERROR: vcpkg toolchain file not found at $VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
  exit 1
fi

echo "vcpkg ready."
echo ""

# =============================================================================
# CLEAN BUILD ARTIFACTS (if requested)
# =============================================================================
if [ "$CLEAN" = true ]; then
  echo "=========================================="
  echo "Cleaning build artifacts..."
  echo "=========================================="

  if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
  fi

  if [ -d "$US3_VCPKG_ROOT/buildtrees" ]; then
    echo "Removing vcpkg build trees..."
    rm -rf "$US3_VCPKG_ROOT/buildtrees"
  fi

  # Uncomment for a full dependency clean (much slower):
  # rm -rf "$US3_VCPKG_ROOT/installed" "$US3_VCPKG_ROOT/packages"

  echo "Clean complete!"
  echo ""
fi

# =============================================================================
# INSTALL PLATFORM-SPECIFIC BUILD TOOLS (if needed)
# =============================================================================
if [ "$PLATFORM" == "Linux" ]; then
  if ! command -v ninja &>/dev/null; then
    echo "Ninja not found. Installing..."
    if command -v apt-get &>/dev/null; then
      sudo apt-get update && sudo apt-get install -y ninja-build
    elif command -v dnf &>/dev/null; then
      sudo dnf install -y ninja-build
    else
      echo "Please install Ninja manually."
      exit 1
    fi
  fi
fi

# =============================================================================
# BUILD SUMMARY
# =============================================================================
echo "=========================================="
echo "Ready to build UltraScan3"
echo "=========================================="
echo "  Preset        : ${PRESET}"
echo "  Profile       : ${PROFILE}"
echo "  Qt version    : ${QT_VERSION}${QWT_VERSION}"
echo "  Architecture  : ${ARCH}"
echo "  Clean build   : ${CLEAN}"
echo "  vcpkg root    : ${VCPKG_ROOT}"
echo "  Build jobs    : ${BUILD_JOBS}"
echo ""
if [ "$NON_INTERACTIVE" = false ]; then
  if [ "$CLEAN" = true ]; then
    echo "Clean build - dependencies will be rebuilt if needed"
  else
    echo "Incremental build - only changed files will be rebuilt"
  fi
  echo "Grab a coffee if this is your first build!"
fi
echo ""

# =============================================================================
# CONFIGURE AND BUILD
#
# NOTE: Do NOT pass -DCMAKE_TOOLCHAIN_FILE here.
#   toolchain.cmake (set in each platform base preset) handles triplet
#   detection and includes vcpkg via $VCPKG_ROOT which is already exported.
# =============================================================================
echo "Configuring..."
cmake --preset "$PRESET" \
  -DUS3_PROFILE="${PROFILE}"

echo ""
echo "Building..."
cmake --build "build/$PRESET" --parallel "$BUILD_JOBS"

echo ""
echo "=========================================="
echo "Build complete!"
echo "=========================================="
echo ""
if [ "$NON_INTERACTIVE" = false ]; then
  echo "Next time you build it will be much faster since dependencies are cached."
  echo ""
  echo "To rebuild from scratch: $0 --clean"
  echo ""
fi


