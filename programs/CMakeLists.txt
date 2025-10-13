# UltraScan3 Application Programs

cmake_minimum_required(VERSION 3.16)

# Enable Qt's automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- Generated headers setup ---
set(US3_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${US3_GENERATED_DIR})

# Version configuration
set(US3_VERSION_MAJOR 0 CACHE STRING "")
set(US3_VERSION_MINOR 0 CACHE STRING "")
set(US3_VERSION_PATCH 0 CACHE STRING "")
set(US3_VERSION_STRING "${US3_VERSION_MAJOR}.${US3_VERSION_MINOR}.${US3_VERSION_PATCH}")
set(US3_GIT_COMMIT_HASH "nogit" CACHE STRING "")
set(US3_GIT_BRANCH "nogit" CACHE STRING "")
set(US3_GIT_TAG "none" CACHE STRING "")
set(US3_BUILD_NUMBER 0 CACHE STRING "")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S %z")

configure_file(
        ${CMAKE_SOURCE_DIR}/admin/cmake/us_version.h.in
        ${US3_GENERATED_DIR}/us_version.h
        @ONLY
)

file(WRITE ${US3_GENERATED_DIR}/us_revision.h
        "#pragma once\n#include \"us_version.h\"\n#ifndef REVISION\n#define REVISION \"${US3_BUILD_NUMBER}\"\n#endif\n"
        )

# Create us/ directory in build root and copy generated headers there for legacy compatibility
set(US_LEGACY_DIR ${CMAKE_CURRENT_BINARY_DIR}/../us)
file(MAKE_DIRECTORY ${US_LEGACY_DIR})

# Copy generated headers to legacy location
configure_file(
        ${US3_GENERATED_DIR}/us_revision.h
        ${US_LEGACY_DIR}/us_revision.h
        COPYONLY
)

configure_file(
        ${US3_GENERATED_DIR}/us_version.h
        ${US_LEGACY_DIR}/us_version.h
        COPYONLY
)

# Create interface library for common program settings
add_library(us_program_common INTERFACE)

target_include_directories(us_program_common INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${US3_GENERATED_DIR}
        )

target_link_libraries(us_program_common INTERFACE
        UltraScan3::Utils
        UltraScan3::Gui
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
        Qt5::Network
        Qt5::Sql
        Qt5::Xml
        Qt5::Svg
        Qt5::OpenGL
        Qt5::PrintSupport
        Qwt::Qwt
        )

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(us_program_common INTERFACE OpenSSL::Crypto)
elseif(APPLE)
    target_link_libraries(us_program_common INTERFACE "-framework QtOpenGL")
endif()

# Programs that should be executable-only (no separate library)
set(EXECUTABLE_ONLY_PROGRAMS
        us # Main UltraScan3 launcher
        us_2dplot us_2dsa us_abde us_analyte us_astfem_sim us_audit_trail_gmp
        us_buffer us_buoyancy
        us_colorgradient us_combine_models us_config
        us_dcdt us_ddist_combine us_density_match us_dmga_init
        us_edit us_esigner_gmp us_equil_mc us_equiltime us_extinction
        us_fds_filemanager
        us_ga_init us_globalequil us_grid_editor
        us_helpdaemon
        us_integral us_investigator
        us_license
        us_manage_data us_model_gui us_modelmetrics us_mwl_species_sim us_mwl_spectra us_mwlr_viewer
        us_pcsa us_predict1 us_predict2 us_project us_pseudo3d_combine us_pseudo_absorbance
        us_ramp us_rasmol_control us_reporter us_rotor us_rotor_calibration
        us_sassoc us_second_moment
        us_solution us_spectrum
        us_tmst_viewer
        us_vhw_combine us_vhw_enhanced
        )

# Programs that will be handled by separate CMakeLists.txt files
set(SEPARATE_CMAKE_PROGRAMS
        # Add programs here as you create separate CMakeLists.txt files for them
        us_autoflow_analysis us_com_project us_com_project_academic us_experiment us_fematch
         us_fit_meniscus us_mwl_species_fit us_protocol_dev us_query_rmsd us_reporter_gmp
        )

# Collect all program directories
set(EXCLUDED_PROGRAMS config2 us_mpi_analysis)
file(GLOB program_dirs LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")
set(ALL_PROGRAMS)
foreach(prog_name ${program_dirs})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${prog_name}"
            AND NOT prog_name IN_LIST EXCLUDED_PROGRAMS
            AND NOT prog_name MATCHES "^\\.")
        list(APPEND ALL_PROGRAMS ${prog_name})
    endif()
endforeach()

# Function to create a modern UltraScan3 program
function(add_us3_program prog_name)
    set(prog_dir "${CMAKE_CURRENT_SOURCE_DIR}/${prog_name}")

    if(NOT IS_DIRECTORY ${prog_dir})
        return()
    endif()

    # CHECK EXECUTABLE_ONLY_PROGRAMS FIRST - this overrides all explicit handling
    if(prog_name IN_LIST EXECUTABLE_ONLY_PROGRAMS)
        # Force executable-only build - discover all sources
        file(GLOB_RECURSE all_sources "${prog_dir}/*.cpp")
        file(GLOB_RECURSE all_headers "${prog_dir}/*.h")
        file(GLOB_RECURSE all_forms "${prog_dir}/*.ui")
        file(GLOB_RECURSE all_resources "${prog_dir}/*.qrc")

        # Add cross-directory sources for specific programs
        if(prog_name STREQUAL "us_abde")
            list(APPEND all_sources "${prog_dir}/../us_buoyancy/us_buoyancy.cpp")
            list(APPEND all_headers "${prog_dir}/../us_buoyancy/us_buoyancy.h")
        endif()

        # Platform filtering
        if(NOT WIN32)
            list(FILTER all_sources EXCLUDE REGEX ".*win_data\\.cpp$")
            list(FILTER all_headers EXCLUDE REGEX ".*win_data\\.h$")
        endif()

        set(main_source "")  # No library splitting
        set(create_library FALSE)
        message(STATUS "${prog_name}: Executable-only build")

        # Handle special programs explicitly based on their .pro files - ONLY if not executable-only
    elseif(prog_name STREQUAL "us_analysis_profile")
        set(main_source "${prog_dir}/us_anapro_main.cpp")
        set(all_sources "${prog_dir}/us_analysis_profile.cpp" "${prog_dir}/us_anapro_utils.cpp")
        file(GLOB_RECURSE all_headers "${prog_dir}/*.h")
        file(GLOB_RECURSE all_forms "${prog_dir}/*.ui")
        file(GLOB_RECURSE all_resources "${prog_dir}/*.qrc")
        message(STATUS "${prog_name}: Created")
    elseif(prog_name STREQUAL "us_legacy_converter")
        # Based on .pro file: convert source + legacy converter source
        set(main_source "")
        set(all_sources
                "${prog_dir}/../us_convert/us_convert.cpp"
                "${prog_dir}/us_legacy_converter.cpp"
                )
        set(all_headers
                "${prog_dir}/../us_convert/us_convert.h"
                "${prog_dir}/us_legacy_converter.h"
                )
        file(GLOB_RECURSE all_forms "${prog_dir}/*.ui")
        file(GLOB_RECURSE all_resources "${prog_dir}/*.qrc")
        set(create_library FALSE)  # Force executable-only
        message(STATUS "${prog_name}: Using .pro file mapping with cross-directory sources")
        message(STATUS "${prog_name}: Executable-only build with us_convert_lib dependency")
        message(STATUS "${prog_name}: Created")
    elseif(prog_name STREQUAL "us_convert")
        set(main_source "${prog_dir}/us_convert_main.cpp")
        set(all_sources
                "${prog_dir}/us_convert.cpp"
                "${prog_dir}/us_convert_gui.cpp"
                "${prog_dir}/us_convertio.cpp"
                "${prog_dir}/us_get_run.cpp"
                "${prog_dir}/us_experiment.cpp"
                "${prog_dir}/us_experiment_gui.cpp"
                "${prog_dir}/us_intensity.cpp"
                "${prog_dir}/us_selectbox.cpp"
                "${prog_dir}/us_select_triples.cpp"
                )
        file(GLOB_RECURSE all_headers "${prog_dir}/*.h")
        file(GLOB_RECURSE all_forms "${prog_dir}/*.ui")
        file(GLOB_RECURSE all_resources "${prog_dir}/*.qrc")
        set(create_library TRUE)
        message(STATUS "${prog_name}: Created")
    elseif(prog_name STREQUAL "us_reassign_reviewers_gmp")
        # Make this executable-only with all sources included
        set(main_source "")
        set(all_sources
                "${prog_dir}/us_reassign_reviewers_gmp_main.cpp"
                "${prog_dir}/../us_esigner_gmp/us_esigner_gmp.cpp"
                )
        set(all_headers
                "${prog_dir}/../us_esigner_gmp/us_esigner_gmp.h"
                )
        file(GLOB_RECURSE all_forms "${prog_dir}/*.ui")
        file(GLOB_RECURSE all_resources "${prog_dir}/*.qrc")
        set(create_library FALSE)  # Force executable-only
        message(STATUS "${prog_name}: Using executable-only build with cross-directory sources")
    elseif(prog_name STREQUAL "us_assign_reviewers_gmp")
        # Make this executable-only with all sources included
        set(main_source "")
        set(all_sources
                "${prog_dir}/us_assign_reviewers_gmp_main.cpp"
                "${prog_dir}/../us_esigner_gmp/us_esigner_gmp.cpp"
                )
        set(all_headers
                "${prog_dir}/../us_esigner_gmp/us_esigner_gmp.h"
                )
        file(GLOB_RECURSE all_forms "${prog_dir}/*.ui")
        file(GLOB_RECURSE all_resources "${prog_dir}/*.qrc")
        set(create_library FALSE)  # Force executable-only
        message(STATUS "${prog_name}: Using executable-only build with cross-directory sources")
    else()
        # Standard file discovery for other programs
        file(GLOB_RECURSE all_sources "${prog_dir}/*.cpp")
        file(GLOB_RECURSE all_headers "${prog_dir}/*.h")
        file(GLOB_RECURSE all_forms "${prog_dir}/*.ui")
        file(GLOB_RECURSE all_resources "${prog_dir}/*.qrc")

        # Platform filtering
        if(NOT WIN32)
            list(FILTER all_sources EXCLUDE REGEX ".*win_data\\.cpp$")
            list(FILTER all_headers EXCLUDE REGEX ".*win_data\\.h$")
        endif()

        # Find main source file
        set(main_source "")
        foreach(src ${all_sources})
            get_filename_component(filename ${src} NAME_WE)
            if(filename MATCHES ".*_main$" OR filename MATCHES "^(main|${prog_name})$")
                set(main_source ${src})
                break()
            endif()
        endforeach()

        if(main_source)
            list(REMOVE_ITEM all_sources ${main_source})
        endif()
    endif()

    # Determine if we should create a library
    if(NOT DEFINED create_library)
        set(create_library TRUE)

        # Don't create library if only one source file
        list(LENGTH all_sources source_count)
        if(source_count LESS_EQUAL 1)
            set(create_library FALSE)
        endif()

        # Separate main from library sources (only if creating library)
        if(create_library)
            list(REMOVE_ITEM all_sources ${main_source})
            # Ensure we still have sources for the library after removing main
            list(LENGTH all_sources remaining_sources)
        endif()
    endif()

    # Create library target if there are non-main sources AND we should create a library
    if(all_sources AND create_library)
        add_library(${prog_name}_lib
                ${all_sources}
                ${all_headers}
                ${all_forms}
                ${all_resources}
                )

        # Set unique AUTOMOC directory to avoid conflicts
        set_target_properties(${prog_name}_lib PROPERTIES
                AUTOMOC ON
                AUTOUIC ON
                AUTORCC ON
                POSITION_INDEPENDENT_CODE ON
                AUTOMOC_MOC_OPTIONS "--no-notes"
                AUTOUIC_SEARCH_PATHS "${prog_dir}"
                )

        # Include directories
        target_include_directories(${prog_name}_lib
                PUBLIC
                ${prog_dir}
                PRIVATE
                ${CMAKE_CURRENT_BINARY_DIR}/${prog_name}_lib_autogen/include
                )

        target_link_libraries(${prog_name}_lib
                PUBLIC
                us_program_common
                )

        # Add Qt DataVisualization for 3D plotting programs
        if(prog_name MATCHES ".*(mwl|3d|plot).*")
            find_package(Qt5 COMPONENTS DataVisualization QUIET)
            if(Qt5DataVisualization_FOUND)
                target_link_libraries(${prog_name}_lib PUBLIC Qt5::DataVisualization)
            endif()
        endif()

        # Create alias for consistent naming
        add_library(UltraScan3::${prog_name} ALIAS ${prog_name}_lib)

        message(STATUS "Created library: ${prog_name}_lib")
    endif()

    # Create executable
    if(create_library AND main_source AND NOT main_source STREQUAL "")
        add_executable(${prog_name} ${main_source})
    else()
        # Executable-only build (no separate library) OR no separate main file
        add_executable(${prog_name} ${all_sources} ${all_headers} ${all_forms} ${all_resources})
    endif()

    # Set unique AUTOMOC directory for executable to avoid conflicts
    set_target_properties(${prog_name} PROPERTIES
            AUTOMOC ON
            AUTOUIC ON
            AUTORCC ON
            OUTPUT_NAME ${prog_name}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            AUTOMOC_MOC_OPTIONS "--no-notes"
            AUTOUIC_SEARCH_PATHS "${prog_dir}"
            )

    # Link executable
    if(TARGET ${prog_name}_lib)
        target_link_libraries(${prog_name}
                PRIVATE
                ${prog_name}_lib
                )
    else()
        target_link_libraries(${prog_name}
                PRIVATE
                us_program_common
                )
        target_include_directories(${prog_name} PRIVATE ${prog_dir})
    endif()

    # Install
    install(TARGETS ${prog_name}
            RUNTIME DESTINATION bin
            COMPONENT Programs
            )

    # Install library if created
    if(TARGET ${prog_name}_lib)
        install(TARGETS ${prog_name}_lib
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                COMPONENT Libraries
                )
    endif()

    message(STATUS "Created program: ${prog_name}")
endfunction()

# Create all programs
foreach(prog_name ${ALL_PROGRAMS})
    if(prog_name IN_LIST SEPARATE_CMAKE_PROGRAMS)
        # Add subdirectory if it has its own CMakeLists.txt
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${prog_name}/CMakeLists.txt")
            add_subdirectory(${prog_name})
            message(STATUS "Using separate CMakeLists.txt for ${prog_name}")
        else()
            message(WARNING "${prog_name} marked for separate CMake but CMakeLists.txt not found")
            add_us3_program(${prog_name})
        endif()
    else()
        add_us3_program(${prog_name})
    endif()
endforeach()

# Summary
list(LENGTH ALL_PROGRAMS program_count)
message(STATUS "Created ${program_count} UltraScan3 programs")