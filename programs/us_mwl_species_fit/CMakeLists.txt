# us_mwl_species_fit

# Program sources from us_mwl_species_fit directory, without us_mwl_species_fit_main.cpp
set(MWL_SOURCES
        us_mwl_species_fit.cpp
        us_mwl_sf_plot3d.cpp
        us_load_run_noise.cpp
        )

set(MWL_HEADERS
        us_mwl_species_fit.h
        us_mwl_sf_plot3d.h
        us_load_run_noise.h
        )

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_mwl_species_fit_objects OBJECT
        ${MWL_SOURCES}
        ${MWL_HEADERS}
        )

set_target_properties(us_mwl_species_fit_objects PROPERTIES
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_mwl_species_fit_objects PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

# Qt DataVisualization is critical for this program
target_link_libraries(us_mwl_species_fit_objects PUBLIC
        us_program_common
        us_analysis_profile_objects
        Qt::DataVisualization
        )

# Find UI and resource files
file(GLOB MWL_FORMS "*.ui")
file(GLOB MWL_RESOURCES "*.qrc")

# Create the executable
add_executable(us_mwl_species_fit
        us_mwl_species_fit_main.cpp
        ${MWL_FORMS}
        ${MWL_RESOURCES}
        )

# Set target properties
set_target_properties(us_mwl_species_fit PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_mwl_species_fit
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Link libraries
target_link_libraries(us_mwl_species_fit PRIVATE
        us_program_common
        us_analysis_profile_objects
        us_mwl_species_fit_objects
        )

# Install
install(TARGETS us_mwl_species_fit
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created program: us_mwl_species_fit")
