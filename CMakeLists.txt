cmake_minimum_required(VERSION 3.21)
# Get current git version
execute_process(
    COMMAND git log --oneline
    COMMAND wc -l
    COMMAND tr -d '\n'
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(US_VERSION "4.0.${GIT_HASH}")
project(UltraScan VERSION ${US_VERSION} LANGUAGES C CXX)
message(STATUS "Compiling ${PROJECT_NAME} ${US_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable AUTOMOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Enable debugging
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add custom FindQwt to cmake modules
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/etc")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/etc")

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g3 -ggdb -ginline-points -gstatement-frontiers")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

# Set Parallel Building
include(ProcessorCount)
ProcessorCount(NUM_CORES)
if(NOT DEFINED ENV{CMAKE_BUILD_PARALLEL_LEVEL})
    if(NOT NUM_CORES EQUAL 0)
        set(ENV{CMAKE_BUILD_PARALLEL_LEVEL} ${NUM_CORES})
    endif()
else()
    if(ENV{CMAKE_BUILD_PARALLEL_LEVEL} GREATER NUM_CORES)
        if(NOT NUM_CORES EQUAL 0)
            set(ENV{CMAKE_BUILD_PARALLEL_LEVEL} ${NUM_CORES})
        endif()
    endif()
endif()


# Configure output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Demo of loading from file
#file(STRINGS <path to config file> ConfigContents)
#foreach(NameAndValue ${ConfigContents})
#    # Strip leading spaces
#    string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
#    # Find variable name
#    string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
#    # Find the value
#    string(REPLACE "${Name}=" "" Value ${NameAndValue})
#    # Set the variable
#    set(${Name} "${Value}")
#endforeach()

# Configure from environment


# Define a macro based on the build configuration.
# This mimics the $$DEBUGORRELEASE variable in QMake.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_MODE)
else()
    add_compile_definitions(RELEASE_MODE)
endif()



# Enable testing
enable_testing()

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Sql Test Xml Widgets Svg PrintSupport OpenGL)
find_package(Threads REQUIRED)
find_package(OpenGL COMPONENTS OpenGL REQUIRED)
find_package(Qwt REQUIRED)
if(UNIX)
    find_package(X11 REQUIRED)
endif()

# Include the src subdirectory
add_subdirectory(qwtplot3d)
add_subdirectory(utils)
add_subdirectory(gui)
add_subdirectory(test)
add_subdirectory(programs)
