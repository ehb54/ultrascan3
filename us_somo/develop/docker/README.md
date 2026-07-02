# SOMO CMake/vcpkg build in Docker

A **minimal Linux build environment** for the SOMO CMake port. vcpkg builds
Qt5/Qt6 + qwt 6.3.0 + qwtplot3d from source; the image only carries the
toolchain, vcpkg, and the system `-dev` libraries Qt links against. This is
deliberately *not* one of the `admin/release/*` packaging images (those build
all of UltraScan and install the distro Qt/qwt).

## Usage

```sh
# 1. Build the environment image (once; ~3 min)
docker build -t somo-cmake-env us_somo/develop/docker

# 2. Configure + build SOMO (bind-mounts the repo; first run ~40 min while
#    vcpkg builds Qt, then minutes thanks to the cached binary volume)
us_somo/develop/docker/build.sh qt5     # Qt5 (default)
us_somo/develop/docker/build.sh qt6     # Qt6 (selects the qt6 vcpkg feature)
```

Artifacts land in `us_somo/develop/build-docker/<feature>/{bin,lib}` (gitignored).
The vcpkg binary cache persists in the `somo-vcpkg-cache` docker volume, so
re-runs and the qt5↔qt6 switch don't rebuild Qt from scratch.

## Notes

- `build.sh` picks the triplet from the container arch: `x64-linux-dynamic` on
  amd64, `arm64-linux-dynamic` on arm64. Dynamic (not the default static
  `x64-linux`) because SOMO needs shared Qt.
- Parallelism defaults to all logical CPUs (`JOBS=` to override). The i9 host
  reports 16 (hyperthreading). An Apple-Silicon (M1) host has fewer cores but is
  much faster per core — build there natively (arm64 image, `arm64-linux-dynamic`)
  rather than emulating amd64.
- The generated `include/us_version.h` / `include/us_revision.h` (from
  `version.sh` / `revision.sh`, which need git + the repo root) are produced by
  the `us_somo_genheaders` CMake target and are gitignored.
