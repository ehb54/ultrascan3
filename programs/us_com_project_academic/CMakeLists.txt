# us_com_project_academic

# Main program sources from us_com_project_academic directory
set(COM_PROJECT_ACADEMIC_SOURCES
        us_com_project_academic_main.cpp
        )

# Find UI and resource files
file(GLOB COM_PROJECT_ACADEMIC_FORMS "*.ui")
file(GLOB COM_PROJECT_ACADEMIC_RESOURCES "*.qrc")

# Create the executable (note: TARGET = us_comproject_academic in .pro file)
add_executable(us_comproject_academic
        ${COM_PROJECT_ACADEMIC_SOURCES}
        ${COM_PROJECT_ACADEMIC_FORMS}
        ${COM_PROJECT_ACADEMIC_RESOURCES}
        )

# Set target properties
set_target_properties(us_comproject_academic PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_comproject_academic
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories
target_include_directories(us_comproject_academic PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

# Link libraries
target_link_libraries(us_comproject_academic PRIVATE
        us_program_common
        us_programs_comproject
        Qt::DataVisualization
        )

# Install
install(TARGETS us_comproject_academic
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created academic version: us_comproject_academic")
