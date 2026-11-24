# us_com_project_academic

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

        # us_autoflow_analysis
        ../us_autoflow_analysis/us_autoflow_analysis.cpp

        # us_reporter_gmp
        ../us_reporter_gmp/us_reporter_gmp.cpp

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
        ../us_autoflow_analysis/us_autoflow_analysis.h
        ../us_reporter_gmp/us_reporter_gmp.h
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
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_autoflow_analysis
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_reporter_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_mwl_species_fit
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        )

# Link libraries
target_link_libraries(us_comproject_academic PRIVATE
        us_program_common
        us_2dsa_objects
        us_analysis_profile_objects
        us_ddist_combine_objects
        us_edit_objects
        us_edited_data_loaders_objects
        us_esigner_gmp_objects
        us_fit_meniscus_objects
        us_integral_objects
        us_mwl_species_sim_objects
        us_pseudo3d_combine_objects
        us_xpn_viewer_objects
        Qt::DataVisualization
        )

# Install
install(TARGETS us_comproject_academic
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created academic version: us_comproject_academic")
