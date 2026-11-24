# us_autoflow_analysis

# Program sources from us_autoflow_analysis directory, without us_autoflow_analysis_main.cpp
set(AUTOFLOW_SOURCES
        us_autoflow_analysis.cpp
        )

set(AUTOFLOW_HEADERS
        us_autoflow_analysis.h
        )

# Everything but main(), which other programs use too, goes into us_programs_gmp.
add_library(us_autoflow_analysis_lib STATIC
        ${AUTOFLOW_SOURCES}
        ${AUTOFLOW_HEADERS}
        )

set_target_properties(us_autoflow_analysis_lib PROPERTIES
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_autoflow_analysis_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(us_autoflow_analysis_lib PUBLIC
        us_program_common
        us_2dsa_lib
        us_abde_lib
        us_analysis_profile_lib
        us_edit_lib
        us_edited_data_loaders_lib
        us_fit_meniscus_lib
        us_mwl_species_fit_lib
        us_mwl_species_sim_lib
        Qt::DataVisualization
        )

target_link_libraries(us_programs_gmp INTERFACE us_autoflow_analysis_lib)

# win32:LIBS += -lpsapi  (from the .pro file)
if(WIN32)
  target_link_libraries(us_autoflow_analysis_lib PUBLIC psapi)
endif()

# Find UI and resource files
file(GLOB AUTOFLOW_FORMS "*.ui")
file(GLOB AUTOFLOW_RESOURCES "*.qrc")

# Create the executable
add_executable(us_autoflow_analysis
        us_autoflow_analysis_main.cpp
        ${AUTOFLOW_FORMS}
        ${AUTOFLOW_RESOURCES}
        )

# Set target properties
set_target_properties(us_autoflow_analysis PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_autoflow_analysis
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Link libraries
target_link_libraries(us_autoflow_analysis PRIVATE
        us_program_common
        us_programs_gmp
        )

# Install
install(TARGETS us_autoflow_analysis
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_autoflow_analysis")
