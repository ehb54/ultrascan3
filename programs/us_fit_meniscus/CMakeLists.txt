# us_fit_meniscus

# Program sources as specified in .pro file
set(FIT_MENISCUS_MAIN us_fit_meniscus_main.cpp)
set(FIT_MENISCUS_SOURCES us_fit_meniscus.cpp)
set(FIT_MENISCUS_HEADERS us_fit_meniscus.h)

# Find UI and resource files
file(GLOB FIT_MENISCUS_FORMS "*.ui")
file(GLOB FIT_MENISCUS_RESOURCES "*.qrc")

# Determine which library types to build
if(BUILD_TESTING)
    set(build_shared_lib FALSE)  # Skip shared lib in testing mode
    set(build_static_lib TRUE)   # Only build static for testing
else()
    set(build_shared_lib TRUE)   # Build shared lib in production
    set(build_static_lib FALSE)  # Skip static lib in production
endif()

# ============================================================================
# SHARED LIBRARY
# ============================================================================
if(build_shared_lib)
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

    # CRITICAL: Force symbol export on ALL platforms
    set_target_properties(us_fit_meniscus_lib PROPERTIES
            CXX_VISIBILITY_PRESET default
            C_VISIBILITY_PRESET default
            VISIBILITY_INLINES_HIDDEN OFF
            )

    # Apply visibility compile flags to ALL platforms
    target_compile_options(us_fit_meniscus_lib PRIVATE -fvisibility=default)

    # Platform-specific link options
    if(APPLE)
        target_link_options(us_fit_meniscus_lib PRIVATE
                "LINKER:-export_dynamic"
                "LINKER:-all_load"
                )
    elseif(UNIX)
        target_link_options(us_fit_meniscus_lib PRIVATE "-Wl,--export-dynamic")
    endif()

    # Include directories for library
    target_include_directories(us_fit_meniscus_lib
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            )

    target_link_libraries(us_fit_meniscus_lib
            PUBLIC
            us_program_common
            )

    # Create alias
    add_library(UltraScan3::us_fit_meniscus ALIAS us_fit_meniscus_lib)

    message(STATUS "Created shared library: us_fit_meniscus_lib")
endif()

# ============================================================================
# STATIC LIBRARY
# ============================================================================
if(build_static_lib)
    add_library(us_fit_meniscus_lib_static STATIC
            ${FIT_MENISCUS_SOURCES}
            ${FIT_MENISCUS_HEADERS}
            ${FIT_MENISCUS_FORMS}
            ${FIT_MENISCUS_RESOURCES}
            )

    # Set library properties
    set_target_properties(us_fit_meniscus_lib_static PROPERTIES
            AUTOMOC ON
            AUTOUIC ON
            AUTORCC ON
            POSITION_INDEPENDENT_CODE ON
            AUTOMOC_MOC_OPTIONS "--no-notes"
            OUTPUT_NAME us_fit_meniscus_lib
            )

    # Static libraries need default visibility
    set_target_properties(us_fit_meniscus_lib_static PROPERTIES
            CXX_VISIBILITY_PRESET default
            C_VISIBILITY_PRESET default
            VISIBILITY_INLINES_HIDDEN OFF
            )

    # Include directories
    target_include_directories(us_fit_meniscus_lib_static
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            )

    # Link libraries
    target_link_libraries(us_fit_meniscus_lib_static
            PUBLIC us_program_common
            )

    # Create aliases
    add_library(UltraScan3::us_fit_meniscus ALIAS us_fit_meniscus_lib_static)
    add_library(UltraScan3::us_fit_meniscus_Static ALIAS us_fit_meniscus_lib_static)

    message(STATUS "Created static library: us_fit_meniscus_lib_static (testing mode)")

    # Install static library
    install(TARGETS us_fit_meniscus_lib_static
            ARCHIVE DESTINATION lib
            COMPONENT Development
            )
endif()

# ============================================================================
# EXECUTABLE
# ============================================================================
add_executable(us_fit_meniscus ${FIT_MENISCUS_MAIN})

# Set executable properties
set_target_properties(us_fit_meniscus PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_fit_meniscus
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Link executable to appropriate library
if(TARGET us_fit_meniscus_lib_static)
    target_link_libraries(us_fit_meniscus PRIVATE us_fit_meniscus_lib_static)
    message(STATUS "us_fit_meniscus executable will link against static library")
elseif(TARGET us_fit_meniscus_lib)
    target_link_libraries(us_fit_meniscus PRIVATE us_fit_meniscus_lib)
endif()

# Install executable
install(TARGETS us_fit_meniscus
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

# Install shared library if it was built
if(TARGET us_fit_meniscus_lib)
    install(TARGETS us_fit_meniscus_lib
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            COMPONENT Libraries
            )
endif()

message(STATUS "Created program: us_fit_meniscus")
