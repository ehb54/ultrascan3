# Project definition
project(TestUSUtils)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Network Sql Test Xml Widgets)

# Find MySQL with better error handling
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(MYSQL mysqlclient)
endif()

# Fallback MySQL detection
if(NOT MYSQL_FOUND)
    find_path(MYSQL_INCLUDE_DIR mysql.h
            PATHS /usr/include/mysql /usr/local/include/mysql
            )
    find_library(MYSQL_LIBRARY mysqlclient
            PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
            )
    if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
        set(MYSQL_FOUND TRUE)
        set(MYSQL_LIBRARIES ${MYSQL_LIBRARY})
        set(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})
    endif()
endif()

# Google Test - Use FetchContent targets from parent
if(TARGET gtest)
    set(GTEST_LIBRARIES gtest gtest_main gmock gmock_main)
    message(STATUS "Using Google Test from FetchContent")
else()
    # Fallback to system installation
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GTEST gtest)
        pkg_check_modules(GMOCK gmock)
    endif()

    if(NOT GTEST_FOUND)
        find_package(GTest QUIET)
        if(GTest_FOUND)
            set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
        endif()
    endif()

    if(NOT GTEST_FOUND AND NOT GTest_FOUND)
        set(GTEST_LIBRARIES gtest gtest_main gmock gmock_main)
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/utils
        ${CMAKE_SOURCE_DIR}/test/utils
        ${CMAKE_SOURCE_DIR}/test/common)

if(MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIRS})
endif()

# AUTO-DISCOVERY - Find all test files automatically
file(GLOB_RECURSE AUTO_TEST_SOURCES "test_us_*.cpp" "test_mock_*.cpp")
file(GLOB_RECURSE AUTO_TEST_HEADERS "test_us_*.h" "test_mock_*.h")

# Remove main file from auto-discovery
list(REMOVE_ITEM AUTO_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_us_utils_main.cpp")

# Print what we found (for debugging)
list(LENGTH AUTO_TEST_SOURCES num_sources)
message(STATUS "Auto-discovered ${num_sources} test source files")

# Add the test executable
add_executable(test_us_utils
        test_us_utils_main.cpp
        ${AUTO_TEST_SOURCES}
        ${AUTO_TEST_HEADERS}
        ../../utils/ius_db2.h
        ../common/qt_test_base.cpp)

target_compile_definitions(test_us_utils PRIVATE NO_DB)

# Link libraries
target_link_libraries(test_us_utils
        PRIVATE
        utils
        Qt5::Core
        Qt5::Network
        Qt5::Sql
        Qt5::Test
        Qt5::Xml
        Qt5::Widgets
        ${GTEST_LIBRARIES}
        )

# Link MySQL if found
if(MYSQL_FOUND)
    target_link_libraries(test_us_utils PRIVATE ${MYSQL_LIBRARIES})
    message(STATUS "MySQL found and linked")
else()
    message(WARNING "MySQL not found - database tests may not work")
endif()

# Add the test to CTest - This registers your test with CTest
add_test(NAME us_utils_tests COMMAND test_us_utils)

# Set test properties with environment for headless Qt
set_tests_properties(us_utils_tests PROPERTIES
        TIMEOUT 300
        LABELS "unit;utils"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        )

# DISABLE gtest_discover_tests for now to avoid the error
# We'll use the basic add_test() above which works fine
#
# include(GoogleTest)
# gtest_discover_tests(test_us_utils
#     PROPERTIES
#         LABELS "gtest;unit"
#         TIMEOUT 60
# )

# Create convenience targets for different test categories
add_custom_target(test_utils
        COMMAND ${CMAKE_CTEST_COMMAND} -L utils --output-on-failure
        DEPENDS test_us_utils
        COMMENT "Running utility tests only"
        )

add_custom_target(test_all
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS test_us_utils
        COMMENT "Running all tests"
        )

add_custom_target(test_verbose
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
        DEPENDS test_us_utils
        COMMENT "Running tests with verbose output"
        )

# Debug target to help troubleshoot the executable
add_custom_target(test_debug
        COMMAND echo "Testing executable directly:"
        COMMAND echo "QT_QPA_PLATFORM=offscreen ${CMAKE_CURRENT_BINARY_DIR}/test_us_utils --help"
        COMMAND QT_QPA_PLATFORM=offscreen ${CMAKE_CURRENT_BINARY_DIR}/test_us_utils --help || echo "Help command failed"
        DEPENDS test_us_utils
        COMMENT "Debug test executable"
        )

# Test if the executable can list individual tests
add_custom_target(test_individual_discovery
        COMMAND echo "Testing if executable supports individual test discovery..."
        COMMAND echo "QT_QPA_PLATFORM=offscreen ${CMAKE_CURRENT_BINARY_DIR}/test_us_utils --gtest_list_tests"
        COMMAND QT_QPA_PLATFORM=offscreen ${CMAKE_CURRENT_BINARY_DIR}/test_us_utils --gtest_list_tests || echo "Individual test discovery not supported"
        DEPENDS test_us_utils
        COMMENT "Test individual test discovery"
        )