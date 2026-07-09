vcpkg_from_sourceforge(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO qwt/qwt
    REF ${VERSION}
    FILENAME "qwt-${VERSION}.zip"
    SHA512 4008c3e4dace0f18e572b473a51a293bb896abbd62b9c5f0a92734b2121923d2e2cbf67c997b84570a13bf4fdd7669b56497c82fbae35049ed856b2f0a65e475
    PATCHES
        config.patch
        fix_dll_install.patch
)

# Qt6 debug libraries are not available via vcpkg on macOS (no debug dylibs built)
if(VCPKG_TARGET_IS_OSX)
    set(VCPKG_BUILD_TYPE release)
endif()

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" IS_DYNAMIC)
set(OPTIONS "")
if(IS_DYNAMIC)
    list(APPEND OPTIONS "QWT_CONFIG+=QwtDll")
endif()

if(VCPKG_TARGET_IS_OSX)
    list(APPEND OPTIONS
            "QMAKE_LIBS_X11="
            "QMAKE_LIBDIR_X11="
            "QMAKE_INCDIR_X11="
            "QMAKE_RPATHDIR_X11="
    )
endif()

if(VCPKG_TARGET_IS_OSX)
    string(REPLACE "-dynamic" "" _static_triplet_dir "${CURRENT_INSTALLED_DIR}")
    # Scrub invalid X11R6 path baked into Qt6 .prl files and .pri modules
    foreach(_lib_dir "${CURRENT_INSTALLED_DIR}/lib" "${_static_triplet_dir}/lib")
        file(GLOB _prl_files "${_lib_dir}/*.prl")
        foreach(_f IN LISTS _prl_files)
            file(READ "${_f}" _c)
            if(_c MATCHES "/usr/X11R6/lib")
                string(REPLACE " /usr/X11R6/lib" "" _c "${_c}")
                string(REPLACE ";/usr/X11R6/lib" "" _c "${_c}")
                file(WRITE "${_f}" "${_c}")
            endif()
        endforeach()
    endforeach()
    foreach(_pri_dir
            "${CURRENT_INSTALLED_DIR}/share/Qt6/mkspecs/modules"
            "${_static_triplet_dir}/share/Qt6/mkspecs/modules"
    )
        set(_pri "${_pri_dir}/qt_lib_gui_private.pri")
        if(EXISTS "${_pri}")
            file(READ "${_pri}" _c)
            if(_c MATCHES "/usr/X11R6/lib")
                string(REPLACE "QMAKE_LIBS_OPENGL = /usr/X11R6/lib" "QMAKE_LIBS_OPENGL =" _c "${_c}")
                file(WRITE "${_pri}" "${_c}")
            endif()
        endif()
    endforeach()
endif()

vcpkg_qmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        QMAKE_OPTIONS
        ${OPTIONS}
        "CONFIG-=debug_and_release"
        "CONFIG+=create_prl"
        "CONFIG+=link_prl"
)

vcpkg_qmake_install()
vcpkg_copy_pdbs()
# Force a version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CURRENT_PACKAGES_DIR}/share/unofficial-qwt/unofficial-qwt-config-version.cmake"
        VERSION ${VERSION}
        COMPATIBILITY AnyNewerVersion
)
# Qt6 pkg-config files not installed https://github.com/microsoft/vcpkg/issues/25988
# vcpkg_fixup_pkgconfig()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib/pkgconfig" "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig")

if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/debug/bin")
endif()

file(COPY "${CMAKE_CURRENT_LIST_DIR}/unofficial-qwt-config.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/unofficial-qwt")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
