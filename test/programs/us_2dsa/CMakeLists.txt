# Tests for us_2dsa:  the models of a meniscus scan against separate fits, and
# the analysis dialog's Monte Carlo and noise options.  The test profile does
# not build the programs, so the tests compile the us_2dsa sources they use.
set(US_2DSA_DIR ${CMAKE_SOURCE_DIR}/programs/us_2dsa)

us3_add_gui_test(test_us_2dsa_process
        SOURCES
            test_us_2dsa_process.cpp
            ${US_2DSA_DIR}/us_2dsa_process.cpp
            ${US_2DSA_DIR}/us_2dsa_process.h
            ${US_2DSA_DIR}/us_worker_2d.cpp
            ${US_2DSA_DIR}/us_worker_2d.h
        LIBRARIES
            UltraScan3::Gui_Static
            Qt::DataVisualization
        INCLUDE_DIRECTORIES
            ${US_2DSA_DIR}
        LABELS
            programs;analysis
        )

# It runs a meniscus scan and a separate fit at each of its meniscus values.
set_tests_properties(test_us_2dsa_process PROPERTIES TIMEOUT 600)

# The dialog, compiled with the rest of us_2dsa as the program compiles it
us3_add_gui_test(test_us_2dsa_analysis_control_gui
        SOURCES
            test_us_2dsa_analysis_control_gui.cpp
            ${US_2DSA_DIR}/us_2dsa.cpp
            ${US_2DSA_DIR}/us_2dsa.h
            ${US_2DSA_DIR}/us_plot_control_2d.cpp
            ${US_2DSA_DIR}/us_plot_control_2d.h
            ${US_2DSA_DIR}/us_resplot_2d.cpp
            ${US_2DSA_DIR}/us_resplot_2d.h
            ${US_2DSA_DIR}/us_analysis_control_2d.cpp
            ${US_2DSA_DIR}/us_analysis_control_2d.h
            ${US_2DSA_DIR}/us_adv_analysis_2d.cpp
            ${US_2DSA_DIR}/us_adv_analysis_2d.h
            ${US_2DSA_DIR}/us_2dsa_process.cpp
            ${US_2DSA_DIR}/us_2dsa_process.h
            ${US_2DSA_DIR}/us_worker_2d.cpp
            ${US_2DSA_DIR}/us_worker_2d.h
            ${US_2DSA_DIR}/us_show_norm.cpp
            ${US_2DSA_DIR}/us_show_norm.h
            ${US_2DSA_DIR}/us_worker_calcnorm.cpp
            ${US_2DSA_DIR}/us_worker_calcnorm.h
        LIBRARIES
            UltraScan3::Gui_Static
            Qt::DataVisualization
        INCLUDE_DIRECTORIES
            ${US_2DSA_DIR}
        LABELS
            programs;analysis
        )

# us_2dsa reads its memory use through psapi on Windows
if(WIN32)
    target_link_libraries(test_us_2dsa_process PRIVATE psapi)
    target_link_libraries(test_us_2dsa_analysis_control_gui PRIVATE psapi)
endif()
