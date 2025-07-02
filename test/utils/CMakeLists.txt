# Project definition
project(TestUSUtils)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Network Sql Test Xml)

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

# Find Google Test
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GTEST gtest)
    pkg_check_modules(GMOCK gmock)
endif()

# Fallback to find_package for GTest
if(NOT GTEST_FOUND)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
    endif()
endif()

# If still not found, use system libraries
if(NOT GTEST_FOUND AND NOT GTest_FOUND)
    set(GTEST_LIBRARIES gtest gtest_main gmock gmock_main)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/utils
        ${CMAKE_SOURCE_DIR}/test/utils)

if(MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIRS})
endif()

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
        test_us_analyte.cpp test_us_analyte.h
        test_us_astfem_math.cpp test_us_astfem_math.h
        test_us_project.cpp test_us_project.h
        test_mock_us_db2.cpp test_mock_us_db2.h
        test_us_astfem_rsa.cpp test_us_astfem_rsa.h
        test_us_stiffbase.cpp test_us_stiffbase.h
        test_us_model.cpp test_us_model.h
        test_us_buffer.cpp
        test_us_simparms.cpp test_us_simparms.h ../../utils/ius_db2.h)

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
        ${GTEST_LIBRARIES}
        )

# Link MySQL if found
if(MYSQL_FOUND)
    target_link_libraries(test_us_utils PRIVATE ${MYSQL_LIBRARIES})
    message(STATUS "MySQL found and linked")
else()
    message(WARNING "MySQL not found - database tests may not work")
endif()

# Add the test to CTest
add_test(NAME test_us_utils COMMAND test_us_utils)