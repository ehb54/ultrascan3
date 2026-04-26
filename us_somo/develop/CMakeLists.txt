cmake_minimum_required(VERSION 3.21)

# CMake build for the us_somo library (qmake: libus_somo.pro)
# Inspired by: makesomo.sh, admin/codeql/docker/local.pri.somo, and the analyze-somo job in .github/workflows/codeql-analysis.yml
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# Zlib is required according to local.pri.somo (LIBS *= -lz)
find_package(ZLIB REQUIRED)

# Include paths from local.pri.somo
set(US_SOMO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${US_SOMO_INCLUDE_DIR})

# Generate include/us_revision.h similar to libus_somo.pro (revision.sh)
# The script updates/creates include/us_revision.h in this directory.
add_custom_command(
        OUTPUT ${US_SOMO_INCLUDE_DIR}/us_revision.h
        COMMAND ${CMAKE_COMMAND} -E env ULTRASCAN=${CMAKE_SOURCE_DIR} us3=${CMAKE_SOURCE_DIR} sh ${CMAKE_CURRENT_SOURCE_DIR}/revision.sh
        COMMAND ${CMAKE_COMMAND} -E env ULTRASCAN=${CMAKE_SOURCE_DIR} us3=${CMAKE_SOURCE_DIR} sh ${CMAKE_CURRENT_SOURCE_DIR}/version.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating us_somo include/us_revision.h via revision.sh"
        VERBATIM)

add_custom_target(us_somo_version_headers DEPENDS ${US_SOMO_INCLUDE_DIR}/us_revision.h)

# define somo_uname interface
add_library(somo_uname INTERFACE)
if(UNIX AND NOT APPLE)
	target_compile_definitions(somo_uname INTERFACE UNIX QT_NO_STL)

	# OS Checks
	if(CMAKE_SYSTEM_NAME MATCHES "Linux")
		target_compile_definitions(somo_uname INTERFACE LINUX)
		message(STATUS "Configuring for the Linux operating system...")
	elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
		target_compile_definitions(somo_uname INTERFACE FREEBSD)
		message(STATUS "Configuring for the FreeBSD operating system...")
	elseif(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
		target_compile_definitions(somo_uname INTERFACE NETBSD)
		message(STATUS "Configuring for the NetBSD operating system...")
	elseif(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
		target_compile_definitions(somo_uname INTERFACE OPENBSD)
		message(STATUS "Configuring for the OpenBSD operating system...")
	elseif(CMAKE_SYSTEM_NAME MATCHES "SunOS")
		target_compile_definitions(somo_uname INTERFACE SOLARIS)
		message(STATUS "Configuring for the Sun Solaris operating system...")
	elseif(CMAKE_SYSTEM_NAME MATCHES "IRIX64")
		target_compile_definitions(somo_uname INTERFACE IRIX)
		message(STATUS "Configuring for SGI Irix 64-bit operating system...")
	elseif(CMAKE_SYSTEM_NAME MATCHES "IRIX")
		target_compile_definitions(somo_uname INTERFACE IRIX)
		message(STATUS "Configuring for SGI Irix operating system...")
	elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
		target_compile_definitions(somo_uname INTERFACE OSX)
		message(STATUS "Configuring for the Darwin Macintosh OS-X operating system...")
	endif()

	# Architecture Checks
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "pentium4")
		target_compile_definitions(somo_uname INTERFACE INTEL)
		message(STATUS "Configuring for the Intel Pentium IV Platform...")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
		target_compile_definitions(somo_uname INTERFACE INTEL)
		message(STATUS "Configuring for the Intel i686 Platform...")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i586")
		target_compile_definitions(somo_uname INTERFACE INTEL)
		message(STATUS "Configuring for the Intel i586 Platform...")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i486")
		target_compile_definitions(somo_uname INTERFACE INTEL)
		message(STATUS "Configuring for the Intel i486 Platform...")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
		target_compile_definitions(somo_uname INTERFACE INTEL QT_NO_STL)
		message(STATUS "Configuring for the Intel i386 Platform...")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
		target_compile_definitions(somo_uname INTERFACE OPTERON)
		message(STATUS "Configuring for the x86_64-bit Platform...")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "mips")
		target_compile_definitions(somo_uname INTERFACE SGI)
		message(STATUS "Configuring for the SGI mips Platform...")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc" OR CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
		target_compile_definitions(somo_uname INTERFACE MAC)
		message(STATUS "Configuring for the Macintosh PowerPC Platform...")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "sparc")
		target_compile_definitions(somo_uname INTERFACE SPARC)
		message(STATUS "Configuring for the Sun Sparc Platform...")
	endif()
endif()

if(APPLE)
	target_compile_definitions(somo_uname INTERFACE MACX MAC OSX)
	message(STATUS "Configuring for Mac Platform...")
endif()

if(WIN32)
	target_compile_definitions(somo_uname INTERFACE WIN32 MINGW __WIN64__)
	message(STATUS "Configuring for Microsoft Windows Platform...")
endif()





set(SOURCES         
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_admin.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_arch.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_average.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_band_broaden.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_container.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_container_grpy.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_cmdline_app.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_color.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_config.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_config_gui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_csv.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_dirhist.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_editor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_eigen.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_encryption.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_ffd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_file_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_fractal_dimension.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_font.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_global.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_help.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_gzip.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_addatom.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_addhybrid.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_addresidue.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_addsaxs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_advanced_config.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_anaflex_core.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_anaflex_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_asa.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_asab1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_batch.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_batch_movie_opts.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_bead_output.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_beads.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_beads_load.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_best.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_bd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_bd_core.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_bd_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_bd_load_results_opts.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_advanced.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_additional.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_best.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_bfnb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_bfnb_nsa.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_config.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_config_server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_dmd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_dammin.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_oned.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_results.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_submit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_cluster_status.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_comparative.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_core.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_csv_viewer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dammin_opts.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_baseline_best.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_conc_csv_frames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_gui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_stack.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_modes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_modes_bb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_modes_baseline2.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_modes_guinier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_modes_powerfit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_modes_wheel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_makeiq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_plots.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_ciq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_cistarq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_dctr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_gg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_p3d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_parameters.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_movie.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_conc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_conc_load.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_fit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_fit_global.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_fit_gsm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_fit_ga.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_gauss_mode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_nth.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_simulate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_svd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dad_scale_trend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dmd_core.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_dmd_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_file.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_file2.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_fractal_dimension.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_fractal_dimension_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_grid.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_grid_atob.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_grpy.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_hydrate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_hydrate_align.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_hydro.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_hydro_zeno.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_info.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_load.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_baseline_best.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_conc_csv_frames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_gui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_stack.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_modes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_modes_guinier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_modes_bb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_modes_wheel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_makeiq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_plots.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_ciq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_cistarq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_dctr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_gg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_p3d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_parameters.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_movie.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_conc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_conc_load.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_fit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_fit_global.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_fit_gsm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_fit_ga.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_gauss_mode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_nth.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_simulate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_svd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_scale_trend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_baseline_best.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_conc_csv_frames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_gui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_stack.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_modes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_modes_guinier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_modes_bb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_modes_scroll.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_modes_scale.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_modes_wheel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_makeiq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_plots.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_ciq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_cistarq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_dctr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_gg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_p3d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_parameters.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_movie.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_conc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_conc_load.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_fit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_fit_global.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_fit_gsm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_fit_ga.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_gauss_mode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_nth.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_simulate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_svd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_mals_saxs_scale_trend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_misc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_other.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_overlap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_overlap_reduction.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pat.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pdb_parsing.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pdb_tool.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pdb_tool_merge.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pdb_tool_renum.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pdb_tool_selres.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pdb_tool_sort.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pdb_tool_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_pdb_visualization.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_results.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_saxs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_sans.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_curve.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_bead_model.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_hydration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_guinier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_xsr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_misc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_sas_options_experimental.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_save.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_conc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_cormap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_external.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_guinier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_fits.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_ift.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_iqq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_iqq_bead_model.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_iqq_load_csv.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_iqq_residuals.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_iqq_sh.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_load_csv.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_loads.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_legend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_mw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_plots.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_pr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_residuals.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_screen.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_search.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_buffer_gui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_buffer_fit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_buffer_conc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_buffer_conc_load.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_buffer_nth.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_baseline_best.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_conc_csv_frames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_gui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_stack.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_modes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_modes_bb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_modes_broaden.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_modes_wheel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_makeiq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_parameters.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_plots.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_ciq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_dctr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_gg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_p3d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_movie.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_conc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_conc_load.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_fit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_fit_global.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_fit_gsm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_fit_ga.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_gauss_mode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_nth.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_options.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_simulate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_svd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_hplc_scale_trend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_1d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_2d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_script.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_settings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_supc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_vdw_overlap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_xsr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_write.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_zeno.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_json.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_mqt.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_license.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_lm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_lud.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_math.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_matrix.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pdb_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_best.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_best_sphere.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_best_cylinder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_best_spheroid.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_best_ellipsoid.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_best_torus.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_best_torus_segment.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_fitness.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_ga.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_objects.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_pm_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_plot_zoom.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_register.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_gp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_asab1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_a2sb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_best.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_c2check.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_crysol.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_cuda.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_dammin.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_dmd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_extern.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_guinier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_gui_only.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_hydrate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_hydrate_align.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_ift.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_iqq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_iqq_1d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_iqq_pdb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_iqq_bead_model.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_loads.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_nsa.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_nsa_ga.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_nsa_gsm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_nsa_sga.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_pat.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_pm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_sgp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_sgp_phys.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_ssbond.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_saxs_util_static.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_sh.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_surfracer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_svd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_efa.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_tar.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_timer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_vector.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_vvv.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_write_config.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_zeno_cxx.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_zeno_cxx_nf.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/qwt/scrollbar.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/qwt/scrollzoomer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_gui_settings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_hydrodyn_saxs_guinier_frames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us_plot_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_color.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_editor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_editor_gui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_gui_settings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_settings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_widgets.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_widgets_dialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_images.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_global.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_gui_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_gzip.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/us3i_plot.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/us_revision.h
	${CMAKE_CURRENT_SOURCE_DIR}/shd_mpi/shs_use.cpp
)

file(GLOB US_SOMO_HEADERS CONFIGURE_DEPENDS
		${US_SOMO_INCLUDE_DIR}/*.h
		${US_SOMO_INCLUDE_DIR}/**/*.h
		${US_SOMO_INCLUDE_DIR}/*.hpp
		${US_SOMO_INCLUDE_DIR}/**/*.hpp
)
list(FILTER US_SOMO_HEADERS EXCLUDE REGEX
		"us_hydrodyn_saxs_hplc_bl.h$"
)

# Create the library target (qmake TARGET = us_somo)
add_library(us_somo ${SOURCES} ${US_SOMO_HEADERS} ${US_SOMO_INCLUDE_DIR}/us_revision.h)
add_dependencies(us_somo us_somo_version_headers)

target_include_directories(us_somo
        PUBLIC
            ${US_SOMO_INCLUDE_DIR}
)

# qmake defines from libus_somo.pro and local.pri.somo
target_compile_definitions(us_somo
        PRIVATE
            THREAD        # enabled threading in fitting algorithm
            NO_DB         # build without DB (as per libus_somo.pro)
            NO_EDITOR_PRINT # temporary fix from qmake project
)
target_compile_definitions(us_somo PUBLIC QT_SHARED)
# if max or unix set UNIX else MINGW
if(WIN32)
    target_compile_definitions(us_somo PUBLIC US_MAKE_DLL US_MAKE_GUI_DLL)
elseif(UNIX OR APPLE)
    target_compile_definitions(us_somo PUBLIC UNIX LINUX)
endif()
if(APPLE)
	# Force all symbols to be exported at link time
	target_link_options(us_somo PRIVATE
			"LINKER:-export_dynamic"
			"LINKER:-all_load"
	)
	# Override any inherited visibility settings
	target_compile_options(us_somo PRIVATE -fvisibility=default)
elseif(UNIX)
	# Force all symbols to be exported at link time
	target_link_options(us_somo PRIVATE
			"LINKER:--export-dynamic"
	)
	# Override any inherited visibility settings
	target_compile_options(us_somo PRIVATE -fvisibility=default)
endif()
target_include_directories(us_somo
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}  # For generated headers
)
# Link libraries
target_link_libraries(us_somo
        PUBLIC
            Qt::Widgets
            Qt::Network
            Qt::Svg
            Qt::PrintSupport
            Qt::Multimedia
            ZLIB::ZLIB
)
if(USE_QT6)
    target_link_libraries(us_somo PUBLIC Qt::OpenGLWidgets)
endif()
target_link_libraries(us_somo PUBLIC Qwt::Qwt qwtplot3d::qwtplot3d)
target_link_libraries(us_somo PUBLIC somo_uname)
# Set library properties
set_target_properties(us_somo PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "us_somo${PROJECT_VERSION_MAJOR}"
        CXX_VISIBILITY_PRESET default
        C_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
        POSITION_INDEPENDENT_CODE ON
)

# Platform-specific properties
if(APPLE)
    set_target_properties(us_somo PROPERTIES
            MACOSX_RPATH ON
            INSTALL_RPATH "@loader_path;@loader_path/../lib"
    )
elseif(UNIX)
    set_target_properties(us_somo PROPERTIES
            INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
    )
elseif(WIN32)
    set_target_properties(us_somo PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()
# Installation: place library into the same layout the root uses (lib dir)
include(GNUInstallDirs)
install(TARGETS us_somo
        EXPORT UltraScan3Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)
# Notes:
# - Applications defined in us_somo.pro (us_admin, us3_config, us3_hydrodyn, us_saxs_cmds_t)
#   can be added later as subdirectories with their own CMakeLists if needed.
add_library(somo_generic INTERFACE)
target_compile_definitions(somo_generic INTERFACE NO_DB NO_EDITOR_PRINT)
target_link_libraries(somo_generic INTERFACE
		Qt::Core
		Qt::Gui
		Qt::Widgets
		Qt::Network
		Qt::Sql
		Qt::Xml
		Qt::Svg
		Qt::OpenGL
		Qt::PrintSupport
		Qwt::Qwt
		qwtplot3d::qwtplot3d
)
if (USE_QT6)
	target_link_libraries(somo_generic INTERFACE Qt::OpenGLWidgets)
endif()
add_dependencies(somo_generic us_somo)
target_link_libraries(somo_generic INTERFACE us_somo)
if(WIN32)
	target_compile_definitions(somo_generic INTERFACE QWT_DLL QT_DLL)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		if(MINGW)
			target_link_options(somo_generic INTERFACE "-mwindows")
		elseif(MSVC)
			target_link_options(somo_generic INTERFACE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
		endif()
	endif()
endif()

add_subdirectory(us_admin)
add_subdirectory(us3_config)
add_subdirectory(us3_hydrodyn)
