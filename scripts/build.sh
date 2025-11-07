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
# PLATFORM DETECTION
# =============================================================================
if [[ "$OSTYPE" == "darwin"* ]]; then
  PRESET="macos-release"
  PLATFORM="macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  PRESET="linux-release"
  PLATFORM="Linux"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
  PRESET="windows-release"
  PLATFORM="Windows"
else
  echo "ERROR: Unsupported platform: $OSTYPE"
  exit 1
fi

echo "Platform: $PLATFORM"
echo ""

# =============================================================================
# PROFILE SELECTION
# =============================================================================
PROFILE="${1:-APP}"  # default to APP if not provided
echo "Selected build profile: ${PROFILE}"
echo ""

if [[ ! "$PROFILE" =~ ^(APP|TEST|HPC)$ ]]; then
  echo "ERROR: Invalid profile: ${PROFILE}"
  echo "Usage: $0 [APP|TEST|HPC]"
  exit 1
fi

# =============================================================================
# DETERMINE BUILD PARALLELISM
# =============================================================================

# Detect core count per platform
if [ "$PLATFORM" = "macOS" ]; then
  CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
elif [ "$PLATFORM" = "Linux" ]; then
  CORES=$(nproc 2>/dev/null || echo 4)
else
  CORES=${NUMBER_OF_PROCESSORS:-4}
fi

# Allow manual override: US3_BUILD_JOBS=<N>
if [ -n "${US3_BUILD_JOBS:-}" ]; then
  BUILD_JOBS="$US3_BUILD_JOBS"
else
  # Default to ~90% of cores, but at least 1
  BUILD_JOBS=$((CORES * 9 / 10))
  if [ "$BUILD_JOBS" -lt 1 ]; then
    BUILD_JOBS=1
  fi
fi

echo "Detected $CORES cores; using $BUILD_JOBS parallel build jobs."
echo ""

# Tell vcpkg to use the same limit
export VCPKG_MAX_CONCURRENCY="$BUILD_JOBS"

# =============================================================================
# CHECK REQUIRED TOOLS
# =============================================================================
REQUIRED_TOOLS=(cmake git)

# Platform-specific requirements
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
# OPTIONAL: Xcode 15 SETUP ON macOS
# =============================================================================
if [ "$PLATFORM" = "macOS" ]; then
  CURRENT_XCODE_PATH=$(xcode-select -p || echo "")
  DESIRED_XCODE_PATH="/Applications/Xcode-15.app/Contents/Developer"

  echo "Checking Xcode configuration..."
  echo "Current Xcode path: ${CURRENT_XCODE_PATH:-<not set>}"
  echo "Desired Xcode path: $DESIRED_XCODE_PATH"
  echo ""

  if [ -d "$DESIRED_XCODE_PATH" ]; then
    if [ "$CURRENT_XCODE_PATH" != "$DESIRED_XCODE_PATH" ]; then
      echo "Xcode 15 is installed but not active."
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
      echo "Xcode 15 is already active."
    fi
  else
    echo "Xcode 15 not found at:"
    echo "  $DESIRED_XCODE_PATH"
    echo ""
    echo "Please install Xcode 15 from the App Store or developer.apple.com."
    echo ""
    echo "Your current Xcode installation will remain untouched."
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

# Allow override via US3_VCPKG_ROOT; default to a central vcpkg in the home dir
US3_VCPKG_ROOT="${US3_VCPKG_ROOT:-$HOME/vcpkg}"

echo ""
echo "Using central vcpkg root: $US3_VCPKG_ROOT"

# Basic validation / clone if needed
if [ -d "$US3_VCPKG_ROOT" ] && [ ! -d "$US3_VCPKG_ROOT/.git" ]; then
  echo "ERROR: $US3_VCPKG_ROOT exists but is not a vcpkg git clone."
  echo "Either set US3_VCPKG_ROOT to a different path or move/rename that directory."
  exit 1
fi

if [ ! -d "$US3_VCPKG_ROOT/.git" ]; then
  echo "vcpkg not found at $US3_VCPKG_ROOT, cloning..."
  git clone https://github.com/microsoft/vcpkg.git "$US3_VCPKG_ROOT"
fi

# Bootstrap vcpkg if the executable is missing
if [ ! -x "$US3_VCPKG_ROOT/vcpkg" ]; then
  echo ""
  echo "Bootstrapping vcpkg at $US3_VCPKG_ROOT..."
  if [[ "$PLATFORM" == "Windows" ]]; then
    ( cd "$US3_VCPKG_ROOT" && ./bootstrap-vcpkg.bat )
  else
    "$US3_VCPKG_ROOT/bootstrap-vcpkg.sh"
  fi
fi

# Enable local binary caching
export VCPKG_ROOT="$US3_VCPKG_ROOT"
export VCPKG_BINARY_SOURCES="clear;files,$HOME/.vcpkg-cache,readwrite"
mkdir -p "$HOME/.vcpkg-cache"

# centralize installed libraries here
export VCPKG_INSTALLED_DIR="$US3_VCPKG_ROOT/installed"

# CMake will use this toolchain file
VCPKG_TOOLCHAIN_FILE="$US3_VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "$VCPKG_TOOLCHAIN_FILE" ]; then
  echo "ERROR: vcpkg toolchain file not found at $VCPKG_TOOLCHAIN_FILE"
  exit 1
fi

echo "=========================================="
echo "UltraScan3 Bootstrap Steps"
echo "=========================================="
echo "  1. Ensure platform toolchain is ready"
echo "  2. Build Qt, Qwt, and other dependencies via vcpkg"
echo "  3. Configure and build UltraScan3 via CMake preset: $PRESET"
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
echo "Ready to build UltraScan3 with preset: $PRESET"
echo "  Profile: ${PROFILE}"
echo "  Platform: ${PLATFORM}"
echo ""
echo "Steps:"
echo "  1. Configure CMake with vcpkg toolchain"
echo "  2. Build other dependencies (~10 min)"
echo "  3. Build UltraScan3 (~5 min)"
echo ""
if [ "$NON_INTERACTIVE" = false ]; then
  echo "Grab a coffee - this is a one-time cost!"
fi
echo ""

# =============================================================================
# CONFIGURE AND BUILD
# =============================================================================
echo "Configuring..."
cmake --preset "$PRESET" \
  -DUS3_PROFILE="${PROFILE}" \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN_FILE" \
  -DVCPKG_ROOT="$US3_VCPKG_ROOT" \
  -DVCPKG_INSTALLED_DIR="$VCPKG_INSTALLED_DIR"

echo ""
echo "Building..."
cmake --build --preset "$PRESET" --parallel "$BUILD_JOBS"

echo ""
echo "=========================================="
echo "Build complete!"
echo "=========================================="
echo ""
if [ "$NON_INTERACTIVE" = false ]; then
  echo "Next time you build, it will be much faster"
  echo "since dependencies are cached."
  echo ""
fi
