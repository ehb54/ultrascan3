# SOMO CMake/vcpkg build in Docker

A **minimal Linux build environment** for the SOMO CMake port. vcpkg builds
Qt and the rest of the root manifest's `qt5-app` / `qt6-app` feature (qwt
6.3.0 included) from source; the image only carries the toolchain, vcpkg, and
the system `-dev` libraries Qt links against. This is deliberately *not* one
of the `admin/release/*` packaging images (those build all of UltraScan and
install the distro Qt/qwt).

## Usage

```sh
# 1. Build the environment image (once; a few minutes)
docker build -t somo-cmake-env us_somo/develop/docker

# 2. Configure + build (bind-mounts the repo)
us_somo/develop/docker/build.sh qt5        # SOMO only, preset vcpkg-qt5 (default)
us_somo/develop/docker/build.sh qt6        # SOMO only, preset vcpkg-qt6
us_somo/develop/docker/build.sh root-qt5   # whole repo, linux-release-qt5 preset, US3_BUILD_SOMO=ON
us_somo/develop/docker/build.sh root-qt6   # whole repo, linux-release-qt6 preset, US3_BUILD_SOMO=ON
```

SOMO-only artifacts land in `us_somo/develop/build/vcpkg-qt5|qt6/{bin,lib}`,
root builds in `build/linux-release-qt5|qt6/{bin,lib}` (both gitignored). The
root modes build only the SOMO targets.

The first build of each Qt takes a long time while vcpkg compiles it; the
binary cache in the `somo-vcpkg-cache` docker volume makes later builds
restore it instead. All modes install the same packages, so a SOMO-only build
and a root build with the same Qt share the cache.

## Notes

- All modes use the root's vcpkg setup: the root manifest (`qt5-app` /
  `qt6-app` feature), the toolchain wrapper and the overlay triplets
  (`admin/cmake/toolchain.cmake`, `admin/cmake/triplets`), so SOMO is built the
  way the rest of UltraScan is: the wrapper picks `arm64-linux` or
  `x64-linux-dynamic` from the container's architecture (both dynamic and
  release-only), installs host tools into the target tree, and keeps each
  build's vcpkg packages in its own build directory.
- Build natively for the host (an arm64 image on Apple Silicon) rather than
  emulating amd64.
- `JOBS=` overrides the parallelism (default: all logical CPUs). Lower it if
  the Docker VM runs short of memory while compiling Qt.
- The generated `include/us_version.h` / `include/us_revision.h` are written
  by the `us_somo_genheaders` CMake target (`cmake/SomoGenHeaders.cmake`, which
  reads git and the repo root, as `version.sh` / `revision.sh` do for qmake)
  and are gitignored.
