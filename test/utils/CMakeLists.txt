# Project definition
project(TestUSUtils)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Test Xml)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/utils
        ${CMAKE_SOURCE_DIR}/test/utils)

# Add the test executable
add_executable(test_us_utils
        test_us_utils_main.cpp
        test_us_datafiles.cpp
        test_us_util.cpp
        )

# Link the utils library and Qt libraries to the test executable
target_link_libraries(test_us_utils utils
        Qt5::Core
        Qt5::Test
        Qt5::Xml
        )

# Add the test to CTest
add_test(NAME test_us_utils COMMAND test_us_utils)