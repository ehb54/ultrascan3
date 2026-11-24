# us_com_project

# Main program sources from us_com_project directory
set(COM_PROJECT_SOURCES
        us_com_project_main.cpp
        us_com_project_gui.cpp
        )

set(COM_PROJECT_HEADERS
        us_com_project_gui.h
        )

# Cross-directory sources as specified in .pro file
set(CROSS_DIR_SOURCES
        # us_experiment
        ../us_experiment/us_experiment_gui_optima.cpp
        ../us_experiment/us_exp_utils.cpp
        ../us_experiment/us_proto_ranges.cpp

        # us_xpn_viewer
        ../us_xpn_viewer/us_xpn_viewer_gui.cpp
        ../us_xpn_viewer/us_xpn_run_raw.cpp
        ../us_xpn_viewer/us_xpn_run_auc.cpp

        # us_autoflow_analysis
        ../us_autoflow_analysis/us_autoflow_analysis.cpp

        # shared edited-data loaders
        ../us_edited_data_loaders.cpp

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

        # us_integral
        ../us_integral/us_integral.cpp
        ../us_integral/us_delete_models.cpp
        )

# Cross-directory headers from .pro file
set(CROSS_DIR_HEADERS
        ../us_experiment/us_experiment_gui_optima.h
        ../us_xpn_viewer/us_xpn_viewer_gui.h
        ../us_xpn_viewer/us_xpn_run_raw.h
        ../us_xpn_viewer/us_xpn_run_auc.h
        ../us_autoflow_analysis/us_autoflow_analysis.h
        ../us_reporter_gmp/us_reporter_gmp.h
        ../us_edited_data_loaders.h
        ../us_ddist_combine/us_ddist_combine.h
        ../us_ddist_combine/us_select_rundd.h
        ../us_pseudo3d_combine/us_pseudo3d_combine.h
        ../us_pseudo3d_combine/us_remove_distros.h
        ../us_esigner_gmp/us_esigner_gmp.h
        ../us_mwl_species_fit/us_mwl_species_fit.h
        ../us_mwl_species_fit/us_mwl_sf_plot3d.h
        ../us_mwl_species_fit/us_load_run_noise.h
        ../us_abde/us_norm_profile.h
        ../us_integral/us_integral.h
        ../us_integral/us_delete_models.h
        )

# Find UI and resource files
file(GLOB COM_PROJECT_FORMS "*.ui")
file(GLOB COM_PROJECT_RESOURCES "*.qrc")

# Create the executable (note: TARGET = us_comproject in .pro file)
add_executable(us_com_project
        ${COM_PROJECT_SOURCES}
        ${COM_PROJECT_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${COM_PROJECT_FORMS}
        ${COM_PROJECT_RESOURCES}
        )

# Set target properties
set_target_properties(us_com_project PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_comproject
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_com_project PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_experiment
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_xpn_viewer
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_autoflow_analysis
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_reporter_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_ddist_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_pseudo3d_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_esigner_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_mwl_species_fit
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_integral
        )

# Link libraries
target_link_libraries(us_com_project PRIVATE
        us_program_common
        us_2dsa_objects
        us_analysis_profile_objects
        us_edit_objects
        us_fit_meniscus_objects
        us_mwl_species_sim_objects
        Qt::DataVisualization  # Required by .pro file
        )

# Set the binary output name
set_target_properties(us_com_project PROPERTIES
        OUTPUT_NAME us_comproject  # Binary will be named "us_comproject"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )

# Install
install(TARGETS us_com_project
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_com_project")
