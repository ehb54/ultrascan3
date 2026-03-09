# qt5-tools overlay port
#
# PURPOSE
# -------
# The upstream qt5-tools@5.15.17#1 port does not declare an "assistant"
# feature in its vcpkg.json, which causes vcpkg to reject the
#   { "name": "qt5-tools", "features": ["assistant"] }
# dependency in our vcpkg.json with:
#   "does not have required feature assistant needed by ultrascan3"
#
# Qt5 builds Qt Assistant as part of qttools unconditionally when the
# required Qt modules (network, sql-sqlite, widgets) are present -- there
# is no separate enable/disable switch needed.  The overlay vcpkg.json
# simply declares the feature so vcpkg's feature resolution accepts it;
# the actual build is unchanged.
#
# The portfile delegates entirely to the upstream qt_submodule_installation
# machinery with no patches, so the build is byte-for-byte identical to
# what the upstream port would produce.

include("${CURRENT_INSTALLED_DIR}/share/qt5/qt_port_functions.cmake")
qt_submodule_installation()
