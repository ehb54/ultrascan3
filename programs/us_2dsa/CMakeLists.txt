# us_2dsa

# Program sources, as listed in SOURCES of us_2dsa.pro
set(TWODSA_SOURCES
        us_2dsa_main.cpp
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

# Program headers, as listed in HEADERS of us_2dsa.pro (needed by AUTOMOC)
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

# Find UI and resource files
file(GLOB TWODSA_FORMS "*.ui")
file(GLOB TWODSA_RESOURCES "*.qrc")

# Create the executable
add_executable(us_2dsa
        ${TWODSA_SOURCES}
        ${TWODSA_HEADERS}
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

# Include directories
target_include_directories(us_2dsa PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

# Link libraries
target_link_libraries(us_2dsa PRIVATE
        us_program_common
        Qt::DataVisualization   # QT += datavisualization
        )

# win32:LIBS += -lpsapi  (from the .pro file)
if(WIN32)
  target_link_libraries(us_2dsa PRIVATE psapi)
endif()

# Install
install(TARGETS us_2dsa
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_2dsa")
