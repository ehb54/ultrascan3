# us_mwl_species_fit

# Main program sources from us_mwl_species_fit directory
file(GLOB MWL_SOURCES "*.cpp")
file(GLOB MWL_HEADERS "*.h")
file(GLOB MWL_FORMS "*.ui")
file(GLOB MWL_RESOURCES "*.qrc")

# Create the executable
add_executable(us_mwl_species_fit
        ${MWL_SOURCES}
        ${MWL_HEADERS}
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

# Include directories
target_include_directories(us_mwl_species_fit PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

# Link libraries - Qt DataVisualization is critical for this program
target_link_libraries(us_mwl_species_fit PRIVATE
        us_program_common
        us_analysis_profile_objects
        Qt::DataVisualization
        )

# Install
install(TARGETS us_mwl_species_fit
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created program: us_mwl_species_fit")
