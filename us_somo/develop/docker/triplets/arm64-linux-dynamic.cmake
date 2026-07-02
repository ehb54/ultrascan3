# Release-only dynamic Linux triplet (arm64) for the SOMO docker build.
# For building natively on Apple Silicon / arm64 Linux hosts. See the x64
# variant for rationale.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_BUILD_TYPE release)
