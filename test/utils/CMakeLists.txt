# Project definition
project(TestUSUtils)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Network Sql Test Xml)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/utils
        ${CMAKE_SOURCE_DIR}/test/utils)

# Add the test executable
add_executable(test_us_utils
        test_us_utils_main.cpp
        test_us_datafiles.cpp
        test_us_util.cpp
        test_us_simparams.cpp
        )

# Link the utils library and Qt libraries to the test executable
target_link_libraries(test_us_utils us_utils
        Qt5::Core
        Qt5::Test
        Qt5::Xml
	    Qt5::Sql
        )

if(NOT "NO_DB" IN_LIST DEFINED)
    target_include_directories(test_us_utils PUBLIC /usr/include)
    target_include_directories(test_us_utils PUBLIC /usr/include/mysql)
    target_include_directories(test_us_utils PUBLIC "$ENV{MYSQLPATH}" "$ENV{MYSQLPATH}/include")
    target_link_libraries(test_us_utils mysqlclient)
endif()


# Add the test to CTest
add_test(NAME test_us_utils COMMAND test_us_utils)
