# us_autoflow_analysis

# Find required Qt components based on .pro file
find_package(Qt5 REQUIRED COMPONENTS
        Widgets Core Gui Network Sql Xml Svg OpenGL PrintSupport DataVisualization
        )

# Main program sources from us_autoflow_analysis directory
set(AUTOFLOW_SOURCES
        us_autoflow_analysis_main.cpp
        us_autoflow_analysis.cpp
        )

set(AUTOFLOW_HEADERS
        us_autoflow_analysis.h
        )

# Cross-directory sources exactly as specified in .pro file
set(CROSS_DIR_SOURCES
        # us_analysis_profile
        ../us_analysis_profile/us_analysis_profile.cpp
        ../us_analysis_profile/us_anapro_utils.cpp

        # us_fit_meniscus
        ../us_fit_meniscus/us_fit_meniscus.cpp

        # us_reporter_gmp
        ../us_reporter_gmp/us_reporter_gmp.cpp

        # us_fematch
        ../us_fematch/us_fematch.cpp
        ../us_fematch/us_plot_control_fem.cpp
        ../us_fematch/us_resplot_fem.cpp
        ../us_fematch/us_advanced_fem.cpp
        ../us_fematch/us_adv_dmgamc.cpp
        ../us_fematch/us_dmga_mc_stats.cpp
        ../us_fematch/us_thread_worker.cpp

        # us_ddist_combine
        ../us_ddist_combine/us_ddist_combine.cpp
        ../us_ddist_combine/us_select_rundd.cpp

        # us_pseudo3d_combine
        ../us_pseudo3d_combine/us_pseudo3d_combine.cpp
        ../us_pseudo3d_combine/us_remove_distros.cpp

        # us_mwl_species_fit
        ../us_mwl_species_fit/us_mwl_species_fit.cpp
        ../us_mwl_species_fit/us_mwl_sf_plot3d.cpp
        ../us_mwl_species_fit/us_load_run_noise.cpp

        # us_convert
        ../us_convert/us_convert.cpp
        ../us_convert/us_experiment.cpp
        ../us_convert/us_experiment_gui.cpp
        ../us_convert/us_convert_gui.cpp
        ../us_convert/us_convertio.cpp
        ../us_convert/us_get_run.cpp
        ../us_convert/us_intensity.cpp
        ../us_convert/us_selectbox.cpp
        ../us_convert/us_select_triples.cpp

        # us_abde
        ../us_abde/us_norm_profile.cpp
        )

# Cross-directory headers from .pro file
set(CROSS_DIR_HEADERS
        ../us_analysis_profile/us_analysis_profile.h
        ../us_fit_meniscus/us_fit_meniscus.h
        ../us_reporter_gmp/us_reporter_gmp.h
        ../us_fematch/us_fematch.h
        ../us_fematch/us_plot_control_fem.h
        ../us_fematch/us_resplot_fem.h
        ../us_fematch/us_advanced_fem.h
        ../us_fematch/us_adv_dmgamc.h
        ../us_fematch/us_dmga_mc_stats.h
        ../us_fematch/us_thread_worker.h
        ../us_ddist_combine/us_ddist_combine.h
        ../us_ddist_combine/us_select_rundd.h
        ../us_pseudo3d_combine/us_pseudo3d_combine.h
        ../us_pseudo3d_combine/us_remove_distros.h
        ../us_mwl_species_fit/us_mwl_species_fit.h
        ../us_mwl_species_fit/us_mwl_sf_plot3d.h
        ../us_mwl_species_fit/us_load_run_noise.h
        ../us_convert/us_convert.h
        ../us_convert/us_experiment.h
        ../us_convert/us_experiment_gui.h
        ../us_convert/us_convert_gui.h
        ../us_convert/us_convertio.h
        ../us_convert/us_get_run.h
        ../us_convert/us_intensity.h
        ../us_convert/us_selectbox.h
        ../us_convert/us_select_triples.h
        ../us_abde/us_norm_profile.h
        )

# Find UI and resource files
file(GLOB AUTOFLOW_FORMS "*.ui")
file(GLOB AUTOFLOW_RESOURCES "*.qrc")

# Create the executable
add_executable(us_autoflow_analysis
        ${AUTOFLOW_SOURCES}
        ${AUTOFLOW_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${AUTOFLOW_FORMS}
        ${AUTOFLOW_RESOURCES}
        )

# Set target properties
set_target_properties(us_autoflow_analysis PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_autoflow_analysis
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_autoflow_analysis PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_analysis_profile
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fit_meniscus
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_reporter_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fematch
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_ddist_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_pseudo3d_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_mwl_species_fit
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_convert
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        )

# Link libraries
target_link_libraries(us_autoflow_analysis PRIVATE
        us_program_common
        Qt5::DataVisualization
        Qt5::OpenGL
        )

# Install
install(TARGETS us_autoflow_analysis
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_autoflow_analysis")