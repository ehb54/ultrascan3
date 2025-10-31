# us_query_rmsd

# Find required Qt components
find_package(Qt5 REQUIRED COMPONENTS DataVisualization)

# Main program sources from us_query_rmsd directory
set(QUERY_RMSD_SOURCES
        us_query_rmsd.cpp
        )

set(QUERY_RMSD_HEADERS
        us_query_rmsd.h
        )

# Cross-directory sources as specified in .pro file
set(CROSS_DIR_SOURCES
        # us_analysis_profile
        ../us_analysis_profile/us_analysis_profile.cpp
        ../us_analysis_profile/us_anapro_utils.cpp

        # us_autoflow_analysis
        ../us_autoflow_analysis/us_autoflow_analysis.cpp

        # us_fit_meniscus
        ../us_fit_meniscus/us_fit_meniscus.cpp

        # us_fematch
        ../us_fematch/us_fematch.cpp
        ../us_fematch/us_plot_control_fem.cpp
        ../us_fematch/us_resplot_fem.cpp
        ../us_fematch/us_advanced_fem.cpp
        ../us_fematch/us_adv_dmgamc.cpp
        ../us_fematch/us_dmga_mc_stats.cpp
        ../us_fematch/us_thread_worker.cpp

        # us_reporter_gmp
        ../us_reporter_gmp/us_reporter_gmp.cpp

        # us_ddist_combine
        ../us_ddist_combine/us_ddist_combine.cpp
        ../us_ddist_combine/us_select_rundd.cpp

        # us_pseudo3d_combine
        ../us_pseudo3d_combine/us_pseudo3d_combine.cpp
        ../us_pseudo3d_combine/us_remove_distros.cpp

        # us_esigner_gmp
        ../us_esigner_gmp/us_esigner_gmp.cpp

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
        ../us_autoflow_analysis/us_autoflow_analysis.h
        ../us_fit_meniscus/us_fit_meniscus.h
        ../us_fematch/us_fematch.h
        ../us_fematch/us_plot_control_fem.h
        ../us_fematch/us_resplot_fem.h
        ../us_fematch/us_advanced_fem.h
        ../us_fematch/us_adv_dmgamc.h
        ../us_fematch/us_dmga_mc_stats.h
        ../us_fematch/us_thread_worker.h
        ../us_reporter_gmp/us_reporter_gmp.h
        ../us_ddist_combine/us_ddist_combine.h
        ../us_ddist_combine/us_select_rundd.h
        ../us_pseudo3d_combine/us_pseudo3d_combine.h
        ../us_pseudo3d_combine/us_remove_distros.h
        ../us_esigner_gmp/us_esigner_gmp.h
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
file(GLOB QUERY_RMSD_FORMS "*.ui")
file(GLOB QUERY_RMSD_RESOURCES "*.qrc")

# Create the executable
add_executable(us_query_rmsd
        ${QUERY_RMSD_SOURCES}
        ${QUERY_RMSD_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${QUERY_RMSD_FORMS}
        ${QUERY_RMSD_RESOURCES}
        )

# Set target properties
set_target_properties(us_query_rmsd PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_query_rmsd
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_query_rmsd PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_analysis_profile
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_autoflow_analysis
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fit_meniscus
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fematch
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_reporter_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_ddist_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_pseudo3d_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_esigner_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_mwl_species_fit
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_convert
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        )

# Link libraries
target_link_libraries(us_query_rmsd PRIVATE
        us_program_common
        Qt::DataVisualization
        )

# Install
install(TARGETS us_query_rmsd
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_query_rmsd")