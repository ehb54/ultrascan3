# admin/cmake/platform-config.cmake
# Common configuration for all platforms

message(STATUS "Applying platform-specific configuration")

# Common settings for all platforms
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Symbol visibility for better library hygiene (GCC/Clang)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fvisibility=hidden)
    add_compile_options(-fvisibility-inlines-hidden)
endif()

# Platform-specific settings
if(APPLE)
    # macOS
    set(CMAKE_FIND_FRAMEWORK LAST)
    add_link_options(-Wl,-dead_strip_dylibs)

    if(Qt5_DIR)
        get_filename_component(QT_FRAMEWORK_DIR "${Qt5_DIR}/../../.." ABSOLUTE)
        list(APPEND CMAKE_INSTALL_RPATH "${QT_FRAMEWORK_DIR}")
        message(STATUS "Qt framework directory: ${QT_FRAMEWORK_DIR}")
    endif()

elseif(UNIX)
    # Linux
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib")

    # Linux-specific link options
    add_link_options(-Wl,--as-needed)  # Only link what's actually used

elseif(WIN32)
    # Windows
    # Windows DLL search path is different - typically handled by deployment
    message(STATUS "Windows platform: ensure Qt DLLs are in PATH or application directory")
endif()

message(STATUS "Platform configuration applied successfully")
