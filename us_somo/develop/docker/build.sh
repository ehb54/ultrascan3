#!/usr/bin/env bash
# =============================================================================
# Drive a SOMO CMake/vcpkg build inside the somo-cmake-env container.
#
#   us_somo/develop/docker/build.sh [qt5|qt6|root-qt5|root-qt6]
#
#   qt5, qt6            standalone: cmake -S us_somo/develop, with SOMO's own
#                       vcpkg.json (qt5 / qt6 feature)
#   root-qt5, root-qt6  the whole repo with the linux-release-qt5 / -qt6 preset
#                       and -DUS3_BUILD_SOMO=ON, building only the SOMO targets
#
# Every mode uses the root's vcpkg toolchain wrapper and overlay triplets
# (admin/cmake/toolchain.cmake, admin/cmake/triplets): dynamic, release-only,
# host tools in the target tree, as in the main UltraScan build. The wrapper
# keeps each build's vcpkg packages in its own build tree; the binary cache in
# a named volume means only the first build of each Qt compiles it.
#
# Bind-mounts the repo read-write (vcpkg + version scripts need git; the
# generated headers are gitignored).
# =============================================================================
set -euo pipefail

MODE="${1:-qt5}"
IMAGE="${IMAGE:-somo-cmake-env}"
CACHE_VOL="${CACHE_VOL:-somo-vcpkg-cache}"

case "$MODE" in
  qt5|qt6|root-qt5|root-qt6) ;;
  *) echo "usage: $0 [qt5|qt6|root-qt5|root-qt6]" >&2; exit 2 ;;
esac

# repo root = three levels up from this script (docker/ -> develop/ -> us_somo/ -> root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# All logical CPUs by default; lower JOBS if the container runs short of memory.
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"

echo "repo root : $REPO_ROOT"
echo "mode      : $MODE"
echo "jobs      : $JOBS"
echo "cache vol : $CACHE_VOL"

docker volume create "$CACHE_VOL" >/dev/null

docker run --rm -i \
  -v "$REPO_ROOT:/src" \
  -v "$CACHE_VOL:/vcpkg-cache" \
  -e "MODE=$MODE" -e "JOBS=$JOBS" \
  "$IMAGE" bash -euo pipefail -c '
    export VCPKG_ROOT=/opt/vcpkg
    export VCPKG_MAX_CONCURRENCY="$JOBS"
    export VCPKG_DEFAULT_BINARY_CACHE=/vcpkg-cache
    SOMO_TARGETS="us_somo us3_somo us_admin us3_config us_saxs_cmds_t"

    case "$MODE" in
      qt5|qt6)
        BUILD="/src/us_somo/develop/build-docker/$MODE"
        ARGS=( -DUSE_QT6=OFF )
        if [ "$MODE" = "qt6" ]; then
          ARGS=( -DUSE_QT6=ON -DVCPKG_MANIFEST_NO_DEFAULT_FEATURES=ON -DVCPKG_MANIFEST_FEATURES=qt6 )
        fi
        echo "=== configuring standalone SOMO ($MODE) ==="
        cmake -S /src/us_somo/develop -B "$BUILD" -G Ninja \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
          -DCMAKE_TOOLCHAIN_FILE=/src/admin/cmake/toolchain.cmake \
          -DVCPKG_OVERLAY_TRIPLETS=/src/admin/cmake/triplets \
          "${ARGS[@]}"
        echo "=== building ==="
        cmake --build "$BUILD" --parallel "$JOBS"
        ;;
      root-qt5|root-qt6)
        PRESET="linux-release-${MODE#root-}"
        BUILD="/src/build/$PRESET"
        echo "=== configuring the repo root, preset $PRESET, with SOMO ==="
        cd /src
        cmake --preset "$PRESET" \
          -DUS3_BUILD_SOMO=ON -DBUILD_DOCUMENTATION=OFF -DBUILD_TESTING=OFF
        echo "=== building the SOMO targets ==="
        cmake --build "$BUILD" --parallel "$JOBS" --target $SOMO_TARGETS
        ;;
    esac
    echo "=== done: artifacts in $BUILD/{bin,lib} ==="
  '
