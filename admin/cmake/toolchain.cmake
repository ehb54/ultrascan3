# admin/cmake/toolchain.cmake

# =============================================================================
# Resolve vcpkg root
# =============================================================================
if(DEFINED ENV{VCPKG_ROOT})
    set(_VCPKG_ROOT "$ENV{VCPKG_ROOT}")
elseif(EXISTS "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(_VCPKG_ROOT "$ENV{HOME}/vcpkg")
    message(STATUS "Using vcpkg at ~/vcpkg")
else()
    message(FATAL_ERROR "vcpkg not found. Set VCPKG_ROOT or install vcpkg to ~/vcpkg")
endif()

# =============================================================================
# Set triplet based on platform and architecture
# =============================================================================
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")
    execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE _HOST_ARCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_HOST_ARCH STREQUAL "arm64")
        set(VCPKG_TARGET_TRIPLET "arm64-osx-dynamic" CACHE STRING "vcpkg triplet" FORCE)
    else()
        set(VCPKG_TARGET_TRIPLET "x64-osx-dynamic" CACHE STRING "vcpkg triplet" FORCE)
    endif()

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(VCPKG_TARGET_TRIPLET "x64-linux-dynamic" CACHE STRING "vcpkg triplet" FORCE)

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg triplet" FORCE)

else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

message(STATUS "Platform: ${CMAKE_HOST_SYSTEM_NAME}, triplet: ${VCPKG_TARGET_TRIPLET}")

# =============================================================================
# Include vcpkg
# =============================================================================
include("${_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")