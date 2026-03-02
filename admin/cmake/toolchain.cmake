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

message(STATUS "Platform: ${CMAKE_HOST_SYSTEM_NAME}, triplet: ${VCPKG_TARGET_TRIPLET}")

# =============================================================================
# Set shared vcpkg installed dir BEFORE including vcpkg.cmake so manifest-mode
# doesn't default to ${CMAKE_BINARY_DIR}/vcpkg_installed.
# Priority: explicit -DVCPKG_INSTALLED_DIR > VCPKG_ROOT/installed
# =============================================================================
if(NOT DEFINED VCPKG_INSTALLED_DIR OR "${VCPKG_INSTALLED_DIR}" STREQUAL "")
    set(VCPKG_INSTALLED_DIR "${_VCPKG_ROOT}/installed" CACHE PATH
        "vcpkg installed packages directory" FORCE)
    message(STATUS "Set VCPKG_INSTALLED_DIR to shared dir: ${VCPKG_INSTALLED_DIR}")
endif()

# =============================================================================
# Include vcpkg
# =============================================================================
include("${_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
