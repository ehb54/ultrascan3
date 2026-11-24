# us_fematch

# Program sources from us_fematch directory, without us_fematch_main.cpp
set(FEMATCH_SOURCES
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

# Everything but main(); other programs link these objects instead of
# compiling the sources again.
add_library(us_fematch_objects OBJECT
        ${FEMATCH_SOURCES}
        ${FEMATCH_HEADERS}
        )

set_target_properties(us_fematch_objects PROPERTIES
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_fematch_objects PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(us_fematch_objects PUBLIC
        us_program_common
        Qt::DataVisualization
        )

# Find UI and resource files
file(GLOB FEMATCH_FORMS "*.ui")
file(GLOB FEMATCH_RESOURCES "*.qrc")

# Create the executable
add_executable(us_fematch
        us_fematch_main.cpp
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

# Link libraries
target_link_libraries(us_fematch PRIVATE
        us_program_common
        us_fematch_objects
        )

# Install
install(TARGETS us_fematch
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_fematch")
