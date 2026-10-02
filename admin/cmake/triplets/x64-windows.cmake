set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# Build release dependencies only. Debug application builds use the release CRT
# to remain ABI-compatible with these libraries.
set(VCPKG_BUILD_TYPE release)
