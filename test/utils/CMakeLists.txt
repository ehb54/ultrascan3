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

# On MSVC GoogleTest compiles gtest into gmock (and each *_main) instead of
# linking it.  As DLLs every copy keeps its own UnitTest, so a SCOPED_TRACE
# pushed in one is popped from another's empty stack.  test_us_utils has its
# own main, and gmock alone carries the whole gtest runtime.
if(MSVC)
    set(GTEST_LIBRARIES gmock)
endif()

include_directories(
        ${CMAKE_SOURCE_DIR}/test/utils
        ${CMAKE_SOURCE_DIR}/test/utils/mock
        ${CMAKE_SOURCE_DIR}/test/common
)

# Discover new test files during incremental builds.
file(GLOB_RECURSE AUTO_TEST_SOURCES CONFIGURE_DEPENDS "test_us_*.cpp" "test_mock_*.cpp")
file(GLOB_RECURSE AUTO_TEST_HEADERS CONFIGURE_DEPENDS "test_us_*.h" "test_mock_*.h")

# Remove main file from auto-discovery
list(REMOVE_ITEM AUTO_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_us_utils_main.cpp")

# Print what we found (for debugging)
list(LENGTH AUTO_TEST_SOURCES num_sources)
message(STATUS "Auto-discovered ${num_sources} test source files")

# Non-unit dependencies are kept out of the unit binary as well as its label.
# This also makes direct --gtest_shuffle/--gtest_repeat runs offline.
set(DB_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_us_db2_connect_authenticated.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_us_db2_roundtrip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_us_db2_transactions.cpp)
set(HOST_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_us_memory.cpp)
set(SOURCE_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_us_help_pages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_us_regex.cpp)
list(REMOVE_ITEM AUTO_TEST_SOURCES
    ${DB_TEST_SOURCES} ${HOST_TEST_SOURCES} ${SOURCE_TEST_SOURCES})

# Inherit NO_DB from US3_NO_DB for both tests and libraries.
# A test-only definition changes the US_DB2 layout and violates the ODR.
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

include(GoogleTest)
function(us3_add_utils_suite target label)
    add_executable(${target}
        test_us_utils_main.cpp
        ../common/qt_test_base.cpp
        ${ARGN})
    target_link_libraries(${target} PRIVATE
        ${US_UTILS_LINK_TARGET} Qt::Core Qt::Network Qt::Sql Qt::Test
        Qt::Xml Qt::Widgets ${GTEST_LIBRARIES})
    if(NOT US3_NO_DB AND TARGET MySQL::Client)
        target_link_libraries(${target} PRIVATE MySQL::Client)
    endif()
    # A single composite label avoids list splitting in discovery properties.
    gtest_discover_tests(${target}
        PROPERTIES LABELS ${label} TIMEOUT 60
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

us3_add_utils_suite(test_us_utils gtest-unit-utils
    ${AUTO_TEST_SOURCES} ${AUTO_TEST_HEADERS}
    support/disk_run_fixture.cpp support/disk_run_fixture.h)
# Unit resource readers use appBaseDir()/etc. Never point that layout at
# source/etc: stage only declared inputs next to the isolated unit executable.
set(US3_UNIT_LAYOUT "${CMAKE_CURRENT_BINARY_DIR}/unit-layout")
set_target_properties(test_us_utils PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${US3_UNIT_LAYOUT}/bin")
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(test_us_utils PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config} "${US3_UNIT_LAYOUT}/bin")
endforeach()
set(US3_UNIT_RESOURCES
    abstractCenterpieces.xml
    bufferComponents.xml
    rotors/abstractRotors.xml
    rotors/R0000001.xml rotors/R0000002.xml rotors/R0000003.xml
    rotors/C0000001.xml rotors/C0000002.xml rotors/C0000003.xml)
set(resource_manifest "")
foreach(resource IN LISTS US3_UNIT_RESOURCES)
    set(input "${CMAKE_SOURCE_DIR}/etc/${resource}")
    set(output "${US3_UNIT_LAYOUT}/etc/${resource}")
    if(IS_SYMLINK "${US3_UNIT_LAYOUT}/etc" OR IS_SYMLINK "${output}")
        message(FATAL_ERROR "Unit resources must be private copies: ${output}")
    endif()
    configure_file("${input}" "${output}" COPYONLY)
    file(CHMOD "${output}" PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
    file(SHA256 "${input}" resource_hash)
    string(APPEND resource_manifest "    { \"${resource}\", \"${resource_hash}\" },\n")
endforeach()
configure_file(support/unit_resources.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/unit_resources.h" @ONLY)
target_include_directories(test_us_utils PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(test_us_utils PRIVATE US3_CHECK_UNIT_RESOURCES)

us3_add_utils_suite(test_us_utils_db gtest-integration-db-utils ${DB_TEST_SOURCES})
us3_add_utils_suite(test_us_utils_host gtest-host-utils ${HOST_TEST_SOURCES})
us3_add_utils_suite(test_us_utils_source gtest-source-utils ${SOURCE_TEST_SOURCES})

add_custom_target(test_utils_unit
    COMMAND ${CMAKE_CTEST_COMMAND} -L gtest-unit-utils --output-on-failure
    DEPENDS test_us_utils
    COMMENT "Running offline utility unit tests")

# Create convenience targets for different test categories
add_custom_target(test_utils
        COMMAND ${CMAKE_CTEST_COMMAND} -L utils --output-on-failure
        DEPENDS test_us_utils test_us_utils_db test_us_utils_host test_us_utils_source
        COMMENT "Running utility tests only"
        )

add_custom_target(test_all
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS test_us_utils test_us_utils_db test_us_utils_host test_us_utils_source
        COMMENT "Running all tests"
        )

add_custom_target(test_verbose
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
        DEPENDS test_us_utils test_us_utils_db test_us_utils_host test_us_utils_source
        COMMENT "Running tests with verbose output"
        )

# Debug target to help troubleshoot the executable
add_custom_target(test_debug
        COMMAND echo "Testing executable directly:"
        COMMAND echo "QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_us_utils> --help"
        COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_us_utils> --help || echo "Help command failed"
        DEPENDS test_us_utils
        COMMENT "Debug test executable"
        )

# Test if the executable can list individual tests
add_custom_target(test_individual_discovery
        COMMAND echo "Testing if executable supports individual test discovery..."
        COMMAND echo "QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_us_utils> --gtest_list_tests"
        COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_us_utils> --gtest_list_tests || echo "Individual test discovery not supported"
        DEPENDS test_us_utils
        COMMENT "Test individual test discovery"
        )
