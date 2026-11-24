# us_experiment

# Program sources from us_experiment directory, without us_experiment_main.cpp
set(EXPERIMENT_SOURCES
        us_experiment_gui_optima.cpp
        us_exp_utils.cpp
        us_proto_ranges.cpp
        )

set(EXPERIMENT_HEADERS
        us_experiment_gui_optima.h
        )

# Everything but main(), which other programs use too, goes into us_programs_experiment.
add_library(us_experiment_lib STATIC
        ${EXPERIMENT_SOURCES}
        ${EXPERIMENT_HEADERS}
        )

set_target_properties(us_experiment_lib PROPERTIES
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_experiment_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(us_experiment_lib PUBLIC
        us_program_common
        us_analysis_profile_lib
        )

target_link_libraries(us_programs_experiment INTERFACE us_experiment_lib)

# Find UI and resource files
file(GLOB EXPERIMENT_FORMS "*.ui")
file(GLOB EXPERIMENT_RESOURCES "*.qrc")

# Create the executable
add_executable(us_experiment
        us_experiment_main.cpp
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

# Link libraries
target_link_libraries(us_experiment PRIVATE
        us_program_common
        us_programs_experiment
        )

# Install
install(TARGETS us_experiment
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created program: us_experiment")
