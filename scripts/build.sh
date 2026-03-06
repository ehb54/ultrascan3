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
INSTALL=false
BUILD_PKG=false        # --pkg: build macOS PKG installer via package_macos_pkg
PROFILE="APP"          # default profile
QT_VARIANT="qt6"       # qt6 | qt5-qwt616 | qt5-qwt630
ARCH=""
US3_VCPKG_ROOT=""

# Parse options
while [[ $# -gt 0 ]]; do
  case $1 in
    --clean)        CLEAN=true;                 shift ;;
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
    --install)
      # macOS packaging is done via --pkg (PKG installer with LaunchDaemon +
      # sysctl). --install is Linux (DEB/RPM) and Windows (NSIS) only.
      if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "ERROR: --install is not supported on macOS."
        echo "       Use --pkg to build the macOS PKG installer instead."
        exit 1
      fi
      INSTALL=true
      shift
      ;;
    --pkg)
      BUILD_PKG=true
      shift
      ;;
    --help)
      echo "Usage: $0 [OPTIONS] [PROFILE]"
      echo ""
      echo "OPTIONS:"
      echo "  --clean              Clean the active build directory and vcpkg buildtrees before building"
      echo "  --pkg                Build macOS PKG installer (macOS only)"
      echo "                       Installs to /Applications/UltraScan3, activates the"
      echo "                       ultrascan_sysctl LaunchDaemon, and applies sysctl values."
      echo "                       Output: build/<preset>/UltraScan3-<version>-macOS.pkg"
      echo "  --install            Build platform installer: DEB+RPM on Linux, NSIS on Windows"
      echo "                       Not supported on macOS (use --pkg)"
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
      echo "  $0 --clean TEST           # Clean build with TEST profile"
      echo "  $0 --pkg                  # macOS: build + PKG installer"
      echo "  $0 --clean --pkg          # macOS: clean build + PKG installer"
      echo "  $0 --install              # Linux/Windows: build + DEB/RPM or NSIS"
      echo ""
      echo "ENVIRONMENT VARIABLES:"
      echo "  US3_BUILD_JOBS      Override number of parallel build jobs"
      echo "  US3_VCPKG_ROOT      Override vcpkg location (default: \$HOME/vcpkg)"
      exit 0
      ;;
    APP|TEST|HPC)
      PROFILE="$1"
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
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  PLATFORM="Linux"
  PLATFORM_PREFIX="linux"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
  PLATFORM="Windows"
  PLATFORM_PREFIX="windows"
else
  echo "ERROR: Unsupported platform: $OSTYPE"
  exit 1
fi

# Guard: --pkg is macOS only
if [ "$BUILD_PKG" = true ] && [ "$PLATFORM" != "macOS" ]; then
  echo "ERROR: --pkg is only supported on macOS."
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
if [ "$CLEAN" = true ]; then
  echo "Clean build requested"
fi
if [ "$BUILD_PKG" = true ]; then
  echo "macOS PKG installer  : enabled"
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
if [[ "$PLATFORM" == "Windows" && "$ARCH" == "arm64" ]]; then
  ARM64_SUFFIX="-arm64"
fi

CONFIGURE_PRESET="${PLATFORM_PREFIX}-release-${QT_VARIANT}${ARM64_SUFFIX}"
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

if [ "$BUILD_PKG" = true ]; then
  REQUIRED_TOOLS+=(pkgbuild productbuild rsync)
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
  elif [ "$PLATFORM" == "Windows" ]; then
    echo "On Windows (MSYS/MinGW), install cmake, git, and a compiler toolchain."
  fi
  exit 1
fi

echo "All required tools are available."
echo ""

# =============================================================================
# OPTIONAL: Xcode 15/16 SETUP ON macOS
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
        if [ "$XCODE_MAJOR" = "15" ] || [ "$XCODE_MAJOR" = "16" ]; then
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
  if [[ "$PLATFORM" == "Windows" ]]; then
    ( cd "$US3_VCPKG_ROOT" && ./bootstrap-vcpkg.bat )
  else
    "$US3_VCPKG_ROOT/bootstrap-vcpkg.sh"
  fi
fi

export VCPKG_ROOT="$US3_VCPKG_ROOT"
export VCPKG_BINARY_SOURCES="clear;files,$HOME/.vcpkg-cache,readwrite"
export VCPKG_INSTALLED_DIR="$US3_VCPKG_ROOT/installed"
mkdir -p "$HOME/.vcpkg-cache"

VCPKG_TOOLCHAIN_FILE="$US3_VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "$VCPKG_TOOLCHAIN_FILE" ]; then
  echo "ERROR: vcpkg toolchain file not found at $VCPKG_TOOLCHAIN_FILE"
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

  BUILD_DIR="build/$CONFIGURE_PRESET"
  if [ -d "$BUILD_DIR" ]; then
    echo "Removing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
  else
    echo "Build directory does not exist, nothing to clean: $BUILD_DIR"
  fi

  if [ -d "$US3_VCPKG_ROOT/buildtrees" ]; then
    echo "Removing vcpkg build trees..."
    rm -rf "$US3_VCPKG_ROOT/buildtrees"
  fi

  echo "Clean complete!"
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
# INSTALL PLATFORM-SPECIFIC BUILD TOOLS (if needed)
# =============================================================================
if [ "$PLATFORM" == "Linux" ]; then
  echo "Checking for Ninja..."
  if ! command -v ninja &>/dev/null; then
    echo "Ninja not found. Installing..."
    if command -v apt-get &>/dev/null; then
      sudo apt-get update
      sudo apt-get install -y ninja-build
    elif command -v dnf &>/dev/null; then
      sudo dnf install -y ninja-build
    else
      echo "Please install Ninja manually."
      exit 1
    fi
  fi
  echo ""
fi

# =============================================================================
# SPHINX CHECK - ensure sphinx-build is on PATH
# =============================================================================
if ! command -v sphinx-build &>/dev/null; then
  if [ "$PLATFORM" = "macOS" ]; then
    for pyver in 3.13 3.12 3.11 3.10 3.9 3.8; do
      candidate="$HOME/Library/Python/${pyver}/bin"
      if [ -x "${candidate}/sphinx-build" ]; then
        export PATH="${candidate}:$PATH"
        echo "Found sphinx-build in ${candidate}, added to PATH"
        break
      fi
    done
  fi

  if ! command -v sphinx-build &>/dev/null; then
    echo "WARNING: sphinx-build not found - documentation will not be built"
    echo "  Install with: pip3 install -r doc/manual/source/requirements.txt"
  fi
fi

# =============================================================================
# FINAL BUILD SUMMARY
# =============================================================================
echo ""
echo "=========================================="
echo "Ready to build UltraScan3"
echo "=========================================="
echo "  Configure preset : ${CONFIGURE_PRESET}"
echo "  Build preset     : ${BUILD_PRESET}"
echo "  Profile          : ${PROFILE}"
echo "  Qt variant       : ${QT_VERSION_LABEL}"
echo "  Architecture     : ${ARCH}"
echo "  Platform         : ${PLATFORM}"
echo "  PKG installer    : ${BUILD_PKG}"
echo "  Linux/Win pkg    : ${INSTALL}"
echo "  Clean build      : ${CLEAN}"
echo "  vcpkg root       : ${VCPKG_ROOT}"
echo "  Build jobs       : ${BUILD_JOBS}"
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
# =============================================================================
echo "Configuring..."
cmake --preset "$CONFIGURE_PRESET" \
  -DUS3_PROFILE="${PROFILE}" \
  -DVCPKG_ROOT="$US3_VCPKG_ROOT" \
  -DVCPKG_INSTALLED_DIR="$VCPKG_INSTALLED_DIR"

echo ""
echo "Building..."
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
  BIN_COUNT=$(find "$BUILD_DIR/bin" -maxdepth 1 \( -name '*.app' -o \( -type f -perm +111 \) \) 2>/dev/null | wc -l | tr -d ' ')
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
  echo "Next time you build, it will be much faster"
  echo "since dependencies are cached."
  echo ""
  echo "To rebuild from scratch: $0 --clean"
  echo ""
fi

# =============================================================================
# LINUX / WINDOWS PACKAGE (--install)
# Runs cpack to produce DEB+RPM on Linux or NSIS installer on Windows.
# Not used on macOS — use --pkg instead.
# =============================================================================
if [ "$INSTALL" = true ]; then
  BUILD_DIR="build/$CONFIGURE_PRESET"

  if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: Build directory not found: $BUILD_DIR"
    exit 1
  fi

  echo ""
  echo "=========================================="
  echo "Packaging (cpack)..."
  echo "=========================================="
  ( cd "$BUILD_DIR" && cpack )

  echo ""
  echo "=========================================="
  echo "Package complete!"
  echo "=========================================="
  echo ""

  case "$PLATFORM" in
    Linux)
      for pkg in "$BUILD_DIR"/*.deb "$BUILD_DIR"/*.rpm; do
        [ -f "$pkg" ] && echo "Created: $pkg"
      done
      ;;
    Windows)
      for pkg in "$BUILD_DIR"/*.exe; do
        [ -f "$pkg" ] && echo "Created: $pkg"
      done
      ;;
  esac
  echo ""
fi

# =============================================================================
# macOS PKG INSTALLER (--pkg)
# Runs the package_macos_pkg CMake target which:
#   1. Calls deploy_macos to stage the app tree via macdeployqt
#   2. Builds pkgroot with the staged app + LaunchDaemon plist
#   3. Runs pkgbuild to create the component package
#   4. Runs productbuild to create the final installer with UI screens
# Output: build/<preset>/UltraScan3-<version>-macOS.pkg
# =============================================================================
if [ "$BUILD_PKG" = true ]; then
  BUILD_DIR="build/$CONFIGURE_PRESET"

  if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: Build directory not found: $BUILD_DIR"
    exit 1
  fi

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
