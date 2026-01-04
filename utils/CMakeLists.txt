# UltraScan3 Utils Library
# Based on library.pri configuration

# Auto-discover source files
file(GLOB UTILS_SOURCES "*.cpp")
file(GLOB UTILS_HEADERS "*.h")

# Create the utils library
add_library(us_utils ${UTILS_SOURCES} ${UTILS_HEADERS})

if(APPLE)
    # Force all symbols to be exported at link time
    target_link_options(us_utils PRIVATE
            "LINKER:-export_dynamic"
            "LINKER:-all_load"
            )
    # Override any inherited visibility settings
    target_compile_options(us_utils PRIVATE -fvisibility=default)
elseif(UNIX)
    # Force all symbols to be exported at link time
    target_link_options(us_utils PRIVATE
            "LINKER:--export-dynamic"
    )
    # Override any inherited visibility settings
    target_compile_options(us_utils PRIVATE -fvisibility=default)
endif()

# Set library properties - MERGED with visibility settings
set_target_properties(us_utils PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "us_utils${PROJECT_VERSION_MAJOR}"
        CXX_VISIBILITY_PRESET default
        C_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
        POSITION_INDEPENDENT_CODE ON
        )

# Platform-specific properties
if(APPLE)
    set_target_properties(us_utils PROPERTIES
            MACOSX_RPATH ON
            INSTALL_RPATH "@loader_path;@loader_path/../lib"
            )
elseif(UNIX)
    set_target_properties(us_utils PROPERTIES
            INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
            )
elseif(WIN32)
    # Windows handles DLLs differently
    set_target_properties(us_utils PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS OFF  # We'll use explicit exports
            )
endif()

# Ensure dynamic Qt linking - ALL PLATFORMS
target_compile_definitions(us_utils
        PUBLIC
        QT_SHARED
        PRIVATE
        QT_PLUGIN
        )

## Symbol visibility flags for GCC/Clang (Linux, macOS)
#if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    target_compile_options(us_utils PRIVATE
#            -fvisibility=hidden
#            -fvisibility-inlines-hidden
#            )
#endif()

# Include directories
target_include_directories(us_utils
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}  # For generated headers
        )

# Core Qt libraries (always needed)
target_link_libraries(us_utils
        PUBLIC
        Qt::Core
        Qt::Xml
        Qt::Network
        OpenSSL::SSL
        OpenSSL::Crypto
        LibArchive::LibArchive
        )

# Optional database support
if(NOT US3_NO_DB)
    target_link_libraries(us_utils
            PUBLIC
            Qt::Sql
            MySQL::Client
            )
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux-specific libraries
    if(X11_FOUND)
        target_link_libraries(us_utils PUBLIC ${X11_LIBRARIES})
        target_include_directories(us_utils PUBLIC ${X11_INCLUDE_DIR})
    endif()
elseif(WIN32)
    # Windows system libraries
    target_link_libraries(us_utils PUBLIC ${WIN32_SYSTEM_LIBS})
    target_compile_definitions(us_utils PRIVATE US_MAKE_DLL US_MAKE_UTIL_DLL)
endif()

# Compiler-specific options for debugging
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(us_utils PRIVATE -g -O0)
endif()

# Export this target for use by other parts of the project
add_library(UltraScan3::Utils ALIAS us_utils)

# =============================================================================
# OPTIONAL: Build static library for unit testing
# =============================================================================
if(BUILD_TESTING OR US3_PREFER_STATIC)
    message(STATUS "Building static utils library for testing")

    # Create STATIC library specifically for testing
    add_library(us_utils_static STATIC ${UTILS_SOURCES} ${UTILS_HEADERS})

    # Set properties for STATIC library
    set_target_properties(us_utils_static PROPERTIES
            OUTPUT_NAME "us_utils${PROJECT_VERSION_MAJOR}_static"
            POSITION_INDEPENDENT_CODE ON
            )

    # Apply same compile definitions
    target_compile_definitions(us_utils_static
            PUBLIC
            QT_SHARED
            )

    # Apply same include directories
    target_include_directories(us_utils_static
            PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
            )

    # Apply same link libraries
    target_link_libraries(us_utils_static
            PUBLIC
            Qt::Core
            Qt::Xml
            Qt::Network
            OpenSSL::SSL
            OpenSSL::Crypto
            LibArchive::LibArchive
            )

    # Optional database support
    if(NOT US3_NO_DB)
        target_link_libraries(us_utils_static
                PUBLIC
                Qt::Sql
                MySQL::Client
                )
    endif()

    # Platform-specific libraries
    if(UNIX AND NOT APPLE)
        if(X11_FOUND)
            target_link_libraries(us_utils_static PUBLIC ${X11_LIBRARIES})
            target_include_directories(us_utils_static PUBLIC ${X11_INCLUDE_DIR})
        endif()
    elseif(WIN32)
        target_link_libraries(us_utils_static PUBLIC ${WIN32_SYSTEM_LIBS})
    endif()

    # Debug options
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(us_utils_static PRIVATE -g -O0)
    endif()

    # Export static library alias
    add_library(UltraScan3::Utils_Static ALIAS us_utils_static)

else()
    message(STATUS "Skipping static utils library (testing disabled)")
endif()
# END: Build static library for unit testing

# Installation - only install the shared library
install(TARGETS us_utils
        EXPORT UltraScan3Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        )
