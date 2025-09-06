#.rst:
# FindQwt
# -------
#
# Find the Qwt library (Qt Widgets for Technical Applications)
#
# This module defines the following variables:
#
# ::
#
#   QWT_FOUND          - True if Qwt is found
#   QWT_INCLUDE_DIRS   - Include directories for Qwt
#   QWT_LIBRARIES      - Libraries to link against Qwt
#   QWT_VERSION        - Version string (e.g. "6.1.4")
#   QWT_VERSION_MAJOR  - Major version number
#   QWT_VERSION_MINOR  - Minor version number
#   QWT_VERSION_PATCH  - Patch version number
#
# This module defines the following imported target:
#
# ::
#
#   Qwt::Qwt          - The Qwt library
#
# You can set the following variables to help guide the search:
#
# ::
#
#   QWT_ROOT          - Root directory of Qwt installation
#   QWT_INCLUDE_DIR   - Directory containing qwt.h
#   QWT_LIBRARY       - Path to Qwt library file
#
# Or via environment variables:
#
#   export QWT_ROOT=/path/to/qwt

cmake_minimum_required(VERSION 3.5)

# Allow user to override search locations
set(QWT_ROOT "" CACHE PATH "Root directory of Qwt installation")

# Check environment variable if not set
if(NOT QWT_ROOT AND DEFINED ENV{QWT_ROOT})
    set(QWT_ROOT $ENV{QWT_ROOT})
endif()

# First, find Qt5 to help with platform detection and library paths
find_package(Qt5 QUIET COMPONENTS Core Widgets)
if(Qt5Core_FOUND)
    get_target_property(_qt5_core_location Qt5::Core IMPORTED_LOCATION_RELEASE)
    if(NOT _qt5_core_location)
        get_target_property(_qt5_core_location Qt5::Core IMPORTED_LOCATION_RELWITHDEBINFO)
    endif()
    if(NOT _qt5_core_location)
        get_target_property(_qt5_core_location Qt5::Core IMPORTED_LOCATION_DEBUG)
    endif()
    if(NOT _qt5_core_location)
        get_target_property(_qt5_core_location Qt5::Core IMPORTED_LOCATION)
    endif()

    if(_qt5_core_location)
        get_filename_component(_qt5_install_prefix "${_qt5_core_location}" DIRECTORY)
        get_filename_component(_qt5_install_prefix "${_qt5_install_prefix}" DIRECTORY)
        if(WIN32)
            get_filename_component(_qt5_install_prefix "${_qt5_install_prefix}" DIRECTORY)
        endif()
    endif()

    get_target_property(_qt5_include_dirs Qt5::Core INTERFACE_INCLUDE_DIRECTORIES)
endif()

# Build comprehensive search paths based on platform
set(_qwt_search_paths)
set(_qwt_include_suffixes)
set(_qwt_library_suffixes)
set(_qwt_library_names)

if(WIN32)
    # Windows-specific paths and naming
    list(APPEND _qwt_search_paths
            ${QWT_ROOT}
            "C:/qwt"
            "$ENV{PROGRAMFILES}/qwt"
            "$ENV{PROGRAMFILES(X86)}/qwt"
            )
    # Add Qt installation directory if found
    if(_qt5_install_prefix)
        list(APPEND _qwt_search_paths "${_qt5_install_prefix}")
    endif()

    list(APPEND _qwt_include_suffixes include src)
    list(APPEND _qwt_library_suffixes lib)
    # Windows can have debug/release variants
    list(APPEND _qwt_library_names qwt qwtd libqwt libqwtd qwt-qt5 qwtd-qt5)

elseif(APPLE)
    # macOS-specific paths (Homebrew, MacPorts, manual installs)
    list(APPEND _qwt_search_paths
            ${QWT_ROOT}
            /usr/local/qwt
            /usr/local/opt/qwt          # Homebrew
            /opt/local/lib/qwt-qt5      # MacPorts Qt5
            /opt/local/lib/qwt          # MacPorts
            /opt/local/qwt
            /usr/local
            /opt/local
            )
    # Add Qt installation directory if found
    if(_qt5_install_prefix)
        list(APPEND _qwt_search_paths "${_qt5_install_prefix}")
    endif()

    list(APPEND _qwt_include_suffixes include include/qwt)
    list(APPEND _qwt_library_suffixes lib)
    list(APPEND _qwt_library_names qwt qwt-qt5)

else()
    # Linux and other Unix systems
    list(APPEND _qwt_search_paths
            ${QWT_ROOT}
            /usr/local/qwt
            /opt/qwt
            /usr
            /usr/local
            )
    # Add Qt installation directory if found
    if(_qt5_install_prefix)
        list(APPEND _qwt_search_paths "${_qt5_install_prefix}")
    endif()

    # Linux package manager locations
    list(APPEND _qwt_include_suffixes
            include/qwt-qt5
            include/qwt6
            include/qwt
            include
            )
    list(APPEND _qwt_library_suffixes
            lib/x86_64-linux-gnu    # Debian/Ubuntu multiarch
            lib64                   # RedHat/CentOS 64-bit
            lib/i386-linux-gnu      # Debian/Ubuntu 32-bit
            lib                     # Standard lib directory
            )
    list(APPEND _qwt_library_names qwt-qt5 qwt6 qwt)
endif()

# Try pkg-config first on Unix systems (most reliable for system packages)
set(_qwt_found_via_pkgconfig FALSE)
if(UNIX AND NOT APPLE)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(PC_QWT QUIET Qt5Qwt)
        if(PC_QWT_FOUND)
            set(QWT_INCLUDE_DIR ${PC_QWT_INCLUDE_DIRS} CACHE PATH "Qwt include directory")
            # pkg-config returns library names, we need the full path
            find_library(_qwt_lib_from_pc
                    NAMES ${PC_QWT_LIBRARIES}
                    HINTS ${PC_QWT_LIBRARY_DIRS}
                    )
            if(_qwt_lib_from_pc)
                set(QWT_LIBRARY ${_qwt_lib_from_pc} CACHE FILEPATH "Qwt library")
                set(QWT_VERSION ${PC_QWT_VERSION})
                set(_qwt_found_via_pkgconfig TRUE)
            endif()
        endif()
    endif()
endif()

# Manual search if pkg-config didn't work or is unavailable
if(NOT _qwt_found_via_pkgconfig)
    # Search for include directory
    find_path(QWT_INCLUDE_DIR
            NAMES qwt.h qwt_global.h
            HINTS
            ${_qwt_search_paths}
            ${_qt5_include_dirs}
            PATH_SUFFIXES
            ${_qwt_include_suffixes}
            DOC "Qwt include directory"
            )

    # Search for library
    find_library(QWT_LIBRARY
            NAMES ${_qwt_library_names}
            HINTS ${_qwt_search_paths}
            PATH_SUFFIXES ${_qwt_library_suffixes}
            DOC "Qwt library"
            )
endif()

# Extract version information from qwt_global.h
if(QWT_INCLUDE_DIR AND EXISTS "${QWT_INCLUDE_DIR}/qwt_global.h")
    file(READ "${QWT_INCLUDE_DIR}/qwt_global.h" _qwt_global_h_contents)

    string(REGEX MATCH "#define[ \t]+QWT_VERSION_STR[ \t]+\"([0-9]+)\\.([0-9]+)\\.([0-9]+)\""
            _qwt_version_match "${_qwt_global_h_contents}")

    if(_qwt_version_match)
        set(QWT_VERSION_MAJOR ${CMAKE_MATCH_1})
        set(QWT_VERSION_MINOR ${CMAKE_MATCH_2})
        set(QWT_VERSION_PATCH ${CMAKE_MATCH_3})
        set(QWT_VERSION "${QWT_VERSION_MAJOR}.${QWT_VERSION_MINOR}.${QWT_VERSION_PATCH}")
    else()
        # Try alternate version format
        string(REGEX MATCH "#define[ \t]+QWT_VERSION[ \t]+0x([0-9A-Fa-f]+)"
                _qwt_hex_version "${_qwt_global_h_contents}")
        if(_qwt_hex_version)
            # Convert hex version to decimal components
            math(EXPR QWT_VERSION_MAJOR "(0x${CMAKE_MATCH_1} & 0xFF0000) >> 16")
            math(EXPR QWT_VERSION_MINOR "(0x${CMAKE_MATCH_1} & 0x00FF00) >> 8")
            math(EXPR QWT_VERSION_PATCH "(0x${CMAKE_MATCH_1} & 0x0000FF)")
            set(QWT_VERSION "${QWT_VERSION_MAJOR}.${QWT_VERSION_MINOR}.${QWT_VERSION_PATCH}")
        endif()
    endif()
endif()

# Set output variables
set(QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR})
set(QWT_LIBRARIES ${QWT_LIBRARY})

# Handle standard CMake find_package arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qwt
        REQUIRED_VARS QWT_LIBRARY QWT_INCLUDE_DIR
        VERSION_VAR QWT_VERSION
        HANDLE_COMPONENTS
        )

# Create imported target if found
if(QWT_FOUND)
    if(NOT TARGET Qwt::Qwt)
        # Determine library type (shared vs static)
        if(WIN32 AND QWT_LIBRARY MATCHES "\\.(lib|a)$")
            set(_qwt_lib_type STATIC)
        else()
            set(_qwt_lib_type UNKNOWN)
        endif()

        add_library(Qwt::Qwt ${_qwt_lib_type} IMPORTED)

        set_target_properties(Qwt::Qwt PROPERTIES
                IMPORTED_LOCATION "${QWT_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIR}"
                )

        # Link dependencies
        if(Qt5Core_FOUND)
            set_target_properties(Qwt::Qwt PROPERTIES
                    INTERFACE_LINK_LIBRARIES "Qt5::Core;Qt5::Widgets;Qt5::Gui;Qt5::PrintSupport"
                    )
        endif()

        # Windows-specific link requirements
        if(WIN32)
            set_target_properties(Qwt::Qwt PROPERTIES
                    INTERFACE_LINK_LIBRARIES "${_qwt_interface_libs};user32;gdi32"
                    )
        endif()

        # Set version properties
        if(QWT_VERSION)
            set_target_properties(Qwt::Qwt PROPERTIES
                    INTERFACE_QWT_VERSION_MAJOR ${QWT_VERSION_MAJOR}
                    INTERFACE_QWT_VERSION_MINOR ${QWT_VERSION_MINOR}
                    INTERFACE_QWT_VERSION_PATCH ${QWT_VERSION_PATCH}
                    )
        endif()
    endif()

    # Provide helpful status message
    if(NOT Qwt_FIND_QUIETLY)
        message(STATUS "Found Qwt: ${QWT_LIBRARY} (found version \"${QWT_VERSION}\")")
    endif()
else()
    # Provide helpful error message
    if(Qwt_FIND_REQUIRED)
        message(STATUS "Could not find Qwt library.")
        message(STATUS "Searched in the following locations:")
        foreach(_path ${_qwt_search_paths})
            message(STATUS "  ${_path}")
        endforeach()
        message(STATUS "")
        message(STATUS "You can help CMake find Qwt by:")
        message(STATUS "  1. Setting -DQWT_ROOT=/path/to/qwt")
        message(STATUS "  2. Setting environment variable: export QWT_ROOT=/path/to/qwt")
        message(STATUS "  3. Installing system packages:")
        if(WIN32)
            message(STATUS "     - Use vcpkg: vcpkg install qwt")
        elseif(APPLE)
            message(STATUS "     - Use Homebrew: brew install qwt")
            message(STATUS "     - Use MacPorts: sudo port install qwt-qt5")
        else()
            message(STATUS "     - Ubuntu/Debian: sudo apt install libqwt-qt5-dev")
            message(STATUS "     - CentOS/RHEL/Fedora: sudo dnf install qwt-qt5-devel")
        endif()
    endif()
endif()

# Clean up internal variables
unset(_qwt_search_paths)
unset(_qwt_include_suffixes)
unset(_qwt_library_suffixes)
unset(_qwt_library_names)
unset(_qwt_found_via_pkgconfig)
unset(_qt5_core_location)
unset(_qt5_install_prefix)
unset(_qt5_include_dirs)

# Mark cache variables as advanced
mark_as_advanced(QWT_INCLUDE_DIR QWT_LIBRARY)