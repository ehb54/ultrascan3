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
# MACOS SDK DETECTION (Qt5 requires SDK 14)
# =============================================================================
if [[ "$PLATFORM" == "macOS" ]]; then
  echo "Detecting macOS SDK for Qt5 compatibility..."

  SDK_FOUND=false

  # Try multiple SDK locations in order of preference
  SDK_PATHS=(
    "/Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk"
    "/Applications/Xcode-15.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.sdk"
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.5.sdk"
  )

  for SDK_PATH in "${SDK_PATHS[@]}"; do
    if [ -d "$SDK_PATH" ]; then
      echo "Found compatible SDK: $SDK_PATH"
      export SDKROOT="$SDK_PATH"
      export CMAKE_OSX_SYSROOT="$SDK_PATH"
      SDK_FOUND=true

      # Switch xcode-select if using Xcode-15
      if [[ "$SDK_PATH" == *"Xcode-15.app"* ]]; then
        CURRENT_XCODE=$(xcode-select -p)
        DESIRED_XCODE="/Applications/Xcode-15.app/Contents/Developer"

        if [[ "$CURRENT_XCODE" != "$DESIRED_XCODE" ]]; then
          echo "Switching active Xcode to Xcode 15..."
          if sudo xcode-select --switch "$DESIRED_XCODE" 2>/dev/null; then
            echo "✓ Active Xcode switched to Xcode 15"
          else
            echo "WARNING: Could not switch Xcode (needs sudo). Trying with environment variables only..."
          fi
        fi
      fi

      break
    fi
  done

  if [ "$SDK_FOUND" = false ]; then
    CURRENT_SDK=$(xcrun --show-sdk-path 2>/dev/null || echo "unknown")
    CURRENT_VERSION=$(xcrun --show-sdk-version 2>/dev/null || echo "unknown")

    echo ""
    echo "ERROR: No compatible macOS SDK found for Qt5"
    echo ""
    echo "Current SDK: $CURRENT_SDK (version $CURRENT_VERSION)"
    echo ""
    echo "Qt5 requires macOS SDK 14. Please install:"
    echo ""
    echo "Command Line Tools for Xcode 15.3 (~500MB)"
    echo "  1. Download from: https://developer.apple.com/download/all/"
    echo "  2. Search for 'Command Line Tools for Xcode 15.3'"
    echo "  3. Install the .dmg file"
    echo "  4. Run this script again"
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
# INSTALL BUILD TOOLS
# =============================================================================
echo "Installing build tools..."

if command -v brew >/dev/null 2>&1; then
  # macOS - Homebrew
  if [ "$NON_INTERACTIVE" = true ]; then
    HOMEBREW_NO_AUTO_UPDATE=1 brew install ninja autoconf autoconf-archive automake libtool pkg-config
  else
    brew install ninja autoconf autoconf-archive automake libtool pkg-config
  fi

elif command -v apt-get >/dev/null 2>&1; then
  # Debian/Ubuntu
  if [ "$NON_INTERACTIVE" = true ]; then
    export DEBIAN_FRONTEND=noninteractive
  fi
  sudo apt-get update
  sudo apt-get install -y ninja-build autoconf autoconf-archive automake \
    libtool pkg-config build-essential

elif command -v dnf >/dev/null 2>&1; then
  # Fedora/RHEL
  if [ "$NON_INTERACTIVE" = true ]; then
    sudo dnf install -y --assumeyes ninja-build autoconf autoconf-archive automake \
      libtool pkg-config gcc-c++
  else
    sudo dnf install -y ninja-build autoconf autoconf-archive automake \
      libtool pkg-config gcc-c++
  fi

else
  echo "ERROR: No supported package manager found"
  exit 1
fi

# =============================================================================
# VCPKG BOOTSTRAP
# =============================================================================
if [ ! -x "./vcpkg/vcpkg" ]; then
  echo ""
  echo "Bootstrapping vcpkg..."
  ./vcpkg/bootstrap-vcpkg.sh
fi

# Enable local binary caching
export VCPKG_BINARY_SOURCES="clear;files,$HOME/.vcpkg-cache,readwrite"
mkdir -p "$HOME/.vcpkg-cache"

echo ""
echo "=========================================="
echo "Starting build with preset: $PRESET"
echo "=========================================="
echo "This will:"
echo "  1. Download and build Qt5 (~20 min)"
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
cmake --preset "$PRESET"

echo ""
echo "Building..."
cmake --build --preset "$PRESET" --parallel

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