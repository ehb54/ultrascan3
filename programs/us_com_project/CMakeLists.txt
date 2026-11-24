# us_com_project

# Program sources from us_com_project directory, without us_com_project_main.cpp
set(COM_PROJECT_SOURCES
        us_com_project_gui.cpp
        )

set(COM_PROJECT_HEADERS
        us_com_project_gui.h
        )

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_com_project_objects OBJECT
        ${COM_PROJECT_SOURCES}
        ${COM_PROJECT_HEADERS}
        )

set_target_properties(us_com_project_objects PROPERTIES
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_com_project_objects PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(us_com_project_objects PUBLIC
        us_program_common
        us_autoflow_analysis_objects
        us_edit_objects
        us_esigner_gmp_objects
        us_experiment_objects
        us_reporter_gmp_objects
        us_xpn_viewer_objects
        Qt::DataVisualization  # Required by .pro file
        )

# Find UI and resource files
file(GLOB COM_PROJECT_FORMS "*.ui")
file(GLOB COM_PROJECT_RESOURCES "*.qrc")

# Create the executable (note: TARGET = us_comproject in .pro file)
add_executable(us_com_project
        us_com_project_main.cpp
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

# Link libraries
target_link_libraries(us_com_project PRIVATE
        us_program_common
        us_2dsa_objects
        us_abde_objects
        us_analysis_profile_objects
        us_autoflow_analysis_objects
        us_com_project_objects
        us_ddist_combine_objects
        us_edit_objects
        us_edited_data_loaders_objects
        us_esigner_gmp_objects
        us_experiment_objects
        us_fit_meniscus_objects
        us_integral_objects
        us_mwl_species_fit_objects
        us_mwl_species_sim_objects
        us_pseudo3d_combine_objects
        us_reporter_gmp_objects
        us_xpn_viewer_objects
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
