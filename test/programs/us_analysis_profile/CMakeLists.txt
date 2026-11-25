# The test profile does not build the programs, so the test compiles the
# us_analysis_profile sources it uses.
set(US_ANALYSIS_PROFILE_DIR ${CMAKE_SOURCE_DIR}/programs/us_analysis_profile)

us3_add_gui_test(test_us_analysis_profile_run_report_gui
        SOURCES
            test_us_analysis_profile_run_report_gui.cpp
            ${US_ANALYSIS_PROFILE_DIR}/us_analysis_profile.cpp
            ${US_ANALYSIS_PROFILE_DIR}/us_analysis_profile.h
            ${US_ANALYSIS_PROFILE_DIR}/us_anapro_utils.cpp
        LIBRARIES
            UltraScan3::Gui_Static
        INCLUDE_DIRECTORIES
            ${CMAKE_SOURCE_DIR}/programs
            ${US_ANALYSIS_PROFILE_DIR}
        LABELS
            programs
        )
# Check list bounds in release builds too.
target_compile_definitions(test_us_analysis_profile_run_report_gui PRIVATE QT_FORCE_ASSERTS)
set_property(TEST test_us_analysis_profile_run_report_gui APPEND PROPERTY ENVIRONMENT "QT_LOGGING_RULES=*.debug=false")
set_tests_properties(test_us_analysis_profile_run_report_gui PROPERTIES TIMEOUT 180)
