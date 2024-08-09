# Project definition
project(TestUSUtils)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Test Xml)
find_package(MySQL)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/utils
        ${CMAKE_SOURCE_DIR}/test/utils)

# Add the test executable
add_executable(test_us_utils
        test_us_utils_main.cpp
        test_us_datafiles.cpp
        test_us_util.cpp
        test_us_ana_profile.cpp
        test_us_math2.cpp
        test_us_crc.cpp test_us_crc.h
        test_us_crypto.cpp test_us_crypto.h
        test_us_matrix.cpp test_us_matrix.h
        test_us_dataIO.cpp test_us_dataIO.h
        test_us_simparms.cpp test_us_simparms.h
        test_us_settings.cpp test_us_settings.h
        test_us_memory.cpp test_us_memory.h
        test_us_time_state.cpp test_us_time_state.h
        test_us_db2.cpp test_us_db2.h
        test_us_analyte.cpp test_us_analyte.h)

# Link the utils library and Qt libraries to the test executable
target_link_libraries(test_us_utils utils
        Qt5::Core
        Qt5::Test
        Qt5::Xml
        )

# Add the test to CTest
add_test(NAME test_us_utils COMMAND test_us_utils)