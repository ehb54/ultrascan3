# us_query_rmsd

# Main program sources from us_query_rmsd directory
set(QUERY_RMSD_SOURCES
        us_query_rmsd.cpp
        )

set(QUERY_RMSD_HEADERS
        us_query_rmsd.h
        )

# Cross-directory sources as specified in .pro file
set(CROSS_DIR_SOURCES
        ../us_fematch/us_adv_dmgamc.cpp
        ../us_fematch/us_advanced_fem.cpp
        ../us_fematch/us_dmga_mc_stats.cpp
        ../us_fematch/us_fematch.cpp
        )

# Cross-directory headers from .pro file
set(CROSS_DIR_HEADERS
        ../us_fematch/us_adv_dmgamc.h
        ../us_fematch/us_advanced_fem.h
        ../us_fematch/us_dmga_mc_stats.h
        ../us_fematch/us_fematch.h
        )

# Find UI and resource files
file(GLOB QUERY_RMSD_FORMS "*.ui")
file(GLOB QUERY_RMSD_RESOURCES "*.qrc")

# Create the executable
add_executable(us_query_rmsd
        ${QUERY_RMSD_SOURCES}
        ${QUERY_RMSD_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${QUERY_RMSD_FORMS}
        ${QUERY_RMSD_RESOURCES}
        )

# Set target properties
set_target_properties(us_query_rmsd PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_query_rmsd
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_query_rmsd PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fematch
        )

# Link libraries
target_link_libraries(us_query_rmsd PRIVATE
        us_program_common
        Qt::DataVisualization
        )

# Install
install(TARGETS us_query_rmsd
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_query_rmsd")
