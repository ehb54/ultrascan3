# us_reporter_gmp

# Main program sources from us_reporter_gmp directory
set(REPORTER_GMP_SOURCES
        us_reporter_gmp_main.cpp
        us_reporter_gmp.cpp
        )

set(REPORTER_GMP_HEADERS
        us_reporter_gmp.h
        us_reporter_gmp_duration.h
        )

# Cross-directory sources as specified in .pro file
set(CROSS_DIR_SOURCES
        ../us_abde/us_norm_profile.cpp
        )

# Cross-directory headers from .pro file
set(CROSS_DIR_HEADERS
        ../us_abde/us_norm_profile.h
        )

# Find UI and resource files
file(GLOB REPORTER_GMP_FORMS "*.ui")
file(GLOB REPORTER_GMP_RESOURCES "*.qrc")

# Create the executable
add_executable(us_reporter_gmp
        ${REPORTER_GMP_SOURCES}
        ${REPORTER_GMP_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${REPORTER_GMP_FORMS}
        ${REPORTER_GMP_RESOURCES}
        )

# dependency on revision.h in programs/us
add_dependencies(us_reporter_gmp us_version_headers)

# Set target properties
set_target_properties(us_reporter_gmp PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_reporter_gmp
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_reporter_gmp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        )

# Link libraries
target_link_libraries(us_reporter_gmp PRIVATE
        us_program_common
        us_analysis_profile_objects
        us_ddist_combine_objects
        us_edited_data_loaders_objects
        us_integral_objects
        us_pseudo3d_combine_objects
        Qt::DataVisualization
        )

# Install
install(TARGETS us_reporter_gmp
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_reporter_gmp")
