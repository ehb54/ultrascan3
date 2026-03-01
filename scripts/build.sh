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
PROFILE="APP"  # default profile
QT_OVERRIDE=""  # empty = use platform default
QWT_VERSION="616"  # default Qwt version

# Parse options
while [[ $# -gt 0 ]]; do
  case $1 in
    --clean)
      CLEAN=true
      shift
      ;;
    --qt5)
      if [ "$QT_OVERRIDE" = "qt6" ]; then
        echo "ERROR: --qt5 and --qt6 cannot both be specified"
        exit 1
      fi
      QT_OVERRIDE="qt5"
      shift
      ;;
    --qt6)
      if [ "$QT_OVERRIDE" = "qt5" ]; then
        echo "ERROR: --qt5 and --qt6 cannot both be specified"
        exit 1
      fi
      QT_OVERRIDE="qt6"
      shift
      ;;
    --qwt630)
      QWT_VERSION="630"
      shift
      ;;
    --install)
      INSTALL=true
      shift
      ;;
    --help)
      echo "Usage: $0 [OPTIONS] [PROFILE]"
      echo ""
      echo "OPTIONS:"
      echo "  --clean       Clean build artifacts before building"
      echo "  --install     After building, stage and package a distributable artifact"
      echo "                  macOS: runs macdeployqt, bundles Qt libs, then creates DMG"
      echo "                  Linux: runs cpack to create DEB/RPM"
      echo "                  Windows: runs cpack to create NSIS installer"
      echo "  --qt5         Force Qt5 build (default on Linux and Windows)"
      echo "  --qt6         Force Qt6 build (default on macOS)"
      echo "  --qwt630      Use Qwt 6.3.0 instead of Qwt 6.1.6 (Qt5 only; ignored for Qt6)"
      echo "  --help        Show this help message"
      echo ""
      echo "PROFILE (positional, optional):"
      echo "  APP           Desktop/user build (GUI + programs + DB) [default]"
      echo "  TEST          Dev/CI build (programs + tests, prefer static libs)"
      echo "  HPC           Headless / no DB / no GUI"
      echo ""
      echo "EXAMPLES:"
      echo "  $0                    # Dev build: Qt6 on macOS, Qt5 on Linux/Windows"
      echo "  $0 --qt5              # Force Qt5 dev build"
      echo "  $0 --qt5 --qwt630     # Qt5 dev build with Qwt 6.3.0"
      echo "  $0 --qt6              # Force Qt6 dev build"
      echo "  $0 --install          # Build then create distributable (DMG/DEB/RPM)"
      echo "  $0 --qt5 --install    # Qt5 distributable"
      echo "  $0 --clean            # Clean and rebuild"
      echo "  $0 --clean TEST       # Clean and rebuild with TEST profile"
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

echo "Selected build profile: ${PROFILE}"
if [ "$CLEAN" = true ]; then
  echo "Clean build requested"
fi
echo ""

# =============================================================================
# PLATFORM DETECTION
# =============================================================================
if [[ "$OSTYPE" == "darwin"* ]]; then
  PLATFORM="macOS"
  PLATFORM_PREFIX="macos"
  DEFAULT_QT="qt6"  # macOS defaults to Qt6
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  PLATFORM="Linux"
  PLATFORM_PREFIX="linux"
  DEFAULT_QT="qt5"  # Linux defaults to Qt5
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
  PLATFORM="Windows"
  PLATFORM_PREFIX="windows"
  DEFAULT_QT="qt5"  # Windows defaults to Qt5
else
  echo "ERROR: Unsupported platform: $OSTYPE"
  exit 1
fi

# Apply Qt override or use platform default
RESOLVED_QT="${QT_OVERRIDE:-$DEFAULT_QT}"

# --qwt630 is Qt5-only; silently ignore for Qt6 builds
if [[ "$RESOLVED_QT" == "qt6" && "$QWT_VERSION" == "630" ]]; then
  echo "NOTE: --qwt630 has no effect for Qt6 builds (Qwt 6.3.0 is always used with Qt6)."
  QWT_VERSION="ignored"
fi

# Derive preset names.
# Configure preset names match CMakePresets.json exactly.
# Build preset names also match CMakePresets.json exactly.
if [[ "$RESOLVED_QT" == "qt6" ]]; then
  CONFIGURE_PRESET="${PLATFORM_PREFIX}-release-qt6"
  BUILD_PRESET="${PLATFORM_PREFIX}-release-qt6"
  QT_VERSION_LABEL="Qt6"
elif [[ "$QWT_VERSION" == "630" ]]; then
  CONFIGURE_PRESET="${PLATFORM_PREFIX}-release-qt5-qwt630"
  BUILD_PRESET="${PLATFORM_PREFIX}-release-qwt630"
  QT_VERSION_LABEL="Qt5 (Qwt 6.3.0)"
else
  CONFIGURE_PRESET="${PLATFORM_PREFIX}-release-qt5-qwt616"
  BUILD_PRESET="${PLATFORM_PREFIX}-release"
  QT_VERSION_LABEL="Qt5 (Qwt 6.1.6)"
fi

echo "Platform:    $PLATFORM"
echo "Qt version:  $QT_VERSION_LABEL  (configure preset: $CONFIGURE_PRESET)"
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
  [ "$BUILD_JOBS" -lt 1 ] && BUILD_JOBS=1
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
# =============================================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

US3_VCPKG_ROOT="${US3_VCPKG_ROOT:-$HOME/vcpkg}"
echo "Using vcpkg root: $US3_VCPKG_ROOT"

if [ -d "$US3_VCPKG_ROOT" ] && [ ! -d "$US3_VCPKG_ROOT/.git" ]; then
  echo "ERROR: $US3_VCPKG_ROOT exists but is not a vcpkg git clone."
  echo "Either set US3_VCPKG_ROOT to a different path or move/rename that directory."
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
mkdir -p "$HOME/.vcpkg-cache"
export VCPKG_INSTALLED_DIR="$US3_VCPKG_ROOT/installed"

VCPKG_TOOLCHAIN_FILE="$US3_VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "$VCPKG_TOOLCHAIN_FILE" ]; then
  echo "ERROR: vcpkg toolchain file not found at $VCPKG_TOOLCHAIN_FILE"
  exit 1
fi

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
echo "  3. Configure and build UltraScan3 via CMake preset: $CONFIGURE_PRESET"
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
# FINAL BUILD SUMMARY
# =============================================================================
echo "Ready to build UltraScan3 with preset: $CONFIGURE_PRESET"
echo "  Profile:         ${PROFILE}"
echo "  Platform:        ${PLATFORM}"
echo "  Qt version:      ${QT_VERSION_LABEL}"
echo "  Clean build:     ${CLEAN}"
echo "  Create package:  ${INSTALL}"
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
QWTPLOT3D_VERSION="unknown"
OPENSSL_VERSION_DISPLAY="unknown"
MARIADB_VERSION_DISPLAY="unknown"
US3_VERSION_DISPLAY="unknown"

_build_info="$BUILD_DIR/us3_build_info.sh"
if [ -f "$_build_info" ]; then
  # shellcheck disable=SC1090
  source "$_build_info"
  [ -n "${US3_QT_VERSION:-}" ]        && QT_EXACT_VERSION="$US3_QT_VERSION"
  [ -n "${US3_QWT_VERSION:-}" ]       && QWT_EXACT_VERSION="$US3_QWT_VERSION"
  [ -n "${US3_QWTPLOT3D_VERSION:-}" ] && QWTPLOT3D_VERSION="$US3_QWTPLOT3D_VERSION"
  [ -n "${US3_OPENSSL_VERSION:-}" ]   && OPENSSL_VERSION_DISPLAY="$US3_OPENSSL_VERSION"
  [ -n "${US3_MARIADB_VERSION:-}" ]   && MARIADB_VERSION_DISPLAY="$US3_MARIADB_VERSION"
  [ -n "${US3_CMAKE_VERSION:-}" ]     && US3_VERSION_DISPLAY="$US3_CMAKE_VERSION"
fi

ARCH=$(uname -m 2>/dev/null || echo "unknown")
US3_VERSION_DISPLAY=$(grep 'US_Version' utils/us_defines.h 2>/dev/null | head -1 | sed 's/.*"//;s/"//' || echo "unknown")

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
echo "  Preset             : ${CONFIGURE_PRESET}"
echo "  Qt                 : ${QT_EXACT_VERSION} (${QT_VERSION_LABEL})"
echo "  Qwt                : ${QWT_EXACT_VERSION}"
echo "  QwtPlot3D          : ${QWTPLOT3D_VERSION}"
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
# INSTALL AND PACKAGE (if requested)
# =============================================================================
if [ "$INSTALL" = true ]; then
  BUILD_DIR="build/$CONFIGURE_PRESET"

  if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: Build directory not found: $BUILD_DIR"
    exit 1
  fi

  if [ "$PLATFORM" = "macOS" ]; then
    # ------------------------------------------------------------------
    # macOS: verify deploy_macos target exists before calling it
    # ------------------------------------------------------------------
    echo "=========================================="
    echo "Staging macOS folder-based deployment..."
    echo "=========================================="

    # Check that macdeployqt was found at configure time
    MACDEPLOYQT_CHECK=$(cmake --preset "$CONFIGURE_PRESET" -N 2>/dev/null \
      | grep -i 'macdeployqt' | head -1 || true)

    # Attempt deploy_macos target; emit a clear error if it doesn't exist
    if ! cmake --build --preset "$BUILD_PRESET" --target deploy_macos --parallel "$BUILD_JOBS"; then
      echo ""
      echo "ERROR: 'deploy_macos' target failed or does not exist."
      echo ""
      echo "Possible causes:"
      echo "  1. macdeployqt was not found in the Qt installation."
      echo "     Check the configure output for 'macdeployqt not found'."
      echo "  2. The 'us' target did not build successfully."
      echo "  3. The Qt installation is incomplete (missing tools component)."
      echo ""
      echo "To diagnose: run 'cmake --preset $CONFIGURE_PRESET' and look for"
      echo "  'Found macdeployqt' vs 'macdeployqt not found' in the output."
      exit 1
    fi

    STAGE_DIR="$BUILD_DIR/_stage/UltraScan3"
    if [ -d "$STAGE_DIR" ]; then
      echo ""
      echo "Staged folder tree: $STAGE_DIR/"
      ls -1 "$STAGE_DIR/" | sed 's/^/    /'
      echo ""
      echo "  bin/ contents (first 20):"
      ls -1 "$STAGE_DIR/bin/" 2>/dev/null | head -20 | sed 's/^/    /'
      echo ""
      # Verify Qt libs are bundled and not pointing to developer paths
      echo "Verifying Qt bundling..."
      if [ -d "$STAGE_DIR/frameworks" ]; then
        QT_FW_COUNT=$(find "$STAGE_DIR/frameworks" -name "*.framework" -maxdepth 1 | wc -l | tr -d ' ')
        QT_DYLIB_COUNT=$(find "$STAGE_DIR/frameworks" -name "*.dylib" -maxdepth 1 | wc -l | tr -d ' ')
        echo "  Frameworks: ${QT_FW_COUNT} .framework bundles, ${QT_DYLIB_COUNT} .dylib files"
      else
        echo "  WARNING: frameworks/ directory not found in staged tree!"
        echo "  The distributable may still depend on developer-installed Qt."
      fi
      # Spot-check that the main binary's rpaths are @-relative
      US_BIN="$STAGE_DIR/bin/us.app/Contents/MacOS/us"
      if [ -f "$US_BIN" ]; then
        RPATH_CHECK=$(otool -l "$US_BIN" 2>/dev/null | grep -A2 LC_RPATH | grep 'path ' | awk '{print $2}' || true)
        echo "  Rpaths in us.app/Contents/MacOS/us:"
        echo "$RPATH_CHECK" | sed 's/^/    /'
        if echo "$RPATH_CHECK" | grep -q "$HOME"; then
          echo ""
          echo "  WARNING: us binary still has rpaths pointing to $HOME."
          echo "  macdeployqt rpath fixup may have been incomplete."
        fi
      fi
    else
      echo "ERROR: Staging directory not found after deploy_macos: $STAGE_DIR"
      exit 1
    fi
  fi

  echo ""
  echo "=========================================="
  echo "Packaging..."
  echo "=========================================="
  ( cd "$BUILD_DIR" && cpack )

  echo ""
  echo "=========================================="
  echo "Package complete!"
  echo "=========================================="
  echo ""

  # Show what was produced
  case "$PLATFORM" in
    macOS)
      for pkg in "$BUILD_DIR"/*.dmg; do
        [ -f "$pkg" ] && echo "Created: $pkg"
      done
      ;;
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

  if [ "$PLATFORM" = "macOS" ]; then
    echo "To verify the DMG (Finder launch test):"
    echo "  1. Double-click the .dmg to mount it"
    echo "  2. Drag UltraScan3/ to /Applications/ (or any location)"
    echo "  3. Right-click UltraScan3/bin/us.app → Open (first launch on unsigned app)"
    echo "  4. Confirm UltraScan3 opens and Help → Assistant works"
    echo ""
    echo "To verify no developer-path dependencies remain:"
    echo "  otool -L \$(find \"\$STAGE_DIR\" -name '*.dylib' -o -name '*/MacOS/*' | head -5)"
    echo ""
  fi
fi
