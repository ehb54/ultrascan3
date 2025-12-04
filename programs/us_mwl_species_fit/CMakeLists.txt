# us_mwl_species_fit

# Find required Qt components
find_package(Qt5 REQUIRED COMPONENTS DataVisualization)

# Check if DataVisualization was found
if(NOT Qt5DataVisualization_FOUND)
    message(FATAL_ERROR "Qt5DataVisualization is required for us_mwl_species_fit")
endif()

# Main program sources from us_mwl_species_fit directory
file(GLOB MWL_SOURCES "*.cpp")
file(GLOB MWL_HEADERS "*.h")
file(GLOB MWL_FORMS "*.ui")
file(GLOB MWL_RESOURCES "*.qrc")

# This program needs US_AnalysisProfileGui class
set(CROSS_DIR_SOURCES
        ../us_analysis_profile/us_analysis_profile.cpp
        ../us_analysis_profile/us_anapro_utils.cpp
        )

set(CROSS_DIR_HEADERS
        ../us_analysis_profile/us_analysis_profile.h
        )

# Create the executable
add_executable(us_mwl_species_fit
        ${MWL_SOURCES}
        ${MWL_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
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
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_analysis_profile
        )

# Link libraries - Qt DataVisualization is critical for this program
target_link_libraries(us_mwl_species_fit PRIVATE
        us_program_common
        Qt::DataVisualization
        )

# Install
install(TARGETS us_mwl_species_fit
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created program: us_mwl_species_fit")
