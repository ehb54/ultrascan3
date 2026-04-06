# us_reporter_gmp

# ── PoDoFo 0.9.x — PDF bookmark injection ────────────────────────────────────
# Strategy mirrors the .pro file: try find_package first (Config or Module
# mode), then fall back to pkg-config, and finally attempt a bare -lpodofo
# link so the behaviour is identical to the original LIBS += -lpodofo line.
# ----------------------------------------------------------------------------
find_package(PoDoFo QUIET)          # works if PoDoFoConfig.cmake is installed

if(NOT PoDoFo_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PODOFO QUIET libpodofo)   # sets PODOFO_FOUND etc.
    endif()
endif()

if(NOT PoDoFo_FOUND AND NOT PODOFO_FOUND)
    # Last-resort bare link — equivalent to LIBS += -lpodofo in the .pro file.
    # A user with a non-standard install should set PODOFO_ROOT or adjust
    # CMAKE_PREFIX_PATH instead of editing this file.
    message(STATUS "PoDoFo: neither find_package nor pkg-config found a config; "
                   "falling back to bare -lpodofo (adjust PODOFO_ROOT if needed)")
endif()
# ─────────────────────────────────────────────────────────────────────────────

# Main program sources from us_reporter_gmp directory
set(REPORTER_GMP_SOURCES
        us_reporter_gmp_main.cpp
        us_reporter_gmp.cpp
        )

set(REPORTER_GMP_HEADERS
        us_reporter_gmp.h
        )


# Cross-directory sources as specified in .pro file
set(CROSS_DIR_SOURCES
        # us_analysis_profile
        ../us_analysis_profile/us_analysis_profile.cpp
        ../us_analysis_profile/us_anapro_utils.cpp

        # us_autoflow_analysis
        ../us_autoflow_analysis/us_autoflow_analysis.cpp

        # us_ddist_combine
        ../us_ddist_combine/us_ddist_combine.cpp
        ../us_ddist_combine/us_select_rundd.cpp

        # us_fit_meniscus
        ../us_fit_meniscus/us_fit_meniscus.cpp

        # us_fematch
        ../us_fematch/us_fematch.cpp
        ../us_fematch/us_plot_control_fem.cpp
        ../us_fematch/us_resplot_fem.cpp
        ../us_fematch/us_advanced_fem.cpp
        ../us_fematch/us_adv_dmgamc.cpp
        ../us_fematch/us_dmga_mc_stats.cpp
        ../us_fematch/us_thread_worker.cpp

        # us_pseudo3d_combine
        ../us_pseudo3d_combine/us_pseudo3d_combine.cpp
        ../us_pseudo3d_combine/us_remove_distros.cpp

        # us_mwl_species_fit
        ../us_mwl_species_fit/us_mwl_species_fit.cpp
        ../us_mwl_species_fit/us_mwl_sf_plot3d.cpp
        ../us_mwl_species_fit/us_load_run_noise.cpp

        # us_abde
        ../us_abde/us_norm_profile.cpp
        )

# Cross-directory headers from .pro file
set(CROSS_DIR_HEADERS
        ../us_analysis_profile/us_analysis_profile.h
        ../us_autoflow_analysis/us_autoflow_analysis.h
        ../us_ddist_combine/us_ddist_combine.h
        ../us_ddist_combine/us_select_rundd.h
        ../us_fit_meniscus/us_fit_meniscus.h
        ../us_fematch/us_fematch.h
        ../us_fematch/us_plot_control_fem.h
        ../us_fematch/us_resplot_fem.h
        ../us_fematch/us_advanced_fem.h
        ../us_fematch/us_adv_dmgamc.h
        ../us_fematch/us_dmga_mc_stats.h
        ../us_fematch/us_thread_worker.h
        ../us_pseudo3d_combine/us_pseudo3d_combine.h
        ../us_pseudo3d_combine/us_remove_distros.h
        ../us_mwl_species_fit/us_mwl_species_fit.h
        ../us_mwl_species_fit/us_mwl_sf_plot3d.h
        ../us_mwl_species_fit/us_load_run_noise.h
        ../us_abde/us_norm_profile.h
        )

# Find UI and resource files
file(GLOB REPORTER_GMP_FORMS "*.ui")
file(GLOB REPORTER_GMP_RESOURCES "*.qrc")

# Create the executable
add_executable(us_reporter_gmp
        ${REPORTER_GMP_SOURCES}
        ${REPORTER_GMP_HEADERS}
        ${CROSS_DIR_SOURCES}
        ${CROSS_DIR_HEADERS}
        ${REPORTER_GMP_FORMS}
        ${REPORTER_GMP_RESOURCES}
        )

# dependency on revision.h in programs/us
add_dependencies(us_reporter_gmp us_version_headers)

# Set target properties
set_target_properties(us_reporter_gmp PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        OUTPUT_NAME us_reporter_gmp
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        AUTOMOC_MOC_OPTIONS "--no-notes"
        )

# Include directories for all the cross-directory headers
target_include_directories(us_reporter_gmp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_analysis_profile
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_autoflow_analysis
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_ddist_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fit_meniscus
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_fematch
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_pseudo3d_combine
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_mwl_species_fit
        ${CMAKE_CURRENT_SOURCE_DIR}/../us_abde
        # PoDoFo include dirs from pkg-config (no-op if empty)
        ${PODOFO_INCLUDE_DIRS}
        )

# Link libraries
target_link_libraries(us_reporter_gmp PRIVATE
        us_program_common
        Qt::DataVisualization
        # PoDoFo — pick whichever discovery path succeeded
        $<$<BOOL:${PoDoFo_FOUND}>:PoDoFo::PoDoFo>
        $<$<AND:$<NOT:$<BOOL:${PoDoFo_FOUND}>>,$<BOOL:${PODOFO_FOUND}>>:${PODOFO_LIBRARIES}>
        $<$<AND:$<NOT:$<BOOL:${PoDoFo_FOUND}>>,$<NOT:$<BOOL:${PODOFO_FOUND}>>>:podofo>
        )
	
# Link libraries
target_compile_options(us_reporter_gmp PRIVATE
        # pkg-config compile flags (e.g. -D_PODOFO_SHAREDLIB), no-op if empty
        ${PODOFO_CFLAGS_OTHER}
        )

# Install
install(TARGETS us_reporter_gmp
        RUNTIME DESTINATION bin
        COMPONENT Programs
        )

message(STATUS "Created: us_reporter_gmp")

