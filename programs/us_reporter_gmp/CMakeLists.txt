# us_reporter_gmp

# Program sources from us_reporter_gmp directory, without us_reporter_gmp_main.cpp
set(REPORTER_GMP_SOURCES
        us_reporter_gmp.cpp
        )

set(REPORTER_GMP_HEADERS
        us_reporter_gmp.h
        us_reporter_gmp_duration.h
        )

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_reporter_gmp_objects OBJECT
        ${REPORTER_GMP_SOURCES}
        ${REPORTER_GMP_HEADERS}
        )

set_target_properties(us_reporter_gmp_objects PROPERTIES
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_reporter_gmp_objects PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(us_reporter_gmp_objects PUBLIC
        us_program_common
        us_abde_objects
        us_analysis_profile_objects
        us_ddist_combine_objects
        us_edited_data_loaders_objects
        us_integral_objects
        us_pseudo3d_combine_objects
        Qt::DataVisualization
        )

# dependency on revision.h in programs/us
add_dependencies(us_reporter_gmp_objects us_version_headers)

# Find UI and resource files
file(GLOB REPORTER_GMP_FORMS "*.ui")
file(GLOB REPORTER_GMP_RESOURCES "*.qrc")

# Create the executable
add_executable(us_reporter_gmp
        us_reporter_gmp_main.cpp
        ${REPORTER_GMP_FORMS}
        ${REPORTER_GMP_RESOURCES}
        )

# Set target properties
set_target_properties(us_reporter_gmp PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_reporter_gmp
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Link libraries
target_link_libraries(us_reporter_gmp PRIVATE
        us_program_common
        us_abde_objects
        us_analysis_profile_objects
        us_ddist_combine_objects
        us_edited_data_loaders_objects
        us_integral_objects
        us_pseudo3d_combine_objects
        us_reporter_gmp_objects
        )

# Install
install(TARGETS us_reporter_gmp
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_reporter_gmp")
