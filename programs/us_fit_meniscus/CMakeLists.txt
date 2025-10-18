# us_fit_meniscus

# Program sources as specified in .pro file
set(FIT_MENISCUS_MAIN us_fit_meniscus_main.cpp)
set(FIT_MENISCUS_SOURCES us_fit_meniscus.cpp)
set(FIT_MENISCUS_HEADERS us_fit_meniscus.h)

# Find UI and resource files
file(GLOB FIT_MENISCUS_FORMS "*.ui")
file(GLOB FIT_MENISCUS_RESOURCES "*.qrc")

# Create library for the implementation
add_library(us_fit_meniscus_lib
        ${FIT_MENISCUS_SOURCES}
        ${FIT_MENISCUS_HEADERS}
        ${FIT_MENISCUS_FORMS}
        ${FIT_MENISCUS_RESOURCES}
        )

# Set library properties
set_target_properties(us_fit_meniscus_lib PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        POSITION_INDEPENDENT_CODE ON
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Apple-specific: Force symbol export
if(APPLE)
    target_link_options(us_fit_meniscus_lib PRIVATE
            "LINKER:-export_dynamic"
            "LINKER:-all_load"
            )
    target_compile_options(us_fit_meniscus_lib PRIVATE -fvisibility=default)

    set_target_properties(us_fit_meniscus_lib PROPERTIES
            CXX_VISIBILITY_PRESET default
            C_VISIBILITY_PRESET default
            VISIBILITY_INLINES_HIDDEN OFF
            )
endif()

# Include directories for library
target_include_directories(us_fit_meniscus_lib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        )

# Link libraries for the lib
target_link_libraries(us_fit_meniscus_lib
        PUBLIC us_program_common
        )

# Create the executable
add_executable(us_fit_meniscus ${FIT_MENISCUS_MAIN})

# Set executable properties
set_target_properties(us_fit_meniscus PROPERTIES
        OUTPUT_NAME us_fit_meniscus
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )

# Link executable to library
target_link_libraries(us_fit_meniscus PRIVATE us_fit_meniscus_lib)

# Install both targets
install(TARGETS us_fit_meniscus
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

install(TARGETS us_fit_meniscus_lib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        COMPONENT Libraries
        )

message(STATUS "Created program: us_fit_meniscus with library + executable structure")