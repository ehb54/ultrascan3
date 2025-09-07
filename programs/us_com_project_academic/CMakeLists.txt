# us_com_project_academic

# Find required Qt components
find_package(Qt5 REQUIRED COMPONENTS DataVisualization)

# Main program sources from us_com_project_academic directory
set(COM_PROJECT_ACADEMIC_SOURCES
        us_com_project_academic_main.cpp
        )

# Core us_com_project source
set(COM_PROJECT_SOURCES
        ../us_com_project/us_com_project_gui.cpp
        )

set(COM_PROJECT_HEADERS
        ../us_com_project/us_com_project_gui.h
        )

# Cross-directory sources as specified in .pro file
set(CROSS_DIR_SOURCES
        # us_experiment
        ../us_experiment/us_experiment_gui_optima.cpp
        ../us_experiment/us_exp_utils.cpp
        ../us_experiment/us_proto_ranges.cpp

        # us_analysis_profile
        ../us_analysis_profile/us_analysis_profile.cpp
        ../us_analysis_profile/us_anapro_utils.cpp

        # us_xpn_viewer
        ../us_xpn_viewer/us_xpn_viewer_gui.cpp
        ../us_xpn_viewer/us_xpn_run_raw.cpp
        ../us_xpn_viewer/us_xpn_run_auc.cpp

        # us_convert
        ../us_convert/us_convert.cpp
        ../us_convert/us_convert_gui.cpp
        ../us_convert/us_convertio.cpp
        ../us_convert/us_get_run.cpp
        ../us_convert/us_experiment.cpp
        ../us_convert/us_experiment_gui.cpp
        ../us_convert/us_intensity.cpp
        ../us_convert/us_selectbox.cpp
        ../us_convert/us_select_triples.cpp

        # us_edit
        ../us_edit/us_edit.cpp
        ../us_edit/us_edit_scan.cpp
        ../us_edit/us_exclude_profile.cpp
        ../us_edit/us_get_edit.cpp
        ../us_edit/us_ri_noise.cpp
        ../us_edit/us_select_lambdas.cpp

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

        # us_abde
        ../us_abde/us_norm_profile.cpp
        )

# Cross-directory headers from .pro file
set(CROSS_DIR_HEADERS
        ../us_experiment/us_experiment_gui_optima.h
        ../us_analysis_profile/us_analysis_profile.h
        ../us_xpn_viewer/us_xpn_viewer_gui.h
        ../us_xpn_viewer/us_xpn_run_raw.h
        ../us_xpn_viewer/us_xpn_run_auc.h
        ../us_convert/us_convert_gui.h
        ../us_convert/us_convertio.h
        ../us_convert/us_get_run.h
        ../us_convert/us_experiment.h
        ../us_convert/us_experiment_gui.h
        ../us_convert/us_intensity.h
        ../us_convert/us_selectbox.h
        ../us_convert/us_select_triples.h
        ../us_edit/us_edit.h
        ../us_edit/us_edit_scan.h
        ../us_edit/us_exclude_profile.h
        ../us_edit/us_get_edit.h
        ../us_edit/us_ri_noise.h
        ../us_edit/us_select_lambdas.h
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
        ../us_abde/us_norm_profile.h
        )

# Find UI and resource files
file(GLOB COM_PROJECT_ACADEMIC_FORMS "*.ui")
file(GLOB COM_PROJECT_ACADEMIC_RESOURCES "*.qrc")

# Create the executable (note: TARGET = us_comproject_academic in .pro file)
add_executable(us_comproject_academic
        ${COM_PROJECT_ACADEMIC_SOURCES}
        ${COM_PROJECT_SOURCES}
        ${COM_PROJECT_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${COM_PROJECT_ACADEMIC_FORMS}
        ${COM_PROJECT_ACADEMIC_RESOURCES}
        )

# Set target properties
set_target_properties(us_comproject_academic PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_comproject_academic
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_comproject_academic PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_com_project
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_experiment
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_analysis_profile
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_xpn_viewer
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_convert
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_edit
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_autoflow_analysis
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fit_meniscus
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fematch
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_reporter_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_ddist_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_pseudo3d_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_esigner_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_mwl_species_fit
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        )

# Link libraries
target_link_libraries(us_comproject_academic PRIVATE
        us_program_common
        Qt5::DataVisualization
        )

# Install
install(TARGETS us_comproject_academic
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created academic version: us_comproject_academic")