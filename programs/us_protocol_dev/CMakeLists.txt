# us_protocol_dev

# Main program sources from us_protocol_dev directory
set(PROTOCOL_DEV_SOURCES
        us_protocol_dev_main.cpp
        us_protocol_dev_gui.cpp
        )

set(PROTOCOL_DEV_HEADERS
        us_protocol_dev_gui.h
        )

# Find UI and resource files
file(GLOB PROTOCOL_DEV_FORMS "*.ui")
file(GLOB PROTOCOL_DEV_RESOURCES "*.qrc")

# Create the executable (note: TARGET = us_protocoldev in .pro file)
add_executable(us_protocoldev
        ${PROTOCOL_DEV_SOURCES}
        ${PROTOCOL_DEV_HEADERS}
        ${PROTOCOL_DEV_FORMS}
        ${PROTOCOL_DEV_RESOURCES}
        )

# Set target properties
set_target_properties(us_protocoldev PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_protocoldev
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories
target_include_directories(us_protocoldev PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

# Link libraries
target_link_libraries(us_protocoldev PRIVATE
        us_program_common
        us_programs_gmp
        Qt::DataVisualization
        )

# Install
install(TARGETS us_protocoldev
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_protocoldev")
