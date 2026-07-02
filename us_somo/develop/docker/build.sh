#!/usr/bin/env bash
# =============================================================================
# Drive a SOMO CMake/vcpkg build inside the somo-cmake-env container.
# Run from the repo root (or anywhere): us_somo/develop/docker/build.sh [qt5|qt6]
#
# Bind-mounts the repo read-write (vcpkg + version scripts need git; the
# generated headers are gitignored) and persists the vcpkg binary cache in a
# named volume so the ~40-min first Qt build happens only once.
# =============================================================================
set -euo pipefail

FEATURE="${1:-qt5}"                     # qt5 (default) | qt6
IMAGE="${IMAGE:-somo-cmake-env}"
CACHE_VOL="${CACHE_VOL:-somo-vcpkg-cache}"

# repo root = three levels up from this script (docker/ -> develop/ -> us_somo/ -> root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# Use all logical CPUs by default (this i9 reports 16 with hyperthreading).
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"

echo "repo root : $REPO_ROOT"
echo "feature   : $FEATURE"
echo "jobs      : $JOBS"
echo "cache vol : $CACHE_VOL"

docker volume create "$CACHE_VOL" >/dev/null

docker run --rm -i \
  -v "$REPO_ROOT:/src" \
  -v "$CACHE_VOL:/vcpkg-cache" \
  -e "FEATURE=$FEATURE" -e "JOBS=$JOBS" \
  "$IMAGE" bash -euo pipefail -c '
    ARCH="$(uname -m)"
    case "$ARCH" in
      x86_64)          TRIPLET=x64-linux-dynamic ;;
      aarch64|arm64)   TRIPLET=arm64-linux-dynamic ;;
      *) echo "unknown arch $ARCH"; exit 1 ;;
    esac
    export VCPKG_MAX_CONCURRENCY="$JOBS"
    export VCPKG_DEFAULT_BINARY_CACHE=/vcpkg-cache

    SRC=/src/us_somo/develop
    BUILD="/src/us_somo/develop/build-docker/$FEATURE"

    FEATURE_ARGS=()
    if [ "$FEATURE" = "qt6" ]; then
      FEATURE_ARGS+=( -DVCPKG_MANIFEST_NO_DEFAULT_FEATURES=ON -DVCPKG_MANIFEST_FEATURES=qt6 )
    fi

    echo "=== configuring ($TRIPLET) ==="
    cmake -S "$SRC" -B "$BUILD" -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET="$TRIPLET" \
      -DVCPKG_INSTALLED_DIR="/src/us_somo/develop/build-docker/vcpkg_installed" \
      "${FEATURE_ARGS[@]}"

    echo "=== building ==="
    cmake --build "$BUILD" --parallel "$JOBS"
    echo "=== done: artifacts in $BUILD/{bin,lib} ==="
  '
