# =============================================================================
# at-spi2-core — no-op overlay port for UltraScan3
# =============================================================================
# The upstream at-spi2-core port requires libXtst (the X11 XTEST extension)
# from the system package manager. On Rocky/RHEL 8 this header is in
# libXtst-devel which is not available in BaseOS/AppStream/EPEL without
# additional repo configuration, and vcpkg hard-errors when meson cannot
# find xtst via pkg-config.
#
# at-spi2-core is the Linux accessibility bus (AT-SPI2). Qt5 uses it
# optionally for accessibility plugin support. UltraScan3 does not require
# accessibility features, so we stub the port out entirely.
#
# The port installs nothing but satisfies vcpkg's dependency resolution so
# the Qt5 build proceeds without attempting to build at-spi2-core from source.
# =============================================================================

set(VCPKG_BUILD_TYPE release)  # no debug artifacts either

# Nothing to download, configure, build, or install.
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
