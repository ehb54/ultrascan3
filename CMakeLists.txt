cmake_minimum_required(VERSION 3.21)
project(UltrascanTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Enable AUTOMOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Enable debugging
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_definitions(-DNO_DB)

# Enable testing
enable_testing()

# Find Qt packages
find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Sql Test Widgets Xml Svg OpenGL PrintSupport)

# Add this after your existing find_package calls
find_path(QWT_INCLUDE_DIR
        NAMES qwt.h
        PATHS
        /usr/include/qwt
        /usr/local/include/qwt
        /usr/include/qwt-qt5
        )

find_library(QWT_LIBRARY
        NAMES qwt qwt-qt5 qwt6-qt5
        PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
        )

if(QWT_INCLUDE_DIR AND QWT_LIBRARY)
    message(STATUS "Qwt found: ${QWT_LIBRARY}")
    message(STATUS "Qwt include: ${QWT_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "Qwt not found. Please install Qwt development libraries.")
endif()

# Include FetchContent module
include(FetchContent)

# Fetch GoogleTest
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1 # Use the latest stable release or specific version
)

# Build GoogleTest
FetchContent_MakeAvailable(googletest)

set_target_properties(gmock PROPERTIES AUTOMOC OFF)
set_target_properties(gmock PROPERTIES AUTOUIC OFF)
set_target_properties(gmock PROPERTIES AUTORCC OFF)
set_target_properties(gtest PROPERTIES AUTOMOC OFF)
set_target_properties(gtest PROPERTIES AUTOUIC OFF)
set_target_properties(gtest PROPERTIES AUTORCC OFF)

# Enable testing
enable_testing()

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Attempt to find the MySQL header files
find_path(MYSQL_INCLUDE_DIR
        NAMES mysql.h
        PATHS /usr/include /usr/local/include /usr/include/mysql /usr/local/include/mysql /usr/local/opt/mysql-client/include
        )

# Attempt to find the MySQL library files
find_library(MYSQL_LIBRARY
        NAMES mysqlclient
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu /usr/local/lib/x86_64-linux-gnu /usr/local/opt/mysql-client/lib
        )

# Check if both the header and library were found
if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    message(STATUS "MySQL include directory: ${MYSQL_INCLUDE_DIR}")
    message(STATUS "MySQL library: ${MYSQL_LIBRARY}")
else()
    message(FATAL_ERROR "MySQL not found. Please install MySQL development libraries.")
endif()

# Check for pkg-config and find MySQL client
find_package(PkgConfig REQUIRED)
pkg_search_module(MYSQL REQUIRED mysqlclient)

include_directories(${MySQL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS})
link_directories(${MySQL_LIBRARY_DIRS} ${OPENSSL_LIBRARY_DIRS})

# Find libarchive
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

# Include the src subdirectory
add_subdirectory(utils)
#add_subdirectory(qwtplot3d)
#add_subdirectory(gui)
#add_subdirectory(programs)
add_subdirectory(test)