# UltraScan3 Utils Library
# Based on library.pri configuration

# Auto-discover source files
file(GLOB UTILS_SOURCES "*.cpp")
file(GLOB UTILS_HEADERS "*.h")

# Remove any sources that shouldn't be compiled based on configuration
if(US3_NO_DB)
    # For supercomputer backends, exclude database-related files if needed
    # list(REMOVE_ITEM UTILS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/us_database.cpp")
endif()

# Create the utils library
add_library(us_utils ${UTILS_SOURCES} ${UTILS_HEADERS})

# Set library properties (version from qmake VERSION = 10.0)
set_target_properties(us_utils PROPERTIES
        VERSION ${US3_VERSION}
        SOVERSION ${US3_VERSION_MAJOR}
        OUTPUT_NAME "us_utils${US3_VERSION_MAJOR}"  # Matches qmake VER = 10
        )

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
elseif(APPLE)
    # macOS-specific setup
    target_include_directories(us_utils PUBLIC /usr/X11R6/include)
    find_library(X11_LIB X11 PATHS /usr/X11R6/lib)
    find_library(XAU_LIB Xau PATHS /usr/X11R6/lib)
    if(X11_LIB AND XAU_LIB)
        target_link_libraries(us_utils PUBLIC ${X11_LIB} ${XAU_LIB})
    endif()
endif()

# Compiler-specific options for debugging
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(us_utils PRIVATE -g -O0)
endif()

# Export this target for use by other parts of the project
add_library(UltraScan3::Utils ALIAS us_utils)

# Installation
install(TARGETS us_utils
        EXPORT UltraScan3Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        )