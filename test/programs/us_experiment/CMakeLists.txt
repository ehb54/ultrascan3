# The test profile does not build the programs, so the test compiles the
# us_experiment and us_analysis_profile sources it uses.
set(US_EXPERIMENT_DIR ${CMAKE_SOURCE_DIR}/programs/us_experiment)
set(US_ANALYSIS_PROFILE_DIR ${CMAKE_SOURCE_DIR}/programs/us_analysis_profile)

us3_add_gui_test(test_us_experiment_disk_gui
        SOURCES
            test_us_experiment_disk_gui.cpp
            ${US_EXPERIMENT_DIR}/us_experiment_gui_optima.cpp
            ${US_EXPERIMENT_DIR}/us_experiment_gui_optima.h
            ${US_EXPERIMENT_DIR}/us_exp_utils.cpp
            ${US_EXPERIMENT_DIR}/us_proto_ranges.cpp
            ${US_ANALYSIS_PROFILE_DIR}/us_analysis_profile.cpp
            ${US_ANALYSIS_PROFILE_DIR}/us_analysis_profile.h
            ${US_ANALYSIS_PROFILE_DIR}/us_anapro_utils.cpp
        LIBRARIES
            UltraScan3::Gui_Static
        INCLUDE_DIRECTORIES
            ${CMAKE_SOURCE_DIR}/programs
            ${US_EXPERIMENT_DIR}
            ${US_ANALYSIS_PROFILE_DIR}
        LABELS
            programs
        )
set_property(TEST test_us_experiment_disk_gui APPEND PROPERTY ENVIRONMENT "QT_LOGGING_RULES=*.debug=false")
