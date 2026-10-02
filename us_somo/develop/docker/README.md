# SOMO CMake/vcpkg build in Docker

A **minimal Linux build environment** for the SOMO CMake port. vcpkg builds
Qt5/Qt6 + qwt 6.3.0 + qwtplot3d from source; the image only carries the
toolchain, vcpkg, and the system `-dev` libraries Qt links against. This is
deliberately *not* one of the `admin/release/*` packaging images (those build
all of UltraScan and install the distro Qt/qwt).

## Usage

```sh
# 1. Build the environment image (once; a few minutes)
docker build -t somo-cmake-env us_somo/develop/docker

# 2. Configure + build (bind-mounts the repo)
us_somo/develop/docker/build.sh qt5        # standalone SOMO, Qt5 (default)
us_somo/develop/docker/build.sh qt6        # standalone SOMO, Qt6
us_somo/develop/docker/build.sh root-qt5   # whole repo, linux-release-qt5 preset, US3_BUILD_SOMO=ON
us_somo/develop/docker/build.sh root-qt6   # whole repo, linux-release-qt6 preset, US3_BUILD_SOMO=ON
```

Standalone artifacts land in `us_somo/develop/build-docker/<mode>/{bin,lib}`,
root builds in `build/linux-release-qt5|qt6/{bin,lib}` (both gitignored). The
root modes build only the SOMO targets.

The first build of each Qt takes a long time while vcpkg compiles it; the
binary cache in the `somo-vcpkg-cache` docker volume makes later builds, and
other modes using the same Qt, restore it instead.

## Notes

- All modes use the root's vcpkg toolchain wrapper and overlay triplets
  (`admin/cmake/toolchain.cmake`, `admin/cmake/triplets`), so SOMO is built the
  way the rest of UltraScan is: the wrapper picks `arm64-linux` or
  `x64-linux-dynamic` from the container's architecture (both dynamic and
  release-only), installs host tools into the target tree, and keeps each
  build's vcpkg packages in its own build directory.
- Build natively for the host (an arm64 image on Apple Silicon) rather than
  emulating amd64.
- `JOBS=` overrides the parallelism (default: all logical CPUs). Lower it if
  the Docker VM runs short of memory while compiling Qt.
- The generated `include/us_version.h` / `include/us_revision.h` (from
  `version.sh` / `revision.sh`, which need git + the repo root) are produced by
  the `us_somo_genheaders` CMake target and are gitignored.
