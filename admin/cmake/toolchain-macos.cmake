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
    message(STATUS "Auto-detected macOS triplet: ${VCPKG_TARGET_TRIPLET}")
endif()

if(DEFINED ENV{VCPKG_ROOT})
    set(_VCPKG_ROOT "$ENV{VCPKG_ROOT}")
else()
    set(_VCPKG_ROOT "$ENV{HOME}/vcpkg")
endif()

include("${_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")