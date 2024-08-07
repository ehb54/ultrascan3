cmake_minimum_required(VERSION 3.21)
project(UltrascanTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable AUTOMOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Enable debugging
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Enable testing
enable_testing()

# Find Qt packages
find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Sql Test Xml)
find_package(OpenSSL REQUIRED)

# Check for pkg-config and find MySQL client
find_package(PkgConfig REQUIRED)
pkg_search_module(MYSQL REQUIRED mysqlclient)

include_directories(${MySQL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
link_directories(${MySQL_LIBRARY_DIRS} ${OPENSSL_LIBRARY_DIRS})

# Include the src subdirectory
add_subdirectory(utils)
add_subdirectory(test)

# Optionally print the paths for debugging
message(STATUS "MySQL include directory: ${MYSQL_INCLUDE_DIR}")
message(STATUS "MySQL libraries: ${MYSQL_LIBRARY}")