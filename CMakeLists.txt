cmake_minimum_required(VERSION 3.21)
project(UltrascanTest)

set(CMAKE_CXX_STANDARD 20)
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

# Include the src subdirectory
add_subdirectory(utils)
add_subdirectory(test)