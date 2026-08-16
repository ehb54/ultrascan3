# Project definition
project(TestUSUtils)


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

include_directories(
        ${CMAKE_SOURCE_DIR}/test/utils
        ${CMAKE_SOURCE_DIR}/test/utils/mock
        ${CMAKE_SOURCE_DIR}/test/common
)

# AUTO-DISCOVERY - Find all test files automatically.  CONFIGURE_DEPENDS makes
# the build system re-run the glob, so a newly added test file is picked up by
# an incremental build instead of only by a fresh configure.
file(GLOB_RECURSE AUTO_TEST_SOURCES CONFIGURE_DEPENDS "test_us_*.cpp" "test_mock_*.cpp")
file(GLOB_RECURSE AUTO_TEST_HEADERS CONFIGURE_DEPENDS "test_us_*.h" "test_mock_*.h")

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
        ../common/qt_test_base.cpp
        test_us_vector.cpp
        test_us_timer.cpp
        )

# Do NOT define NO_DB here.  The whole tree, including the utilities library the
# tests link, gets NO_DB from the top-level US3_NO_DB option; defining it for the
# test executable alone made the test translation units see a different private
# layout of US_DB2 (us_db2.h drops the three MYSQL members under NO_DB) than the
# library they link against, which is an ODR violation.  The variant is therefore
# a property of the whole build tree: configure with -DUS3_NO_DB=ON for the NO_DB
# variant, and leave it OFF (the TEST profile default) for the DB-enabled one.
if(US3_NO_DB)
    message(STATUS "test_us_utils: NO_DB variant (utils library is also NO_DB)")
else()
    message(STATUS "test_us_utils: DB-enabled variant (utils library is also DB-enabled)")
endif()

# Determine which library to link against
# Prefer static for tests, fallback to shared if static not available
if(TARGET UltraScan3::Utils_Static)
    set(US_UTILS_LINK_TARGET UltraScan3::Utils_Static)
    message(STATUS "Linking tests against static utils library")
else()
    set(US_UTILS_LINK_TARGET UltraScan3::Utils)
    message(STATUS "Linking tests against shared utils library (static not available)")
endif()

# Link libraries
target_link_libraries(test_us_utils
        PRIVATE
        ${US_UTILS_LINK_TARGET}
        Qt::Core
        Qt::Network
        Qt::Sql
        Qt::Test
        Qt::Xml
        Qt::Widgets
        ${GTEST_LIBRARIES}
        )

# Link MySQL for the DB-enabled variant.  The utilities library already exports
# it PUBLIC, so this is belt-and-braces; the NO_DB variant must not have it at
# all, and its absence there is expected rather than a problem.
if(US3_NO_DB)
    # Nothing to link: the whole tree, tests included, is built without the DB.
elseif(TARGET MySQL::Client)
    target_link_libraries(test_us_utils PRIVATE MySQL::Client)
    message(STATUS "MySQL found and linked")
else()
    message(WARNING "MySQL not found - database tests may not work")
endif()

# ENABLE Google Test discovery - this will register individual test cases
include(GoogleTest)
# The label is one composite word on purpose, and the alternative does not work.
#
# gtest_discover_tests flattens PROPERTIES into a single semicolon-separated
# string before its generated script re-parses it, so the natural
# LABELS "gtest;unit;utils" arrives there as three arguments: LABELS takes
# "gtest", and "unit" and "utils" are silently read as property names.  No
# escaping at this call survives that round trip, and set_property(TEST ...)
# from a later CTest include file cannot see tests added by another one.  The
# result was that `ctest -L utils` -- and therefore the test_utils target below
# and the `./test-docker.sh -f utils` command in test/README -- matched nothing
# at all while appearing to work.
#
# CTest matches -L as a regular expression against each label, so a single
# "gtest-unit-utils" label is selected by -L gtest, -L unit and -L utils alike.
gtest_discover_tests(test_us_utils
        PROPERTIES
        LABELS gtest-unit-utils
        TIMEOUT 60
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        )

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
