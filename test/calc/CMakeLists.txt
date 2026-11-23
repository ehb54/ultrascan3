# Calculation fixtures and tests (see README.md).
#
# calc_oracle.py checks that fixtures/expected/reporting.json still follows
# from the fixture models and report items.  test_calc_legacy_gui runs the
# current UltraScan calculations on the fixtures and compares them with the
# captured outputs in fixtures/expected.

# The calculations are part of us_utils; the tests link it as test/utils
# does (static when available).  No GUI, database or display is used.
if(TARGET UltraScan3::Utils_Static)
    set(US_CALC_LINK_TARGET UltraScan3::Utils_Static)
else()
    set(US_CALC_LINK_TARGET UltraScan3::Utils)
endif()

# Unit tests of the calculations (namespace US_Calc).
add_executable(test_us_calc
        test_us_calc_preparation.cpp
        test_us_calc_rules.cpp
        test_us_calc_units.cpp
        test_us_calc_validation.cpp
        test_us_calc_version.cpp
        )
target_link_libraries(test_us_calc PRIVATE ${US_CALC_LINK_TARGET} gtest gtest_main)
set_target_properties(test_us_calc PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
include(GoogleTest)
# One label, as in test/utils; ctest -L calc and -L gtest both match it.
gtest_discover_tests(test_us_calc PROPERTIES LABELS gtest-unit-calc TIMEOUT 60)

# The reporting calculations against the calculation fixtures.
add_executable(test_us_calc_fixtures test_us_calc_fixtures.cpp)
target_link_libraries(test_us_calc_fixtures PRIVATE ${US_CALC_LINK_TARGET} Qt::Core gtest gtest_main)
target_compile_definitions(test_us_calc_fixtures PRIVATE
        CALC_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
set_target_properties(test_us_calc_fixtures PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
gtest_discover_tests(test_us_calc_fixtures PROPERTIES LABELS gtest-unit-calc TIMEOUT 60)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_test(NAME calc_oracle_selftest
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/calc_oracle.py selftest)
    add_test(NAME calc_oracle_expected_current
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/calc_oracle.py check)
    set_tests_properties(calc_oracle_selftest calc_oracle_expected_current
            PROPERTIES LABELS "calc;oracle")
else()
    message(STATUS "test: skipping calc oracle tests (no Python 3 interpreter)")
endif()

if(TARGET UltraScan3::Gui_Static)
    us3_add_gui_test(test_calc_legacy_gui
            SOURCES
                test_calc_legacy_gui.cpp
                ${CMAKE_SOURCE_DIR}/programs/us_integral/us_integral.cpp
                ${CMAKE_SOURCE_DIR}/programs/us_integral/us_integral.h
                ${CMAKE_SOURCE_DIR}/programs/us_integral/us_delete_models.cpp
                ${CMAKE_SOURCE_DIR}/programs/us_integral/us_delete_models.h
                ${CMAKE_SOURCE_DIR}/programs/us_ga_init/us_solutedata.cpp
                ${CMAKE_SOURCE_DIR}/programs/us_ga_init/us_solutedata.h
            LIBRARIES
                UltraScan3::Gui_Static
            INCLUDE_DIRECTORIES
                ${CMAKE_SOURCE_DIR}/programs/us_integral
                ${CMAKE_SOURCE_DIR}/programs/us_ga_init
            LABELS
                calc
            )
    target_compile_definitions(test_calc_legacy_gui PRIVATE
            CALC_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
    set_tests_properties(test_calc_legacy_gui PROPERTIES TIMEOUT 600)
else()
    message(STATUS "test: skipping test_calc_legacy_gui (no UltraScan3::Gui_Static in this variant)")
endif()
