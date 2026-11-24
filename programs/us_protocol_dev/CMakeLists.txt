# us_protocol_dev

# Main program sources from us_protocol_dev directory
set(PROTOCOL_DEV_SOURCES
        us_protocol_dev_main.cpp
        us_protocol_dev_gui.cpp
        )

set(PROTOCOL_DEV_HEADERS
        us_protocol_dev_gui.h
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
file(GLOB PROTOCOL_DEV_FORMS "*.ui")
file(GLOB PROTOCOL_DEV_RESOURCES "*.qrc")

# Create the executable (note: TARGET = us_protocoldev in .pro file)
add_executable(us_protocoldev
        ${PROTOCOL_DEV_SOURCES}
        ${PROTOCOL_DEV_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${PROTOCOL_DEV_FORMS}
        ${PROTOCOL_DEV_RESOURCES}
        )

# Set target properties
set_target_properties(us_protocoldev PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_protocoldev
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_protocoldev PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_experiment
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_autoflow_analysis
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_reporter_gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_mwl_species_fit
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        )

# Link libraries
target_link_libraries(us_protocoldev PRIVATE
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
install(TARGETS us_protocoldev
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_protocoldev")
