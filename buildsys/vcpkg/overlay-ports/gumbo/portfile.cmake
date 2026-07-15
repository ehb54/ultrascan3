vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# Upstream port downloads a Codeberg tag-archive tarball and pins its SHA512.
# Codeberg (Forgejo) regenerates those archives on demand rather than serving
# a byte-stable file, so the pinned hash drifts out of date over time and the
# download fails with "unexpected hash" (observed on both 0.12.3 and the
# upstream-bumped 0.13.2). Fetching by commit SHA via git sidesteps this,
# since git objects are content-addressed and immutable.
vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL "https://codeberg.org/gumbo-parser/gumbo-parser.git"
    REF cab9cc5548ccaa33fa436e9acbba6ef3a0d8e5d6
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-gumbo CONFIG_PATH share/unofficial-gumbo)

vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/doc/COPYING")
