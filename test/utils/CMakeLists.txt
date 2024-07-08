# Project definition
project(TestUSDataFiles)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Test Xml)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/utils)

# Add the test executable
add_executable(test_us_datafiles
        test_us_datafiles.cpp
       )

# Link the utils library and Qt libraries to the test executable
target_link_libraries(test_us_datafiles utils
        Qt6::Core
        Qt6::Test
        Qt6::Xml
        )

# Add the test to CTest
add_test(NAME test_us_datafiles COMMAND test_us_datafiles)