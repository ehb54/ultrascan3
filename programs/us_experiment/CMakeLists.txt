# us_experiment

# Main program sources from us_experiment directory
set(EXPERIMENT_SOURCES
        us_experiment_main.cpp
        us_experiment_gui_optima.cpp
        us_exp_utils.cpp
        us_proto_ranges.cpp
        )

set(EXPERIMENT_HEADERS
        us_experiment_gui_optima.h
        )

# Cross-directory sources as specified in .pro file
set(CROSS_DIR_SOURCES
        # us_analysis_profile
        ../us_analysis_profile/us_analysis_profile.cpp
        ../us_analysis_profile/us_anapro_utils.cpp

        # us_esigner_gmp
        ../us_esigner_gmp/us_esigner_gmp.cpp
        )

# Cross-directory headers from .pro file
set(CROSS_DIR_HEADERS
        ../us_analysis_profile/us_analysis_profile.h
        ../us_esigner_gmp/us_esigner_gmp.h
        )

# Find UI and resource files
file(GLOB EXPERIMENT_FORMS "*.ui")
file(GLOB EXPERIMENT_RESOURCES "*.qrc")

# Create the executable
add_executable(us_experiment
        ${EXPERIMENT_SOURCES}
        ${EXPERIMENT_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${EXPERIMENT_FORMS}
        ${EXPERIMENT_RESOURCES}
        )

# Set target properties
set_target_properties(us_experiment PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_experiment
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_experiment PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_analysis_profile
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_esigner_gmp
        )

# Link libraries
target_link_libraries(us_experiment PRIVATE
        us_program_common
        )

# Install
install(TARGETS us_experiment
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created program: us_experiment")
