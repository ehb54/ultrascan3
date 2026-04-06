cmake_minimum_required(VERSION 3.21)

# =============================================================================
# VERSION EXTRACTION FROM us_defines.h
# =============================================================================
# Extract version before project() call
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/admin/cmake")
list(APPEND VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/admin/cmake/triplets")
list(APPEND VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/buildsys/vcpkg/overlay-ports")
include(UsDefinesParser)
parse_header("${CMAKE_CURRENT_SOURCE_DIR}/utils/us_defines.h")

# Project definition with extracted version and description
project(UltraScan3
        VERSION ${US3_VERSION_MAJOR}.${US3_VERSION_MINOR}.${US3_VERSION_PATCH}
        DESCRIPTION "${US3_DESCRIPTION}"
        LANGUAGES CXX C
        )

# Make full version string available (including -dev suffix if present)
set(US3_VERSION_STRING ${US3_VERSION})

message(STATUS "UltraScan3 version: ${US3_VERSION_STRING}")
message(STATUS "UltraScan3 description: ${US3_DESCRIPTION}")

# =============================================================================
# VERSION HEADER GENERATION
# =============================================================================
# Generate us/us_revision.h only when Git state changes (not on every build)
set(REVISION_HEADER "${CMAKE_SOURCE_DIR}/programs/us/us_revision.h")

find_package(Git)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    add_custom_target(us_version_headers ALL
            COMMAND ${CMAKE_COMMAND}
            -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
            -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
            -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
            -DUS3_VERSION=${US3_VERSION}
            -DUS3_SOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DUS3_BINARY_DIR=${CMAKE_BINARY_DIR}
            -P ${CMAKE_SOURCE_DIR}/admin/cmake/GenerateVersion.cmake
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Updating us_revision.h"
            VERBATIM
    )
else()
    add_custom_target(us_version_headers)
endif()

include_directories(${CMAKE_BINARY_DIR})

# Set minimum macOS version - binaries will work on this version and newer
if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

# =============================================================================
# CMAKE CONFIGURATION
# =============================================================================
if(USE_QT6)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS ON)
link_directories(${CMAKE_BINARY_DIR}/lib)

# Keep CPack working/output directories inside the build tree
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_BINARY_DIR}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/cpack")
set(CPACK_TOPLEVEL_TAG "")

# Enable deprecation warnings and treat them as errors
#if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#    add_compile_options(
#            -Wdeprecated
#            -Wdeprecated-declarations
#            -Werror=deprecated
#            -Werror=deprecated-declarations
#    )
#elseif(MSVC)
#    add_compile_options(
#            /W4           # Enable level 4 warnings (includes deprecation)
#            /we4996       # Treat deprecation warnings as errors
#    )
#endif()

if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
    add_compile_options(-w)
endif()

# For Qt-specific deprecation warnings
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x060900)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler flags based on qmake configuration
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        add_compile_options(/Zi /Od /DDEBUG)
    else()
        add_compile_options(-g -O0 -DDEBUG)
    endif()
endif()

# Platform-specific compiler flags
if(UNIX AND NOT APPLE)
    add_compile_options(-Wno-unused-local-typedefs)
    # Match qmake flags for better compatibility
    add_compile_options(-pipe -Wall -Wextra)
    add_compile_definitions(_REENTRANT)

    # Optional: Enable LTO for Release builds (matches qmake)
    # Note: LTO can cause longer build times and may need to be disabled for debugging
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        include(CheckIPOSupported)
        check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
        if(IPO_SUPPORTED)
            message(STATUS "Enabling Link-Time Optimization (LTO)")
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        else()
            message(STATUS "LTO not supported: ${IPO_ERROR}")
        endif()
    endif()
elseif(MSVC)
    add_compile_options(/wd4251 /wd4275 /wd4996 /wd4267 /wd4305 /wd4309)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

# Enable Qt's MOC, RCC, and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Enable testing
enable_testing()

# Add our custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/admin/cmake")

# =============================================================================
# CONFIGURATION OPTIONS
# =============================================================================
option(US3_NO_DB "Build without database support (for supercomputer backends)" OFF)
option(US3_BUILD_PROGRAMS "Build application programs" ON)
option(USE_QT6 "Use Qt6 instead of Qt5" OFF)
option(BUILD_DOCUMENTATION "Build Sphinx documentation" ON)

# High-level profile to make builds scriptable:
#   APP  = desktop/user build (GUI + programs + DB)
#   TEST = dev/CI build (programs + tests, prefer static US3 libs)
#   HPC  = headless / no DB / no GUI
set(US3_PROFILE "" CACHE STRING "UltraScan3 build profile: APP | TEST | HPC")
set_property(CACHE US3_PROFILE PROPERTY STRINGS APP TEST HPC)

# Lower-level knob used by gui/programs to decide static vs shared for US3 libs
option(US3_PREFER_STATIC "Prefer UltraScan3 static libraries when available" OFF)
option(BUILD_TESTING "Build Testing" OFF)

# Apply profile -> concrete options
if(US3_PROFILE STREQUAL "APP")
    set(US3_NO_DB OFF CACHE BOOL "" FORCE)
    set(US3_BUILD_PROGRAMS ON CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(US3_PREFER_STATIC OFF CACHE BOOL "" FORCE)
    set(BUILD_DOCUMENTATION ON CACHE BOOL "" FORCE)

elseif(US3_PROFILE STREQUAL "TEST")
    set(US3_NO_DB OFF CACHE BOOL "" FORCE)
    set(US3_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING ON CACHE BOOL "" FORCE)
    set(US3_PREFER_STATIC ON CACHE BOOL "" FORCE)
    set(BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)

elseif(US3_PROFILE STREQUAL "HPC")
    set(US3_NO_DB ON CACHE BOOL "" FORCE)
    set(US3_BUILD_PROGRAMS ON CACHE BOOL "" FORCE)  # us_mpi_analysis is built
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(US3_PREFER_STATIC ON CACHE BOOL "" FORCE)
    set(BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
endif()

# Configuration-based definitions
if(US3_NO_DB)
    add_compile_definitions(NO_DB)
endif()

# Platform-specific definitions
if(UNIX AND NOT APPLE)
    add_compile_definitions(LINUX PLATFORM="intel" TITLE="Intel")
elseif(WIN32)
    add_compile_definitions(__LCC__ PLATFORM="intel" TITLE="Intel")
    add_compile_definitions(QT_SHAREDMEMORY QT_SYSTEMSEMAPHORE WIN32)
    if(NOT US3_PREFER_STATIC)
        add_compile_definitions(US_MAKE_DLL)
    endif()
elseif(APPLE)
    add_compile_definitions(MAC OSX PLATFORM="mac")
endif()

# Platform configuratin
include(${CMAKE_SOURCE_DIR}/admin/cmake/platform-config.cmake)

# =============================================================================
# DEPENDENCY MANAGEMENT
# =============================================================================
# Platform-specific Qt5 handling
if(UNIX AND NOT APPLE)
    # Linux: prefer system Qt5
    list(APPEND CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake")
endif()

# Find Qt
if(USE_QT6)
    if(US3_PROFILE STREQUAL "HPC")
        find_package(Qt6 REQUIRED COMPONENTS
                Core
                Network
                Sql
                Xml
        )
    else()
        find_package(Qt6 REQUIRED COMPONENTS
                Core
                Widgets
                Gui
                Network
                Sql
                Xml
                Svg
                OpenGL
                PrintSupport
                DataVisualization
                OpenGLWidgets
                Multimedia
        )
    endif()

    set(QT_VERSION_MAJOR 6)
    set(QT_VERSION_BASE "Qt6")
    set(QT_VERSION_BASE_LOWER "qt6")

else()
    if(US3_PROFILE STREQUAL "HPC")
        find_package(Qt5 REQUIRED COMPONENTS
                Core
                Network
                Sql
                Xml
        )
    else()
        find_package(Qt5 REQUIRED COMPONENTS
                Core
                Widgets
                Gui
                Network
                Sql
                Xml
                Svg
                OpenGL
                PrintSupport
                DataVisualization
                Multimedia
        )
    endif()

    set(QT_VERSION_MAJOR 5)
    set(QT_VERSION_BASE "Qt5")
    set(QT_VERSION_BASE_LOWER "qt5")

    # -------------------------------------------------------------------------
    # Qt5 + vcpkg macOS dynamic-triplet tool fix
    # -------------------------------------------------------------------------
    if(APPLE AND VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
        string(REPLACE "-dynamic" "" _qt5_tools_triplet "${VCPKG_TARGET_TRIPLET}")
        set(_qt5_tools_dir "${VCPKG_INSTALLED_DIR}/${_qt5_tools_triplet}/tools/qt5/bin")

        if(EXISTS "${_qt5_tools_dir}/moc")
            message(STATUS "Qt5 tools override dir: ${_qt5_tools_dir}")

            if(TARGET Qt5::moc)
                set_target_properties(Qt5::moc PROPERTIES
                        IMPORTED_LOCATION "${_qt5_tools_dir}/moc")
            endif()

            if(TARGET Qt5::uic)
                set_target_properties(Qt5::uic PROPERTIES
                        IMPORTED_LOCATION "${_qt5_tools_dir}/uic")
            endif()

            if(TARGET Qt5::rcc)
                set_target_properties(Qt5::rcc PROPERTIES
                        IMPORTED_LOCATION "${_qt5_tools_dir}/rcc")
            endif()
        else()
            message(WARNING "Expected Qt5 tools dir not found: ${_qt5_tools_dir}")
        endif()
    endif()
endif()

if(NOT TARGET Qt::Core)
    if(USE_QT6)
        add_library(Qt::Core ALIAS Qt6::Core)
        add_library(Qt::Network ALIAS Qt6::Network)
        add_library(Qt::Sql ALIAS Qt6::Sql)
        add_library(Qt::Xml ALIAS Qt6::Xml)

        if(TARGET Qt6::Gui AND NOT TARGET Qt::Gui)
            add_library(Qt::Gui ALIAS Qt6::Gui)
        endif()
        if(TARGET Qt6::Widgets AND NOT TARGET Qt::Widgets)
            add_library(Qt::Widgets ALIAS Qt6::Widgets)
        endif()
        if(TARGET Qt6::Svg AND NOT TARGET Qt::Svg)
            add_library(Qt::Svg ALIAS Qt6::Svg)
        endif()
        if(TARGET Qt6::OpenGL AND NOT TARGET Qt::OpenGL)
            add_library(Qt::OpenGL ALIAS Qt6::OpenGL)
        endif()
        if(TARGET Qt6::PrintSupport AND NOT TARGET Qt::PrintSupport)
            add_library(Qt::PrintSupport ALIAS Qt6::PrintSupport)
        endif()
        if(TARGET Qt6::DataVisualization AND NOT TARGET Qt::DataVisualization)
            add_library(Qt::DataVisualization ALIAS Qt6::DataVisualization)
        endif()
        if(TARGET Qt6::OpenGLWidgets AND NOT TARGET Qt::OpenGLWidgets)
            add_library(Qt::OpenGLWidgets ALIAS Qt6::OpenGLWidgets)
        endif()
        if(TARGET Qt6::Multimedia AND NOT TARGET Qt::Multimedia)
            add_library(Qt::Multimedia ALIAS Qt6::Multimedia)
        endif()
    else()
        add_library(Qt::Core ALIAS Qt5::Core)
        add_library(Qt::Network ALIAS Qt5::Network)
        add_library(Qt::Sql ALIAS Qt5::Sql)
        add_library(Qt::Xml ALIAS Qt5::Xml)

        if(TARGET Qt5::Gui AND NOT TARGET Qt::Gui)
            add_library(Qt::Gui ALIAS Qt5::Gui)
        endif()
        if(TARGET Qt5::Widgets AND NOT TARGET Qt::Widgets)
            add_library(Qt::Widgets ALIAS Qt5::Widgets)
        endif()
        if(TARGET Qt5::Svg AND NOT TARGET Qt::Svg)
            add_library(Qt::Svg ALIAS Qt5::Svg)
        endif()
        if(TARGET Qt5::OpenGL AND NOT TARGET Qt::OpenGL)
            add_library(Qt::OpenGL ALIAS Qt5::OpenGL)
        endif()
        if(TARGET Qt5::PrintSupport AND NOT TARGET Qt::PrintSupport)
            add_library(Qt::PrintSupport ALIAS Qt5::PrintSupport)
        endif()
        if(TARGET Qt5::DataVisualization AND NOT TARGET Qt::DataVisualization)
            add_library(Qt::DataVisualization ALIAS Qt5::DataVisualization)
        endif()
        if(TARGET Qt5::Multimedia AND NOT TARGET Qt::Multimedia)
            add_library(Qt::Multimedia ALIAS Qt5::Multimedia)
        endif()
    endif()
endif()

# ============================================================================
# Qt Dynamic Linking Configuration - ALL PLATFORMS
# ============================================================================

# Verify Qt is not static (with proper error handling)
if(TARGET Qt::Core)
    get_target_property(QT_CORE_TYPE Qt::Core TYPE)
    get_target_property(QT_CORE_LOCATION Qt::Core LOCATION)

    message(STATUS "Qt::Core type: ${QT_CORE_TYPE}")
    message(STATUS "Qt::Core location: ${QT_CORE_LOCATION}")

    if(QT_CORE_TYPE STREQUAL "STATIC_LIBRARY")
        message(FATAL_ERROR "Qt must be built as shared libraries")
    elseif(QT_CORE_TYPE MATCHES "SHARED|MODULE|INTERFACE")
        message(STATUS "Qt is using dynamic/shared libraries - OK")
    else()
        # For UNKNOWN or IMPORTED types, check the actual file
        if(QT_CORE_LOCATION)
            if(QT_CORE_LOCATION MATCHES "\\.(dylib|so|dll)$")
                message(STATUS "Qt appears to be dynamic (based on file extension) - OK")
            else()
                message(WARNING "Qt::Core type is ${QT_CORE_TYPE}, location: ${QT_CORE_LOCATION}")
                message(WARNING "Cannot definitively determine if Qt is static or dynamic - proceeding anyway")
            endif()
        endif()
    endif()
else()
    message(WARNING "Qt::Core target not found - skipping static/dynamic check")
endif()

add_definitions(-DQT_SHARED)
set(QT_STATICPLUGIN 0 CACHE BOOL "Disable static Qt plugins" FORCE)

# vcpkg's FindWrapOpenGL.cmake on macOS spuriously adds /usr/X11R6/lib as a
# link directory (a legacy X11 path that doesn't exist on modern macOS).
# CMake warns about this for every target.  Suppress it by pre-emptively
# marking that path as already handled.
if(APPLE)
    set_property(DIRECTORY APPEND PROPERTY LINK_DIRECTORIES "")
    list(REMOVE_ITEM CMAKE_EXE_LINKER_FLAGS "-L/usr/X11R6/lib")
    # The warning comes from target_link_libraries being given a directory path.
    # We cannot silence it globally, but we can prevent it from being added
    # by overriding the offending variable that FindWrapOpenGL.cmake reads.
    set(X11_X11_LIB "" CACHE STRING "Suppress X11R6 link dir warning" FORCE)
endif()

# Platform-specific Qt configuration
if(APPLE)
    message(STATUS "Configured Qt for dynamic linking on macOS")

    foreach(QT_MODULE Core Network Sql Xml Gui Widgets Svg OpenGL PrintSupport DataVisualization OpenGLWidgets Multimedia)
        if(TARGET Qt::${QT_MODULE})
            get_target_property(QT_MOD_TYPE Qt::${QT_MODULE} TYPE)
            message(STATUS "  Qt::${QT_MODULE} type: ${QT_MOD_TYPE}")
        endif()
    endforeach()

elseif(UNIX)
    message(STATUS "Configured Qt for dynamic linking on Linux")

    # Linux-specific: ensure RPATH is set
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib")

elseif(WIN32)
    message(STATUS "Configured Qt for dynamic linking on Windows")
endif()

message(STATUS "Qt dynamic linking configured for all platforms")

# ============================================================================
# Resources
# ============================================================================
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/admin/cmake/Us3Resources.cmake)

set(US3_ETC_DIR "${CMAKE_SOURCE_DIR}/etc")

# Re-run CMake when any file in etc/ changes (including deletes/creates)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${US3_ETC_DIR}/*"
        "${US3_ETC_DIR}/*/*"
        "${US3_ETC_DIR}/*/*/*"
        )


# Subfolders you want to keep editable on disk (not embedded in the binary)
set(US3_ETC_EXCLUDE_DIRS
        rotors
        dmd
        mysql
        )

# Individual files you want to keep editable on disk
set(US3_ETC_EXCLUDE_FILES
        cluster.config
        somo.cromer68.saxs_atoms)

# Outputs
set(US3_QRC_XML "${CMAKE_BINARY_DIR}/us3_xml_auto.qrc")

us3_generate_qrc_from_dir(
        OUT_QRC "${US3_QRC_XML}" PREFIX "/us3"
        BASE "${US3_ETC_DIR}"
        GLOBS "*.xml"
        EXCLUDE_DIRS ${US3_ETC_EXCLUDE_DIRS}
        EXCLUDE_FILES ${US3_ETC_EXCLUDE_FILES}
        USE_ABSOLUTE
)

# Validation/target now only for XML
add_custom_command(
        OUTPUT "${US3_QRC_XML}.checked"
        COMMAND ${CMAKE_COMMAND}
        -DUS3_QRC="${US3_QRC_XML}"
        -DUS3_BASE="${US3_ETC_DIR}"
        -P "${CMAKE_SOURCE_DIR}/admin/cmake/Us3QrcCheck.cmake"
        COMMAND ${CMAKE_COMMAND} -E touch "${US3_QRC_XML}.checked"
        DEPENDS "${US3_QRC_XML}"
        COMMENT "Validating XML QRC references"
)

add_custom_target(us3_validate_qrc ALL
        DEPENDS "${US3_QRC_XML}.checked"
        )

# Resources library now only includes XML
add_library(us3_resources STATIC "${US3_QRC_XML}")
set_target_properties(us3_resources PROPERTIES AUTOMOC OFF)
add_dependencies(us3_resources us3_validate_qrc)

if(US3_PROFILE STREQUAL "HPC")
    target_link_libraries(us3_resources PRIVATE Qt::Core)
else()
    target_link_libraries(us3_resources PRIVATE Qt::Core Qt::Gui)
endif()

add_library(UltraScan3::Resources ALIAS us3_resources)

# Create symbolic link from source etc to build etc
set(US3_BUILD_ETC_DIR "${CMAKE_BINARY_DIR}/etc")
if (NOT EXISTS "${US3_BUILD_ETC_DIR}")
    file(CREATE_LINK "${US3_ETC_DIR}" "${US3_BUILD_ETC_DIR}" SYMBOLIC)
    message(STATUS "Created symbolic link: ${US3_BUILD_ETC_DIR} -> ${US3_ETC_DIR}")
endif ()

# Install editable trees/files we excluded from the QRC
foreach(d IN LISTS US3_ETC_EXCLUDE_DIRS)
    if(EXISTS "${US3_ETC_DIR}/${d}")
        install(DIRECTORY "${US3_ETC_DIR}/${d}/"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/ultrascan/${d}")
    endif()
endforeach()
foreach(f IN LISTS US3_ETC_EXCLUDE_FILES)
    if(EXISTS "${US3_ETC_DIR}/${f}")
        install(FILES "${US3_ETC_DIR}/${f}"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/ultrascan")
    endif()
endforeach()

# ============================================================================
# Find Dependencies
# ============================================================================

if(BUILD_TESTING)
    if(USE_QT6)
        find_package(Qt6 REQUIRED COMPONENTS Test)
        if(NOT TARGET Qt::Test)
            add_library(Qt::Test ALIAS Qt6::Test)
        endif()
    else()
        find_package(Qt5 REQUIRED COMPONENTS Test)
        if(NOT TARGET Qt::Test)
            add_library(Qt::Test ALIAS Qt5::Test)
        endif()
    endif()
endif()

# Find OpenGL
# On macOS, temporarily prefer frameworks so we get Apple's native
# OpenGL.framework (which includes GLU)
if(APPLE)
    set(_save_find_framework ${CMAKE_FIND_FRAMEWORK})
    set(CMAKE_FIND_FRAMEWORK FIRST)
endif()
find_package(OpenGL REQUIRED)
if(APPLE)
    set(CMAKE_FIND_FRAMEWORK ${_save_find_framework})
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# =============================================================================
# Find Qwt - Unified approach for vcpkg and system packages
# Not needed for HPC builds.
# =============================================================================
if(NOT US3_PROFILE STREQUAL "HPC")

    find_package(unofficial-qwt CONFIG QUIET)

    if(unofficial-qwt_FOUND)
        message(STATUS "Found Qwt via vcpkg")
        if(NOT TARGET Qwt::Qwt)
            add_library(Qwt::Qwt ALIAS unofficial::qwt::qwt)
        endif()
        set(QWT_VERSION ${unofficial-qwt_VERSION})
    else()
        message(STATUS "Searching for system Qwt...")
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            if(USE_QT6)
                pkg_check_modules(QWT QUIET Qt6Qwt6)
                if(NOT QWT_FOUND)
                    pkg_check_modules(QWT QUIET qwt-qt6)
                endif()
            else()
                pkg_check_modules(QWT QUIET Qt5Qwt6)
                if(NOT QWT_FOUND)
                    pkg_check_modules(QWT QUIET qwt-qt5)
                endif()
            endif()
        endif()

        if(NOT QWT_FOUND)
            if(USE_QT6)
                find_path(QWT_INCLUDE_DIR NAMES qwt.h
                        PATHS /usr/include/qwt /usr/include/qt6/qwt /usr/local/include/qwt)
                find_library(QWT_LIBRARY NAMES qwt-qt6 qwt qwt6-qt6
                        PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
            else()
                find_path(QWT_INCLUDE_DIR NAMES qwt.h
                        PATHS /usr/include/qwt /usr/include/qt5/qwt /usr/local/include/qwt)
                find_library(QWT_LIBRARY NAMES qwt-qt5 qwt qwt6-qt5
                        PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
            endif()
            if(QWT_INCLUDE_DIR AND QWT_LIBRARY)
                set(QWT_FOUND TRUE)
                message(STATUS "Found system Qwt: ${QWT_LIBRARY}")
            endif()
        else()
            message(STATUS "Found system Qwt via pkg-config")
        endif()

        if(QWT_FOUND)
            add_library(unofficial_qwt_qwt INTERFACE IMPORTED)
            if(QWT_INCLUDE_DIRS)
                set_target_properties(unofficial_qwt_qwt PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIRS}"
                        INTERFACE_LINK_DIRECTORIES "${QWT_LIBRARY_DIRS}"
                        INTERFACE_LINK_LIBRARIES "${QWT_LIBRARIES}")
            else()
                set_target_properties(unofficial_qwt_qwt PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIR}"
                        INTERFACE_LINK_LIBRARIES "${QWT_LIBRARY}")
            endif()

            add_library(unofficial::qwt::qwt ALIAS unofficial_qwt_qwt)
            add_library(Qwt::Qwt ALIAS unofficial_qwt_qwt)
        else()
            message(FATAL_ERROR "Qwt not found. Install: apt-get install libqwt-qt5-dev")
        endif()
    endif()

endif()

# Find QwtPlot3D - not needed for HPC
if(NOT US3_PROFILE STREQUAL "HPC")
    find_package(qwtplot3d CONFIG QUIET)

    if(qwtplot3d_FOUND)
        message(STATUS "Found QwtPlot3D via vcpkg")
        if(NOT TARGET qwtplot3d::qwtplot3d)
            message(FATAL_ERROR "qwtplot3d::qwtplot3d target not created by find_package")
        endif()
    else()
        message(STATUS "Using in-tree QwtPlot3D build")
        add_subdirectory(qwtplot3d)

        if(TARGET qwtplot3d AND NOT TARGET qwtplot3d::qwtplot3d)
            add_library(qwtplot3d::qwtplot3d ALIAS qwtplot3d)
            message(STATUS "Created qwtplot3d::qwtplot3d alias for in-tree build")
        endif()
    endif()
endif()

# Find libarchive - vcpkg provides a proper CMake target
find_package(LibArchive REQUIRED)

# Find GL2PS directly (no pkg-config file available)
find_path(GL2PS_INCLUDE_DIR
        NAMES gl2ps.h
        PATHS /usr/include /usr/local/include
        )

find_library(GL2PS_LIBRARY
        NAMES gl2ps
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
        )

if(GL2PS_INCLUDE_DIR AND GL2PS_LIBRARY)
    set(GL2PS_FOUND TRUE)
    message(STATUS "GL2PS found: ${GL2PS_LIBRARY}")

    add_library(GL2PS::GL2PS INTERFACE IMPORTED)
    set_target_properties(GL2PS::GL2PS PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${GL2PS_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${GL2PS_LIBRARY}"
            )
else()
    set(GL2PS_FOUND FALSE)
    message(STATUS "GL2PS not found")
endif()

# Find MySQL/MariaDB (only if not NO_DB)
if(NOT US3_NO_DB)
    find_package(unofficial-libmariadb CONFIG QUIET)

    if(unofficial-libmariadb_FOUND)
        message(STATUS "Found MariaDB via vcpkg")
        if(NOT TARGET MySQL::Client)
            add_library(MySQL::Client ALIAS unofficial::libmariadb)
        endif()
        set(MYSQL_MARIADB_VERSION ${unofficial-libmariadb_VERSION})
        set(MYSQL_MARIADB_MODULE "libmariadb")
    else()
        message(STATUS "Searching for system MySQL/MariaDB...")
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(MYSQL QUIET mysqlclient)
            if(NOT MYSQL_FOUND)
                pkg_check_modules(MYSQL QUIET mariadb libmariadb)
            endif()
        endif()

        if(NOT MYSQL_FOUND)
            find_path(MYSQL_INCLUDE_DIR NAMES mysql.h
                    PATHS /usr/include/mysql /usr/local/include/mysql /usr/include/mariadb)
            find_library(MYSQL_LIBRARY NAMES mysqlclient mariadb
                    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

            if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
                set(MYSQL_FOUND TRUE)
                message(STATUS "Found system MySQL: ${MYSQL_LIBRARY}")
            endif()
        else()
            message(STATUS "Found system MySQL via pkg-config")
            set(MYSQL_MARIADB_VERSION ${MYSQL_VERSION})
            set(MYSQL_MARIADB_MODULE ${MYSQL_MODULE_NAME})
        endif()

        if(MYSQL_FOUND)
            add_library(MySQL::Client INTERFACE IMPORTED)
            if(MYSQL_INCLUDE_DIRS)
                set_target_properties(MySQL::Client PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIRS}"
                        INTERFACE_LINK_LIBRARIES "${MYSQL_LIBRARIES}")
            else()
                set_target_properties(MySQL::Client PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIR}"
                        INTERFACE_LINK_LIBRARIES "${MYSQL_LIBRARY}")
            endif()
        else()
            message(FATAL_ERROR "MySQL not found. Install: apt-get install libmysqlclient-dev")
        endif()
    endif()
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    find_library(GLU_LIBRARY GLU REQUIRED)
    find_package(X11 REQUIRED)
elseif(WIN32)
    # Windows system libraries
    set(WIN32_SYSTEM_LIBS ws2_32 advapi32 gdi32 user32 kernel32 psapi)
endif()

# ---------------------------------------------------------------------------
# GoogleTest for unit testing
# ---------------------------------------------------------------------------
if(BUILD_TESTING)
    # Try config mode first (vcpkg / modern installs)
    find_package(GTest CONFIG QUIET)

    if(NOT GTest_FOUND)
        # Fall back to module / system packages
        find_package(GTest MODULE QUIET)
    endif()

    # At this point either we found it, or we didn't
    if(NOT GTest_FOUND)
        # We got here because:
        #  - user passed -DBUILD_TESTING=ON   OR
        #  - US3_PROFILE=TEST forced BUILD_TESTING=ON higher up
        # In both cases the user's intent is "I want tests".
        # Fail fast.
        message(FATAL_ERROR
                "BUILD_TESTING=ON but GoogleTest/GMock was not found.\n"
                "Install GTest in the build (Docker) image, or re-run CMake with -DBUILD_TESTING=OFF.")
    endif()

    # If we are here, GTest was found → set up the usual aliases
    message(STATUS "GoogleTest found – building test targets")

    # Create aliases for gtest (always available)
    if(NOT TARGET gtest AND TARGET GTest::gtest)
        add_library(gtest ALIAS GTest::gtest)
    endif()
    if(NOT TARGET gtest_main AND TARGET GTest::gtest_main)
        add_library(gtest_main ALIAS GTest::gtest_main)
    endif()

    # Create aliases for gmock (only if available)
    if(NOT TARGET gmock AND TARGET GTest::gmock)
        add_library(gmock ALIAS GTest::gmock)
    endif()
    if(NOT TARGET gmock_main AND TARGET GTest::gmock_main)
        add_library(gmock_main ALIAS GTest::gmock_main)
    endif()

    # Disable Qt AUTOMOC for GoogleTest
    if(TARGET GTest::gtest)
        set_target_properties(GTest::gtest PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
    endif()
    if(TARGET GTest::gmock)
        set_target_properties(GTest::gmock PROPERTIES AUTOMOC OFF AUTORCC OFF AUTOUIC OFF)
    endif()
endif()

# =============================================================================
# SUBDIRECTORIES
# =============================================================================
add_subdirectory(utils)

if(BUILD_DOCUMENTATION AND NOT US3_PROFILE STREQUAL "HPC")
    add_subdirectory(doc/manual)
endif()

if(NOT US3_NO_DB)
    add_subdirectory(gui)
endif()

# APP: full programs suite (requires GUI + DB)
# HPC: only us_mpi_analysis (NO_DB, no GUI)
if(US3_BUILD_PROGRAMS)
    add_subdirectory(programs)
endif()

if(BUILD_TESTING)
    add_compile_definitions(US3_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
    add_subdirectory(test)
endif()

# =============================================================================
# INSTALLATION
# =============================================================================
# Install binaries
install(TARGETS
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

# Install headers for development
if(NOT US3_NO_DB)
    install(DIRECTORY utils/
            DESTINATION include/ultrascan3/utils
            FILES_MATCHING PATTERN "*.h"
            )

    install(DIRECTORY gui/
            DESTINATION include/ultrascan3/gui
            FILES_MATCHING PATTERN "*.h"
            )
endif()

# =============================================================================
# SoMo BUILD (CMake native)
# =============================================================================
# build has errors
# add_subdirectory(us_somo)

# =============================================================================
# qhelpgenerator — needed for Qt help file generation (all platforms)
# =============================================================================
set(_QHELPGEN_HINTS "")
foreach (_vcpkg_dir IN ITEMS "${VCPKG_INSTALLED_DIR}" "${_VCPKG_INSTALLED_DIR}")
    if (_vcpkg_dir AND VCPKG_TARGET_TRIPLET)
        string(REGEX REPLACE "-dynamic$" "" _vcpkg_host_triplet "${VCPKG_TARGET_TRIPLET}")
        foreach (_triplet IN ITEMS "${VCPKG_TARGET_TRIPLET}" "${_vcpkg_host_triplet}")
            list(APPEND _QHELPGEN_HINTS
                    "${_vcpkg_dir}/${_triplet}/tools/qttools/bin"
                    "${_vcpkg_dir}/${_triplet}/tools/Qt6/bin"
                    "${_vcpkg_dir}/${_triplet}/tools/qt5-tools/bin"
            )
        endforeach ()
    endif ()
endforeach ()
foreach (_qt_dir IN ITEMS "${Qt6_DIR}" "${Qt5_DIR}")
    if (_qt_dir)
        list(APPEND _QHELPGEN_HINTS "${_qt_dir}/../../../bin")
    endif ()
endforeach ()
if (APPLE)
    list(APPEND _QHELPGEN_HINTS
            /opt/homebrew/opt/qt/bin
            /opt/homebrew/opt/qt@5/bin
            /usr/local/opt/qt/bin
            /usr/local/opt/qt@5/bin
    )
elseif (UNIX)
    list(APPEND _QHELPGEN_HINTS
            /usr/lib/qt6/bin
            /usr/lib/qt5/bin
            /usr/lib/x86_64-linux-gnu/qt5/bin
            /usr/lib/aarch64-linux-gnu/qt5/bin
    )
elseif (WIN32)
    list(APPEND _QHELPGEN_HINTS "$ENV{QTDIR}/bin")
endif ()
find_program(QHELPGENERATOR_EXECUTABLE NAMES qhelpgenerator
        HINTS ${_QHELPGEN_HINTS}
        DOC "Qt Help Generator executable"
)
if (QHELPGENERATOR_EXECUTABLE)
    message(STATUS "Found qhelpgenerator: ${QHELPGENERATOR_EXECUTABLE}")
else ()
    message(STATUS "qhelpgenerator not found - Qt help files will not be built")
endif ()

# =============================================================================
# macOS DEPLOYMENT (folder-based layout)
# =============================================================================
if(APPLE AND US3_BUILD_PROGRAMS AND NOT US3_PROFILE STREQUAL "HPC")
    # Find macdeployqt from the same Qt installation.
    #
    # vcpkg puts macdeployqt in the *static* triplet's tools dir even when
    # building a dynamic triplet (e.g. arm64-osx-dynamic), because qttools
    # is only compiled once. We therefore search both the dynamic and the
    # corresponding static triplet paths, plus the standard Qt install dirs.
    #
    # Derive the static-triplet tools path from VCPKG_INSTALLED_DIR:
    #   arm64-osx-dynamic  →  arm64-osx
    #   x64-osx-dynamic    →  x64-osx
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        # Dynamic triplet tools dir
        set(_VCPKG_TOOLS_DYN "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/Qt6/bin")
        # Derive static counterpart by stripping "-dynamic" suffix
        string(REPLACE "-dynamic" "" _STATIC_TRIPLET "${VCPKG_TARGET_TRIPLET}")
        set(_VCPKG_TOOLS_STA "${VCPKG_INSTALLED_DIR}/${_STATIC_TRIPLET}/tools/Qt6/bin")
        # Also check qt5 tools subdir for Qt5 builds
        set(_VCPKG_TOOLS_DYN5 "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/qt5-tools/bin")
        set(_VCPKG_TOOLS_STA5 "${VCPKG_INSTALLED_DIR}/${_STATIC_TRIPLET}/tools/qt5-tools/bin")
    else()
        set(_VCPKG_TOOLS_DYN "")
        set(_VCPKG_TOOLS_STA "")
        set(_VCPKG_TOOLS_DYN5 "")
        set(_VCPKG_TOOLS_STA5 "")
    endif()

    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt
        HINTS
            # vcpkg dynamic triplet (unlikely to have it, but try first)
            "${_VCPKG_TOOLS_DYN}"
            # vcpkg static triplet tools dir (where vcpkg actually installs it)
            "${_VCPKG_TOOLS_STA}"
            # Qt5 tool paths
            "${_VCPKG_TOOLS_DYN5}"
            "${_VCPKG_TOOLS_STA5}"
            # Qt6_DIR / Qt5_DIR relative paths (Homebrew / official installers)
            "${Qt6_DIR}/../../../bin"
            "${Qt5_DIR}/../../../bin"
            "$ENV{QTDIR}/bin"
        NO_DEFAULT_PATH
    )
    if(MACDEPLOYQT_EXECUTABLE)
        message(STATUS "Found macdeployqt: ${MACDEPLOYQT_EXECUTABLE}")
    else()
        message(WARNING "macdeployqt not found - macOS deployment will not work")
    endif()

    # ---------------------------------------------------------------------------
    # Option A: relocate macdeployqt into the dynamic triplet's tools dir.
    #
    # macdeployqt resolves its Qt prefix by walking up from its own binary path
    # to find qt.conf.  When it lives in the static triplet (arm64-osx), its
    # qt.conf Prefix points to arm64-osx/ which contains only .a archives —
    # so it deploys nothing.  By copying the binary into the dynamic triplet's
    # tools/Qt6/bin/ directory it will instead read arm64-osx-dynamic's qt.conf,
    # whose Prefix points to the real shared .dylib files and Qt6/plugins/.
    #
    # We copy (not symlink) to avoid path-canonicalization surprises: macdeployqt
    # uses argv[0] / dladdr to find itself, and a symlink could resolve back to
    # the static triplet directory.
    #
    # This only applies to vcpkg Qt6 builds where the tool lives in a different
    # triplet than the libraries.  For Homebrew/official Qt installs macdeployqt
    # is already co-located with the dylibs, so no action is needed.
    # ---------------------------------------------------------------------------
    if(USE_QT6
       AND MACDEPLOYQT_EXECUTABLE
       AND DEFINED _VCPKG_TOOLS_STA
       AND DEFINED _VCPKG_TOOLS_DYN)

        # Always create a build-tree wrapper for macdeployqt so we can write
        # our own qt.conf next to it.  The vcpkg dynamic triplet's qt.conf
        # contains HostPrefix=arm64-osx/ which redirects macdeployqt's host
        # library search back to the static triplet (.a archives only), causing
        # it to skip all transitive deps (libb2, libicuXX, libpcre2, etc.).
        # We own the build tree so we can write whatever qt.conf we need.

        # vcpkg's installed/ tree is read-only — we cannot write into it.
        # Instead, copy macdeployqt into the build tree and write our own
        # qt.conf alongside it, pointing Prefix at the dynamic triplet root
        # (absolute path) so macdeployqt finds .dylib files and Qt6/plugins/.
        set(_MACDEPLOYQT_WRAPPER_DIR "${CMAKE_BINARY_DIR}/tools/macdeployqt-dyn")
        set(_MACDEPLOYQT_DYN_PATH    "${_MACDEPLOYQT_WRAPPER_DIR}/macdeployqt")
        set(_DYN_QT_CONF             "${_MACDEPLOYQT_WRAPPER_DIR}/qt.conf")

        file(MAKE_DIRECTORY "${_MACDEPLOYQT_WRAPPER_DIR}")

        # Copy macdeployqt binary if absent or stale
        if(NOT EXISTS "${_MACDEPLOYQT_DYN_PATH}" OR
           "${MACDEPLOYQT_EXECUTABLE}" IS_NEWER_THAN "${_MACDEPLOYQT_DYN_PATH}")
            message(STATUS "Copying macdeployqt to build-tree wrapper dir...")
            message(STATUS "  from: ${MACDEPLOYQT_EXECUTABLE}")
            message(STATUS "  to:   ${_MACDEPLOYQT_DYN_PATH}")
            file(COPY_FILE "${MACDEPLOYQT_EXECUTABLE}" "${_MACDEPLOYQT_DYN_PATH}"
                 RESULT _copy_result)
            if(NOT _copy_result EQUAL 0)
                message(WARNING "Failed to copy macdeployqt to build tree: ${_copy_result}")
            else()
                file(CHMOD "${_MACDEPLOYQT_DYN_PATH}"
                     PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                 GROUP_READ GROUP_EXECUTE
                                 WORLD_READ WORLD_EXECUTE)
                message(STATUS "macdeployqt copied successfully")
            endif()
        else()
            message(STATUS "macdeployqt build-tree copy is up to date")
        endif()

        # Always (re)write qt.conf — we own this file entirely.
        # Use an absolute Prefix path pointing at the dynamic triplet root so
        # macdeployqt finds .dylib files in lib/ and plugins in Qt6/plugins/.
        # HostPrefix is intentionally omitted so no search redirects to arm64-osx/.
        get_filename_component(_DYN_TRIPLET_ROOT
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" ABSOLUTE)
        file(WRITE "${_DYN_QT_CONF}"
            "# Auto-generated by UltraScan3 CMakeLists.txt (Option A)\n"
            "# Points macdeployqt at the dynamic triplet: ${_DYN_TRIPLET_ROOT}\n"
            "# HostPrefix omitted — all resolution goes through dynamic triplet.\n"
            "[Paths]\n"
            "Prefix = ${_DYN_TRIPLET_ROOT}\n"
            "Libraries = lib\n"
            "Plugins = Qt6/plugins\n"
            "Binaries = bin\n"
            "LibraryExecutables = tools/Qt6/bin\n"
        )
        message(STATUS "Wrote qt.conf (build-tree): ${_DYN_QT_CONF}")
        message(STATUS "  Qt prefix: ${_DYN_TRIPLET_ROOT}")

        # Point MACDEPLOYQT_EXECUTABLE at the build-tree copy
        if(EXISTS "${_MACDEPLOYQT_DYN_PATH}")
            set(MACDEPLOYQT_EXECUTABLE "${_MACDEPLOYQT_DYN_PATH}" CACHE FILEPATH
                "Path to macdeployqt" FORCE)
            message(STATUS "macdeployqt (active): ${MACDEPLOYQT_EXECUTABLE}")
        endif()
    endif()

    # Find Assistant.app - derive from macdeployqt location (same Qt install)
    if(MACDEPLOYQT_EXECUTABLE)
        get_filename_component(_deploy_dir "${MACDEPLOYQT_EXECUTABLE}" DIRECTORY)
        set(_assistant_candidates
            "${_deploy_dir}/Assistant.app"
            "${_deploy_dir}/../bin/Assistant.app"
            # vcpkg static triplet tools dir (where qttools is actually installed)
            "${_VCPKG_TOOLS_STA}/Assistant.app"
            "${_VCPKG_TOOLS_STA5}/Assistant.app"
            "${_deploy_dir}/../../qt6/bin/Assistant.app"
            "${_deploy_dir}/../../qt6-tools/bin/Assistant.app"
            "${_deploy_dir}/../../qttools/bin/Assistant.app"
            "${_deploy_dir}/../../qt5/bin/Assistant.app"
            "${_deploy_dir}/../../qt5-tools/bin/Assistant.app"
        )
        set(ASSISTANT_APP_PATH "" CACHE FILEPATH "Path to Qt Assistant.app for packaging")
        if(NOT ASSISTANT_APP_PATH)
            foreach(_cand IN LISTS _assistant_candidates)
                get_filename_component(_real "${_cand}" REALPATH)
                if(EXISTS "${_real}")
                    set(ASSISTANT_APP_PATH "${_real}")
                    break()
                endif()
            endforeach()
        endif()
        if(ASSISTANT_APP_PATH)
            message(STATUS "Assistant.app for packaging: ${ASSISTANT_APP_PATH}")
        else()
            message(STATUS "Assistant.app not found - help will not work in packaged DMG")
        endif()
    endif()

    # Derive vcpkg lib/plugin dirs for MacDeploy.cmake.
    # VCPKG_INSTALLED_DIR may point to the per-build-tree vcpkg_installed/ in
    # manifest mode. Prefer the shared installed dir under VCPKG_ROOT if the
    # tools we need (macdeployqt) were found there instead.
    set(_VCPKG_LIB_DIR "")
    set(_VCPKG_PLUGIN_DIR "")
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_VCPKG_LIB_DIR    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
        set(_VCPKG_PLUGIN_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/plugins")
    endif()
    # Also expose the static-triplet lib/plugin dirs so MacDeploy can find
    # things that vcpkg only installs once (e.g. sqldrivers in static triplet).
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED _STATIC_TRIPLET)
        set(_VCPKG_LIB_DIR_STA    "${VCPKG_INSTALLED_DIR}/${_STATIC_TRIPLET}/lib")
        set(_VCPKG_PLUGIN_DIR_STA "${VCPKG_INSTALLED_DIR}/${_STATIC_TRIPLET}/plugins")
        # Fall back to static dirs if dynamic ones don't exist
        if(NOT EXISTS "${_VCPKG_LIB_DIR}")
            set(_VCPKG_LIB_DIR    "${_VCPKG_LIB_DIR_STA}")
            set(_VCPKG_PLUGIN_DIR "${_VCPKG_PLUGIN_DIR_STA}")
        endif()
    endif()
    # Derive the Qt6/plugins/ dir from the dynamic triplet.
    # vcpkg Qt6 puts all plugin .dylibs under <triplet>/Qt6/plugins/ rather than
    # the legacy <triplet>/plugins/ path.  macdeployqt from the static triplet
    # reads its own qt.conf and won't find these unless we pass -pluginpath.
    # Prefer the build-local vcpkg_installed dir (manifest mode) first, then
    # fall back to the shared installed dir.
    set(_VCPKG_QT6_PLUGIN_DIR "")
    foreach(_SEARCH_TRIPLET_DIR IN ITEMS
            "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}"
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
        if(EXISTS "${_SEARCH_TRIPLET_DIR}/Qt6/plugins")
            set(_VCPKG_QT6_PLUGIN_DIR "${_SEARCH_TRIPLET_DIR}/Qt6/plugins")
            break()
        endif()
    endforeach()
    if(NOT _VCPKG_QT6_PLUGIN_DIR AND _STATIC_TRIPLET)
        foreach(_SEARCH_TRIPLET_DIR IN ITEMS
                "${CMAKE_BINARY_DIR}/vcpkg_installed/${_STATIC_TRIPLET}"
                "${VCPKG_INSTALLED_DIR}/${_STATIC_TRIPLET}")
            if(EXISTS "${_SEARCH_TRIPLET_DIR}/Qt6/plugins")
                set(_VCPKG_QT6_PLUGIN_DIR "${_SEARCH_TRIPLET_DIR}/Qt6/plugins")
                break()
            endif()
        endforeach()
    endif()
    message(STATUS "vcpkg lib dir for deploy:         ${_VCPKG_LIB_DIR}")
    message(STATUS "vcpkg plugin dir for deploy:      ${_VCPKG_PLUGIN_DIR}")
    message(STATUS "vcpkg Qt6 plugin dir for deploy:  ${_VCPKG_QT6_PLUGIN_DIR}")

    # Staging directory for the folder-based install
    set(US3_STAGE_DIR "${CMAKE_BINARY_DIR}/_stage/UltraScan3" CACHE PATH
        "Staging directory for macOS folder-based deployment")

    # Create deploy_macos target — builds the folder tree in _stage/UltraScan3/
    if(MACDEPLOYQT_EXECUTABLE AND TARGET us)
        # Core deps only — optional targets (us3_somo, us3_config, us_admin, us_saxs_cmds_t)
        # are NOT listed here; they are staged opportunistically by MacDeploy.cmake
        # if their output dirs exist, but we don't hard-depend on them.
        set(_deploy_macos_deps us)
        if(TARGET documentation)
            list(APPEND _deploy_macos_deps documentation)
        endif()

        add_custom_target(deploy_macos
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${US3_STAGE_DIR}"
            COMMAND ${CMAKE_COMMAND}
                -DSTAGE_DIR=${US3_STAGE_DIR}
                -DMACDEPLOYQT=${MACDEPLOYQT_EXECUTABLE}
                -DASSISTANT_APP=${ASSISTANT_APP_PATH}
                -DQCH_DIR=${CMAKE_BINARY_DIR}/bin
                -DBIN_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                -DLIB_DIR=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                -DETC_SOURCE_DIR=${CMAKE_SOURCE_DIR}/etc
                -DSOMO_SOURCE_DIR=${CMAKE_SOURCE_DIR}/somo
                -DSOMO_BIN_DIR=${CMAKE_SOURCE_DIR}/bin
                -DSOMO_LIB_DIR=${CMAKE_SOURCE_DIR}/lib
                -DLICENSE_FILE=${CMAKE_SOURCE_DIR}/LICENSE.txt
                -DVCPKG_LIB_DIR=${_VCPKG_LIB_DIR}
                -DVCPKG_PLUGIN_DIR=${_VCPKG_PLUGIN_DIR}
                -DVCPKG_PLUGIN_DIR_STA=${_VCPKG_PLUGIN_DIR_STA}
                -DVCPKG_QT6_PLUGIN_DIR=${_VCPKG_QT6_PLUGIN_DIR}
                -P ${CMAKE_SOURCE_DIR}/admin/cmake/packaging/macos/MacDeploy.cmake
            DEPENDS ${_deploy_macos_deps}
            COMMENT "Staging macOS folder-based deployment into ${US3_STAGE_DIR}"
            VERBATIM
        )
    else()
        # Stub so --target deploy_macos gives a clear error instead of
        # "No rule to make target deploy_macos".
        if(NOT MACDEPLOYQT_EXECUTABLE)
            set(_deploy_macos_reason "macdeployqt not found — install Qt Tools component")
        elseif(NOT TARGET us)
            set(_deploy_macos_reason "'us' target not defined (check US3_BUILD_PROGRAMS / US3_NO_DB)")
        else()
            set(_deploy_macos_reason "unknown reason")
        endif()
        add_custom_target(deploy_macos
            COMMAND ${CMAKE_COMMAND} -E echo
                "ERROR: deploy_macos is unavailable: ${_deploy_macos_reason}"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "deploy_macos unavailable: ${_deploy_macos_reason}"
        )
        message(STATUS "deploy_macos stub created (unavailable: ${_deploy_macos_reason})")
    endif()
endif()

# =============================================================================
# macOS PKG INSTALLER
# Provides the package_macos_pkg custom target (macOS only).
# Requires: deploy_macos target (staged files), pkgbuild, productbuild.
# Output  : <build-dir>/UltraScan3-<version>-macOS.pkg
# =============================================================================
if(APPLE AND US3_BUILD_PROGRAMS AND NOT US3_PROFILE STREQUAL "HPC")
    include(${CMAKE_SOURCE_DIR}/admin/cmake/packaging/macos/MacOsPkg.cmake)
endif()

# =============================================================================
# WINDOWS DEPLOYMENT (flat directory layout)
#
# Mirrors the macOS deploy_macos / package_macos_pkg pattern:
#   deploy_windows  → stages the install tree into _stage/UltraScan3/
#                     by running WinDeploy.cmake as a -P script
#   package_windows_nsis  → CPack/NSIS installer (defined by WinPkg.cmake)
#
# Build pipeline:
#   cmake --build .                       (compile)
#   cmake --install . --prefix ...        (install, if needed separately)
#   cmake --build . --target deploy_windows
#   cmake --build . --target package_windows_nsis
# =============================================================================
if(WIN32 AND US3_BUILD_PROGRAMS)

    # -------------------------------------------------------------------------
    # Locate windeployqt from the Qt installation
    # -------------------------------------------------------------------------
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_WIN_VCPKG_TOOLS_DYN  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/Qt6/bin")
        set(_WIN_VCPKG_TOOLS_STA5 "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/qt5-tools/bin")
        string(REPLACE "-dynamic" "" _WIN_STATIC_TRIPLET "${VCPKG_TARGET_TRIPLET}")
        set(_WIN_VCPKG_TOOLS_STA  "${VCPKG_INSTALLED_DIR}/${_WIN_STATIC_TRIPLET}/tools/Qt6/bin")
    else()
        set(_WIN_VCPKG_TOOLS_DYN "")
        set(_WIN_VCPKG_TOOLS_STA "")
        set(_WIN_VCPKG_TOOLS_STA5 "")
    endif()

    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS
            "${_WIN_VCPKG_TOOLS_DYN}"
            "${_WIN_VCPKG_TOOLS_STA}"
            "${_WIN_VCPKG_TOOLS_STA5}"
            "${Qt6_DIR}/../../../bin"
            "${Qt5_DIR}/../../../bin"
            "$ENV{QTDIR}/bin"
        NAMES windeployqt.exe windeployqt
        DOC "Path to windeployqt"
    )
    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
    else()
        message(WARNING "windeployqt not found - Windows deployment will not work")
    endif()

    # -------------------------------------------------------------------------
    # Locate Qt Assistant (assistant.exe)
    # -------------------------------------------------------------------------
    if(WINDEPLOYQT_EXECUTABLE)
        get_filename_component(_win_deploy_dir "${WINDEPLOYQT_EXECUTABLE}" DIRECTORY)
        set(_win_assistant_candidates
            "${_win_deploy_dir}/assistant.exe"
            "${_WIN_VCPKG_TOOLS_STA}/assistant.exe"
            "${_WIN_VCPKG_TOOLS_STA5}/assistant.exe"
            "${Qt6_DIR}/../../../bin/assistant.exe"
            "${Qt5_DIR}/../../../bin/assistant.exe"
            "$ENV{QTDIR}/bin/assistant.exe"
        )
        if(NOT ASSISTANT_EXECUTABLE)
            find_program(ASSISTANT_EXECUTABLE
                    NAMES assistant.exe Assistant.exe
                    HINTS ${_win_assistant_candidates}
                    DOC "Path to assistant.exe for Windows packaging"
            )
        endif()
        if(ASSISTANT_EXECUTABLE)
            message(STATUS "assistant.exe for packaging: ${ASSISTANT_EXECUTABLE}")
        else()
            message(STATUS "assistant.exe not found - help will not work in packaged installer")
        endif()
    endif()

    # -------------------------------------------------------------------------
    # Derive vcpkg plugin dir for WinDeploy safety-net searches
    # -------------------------------------------------------------------------
    set(_WIN_VCPKG_PLUGIN_DIR "")
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_WIN_VCPKG_PLUGIN_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/plugins")
        if(NOT EXISTS "${_WIN_VCPKG_PLUGIN_DIR}" AND DEFINED _WIN_STATIC_TRIPLET)
            set(_WIN_VCPKG_PLUGIN_DIR "${VCPKG_INSTALLED_DIR}/${_WIN_STATIC_TRIPLET}/plugins")
        endif()
    endif()
    message(STATUS "vcpkg plugin dir (Windows): ${_WIN_VCPKG_PLUGIN_DIR}")

    # -------------------------------------------------------------------------
    # Staging directory (mirrors macOS US3_STAGE_DIR)
    # -------------------------------------------------------------------------
    set(US3_WIN_STAGE_DIR "${CMAKE_BINARY_DIR}/_stage/UltraScan3" CACHE PATH
        "Staging directory for Windows deployment")

    # -------------------------------------------------------------------------
    # deploy_windows target
    #
    # Invokes WinDeploy.cmake as a -P script (same pattern as deploy_macos).
    # Operates on the build-tree bin/ and lib/ — windeployqt is run against
    # executables in the staged bin/ after they are copied there by the script,
    # so all DLLs land in the same tree that CPack will package.
    # -------------------------------------------------------------------------
    if(WINDEPLOYQT_EXECUTABLE AND TARGET us)
        # Only depend on 'documentation' if Sphinx was found and the target
        # actually exists. doc/manual/CMakeLists.txt does an early return()
        # when sphinx-build is not installed, leaving no documentation target.
        set(_deploy_windows_deps us)
        if(TARGET documentation)
            list(APPEND _deploy_windows_deps documentation)
        endif()

        add_custom_target(deploy_windows
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${US3_WIN_STAGE_DIR}"
            COMMAND ${CMAKE_COMMAND}
                -DSTAGE_DIR=${US3_WIN_STAGE_DIR}
                -DWINDEPLOYQT=${WINDEPLOYQT_EXECUTABLE}
                -DASSISTANT_EXE=${ASSISTANT_EXECUTABLE}
                -DQCH_DIR=${CMAKE_BINARY_DIR}/bin
                -DBIN_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                -DLIB_DIR=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                -DETC_SOURCE_DIR=${CMAKE_SOURCE_DIR}/etc
                -DSOMO_SOURCE_DIR=${CMAKE_SOURCE_DIR}/somo
                -DSOMO_BIN_DIR=${CMAKE_SOURCE_DIR}/bin
                -DSOMO_LIB_DIR=${CMAKE_SOURCE_DIR}/lib
                -DLICENSE_FILE=${CMAKE_SOURCE_DIR}/LICENSE.txt
                -DVCPKG_PLUGIN_DIR=${_WIN_VCPKG_PLUGIN_DIR}
                -P ${CMAKE_SOURCE_DIR}/admin/cmake/packaging/windows/WinDeploy.cmake
            DEPENDS ${_deploy_windows_deps}
            COMMENT "Staging Windows deployment into ${US3_WIN_STAGE_DIR}"
            VERBATIM
        )
    else()
        # Stub target so --target deploy_windows gives a clear error
        if(NOT WINDEPLOYQT_EXECUTABLE)
            set(_deploy_windows_reason "windeployqt not found — install Qt Tools component")
        elseif(NOT TARGET us)
            set(_deploy_windows_reason "'us' target not defined (check US3_BUILD_PROGRAMS / US3_NO_DB)")
        else()
            set(_deploy_windows_reason "unknown reason")
        endif()
        add_custom_target(deploy_windows
            COMMAND ${CMAKE_COMMAND} -E echo
                "ERROR: deploy_windows is unavailable: ${_deploy_windows_reason}"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "deploy_windows unavailable: ${_deploy_windows_reason}"
        )
        message(STATUS "deploy_windows stub created (unavailable: ${_deploy_windows_reason})")
    endif()

    # -------------------------------------------------------------------------
    # WinPkg.cmake: sets CPACK_* variables and defines package_windows_nsis.
    # Must be included BEFORE include(CPack) (which follows in the PACKAGING
    # section below) so all CPACK_* variables are visible to CPack.
    # -------------------------------------------------------------------------
    include(${CMAKE_SOURCE_DIR}/admin/cmake/packaging/windows/WinPkg.cmake)

endif() # WIN32 AND US3_BUILD_PROGRAMS

# =============================================================================
# LINUX DEPLOYMENT (portable tarball layout)
#
# Mirrors the macOS deploy_macos / package_macos_pkg pattern:
#   deploy_linux           -> stages the install tree into _stage/UltraScan3/
#                             by running LinuxDeploy.cmake as a -P script
#   package_linux_tarball  -> tar.gz archive (defined by LinuxPkg.cmake)
#
# Legacy equivalent: copypkg-lnx.sh + make_tardistro.sh.template
#
# Build pipeline:
#   cmake --build . --target deploy_linux
#   cmake --build . --target package_linux_tarball
# Or via build.sh:
#   ./scripts/build.sh --pkg
# =============================================================================
if(UNIX AND NOT APPLE AND US3_BUILD_PROGRAMS AND NOT US3_PROFILE STREQUAL "HPC")

    # -------------------------------------------------------------------------
    # Locate Qt assistant binary for the help system
    # -------------------------------------------------------------------------
    # Build search paths for the Qt assistant binary.
    # Use both the CMake cache variables (set by toolchain) and the VCPKG_ROOT
    # environment variable (exported by build.sh) as sources for the installed dir,
    # since DEFINED checks on toolchain-set cache variables can evaluate before
    # the cache is fully populated on first configure.
    # Derive the vcpkg tools directory for Qt assistant.
    #
    # VCPKG_INSTALLED_DIR in manifest mode points to the per-build-tree
    # vcpkg_installed/ directory which contains only libraries — tools (like
    # assistant) live in the *shared* vcpkg installation under ~/vcpkg/installed/.
    #
    # Qt6_DIR is always reliable: it points into the shared installed tree at
    #   <vcpkg-root>/installed/<triplet>/share/Qt6/
    # so stripping 3 components gives the triplet root, and tools/Qt6/bin
    # from there is where assistant lives.
    #
    # We use Qt6_DIR (Qt6 builds) or Qt5_DIR (Qt5 builds) as the primary source,
    # with VCPKG_INSTALLED_DIR + ENV{VCPKG_ROOT} as fallbacks.

    # Determine triplet root from Qt cmake dir (most reliable)
    set(_lnx_triplet_root "")
    if(USE_QT6 AND Qt6_DIR)
        # Qt6_DIR = .../installed/<triplet>/share/Qt6  -> 2 levels up = triplet root
        get_filename_component(_lnx_triplet_root "${Qt6_DIR}/../.." ABSOLUTE)
    elseif(Qt5_DIR)
        # Qt5_DIR = .../installed/<triplet>/share/cmake/Qt5  -> 3 levels up = triplet root
        get_filename_component(_lnx_triplet_root "${Qt5_DIR}/../../.." ABSOLUTE)
    endif()

    # Fallback: VCPKG_INSTALLED_DIR + triplet
    if(NOT _lnx_triplet_root)
        set(_lnx_vcpkg_installed "")
        if(VCPKG_INSTALLED_DIR AND EXISTS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
            # Only use build-tree installed dir if it actually has libraries (i.e. it's populated)
            set(_lnx_vcpkg_installed "${VCPKG_INSTALLED_DIR}")
        endif()
        if(NOT _lnx_vcpkg_installed AND DEFINED ENV{VCPKG_ROOT})
            set(_lnx_vcpkg_installed "$ENV{VCPKG_ROOT}/installed")
        endif()
        set(_lnx_triplet "")
        if(VCPKG_TARGET_TRIPLET)
            set(_lnx_triplet "${VCPKG_TARGET_TRIPLET}")
        else()
            execute_process(COMMAND uname -m OUTPUT_VARIABLE _lnx_uname OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(_lnx_uname MATCHES "^(aarch64|arm64)$")
                set(_lnx_triplet "arm64-linux")
            else()
                set(_lnx_triplet "x64-linux-dynamic")
            endif()
        endif()
        if(_lnx_vcpkg_installed AND _lnx_triplet)
            set(_lnx_triplet_root "${_lnx_vcpkg_installed}/${_lnx_triplet}")
        endif()
    endif()

    # Qt6: assistant lives in tools/Qt6/bin
    set(_LNX_VCPKG_TOOLS_QT6  "${_lnx_triplet_root}/tools/Qt6/bin")
    # Qt5: upstream vcpkg qt5-tools installs assistant to tools/qt5-tools/bin
    set(_LNX_VCPKG_TOOLS_QT5  "${_lnx_triplet_root}/tools/qt5-tools/bin")
    # Fallback: some vcpkg versions use tools/qttools/bin for Qt5 as well
    set(_LNX_VCPKG_TOOLS_QT5B "${_lnx_triplet_root}/tools/qttools/bin")

    message(STATUS "[Linux] assistant search paths:")
    message(STATUS "  Qt6: ${_LNX_VCPKG_TOOLS_QT6}")
    message(STATUS "  Qt5: ${_LNX_VCPKG_TOOLS_QT5}")

    find_program(_LNX_ASSISTANT_EXE assistant
        HINTS
            "${_LNX_VCPKG_TOOLS_QT6}"
            "${_LNX_VCPKG_TOOLS_QT5}"
            "${_LNX_VCPKG_TOOLS_QT5B}"
            "${Qt6_DIR}/../../../bin"
            "${Qt5_DIR}/../../../bin"
            "$ENV{QTDIR}/bin"
        DOC "Path to Qt assistant for Linux packaging"
        NO_CACHE
    )
    if(_LNX_ASSISTANT_EXE)
        message(STATUS "assistant for Linux packaging: ${_LNX_ASSISTANT_EXE}")
    else()
        message(STATUS "assistant not found -- help system may not work in Linux tarball")
    endif()

    # -------------------------------------------------------------------------
    # Derive vcpkg lib/plugin dirs for LinuxDeploy.cmake
    # -------------------------------------------------------------------------
    set(_LNX_VCPKG_LIB_DIR "")
    set(_LNX_VCPKG_PLUGIN_DIR "")
    set(_LNX_VCPKG_QT6_PLUGIN_DIR "")
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_LNX_VCPKG_LIB_DIR    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
        set(_LNX_VCPKG_PLUGIN_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/plugins")
    endif()
    # Qt6 vcpkg on Linux puts plugins in <triplet>/Qt6/plugins/ rather than
    # <triplet>/plugins/.  Search the shared installed dir (via _lnx_triplet_root,
    # derived from Qt6_DIR above) first, then the per-build-tree dir.
    foreach(_lnx_search_root IN ITEMS
            "${_lnx_triplet_root}"
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
        if(EXISTS "${_lnx_search_root}/Qt6/plugins")
            set(_LNX_VCPKG_QT6_PLUGIN_DIR "${_lnx_search_root}/Qt6/plugins")
            break()
        endif()
    endforeach()
    message(STATUS "vcpkg lib dir (Linux):        ${_LNX_VCPKG_LIB_DIR}")
    message(STATUS "vcpkg plugin dir (Linux):     ${_LNX_VCPKG_PLUGIN_DIR}")
    message(STATUS "vcpkg Qt6 plugin dir (Linux): ${_LNX_VCPKG_QT6_PLUGIN_DIR}")

    # -------------------------------------------------------------------------
    # Copy assistant into the dev-build bin/ so us_helpdaemon can find it.
    # us_helpdaemon looks for appBaseDir()/bin/assistant at runtime;
    # in a dev build appBaseDir() resolves to CMAKE_BINARY_DIR, so the binary
    # must land in ${CMAKE_BINARY_DIR}/bin/assistant.
    # -------------------------------------------------------------------------
    if(_LNX_ASSISTANT_EXE)
        # assistant needs its RPATH patched after copying so it can find Qt
        # shared libs at runtime. Without this it has no RPATH and fails to
        # start because libQt6Help.so.6 etc. are not on LD_LIBRARY_PATH.
        find_program(_PATCHELF_EXE patchelf DOC "patchelf for RPATH patching")
        if(_PATCHELF_EXE)
            add_custom_target(copy_assistant_to_bin ALL
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_LNX_ASSISTANT_EXE}"
                    "${CMAKE_BINARY_DIR}/bin/assistant"
                COMMAND ${_PATCHELF_EXE} --set-rpath
                    "$ORIGIN:$ORIGIN/../lib:${_lnx_triplet_root}/lib"
                    "${CMAKE_BINARY_DIR}/bin/assistant"
                COMMENT "Copying Qt assistant -> bin/ and patching RPATH (dev build)"
                VERBATIM
            )
        else()
            add_custom_target(copy_assistant_to_bin ALL
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_LNX_ASSISTANT_EXE}"
                    "${CMAKE_BINARY_DIR}/bin/assistant"
                COMMENT "Copying Qt assistant -> bin/ (dev build, patchelf not found - RPATH not set)"
                VERBATIM
            )
            message(WARNING "patchelf not found - assistant RPATH will not be patched, help system may fail")
        endif()
        message(STATUS "assistant will be copied to bin/ at build time (dev build)")
    else()
        message(WARNING "assistant not found -- help system will not work in dev build")
        message(WARNING "  Searched: ${_LNX_VCPKG_TOOLS_QT6}, ${_LNX_VCPKG_TOOLS_QT5}")
    endif()

    # -------------------------------------------------------------------------
    # Staging directory
    # -------------------------------------------------------------------------
    set(US3_LNX_STAGE_DIR "${CMAKE_BINARY_DIR}/_stage/UltraScan3" CACHE PATH
        "Staging directory for Linux deployment")

    # -------------------------------------------------------------------------
    # deploy_linux target
    # -------------------------------------------------------------------------
    if(TARGET us)
        set(_deploy_linux_deps us)
        if(TARGET documentation)
            list(APPEND _deploy_linux_deps documentation)
        endif()

        add_custom_target(deploy_linux
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${US3_LNX_STAGE_DIR}"
            COMMAND ${CMAKE_COMMAND}
                -DSTAGE_DIR=${US3_LNX_STAGE_DIR}
                -DBIN_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                -DLIB_DIR=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                -DETC_SOURCE_DIR=${CMAKE_SOURCE_DIR}/etc
                -DSOMO_SOURCE_DIR=${CMAKE_SOURCE_DIR}/somo
                -DSOMO_BIN_DIR=${CMAKE_SOURCE_DIR}/bin
                -DSOMO_LIB_DIR=${CMAKE_SOURCE_DIR}/lib
                -DLICENSE_FILE=${CMAKE_SOURCE_DIR}/LICENSE.txt
                -DQCH_DIR=${CMAKE_BINARY_DIR}/bin
                -DASSISTANT_EXE=${_LNX_ASSISTANT_EXE}
                -DVCPKG_LIB_DIR=${_LNX_VCPKG_LIB_DIR}
                -DVCPKG_PLUGIN_DIR=${_LNX_VCPKG_PLUGIN_DIR}
                -DVCPKG_QT6_PLUGIN_DIR=${_LNX_VCPKG_QT6_PLUGIN_DIR}
                -P ${CMAKE_SOURCE_DIR}/admin/cmake/packaging/linux/LinuxDeploy.cmake
            DEPENDS ${_deploy_linux_deps}
            COMMENT "Staging Linux deployment into ${US3_LNX_STAGE_DIR}"
            VERBATIM
        )
    else()
        add_custom_target(deploy_linux
            COMMAND ${CMAKE_COMMAND} -E echo
                "ERROR: deploy_linux is unavailable: 'us' target not defined"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "deploy_linux unavailable: 'us' target not defined"
        )
        message(STATUS "deploy_linux stub created (unavailable: 'us' target not defined)")
    endif()

    # -------------------------------------------------------------------------
    # LinuxPkg.cmake: defines package_linux_tarball target
    # -------------------------------------------------------------------------
    include(${CMAKE_SOURCE_DIR}/admin/cmake/packaging/linux/LinuxPkg.cmake)

endif() # UNIX AND NOT APPLE AND US3_BUILD_PROGRAMS AND NOT HPC

# =============================================================================
# LINUX HPC DEPLOYMENT (headless tarball, no GUI, no help docs)
#
# Active when US3_PROFILE=HPC on Linux.  Uses a separate stage directory
# (_hpc_stage/) so it coexists with the APP build tree without conflict.
#
# Build pipeline:
#   cmake --build . --target deploy_linux_hpc
#   cmake --build . --target package_linux_hpc_tarball
# Or via build.sh:
#   ./scripts/build.sh HPC --pkg
# =============================================================================
if(UNIX AND NOT APPLE AND US3_PROFILE STREQUAL "HPC")

    # -------------------------------------------------------------------------
    # Derive vcpkg lib/plugin dirs
    # -------------------------------------------------------------------------
    set(_HPC_VCPKG_LIB_DIR "")
    set(_HPC_VCPKG_PLUGIN_DIR "")
    set(_HPC_VCPKG_QT6_PLUGIN_DIR "")

    set(_hpc_triplet_root "")
    if(USE_QT6 AND Qt6_DIR)
        get_filename_component(_hpc_triplet_root "${Qt6_DIR}/../.." ABSOLUTE)
    elseif(Qt5_DIR)
        get_filename_component(_hpc_triplet_root "${Qt5_DIR}/../../.." ABSOLUTE)
    endif()

    if(NOT _hpc_triplet_root)
        set(_hpc_vcpkg_installed "")
        if(VCPKG_INSTALLED_DIR AND EXISTS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
            set(_hpc_vcpkg_installed "${VCPKG_INSTALLED_DIR}")
        endif()
        if(NOT _hpc_vcpkg_installed AND DEFINED ENV{VCPKG_ROOT})
            set(_hpc_vcpkg_installed "$ENV{VCPKG_ROOT}/installed")
        endif()
        set(_hpc_triplet "")
        if(VCPKG_TARGET_TRIPLET)
            set(_hpc_triplet "${VCPKG_TARGET_TRIPLET}")
        else()
            execute_process(COMMAND uname -m OUTPUT_VARIABLE _hpc_uname OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(_hpc_uname MATCHES "^(aarch64|arm64)$")
                set(_hpc_triplet "arm64-linux")
            else()
                set(_hpc_triplet "x64-linux-dynamic")
            endif()
        endif()
        if(_hpc_vcpkg_installed AND _hpc_triplet)
            set(_hpc_triplet_root "${_hpc_vcpkg_installed}/${_hpc_triplet}")
        endif()
    endif()

    if(_hpc_triplet_root)
        set(_HPC_VCPKG_LIB_DIR    "${_hpc_triplet_root}/lib")
        set(_HPC_VCPKG_PLUGIN_DIR "${_hpc_triplet_root}/plugins")
        if(EXISTS "${_hpc_triplet_root}/Qt6/plugins")
            set(_HPC_VCPKG_QT6_PLUGIN_DIR "${_hpc_triplet_root}/Qt6/plugins")
        endif()
    elseif(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_HPC_VCPKG_LIB_DIR    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
        set(_HPC_VCPKG_PLUGIN_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/plugins")
    endif()

    message(STATUS "[HPC] vcpkg lib dir:        ${_HPC_VCPKG_LIB_DIR}")
    message(STATUS "[HPC] vcpkg plugin dir:     ${_HPC_VCPKG_PLUGIN_DIR}")
    message(STATUS "[HPC] vcpkg Qt6 plugin dir: ${_HPC_VCPKG_QT6_PLUGIN_DIR}")

    # -------------------------------------------------------------------------
    # HPC stage directory (separate from APP _stage/)
    # -------------------------------------------------------------------------
    set(US3_HPC_STAGE_DIR "${CMAKE_BINARY_DIR}/_hpc_stage/UltraScan3-HPC" CACHE PATH
        "Staging directory for Linux HPC deployment")

    # -------------------------------------------------------------------------
    # deploy_linux_hpc target
    # Does NOT require 'us', 'documentation', or any GUI target.
    # Depends on usutils so it runs after core libs are compiled.
    # -------------------------------------------------------------------------
    set(_deploy_hpc_deps "")
    if(TARGET usutils)
        list(APPEND _deploy_hpc_deps usutils)
    endif()
    if(TARGET us_mpi_analysis)
        list(APPEND _deploy_hpc_deps us_mpi_analysis)
    endif()

    add_custom_target(deploy_linux_hpc
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${US3_HPC_STAGE_DIR}"
        COMMAND ${CMAKE_COMMAND}
            -DHPC_STAGE_DIR=${US3_HPC_STAGE_DIR}
            -DBIN_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            -DLIB_DIR=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            -DLICENSE_FILE=${CMAKE_SOURCE_DIR}/LICENSE.txt
            -DVCPKG_LIB_DIR=${_HPC_VCPKG_LIB_DIR}
            -DUS3_VERSION_STRING=${US3_VERSION_STRING}
            -P ${CMAKE_SOURCE_DIR}/admin/cmake/packaging/linux/LinuxHpcDeploy.cmake
        DEPENDS ${_deploy_hpc_deps}
        COMMENT "Staging Linux HPC deployment into ${US3_HPC_STAGE_DIR}"
        VERBATIM
    )

    # -------------------------------------------------------------------------
    # LinuxHpcPkg.cmake: defines package_linux_hpc_tarball target
    # -------------------------------------------------------------------------
    include(${CMAKE_SOURCE_DIR}/admin/cmake/packaging/linux/LinuxHpcPkg.cmake)

endif() # UNIX AND NOT APPLE AND HPC

# =============================================================================
# macOS HPC DEPLOYMENT (headless tarball, no GUI launcher, no help docs)
#
# Active when APPLE AND US3_PROFILE=HPC.  Uses a separate stage directory
# (_hpc_stage/) so it coexists with the APP build tree without conflict.
#
# Build pipeline:
#   cmake --build . --target deploy_macos_hpc
#   cmake --build . --target package_macos_hpc_tarball
# Or via build.sh:
#   ./scripts/build.sh HPC --pkg
# =============================================================================
if(APPLE AND US3_PROFILE STREQUAL "HPC")

    # -------------------------------------------------------------------------
    # Derive vcpkg lib dir for MacHpcDeploy.cmake
    # Only VCPKG_LIB_DIR is needed — the new HPC deploy uses a flat lib/
    # layout with DYLD_LIBRARY_PATH, not .app bundles or macdeployqt.
    # -------------------------------------------------------------------------
    set(_MAC_HPC_VCPKG_LIB_DIR "")
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_MAC_HPC_VCPKG_LIB_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
        if(NOT EXISTS "${_MAC_HPC_VCPKG_LIB_DIR}")
            string(REPLACE "-dynamic" "" _mac_hpc_static_triplet "${VCPKG_TARGET_TRIPLET}")
            set(_MAC_HPC_VCPKG_LIB_DIR "${VCPKG_INSTALLED_DIR}/${_mac_hpc_static_triplet}/lib")
        endif()
    elseif(USE_QT6 AND Qt6_DIR)
        get_filename_component(_MAC_HPC_VCPKG_LIB_DIR "${Qt6_DIR}/../../lib" ABSOLUTE)
    elseif(Qt5_DIR)
        get_filename_component(_MAC_HPC_VCPKG_LIB_DIR "${Qt5_DIR}/../../../lib" ABSOLUTE)
    endif()
    message(STATUS "[macOS HPC] vcpkg lib dir: ${_MAC_HPC_VCPKG_LIB_DIR}")

    # -------------------------------------------------------------------------
    # HPC stage directory (separate from APP _stage/)
    # -------------------------------------------------------------------------
    set(US3_MAC_HPC_STAGE_DIR "${CMAKE_BINARY_DIR}/_hpc_stage/UltraScan3-HPC" CACHE PATH
        "Staging directory for macOS HPC deployment")

    # -------------------------------------------------------------------------
    # deploy_macos_hpc target
    # Does NOT require 'us', 'documentation', or any GUI target.
    # Depends on usutils so it runs after core libs are compiled.
    # -------------------------------------------------------------------------
    set(_deploy_mac_hpc_deps "")
    if(TARGET usutils)
        list(APPEND _deploy_mac_hpc_deps usutils)
    endif()
    if(TARGET us_mpi_analysis)
        list(APPEND _deploy_mac_hpc_deps us_mpi_analysis)
    endif()

    add_custom_target(deploy_macos_hpc
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${US3_MAC_HPC_STAGE_DIR}"
        COMMAND ${CMAKE_COMMAND}
            -DHPC_STAGE_DIR=${US3_MAC_HPC_STAGE_DIR}
            -DBIN_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            -DLIB_DIR=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            -DLICENSE_FILE=${CMAKE_SOURCE_DIR}/LICENSE.txt
            -DVCPKG_LIB_DIR=${_MAC_HPC_VCPKG_LIB_DIR}
            -DUS3_VERSION_STRING=${US3_VERSION_STRING}
            -P ${CMAKE_SOURCE_DIR}/admin/cmake/packaging/macos/MacHpcDeploy.cmake
        DEPENDS ${_deploy_mac_hpc_deps}
        COMMENT "Staging macOS HPC deployment into ${US3_MAC_HPC_STAGE_DIR}"
        VERBATIM
    )

    # -------------------------------------------------------------------------
    # MacHpcPkg.cmake: defines package_macos_hpc_tarball target
    # -------------------------------------------------------------------------
    include(${CMAKE_SOURCE_DIR}/admin/cmake/packaging/macos/MacHpcPkg.cmake)

endif() # APPLE AND HPC

# =============================================================================
# PACKAGING
# =============================================================================
set(CPACK_PACKAGE_NAME "UltraScan3")
set(CPACK_PACKAGE_VERSION ${US3_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced analysis of analytical ultracentrifugation experiments")
set(CPACK_PACKAGE_VENDOR "AUC Solutions, LLC")

# macOS: uses package_macos_pkg target (pkgbuild + productbuild). CPack not used.
# Windows: CPACK_GENERATOR and NSIS options set by WinPkg.cmake above.
# Linux: uses package_linux_tarball target. CPack TGZ kept as fallback.
if(NOT APPLE AND NOT WIN32)
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)

# Wire the CPack 'package' target to depend on deploy_windows so that
# 'cmake --build . --target package' always stages DLLs first.
# The 'package' target is created by include(CPack) above.
if(WIN32 AND US3_BUILD_PROGRAMS AND TARGET package AND TARGET deploy_windows)
    add_dependencies(package deploy_windows)
    message(STATUS "Wired: package depends on deploy_windows")
endif()

# =============================================================================
# DEVELOPMENT UTILITIES
# =============================================================================
# Add custom target for cleaning generated files (equivalent to qmakeall.sh clean)
add_custom_target(distclean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
        COMMENT "Cleaning all generated files"
        )

# Resolve Qt exact version string
if(USE_QT6)
    set(_US3_QT_VERSION "${Qt6_VERSION}")
else()
    set(_US3_QT_VERSION "${Qt5_VERSION}")
endif()

if(NOT DEFINED QWT_VERSION)
    set(QWT_VERSION "not-used")
endif()

if(NOT DEFINED qwtplot3d_VERSION)
    set(qwtplot3d_VERSION "not-used")
endif()

if(NOT DEFINED MYSQL_MARIADB_VERSION)
    set(MYSQL_MARIADB_VERSION "not-used")
endif()

file(WRITE "${CMAKE_BINARY_DIR}/us3_build_info.sh"
        "# Auto-generated by CMake configure — do not edit\n"
        "US3_CMAKE_VERSION=\"${US3_VERSION_STRING}\"\n"
        "US3_QT_VERSION=\"${_US3_QT_VERSION}\"\n"
        "US3_QWT_VERSION=\"${QWT_VERSION}\"\n"
        "US3_QWTPLOT3D_VERSION=\"${qwtplot3d_VERSION}\"\n"
        "US3_OPENSSL_VERSION=\"${OPENSSL_VERSION}\"\n"
        "US3_MARIADB_VERSION=\"${MYSQL_MARIADB_VERSION}\"\n"
        "US3_PROFILE=\"${US3_PROFILE}\"\n"
        "US3_PRESET=\"${CMAKE_BUILD_TYPE}\"\n"
)

# Print configuration summary
message(STATUS "")
message(STATUS "UltraScan3 Configuration Summary:")
message(STATUS "  Version:        ${US3_VERSION_STRING}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Profile:        ${US3_PROFILE}")
message(STATUS "  Qt version:     ${_US3_QT_VERSION}")
message(STATUS "  Qwt version:    ${QWT_VERSION}")
message(STATUS "  OpenSSL:        ${OPENSSL_VERSION}")
message(STATUS "")
