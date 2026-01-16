# Point to the vendored source inside this repo
set(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../../qwtplot3d")

# Verify source exists
if(NOT EXISTS "${SOURCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "qwtplot3d source not found at: ${SOURCE_PATH}")
endif()

# Configure
vcpkg_configure_cmake(
        SOURCE_PATH "${SOURCE_PATH}"
        PREFER_NINJA
        OPTIONS
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
)

# Build
vcpkg_build_cmake()

# Install
vcpkg_install_cmake()

# Create proper config file
file(WRITE "${CURRENT_PACKAGES_DIR}/share/qwtplot3d/qwtplot3d-config.cmake" "
include(CMakeFindDependencyMacro)
find_dependency(Qt5 COMPONENTS Core Gui OpenGL Widgets)

# Find qwt dependency
if(NOT TARGET unofficial::qwt::qwt)
    find_dependency(unofficial-qwt CONFIG REQUIRED)
endif()

add_library(qwtplot3d::qwtplot3d UNKNOWN IMPORTED)

get_filename_component(_IMPORT_PREFIX \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
get_filename_component(_IMPORT_PREFIX \"\${_IMPORT_PREFIX}\" PATH)
get_filename_component(_IMPORT_PREFIX \"\${_IMPORT_PREFIX}\" PATH)

find_library(QWTPLOT3D_LIBRARY_RELEASE NAMES qwtplot3d qwtplot3d-qt5
    PATHS \"\${_IMPORT_PREFIX}/lib\" NO_DEFAULT_PATH)
find_library(QWTPLOT3D_LIBRARY_DEBUG NAMES qwtplot3d qwtplot3d-qt5 qwtplot3dd
    PATHS \"\${_IMPORT_PREFIX}/debug/lib\" NO_DEFAULT_PATH)

if(QWTPLOT3D_LIBRARY_RELEASE)
    set_property(TARGET qwtplot3d::qwtplot3d APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(qwtplot3d::qwtplot3d PROPERTIES IMPORTED_LOCATION_RELEASE \"\${QWTPLOT3D_LIBRARY_RELEASE}\")
endif()

if(QWTPLOT3D_LIBRARY_DEBUG)
    set_property(TARGET qwtplot3d::qwtplot3d APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(qwtplot3d::qwtplot3d PROPERTIES IMPORTED_LOCATION_DEBUG \"\${QWTPLOT3D_LIBRARY_DEBUG}\")
endif()

set_target_properties(qwtplot3d::qwtplot3d PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES \"\${_IMPORT_PREFIX}/include\"
    INTERFACE_LINK_LIBRARIES \"Qt5::Core;Qt5::Gui;Qt5::OpenGL;Qt5::Widgets;unofficial::qwt::qwt\"
)
")

# Cleanup
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE
        "${CURRENT_PACKAGES_DIR}/lib/cmake"
        "${CURRENT_PACKAGES_DIR}/debug/lib/cmake"
        )

# Install license
if(EXISTS "${SOURCE_PATH}/COPYING")
    file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
elseif(EXISTS "${SOURCE_PATH}/LICENSE")
    file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
else()
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "See qwtplot3d source for license\n")
endif()