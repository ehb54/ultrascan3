# The test profile does not configure programs/, so take the library of the
# program from its own directory.
if(NOT TARGET us_analysis_profile_lib)
    if(NOT TARGET us_program_common)
        add_library(us_program_common INTERFACE)
        target_link_libraries(us_program_common INTERFACE UltraScan3::Gui_Static)
        target_include_directories(us_program_common INTERFACE ${CMAKE_SOURCE_DIR}/programs)
    endif()
    if(NOT TARGET us_programs_analysis)
        add_library(us_programs_analysis INTERFACE)
    endif()
    add_subdirectory(${CMAKE_SOURCE_DIR}/programs/us_analysis_profile
            ${CMAKE_CURRENT_BINARY_DIR}/production EXCLUDE_FROM_ALL)
    # Check list bounds in release builds too.
    target_compile_definitions(us_analysis_profile_lib PRIVATE QT_FORCE_ASSERTS)
endif()

us3_add_gui_test(test_us_analysis_profile_run_report_gui
        SOURCES
            test_us_analysis_profile_run_report_gui.cpp
        LIBRARIES
            us_analysis_profile_lib
        LABELS
            programs
        )
set_property(TEST test_us_analysis_profile_run_report_gui APPEND PROPERTY ENVIRONMENT "QT_LOGGING_RULES=*.debug=false")
set_tests_properties(test_us_analysis_profile_run_report_gui PROPERTIES TIMEOUT 180)

# Runs each case in its own process and compares it with its record
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_executable(test_us_analysis_profile_inherit_gui test_us_analysis_profile_inherit_gui.cpp)
target_link_libraries(test_us_analysis_profile_inherit_gui PRIVATE us3_gui_test_support us_analysis_profile_lib)
add_test(NAME test_us_analysis_profile_inherit_gui
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/check_snapshots.py
                $<TARGET_FILE:test_us_analysis_profile_inherit_gui>)
set_tests_properties(test_us_analysis_profile_inherit_gui PROPERTIES LABELS "qtest;headless;programs" TIMEOUT 180)
