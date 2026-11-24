# us_autoflow_analysis

# Main program sources from us_autoflow_analysis directory
set(AUTOFLOW_SOURCES
        us_autoflow_analysis_main.cpp
        us_autoflow_analysis.cpp
        ../us_edited_data_loaders.cpp
        )

set(AUTOFLOW_HEADERS
        us_autoflow_analysis.h
        ../us_edited_data_loaders.h
        )

# Cross-directory sources exactly as specified in .pro file
set(CROSS_DIR_SOURCES
        ../us_abde/us_norm_profile.cpp
        ../us_mwl_species_fit/us_load_run_noise.cpp
        ../us_mwl_species_fit/us_mwl_sf_plot3d.cpp
        ../us_mwl_species_fit/us_mwl_species_fit.cpp
        )

# Cross-directory headers from .pro file
set(CROSS_DIR_HEADERS
        ../us_abde/us_norm_profile.h
        ../us_mwl_species_fit/us_load_run_noise.h
        ../us_mwl_species_fit/us_mwl_sf_plot3d.h
        ../us_mwl_species_fit/us_mwl_species_fit.h
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
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_mwl_species_fit
        )

# Link libraries
target_link_libraries(us_autoflow_analysis PRIVATE
        us_program_common
        us_2dsa_objects
        us_analysis_profile_objects
        us_edit_objects
        us_fit_meniscus_objects
        us_mwl_species_sim_objects
        Qt::DataVisualization
        )

# win32:LIBS += -lpsapi  (from the .pro file)
if(WIN32)
  target_link_libraries(us_autoflow_analysis PRIVATE psapi)
endif()

# Install
install(TARGETS us_autoflow_analysis
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_autoflow_analysis")
