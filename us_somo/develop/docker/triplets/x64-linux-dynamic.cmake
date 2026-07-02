# Release-only dynamic Linux triplet for the SOMO docker build.
# Mirrors admin/cmake/triplets/arm64-linux.cmake but for x64, and overrides
# vcpkg's community x64-linux-dynamic to build release only (no debug Qt) —
# roughly halving the first-build time. SOMO needs shared (dynamic) Qt.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_BUILD_TYPE release)
