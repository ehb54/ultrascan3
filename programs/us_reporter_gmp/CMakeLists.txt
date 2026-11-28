# us_reporter_gmp

# Program sources from us_reporter_gmp directory, without us_reporter_gmp_main.cpp
set(REPORTER_GMP_SOURCES
        us_reporter_gmp.cpp
        )

set(REPORTER_GMP_HEADERS
        us_reporter_gmp.h
        us_reporter_gmp_duration.h
        )

# Everything but main(), which other programs use too, goes into us_programs_gmp.
add_library(us_reporter_gmp_lib STATIC
        ${REPORTER_GMP_SOURCES}
        ${REPORTER_GMP_HEADERS}
        )

set_target_properties(us_reporter_gmp_lib PROPERTIES
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_reporter_gmp_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(us_reporter_gmp_lib PUBLIC
        us_program_common
        us_abde_lib
        us_analysis_profile_lib
        us_ddist_combine_lib
        us_edited_data_loaders_lib
        us_integral_lib
        us_pseudo3d_combine_lib
        Qt::DataVisualization
        )

# dependency on revision.h in programs/us
add_dependencies(us_reporter_gmp_lib us_version_headers)

target_link_libraries(us_programs_gmp INTERFACE us_reporter_gmp_lib)

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
        us_programs_gmp
        )

# Install
install(TARGETS us_reporter_gmp
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_reporter_gmp")
