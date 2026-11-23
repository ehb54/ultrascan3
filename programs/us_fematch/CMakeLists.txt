# us_fematch

# Main program sources from us_fematch directory
set(FEMATCH_SOURCES
        us_fematch_main.cpp
        us_fematch.cpp
        us_advanced_fem.cpp
        us_adv_dmgamc.cpp
        us_dmga_mc_stats.cpp
        )

set(FEMATCH_HEADERS
        us_fematch.h
        us_advanced_fem.h
        us_adv_dmgamc.h
        us_dmga_mc_stats.h
        )

# Find UI and resource files
file(GLOB FEMATCH_FORMS "*.ui")
file(GLOB FEMATCH_RESOURCES "*.qrc")

# Create the executable
add_executable(us_fematch
        ${FEMATCH_SOURCES}
        ${FEMATCH_HEADERS}
        ${FEMATCH_FORMS}
        ${FEMATCH_RESOURCES}
        )

# Set target properties
set_target_properties(us_fematch PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_fematch
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_fematch PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

# Link libraries
target_link_libraries(us_fematch PRIVATE
        us_program_common
        Qt::DataVisualization
        )

# Install
install(TARGETS us_fematch
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_fematch")
