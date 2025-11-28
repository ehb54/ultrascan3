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
PROFILE="APP"  # default profile

# Parse options
while [[ $# -gt 0 ]]; do
  case $1 in
    --clean)
      CLEAN=true
      shift
      ;;
    --help)
      echo "Usage: $0 [OPTIONS] [PROFILE]"
      echo ""
      echo "OPTIONS:"
      echo "  --clean       Clean build artifacts before building"
      echo "  --help        Show this help message"
      echo ""
      echo "PROFILE:"
      echo "  APP           Desktop/user build (GUI + programs + DB) [default]"
      echo "  TEST          Dev/CI build (programs + tests, prefer static libs)"
      echo "  HPC           Headless / no DB / no GUI"
      echo ""
      echo "EXAMPLES:"
      echo "  $0                  # Build with APP profile"
      echo "  $0 TEST             # Build with TEST profile"
      echo "  $0 --clean          # Clean and build with APP profile"
      echo "  $0 --clean TEST     # Clean and build with TEST profile"
      echo ""
      echo "ENVIRONMENT VARIABLES:"
      echo "  US3_BUILD_JOBS      Override number of parallel build jobs"
      echo "  US3_VCPKG_ROOT      Override vcpkg location (default: ./vcpkg)"
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

  # Check for Xcode 15 in either location
  XCODE_15_PATH="/Applications/Xcode-15.app/Contents/Developer"
  XCODE_DEFAULT_PATH="/Applications/Xcode.app/Contents/Developer"

  echo "Checking Xcode configuration..."
  echo "Current Xcode path: ${CURRENT_XCODE_PATH:-<not set>}"
  echo ""

  # Determine which Xcode 15 path exists (if any)
  DESIRED_XCODE_PATH=""
  if [ -d "$XCODE_15_PATH" ]; then
    DESIRED_XCODE_PATH="$XCODE_15_PATH"
  elif [ -d "$XCODE_DEFAULT_PATH" ]; then
    # Verify it's actually Xcode 15.x or 16.x
    XCODE_VERSION=$("$XCODE_DEFAULT_PATH/usr/bin/xcodebuild" -version 2>/dev/null | head -n1 | awk '{print $2}' || echo "0")
    XCODE_MAJOR=$(echo "$XCODE_VERSION" | cut -d. -f1)

    if [ "$XCODE_MAJOR" = "15" ] || [ "$XCODE_MAJOR" = "16" ]; then
      DESIRED_XCODE_PATH="$XCODE_DEFAULT_PATH"
      echo "Found Xcode $XCODE_VERSION at default location"
    fi
  fi

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
    echo "Xcode 15 or 16 not found at either:"
    echo "  $XCODE_15_PATH"
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
# VCPKG SETUP - Use local vcpkg in project
# =============================================================================

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Use local vcpkg by default, allow override via US3_VCPKG_ROOT
US3_VCPKG_ROOT="${US3_VCPKG_ROOT:-$SCRIPT_DIR/vcpkg}"

echo ""
echo "Using vcpkg root: $US3_VCPKG_ROOT"

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

# =============================================================================
# CLEAN BUILD ARTIFACTS (if requested)
# =============================================================================
if [ "$CLEAN" = true ]; then
  echo "=========================================="
  echo "Cleaning build artifacts..."
  echo "=========================================="

  # Clean CMake build directory
  if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
  fi

  # Clean vcpkg build artifacts (but keep installed packages)
  # This allows faster rebuilds while still being "clean"
  if [ -d "$US3_VCPKG_ROOT/buildtrees" ]; then
    echo "Removing vcpkg build trees..."
    rm -rf "$US3_VCPKG_ROOT/buildtrees"
  fi

  # Optionally remove installed packages for a completely clean build
  # Uncomment if you want a full clean (slower but more thorough)
  # if [ -d "$US3_VCPKG_ROOT/installed" ]; then
  #   echo "Removing vcpkg installed packages..."
  #   rm -rf "$US3_VCPKG_ROOT/installed"
  # fi

  # if [ -d "$US3_VCPKG_ROOT/packages" ]; then
  #   echo "Removing vcpkg packages..."
  #   rm -rf "$US3_VCPKG_ROOT/packages"
  # fi

  echo "Clean complete!"
  echo ""
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
echo "  Clean build: ${CLEAN}"
echo ""
echo "Steps:"
echo "  1. Configure CMake with vcpkg toolchain"
echo "  2. Build dependencies (~10 min first time)"
echo "  3. Build UltraScan3 (~5 min)"
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
  echo "To rebuild from scratch: $0 --clean"
  echo ""
fi
