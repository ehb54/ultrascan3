# admin/cmake/toolchain.cmake

# =============================================================================
# Resolve vcpkg root
# =============================================================================
if(DEFINED ENV{VCPKG_ROOT})
    set(_VCPKG_ROOT "$ENV{VCPKG_ROOT}")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(_VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")
    message(STATUS "Using in-source vcpkg")
elseif(EXISTS "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(_VCPKG_ROOT "$ENV{HOME}/vcpkg")
    message(STATUS "Using vcpkg at ~/vcpkg")
else()
    message(FATAL_ERROR "vcpkg not found. Set VCPKG_ROOT, place vcpkg/ in source tree, or install vcpkg to ~/vcpkg")
endif()

# =============================================================================
# Set triplet based on platform and architecture
# Only set if not already specified by a preset or the user.
# =============================================================================
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        execute_process(
                COMMAND uname -m
                OUTPUT_VARIABLE _HOST_ARCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(_HOST_ARCH STREQUAL "arm64")
            set(VCPKG_TARGET_TRIPLET "arm64-osx-dynamic" CACHE STRING "vcpkg triplet")
        else()
            set(VCPKG_TARGET_TRIPLET "x64-osx-dynamic" CACHE STRING "vcpkg triplet")
        endif()
    endif()

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        execute_process(
                COMMAND uname -m
                OUTPUT_VARIABLE _HOST_ARCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(_HOST_ARCH MATCHES "^(aarch64|arm64)$")
            set(VCPKG_TARGET_TRIPLET "arm64-linux" CACHE STRING "vcpkg triplet")
            set(ENV{PKG_CONFIG_PATH} "/usr/lib/aarch64-linux-gnu/pkgconfig:$ENV{PKG_CONFIG_PATH}")
        else()
            set(VCPKG_TARGET_TRIPLET "x64-linux-dynamic" CACHE STRING "vcpkg triplet")
            set(ENV{PKG_CONFIG_PATH} "/usr/lib/x86_64-linux-gnu/pkgconfig:$ENV{PKG_CONFIG_PATH}")
        endif()
    endif()

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg triplet")
    endif()

else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

# Native builds use one triplet for target libraries and host tools.
# This prevents duplicate dependency graphs and keeps Qt tools in the target tree.
if(NOT DEFINED VCPKG_HOST_TRIPLET)
    set(VCPKG_HOST_TRIPLET "${VCPKG_TARGET_TRIPLET}" CACHE STRING "vcpkg host triplet")
endif()

message(STATUS "Platform: ${CMAKE_HOST_SYSTEM_NAME}, triplet: ${VCPKG_TARGET_TRIPLET}, host triplet: ${VCPKG_HOST_TRIPLET}")

# =============================================================================
# vcpkg installed dir: left at vcpkg's manifest-mode default,
# ${CMAKE_BINARY_DIR}/vcpkg_installed.
#
# A tree under VCPKG_ROOT cannot be shared between build trees: manifest mode
# makes it match the CURRENT VCPKG_MANIFEST_FEATURES exactly, so an HPC
# configure uninstalls the APP profile's qwt and vice versa, and each
# invalidates the other's configure stamp. Keeping packages in the build tree
# removes the sharing that made that possible. They come from the binary cache
# (VCPKG_BINARY_SOURCES), so a new tree restores rather than rebuilds.
# =============================================================================
# string(FIND), not MATCHES: the needle is a path, so a regex metacharacter in
# it (+, (, a Windows backslash) would silently fail to match. Prefix rather
# than equality, to catch the feature-keyed variants (installed-qt6-app, ...).
set(_us3_shared_installed_dir "${_VCPKG_ROOT}/installed")
if(DEFINED VCPKG_INSTALLED_DIR AND NOT "${VCPKG_INSTALLED_DIR}" STREQUAL "")
    # Migrate caches written while a shared tree was in use.
    string(FIND "${VCPKG_INSTALLED_DIR}" "${_us3_shared_installed_dir}" _us3_shared_pos)
    if(_us3_shared_pos EQUAL 0)
        unset(VCPKG_INSTALLED_DIR CACHE)
        unset(_VCPKG_INSTALLED_DIR CACHE)
        message(STATUS "vcpkg installed dir: migrated to the build-local default")
    endif()
endif()

# =============================================================================
# Include vcpkg
# =============================================================================
include("${_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
