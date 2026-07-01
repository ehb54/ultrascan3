# us_fit_meniscus

# Program sources as specified in .pro file
set(FIT_MENISCUS_SOURCES us_fit_meniscus.cpp us_fit_meniscus_main.cpp)
set(FIT_MENISCUS_HEADERS us_fit_meniscus.h)

add_executable(us_fit_meniscus
       ${FIT_MENISCUS_SOURCES}
       ${FIT_MENISCUS_HEADERS}
       )

# Set executable properties
set_target_properties(us_fit_meniscus PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_fit_meniscus
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

target_include_directories(us_fit_meniscus PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(us_fit_meniscus PRIVATE
        us_program_common
        )

# Install executable
install(TARGETS us_fit_meniscus
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created program: us_fit_meniscus")
