# us_2dsa

# Shared 2DSA sources in the analysis area, also listed in us_programs_analysis.pro.
set(TWODSA_SOURCES
        us_2dsa.cpp
        us_plot_control_2d.cpp
        us_resplot_2d.cpp
        us_analysis_control_2d.cpp
        us_adv_analysis_2d.cpp
        us_2dsa_process.cpp
        us_worker_2d.cpp
        us_show_norm.cpp
        us_worker_calcnorm.cpp
        )

# Shared 2DSA headers for AUTOMOC, also listed in us_programs_analysis.pro.
set(TWODSA_HEADERS
        us_2dsa.h
        us_plot_control_2d.h
        us_resplot_2d.h
        us_analysis_control_2d.h
        us_adv_analysis_2d.h
        us_2dsa_process.h
        us_worker_2d.h
        us_show_norm.h
        us_worker_calcnorm.h
        )

# Everything but main(), which other programs use too, goes into us_programs_analysis.
add_library(us_2dsa_lib STATIC
        ${TWODSA_SOURCES}
        ${TWODSA_HEADERS}
        )

set_target_properties(us_2dsa_lib PROPERTIES
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_2dsa_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(us_2dsa_lib PUBLIC
        us_program_common
        Qt::DataVisualization   # QT += datavisualization
        )

target_link_libraries(us_programs_analysis INTERFACE us_2dsa_lib)

# win32:LIBS += -lpsapi  (from the .pro file)
if(WIN32)
  target_link_libraries(us_2dsa_lib PUBLIC psapi)
endif()

# Find UI and resource files
file(GLOB TWODSA_FORMS "*.ui")
file(GLOB TWODSA_RESOURCES "*.qrc")

# Create the executable
add_executable(us_2dsa
        us_2dsa_main.cpp
        ${TWODSA_FORMS}
        ${TWODSA_RESOURCES}
        )

# Set target properties
set_target_properties(us_2dsa PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_2dsa
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )
if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set_target_properties(us_2dsa PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# Link libraries
target_link_libraries(us_2dsa PRIVATE
        us_program_common
        us_programs_analysis
        )

# Install
install(TARGETS us_2dsa
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_2dsa")
