set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_BUILD_TYPE release)

# Prevent vcpkg's qt_install_submodule.cmake from injecting
# -DCMAKE_OSX_DEPLOYMENT_TARGET=14 into the inner qtbase configure.
# Qt interprets any non-empty CMAKE_OSX_DEPLOYMENT_TARGET on a non-Darwin
# host as an Android cross-compile indicator, breaking the xcb feature probe.
set(VCPKG_OSX_DEPLOYMENT_TARGET "")

# Pass Qt feature overrides directly into every port's cmake configure.
# - FEATURE_xcb_syslibs=ON: bootstrap-linux.sh installs all required xcb-util-*
#   packages; this tells Qt the syslibs condition is satisfied.
#
# Rocky/RHEL 8 ships Python 3.6 as /usr/bin/python3, which is below meson's
# minimum requirement of 3.7. bootstrap-linux.sh installs python3.9 alongside
# the system python. We tell vcpkg-tool-meson's portfile exactly which binary
# to use via the PYTHON3 cache variable so CMake find_program() doesn't pick
# up the system 3.6 regardless of PATH ordering.
if(EXISTS "/usr/bin/python3.9")
    set(VCPKG_CMAKE_CONFIGURE_OPTIONS
        -DFEATURE_xcb_syslibs=ON
        -DPYTHON3=/usr/bin/python3.9
    )
else()
    set(VCPKG_CMAKE_CONFIGURE_OPTIONS
        -DFEATURE_xcb_syslibs=ON
    )
endif()
