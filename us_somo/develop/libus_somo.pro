# Messages
include ( local.pri )

TEMPLATE       = lib
TRANSLATIONS   = lib.ts
VERSION        = 10
MOC_DIR        = src/moc
OBJECTS_DIR    = src/obj

# enabled threading in fitting algorithm:
DEFINES += THREAD

# Automatic hardware platform and operating system configurations:

DEPENDPATH += src include
DEFINES += NO_DB

# temporary fix (us2 code was using qt2 qpdevmnt which I think need to be replaced by qprintdevicemetrics)
DEFINES += NO_EDITOR_PRINT

TARGET  = us_somo

revision.target      = include/us_revision.h
revision.commands    = sh revision.sh
revision.depends     = FORCE
QMAKE_EXTRA_TARGETS += revision

# OSX also reports UNIX
contains( DEFINES, "OSX" ) {
   QT += network
} else {
    unix {
      QMAKE_CXXFLAGS_WARN_ON += -Wno-non-virtual-dtor
      DEFINES                += UNIX
      CONFIG                 += qt thread warn
    
      contains(UNAME,x86_64) {
        LIBS    += -L$$QWTPATH/lib/ -L$$QWTPATH/lib/ -lqwt 
        DEFINES += BIN64
        DESTDIR  = $$US3SOMOPATH/lib
      } else {
        LIBS += -L$$QWTPATH/lib -lqwt
        DESTDIR  = $$US3SOMOPATH/lib
      }
   }
}

win32 {

  DEFINES      += MINGW

  # QWT3D is right for libraries, but gui apps need ../$$QWT3D
  # due to us3 directory structure

  QWT3D       = ../qwtplot3d
  ##OPENSSL     = C:/openssl
  ##OPENSSL     = C:/mingw64/opt
#  OPENSSL     = C:/utils/openssl
#  MYSQLPATH   = C:/utils/mysql
#  MYSQLDIR    = $$MYSQLPATH/lib
#  QTMYSQLPATH = C:/utils/Qt/5.4.1/plugins/sqldrivers
#  QTPATH      = C:/utils/Qt/5.4.1
#  QMAKESPEC   = $$QTPATH/mkspecs/win32-g++
#  QTMAKESPEC  = $$QMAKESPEC
#  QWTPATH     = C:/utils/Qwt/6.1.2
#  SINGLEDIR   = C:/utils/Qt/5.4.1/addons/qtsingleapplication-2.6_1-opensource/src/
#  MINGWDIR    = C:/mingw64/x86_64-w64-mingw32
  
#  contains( DEBUGORRELEASE, debug ) {
#    QWTLIB      = $$QWTPATH/lib/libqwtd.a
#    MYSQLLIB    = $$MYSQLDIR/libmysqld.lib
#  } else {
#    QWTLIB      = $$QWTPATH/lib/libqwt.a
#    MYSQLLIB    = $$MYSQLDIR/libmysql.lib
#    INCLUDEPATH += c:/mingw64/opt/include
#  }
  ##LIBS        += $$MYSQLLIB
  LIBS        += -L$$MYSQLDIR -lmysqlclient
  LIBS        += -lpsapi

  #  __LCC__ is needed on W32 to make mysql headers include the right W32 includes
  ##DEFINES    += __LCC__
  DEFINES    += __WIN64__

  DESTDIR         = $$US3SOMOPATH/bin

  QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_STL
#   QMAKESPEC       = win32-g++-4.6

  CONFIG         += qt warn_on opengl thread zlib
  CONFIG         += dll exceptions

  DEFINES        += US_MAKE_DLL US_MAKE_GUI_DLL
  DEFINES        += MINGW

}

macx {
    DESTDIR  = $$US3SOMOPATH/lib
}


# Do not remake cpp and h files from ui files
#FORMS = 3dplot/mesh2mainwindowbase.ui 3dplot/lightingdlgbase.ui


SOURCES *= \
        src/us_admin.cpp \
        src/us_arch.cpp \
        src/us_average.cpp \
#  3dplot/mesh2mainwindowbase.cpp \
#  3dplot/mesh2mainwindow.cpp \
#  3dplot/colormapreader.cpp \
#  3dplot/lightingdlgbase.cpp \
#  3dplot/lightingdlg.cpp \
#  3dplot/mesh.cpp \
#  D_calc.cpp \
#  us_2dplot.cpp \
#  us_3dsolutes.cpp \
#  us_archive.cpp \
#  us_astfem_math.cpp \
#  us_astfem_rsa.cpp \
#  us_astfem_sim.cpp \
#  us_average.cpp \
#  us_buffer.cpp \
#  us_calendar.cpp \ 
#  us_clipdata.cpp \ 
        src/us_container.cpp \
        src/us_container_grpy.cpp \
        src/us_cmdline_app.cpp \
#  us_cofdistro.cpp \
#  us_cofs.cpp \ 
        src/us_color.cpp \
#  us_colorgradient.cpp \
#  us_combine.cpp \
        src/us_config.cpp \
        src/us_config_gui.cpp \
#  us_constraint_control.cpp \
#  us_create_global.cpp \
        src/us_csv.cpp \
#  us_data_io.cpp \
#  us_database.cpp \ 
#  us_db.cpp \
#  us_db_admin.cpp \
#  us_db_veloc.cpp \
#  us_db_equil.cpp \
#  us_db_rst_equilproject.cpp \
#  us_db_rst_montecarlo.cpp \
#  us_db_rtv_date.cpp \
#  us_db_rtv_description.cpp \
#  us_db_rtv_edittype.cpp \
#  us_db_rtv_image.cpp \
#  us_db_rtv_investigator.cpp \
#  us_db_rtv_request.cpp \
#  us_db_rtv_requeststatus.cpp \
#  us_db_rtv_runrequest.cpp \
#  us_db_rtv_sample.cpp \
#  us_db_runrequest.cpp \
#  us_db_t.cpp \
#  us_db_tbl_buffer.cpp \
#  us_db_tbl_cell.cpp \
#  us_db_tbl_expdata.cpp \
#  us_db_tbl_investigator.cpp \
#  us_db_tbl_nucleotide.cpp \
#  us_db_tbl_vbar.cpp \
#  us_db_template.cpp \
#  us_db_widgets.cpp \
#  us_dcdrdat.cpp \
#  us_dcdtdat.cpp \
#  us_diagnostics.cpp \
        src/us_dirhist.cpp \
#  us_dma60.cpp \
#  us_dtactrl.cpp \
#  us_edeabs.cpp \
#  us_edeflo.cpp \
#  us_edeint.cpp \
#  us_edit.cpp \
#  us_editdb.cpp \
        src/us_editor.cpp \
#  us_editwavelength.cpp \
#  us_edscan.cpp \
#  us_edvabs.cpp \
#  us_edvflo.cpp \
#  us_edvint.cpp \
        src/us_eigen.cpp \
        src/us_encryption.cpp \
#  us_enter_dna.cpp \
#  us_eqfitter.cpp \
#  us_eqmodelctrl.cpp \
#  us_equilprojectreport.cpp \
#  us_equilreport.cpp \
#  us_equilsim.cpp \
#  us_equilspeed.cpp \
#  us_equiltime.cpp \
#  us_exclude_profile.cpp \
#  us_export.cpp \
#  us_extinctfitter.cpp \
#  us_extinction.cpp \
#  us_fefit.cpp \
#  us_fematch.cpp \
#  us_fematch_ra.cpp \
#  us_femglobal.cpp \
        src/us_ffd.cpp \
        src/us_file_util.cpp \
        src/us_fractal_dimension.cpp \
#  us_finite1.cpp \
#  us_finite2.cpp \
        src/us_font.cpp \
#  us_ga_initialize.cpp \
#  us_ga_model_editor.cpp \
        src/us_global.cpp \
#  us_globalequil.cpp \
        src/us_help.cpp \
        src/us_gzip.cpp \
#  us_htmledit.cpp \
#  us_hydro.cpp \
        src/us_hydrodyn.cpp \
        src/us_hydrodyn_addatom.cpp \
        src/us_hydrodyn_addhybrid.cpp \
        src/us_hydrodyn_addresidue.cpp \
        src/us_hydrodyn_addsaxs.cpp \
        src/us_hydrodyn_advanced_config.cpp \
        src/us_hydrodyn_anaflex_core.cpp \
        src/us_hydrodyn_anaflex_options.cpp \
        src/us_hydrodyn_asa.cpp \
        src/us_hydrodyn_asab1.cpp \
        src/us_hydrodyn_batch.cpp \
        src/us_hydrodyn_batch_movie_opts.cpp \
        src/us_hydrodyn_bead_output.cpp \
        src/us_hydrodyn_beads.cpp \
        src/us_hydrodyn_beads_load.cpp \
        src/us_hydrodyn_best.cpp \
        src/us_hydrodyn_bd.cpp \
        src/us_hydrodyn_bd_core.cpp \
        src/us_hydrodyn_bd_options.cpp \
        src/us_hydrodyn_bd_load_results_opts.cpp \
        src/us_hydrodyn_cluster.cpp \
        src/us_hydrodyn_cluster_advanced.cpp \
        src/us_hydrodyn_cluster_additional.cpp \
        src/us_hydrodyn_cluster_best.cpp \
        src/us_hydrodyn_cluster_bfnb.cpp \
        src/us_hydrodyn_cluster_bfnb_nsa.cpp \
        src/us_hydrodyn_cluster_config.cpp \
        src/us_hydrodyn_cluster_config_server.cpp \
        src/us_hydrodyn_cluster_dmd.cpp \
        src/us_hydrodyn_cluster_dammin.cpp \
        src/us_hydrodyn_cluster_oned.cpp \
        src/us_hydrodyn_cluster_results.cpp \
        src/us_hydrodyn_cluster_submit.cpp \
        src/us_hydrodyn_cluster_status.cpp \
        src/us_hydrodyn_comparative.cpp \
        src/us_hydrodyn_core.cpp \
        src/us_hydrodyn_csv_viewer.cpp \
        src/us_hydrodyn_dammin_opts.cpp \
        src/us_hydrodyn_dad.cpp \
        src/us_hydrodyn_dad_baseline_best.cpp \
        src/us_hydrodyn_dad_conc_csv_frames.cpp \
        src/us_hydrodyn_dad_gui.cpp \
        src/us_hydrodyn_dad_stack.cpp \
        src/us_hydrodyn_dad_modes.cpp \
        src/us_hydrodyn_dad_modes_bb.cpp \
        src/us_hydrodyn_dad_modes_baseline2.cpp \
        src/us_hydrodyn_dad_modes_guinier.cpp \
        src/us_hydrodyn_dad_modes_powerfit.cpp \
        src/us_hydrodyn_dad_modes_wheel.cpp \
        src/us_hydrodyn_dad_makeiq.cpp \
        src/us_hydrodyn_dad_plots.cpp \
        src/us_hydrodyn_dad_ciq.cpp \
        src/us_hydrodyn_dad_cistarq.cpp \
        src/us_hydrodyn_dad_dctr.cpp \
        src/us_hydrodyn_dad_gg.cpp \
        src/us_hydrodyn_dad_p3d.cpp \
        src/us_hydrodyn_dad_parameters.cpp \
        src/us_hydrodyn_dad_util.cpp \
        src/us_hydrodyn_dad_movie.cpp \
        src/us_hydrodyn_dad_conc.cpp \
        src/us_hydrodyn_dad_conc_load.cpp \
        src/us_hydrodyn_dad_fit.cpp \
        src/us_hydrodyn_dad_fit_global.cpp \
        src/us_hydrodyn_dad_fit_gsm.cpp \
        src/us_hydrodyn_dad_fit_ga.cpp \
        src/us_hydrodyn_dad_gauss_mode.cpp \
        src/us_hydrodyn_dad_nth.cpp \
        src/us_hydrodyn_dad_options.cpp \
        src/us_hydrodyn_dad_simulate.cpp \
        src/us_hydrodyn_dad_svd.cpp \
        src/us_hydrodyn_dad_scale_trend.cpp \
        src/us_hydrodyn_dmd_core.cpp \
        src/us_hydrodyn_dmd_options.cpp \
        src/us_hydrodyn_file.cpp \
        src/us_hydrodyn_file2.cpp \
        src/us_hydrodyn_fractal_dimension.cpp \
        src/us_hydrodyn_fractal_dimension_options.cpp \
        src/us_hydrodyn_grid.cpp \
        src/us_hydrodyn_grid_atob.cpp \
        src/us_hydrodyn_grpy.cpp \
        src/us_hydrodyn_hydrate.cpp \
        src/us_hydrodyn_hydrate_align.cpp \
        src/us_hydrodyn_hydro.cpp \
        src/us_hydrodyn_hydro_zeno.cpp \
        src/us_hydrodyn_info.cpp \
        src/us_hydrodyn_load.cpp \
        src/us_hydrodyn_mals.cpp \
        src/us_hydrodyn_mals_baseline_best.cpp \
        src/us_hydrodyn_mals_conc_csv_frames.cpp \
        src/us_hydrodyn_mals_gui.cpp \
        src/us_hydrodyn_mals_stack.cpp \
        src/us_hydrodyn_mals_modes.cpp \
        src/us_hydrodyn_mals_modes_guinier.cpp \
        src/us_hydrodyn_mals_modes_bb.cpp \
        src/us_hydrodyn_mals_modes_wheel.cpp \
        src/us_hydrodyn_mals_makeiq.cpp \
        src/us_hydrodyn_mals_plots.cpp \
        src/us_hydrodyn_mals_ciq.cpp \
        src/us_hydrodyn_mals_cistarq.cpp \
        src/us_hydrodyn_mals_dctr.cpp \
        src/us_hydrodyn_mals_gg.cpp \
        src/us_hydrodyn_mals_p3d.cpp \
        src/us_hydrodyn_mals_parameters.cpp \
        src/us_hydrodyn_mals_util.cpp \
        src/us_hydrodyn_mals_movie.cpp \
        src/us_hydrodyn_mals_conc.cpp \
        src/us_hydrodyn_mals_conc_load.cpp \
        src/us_hydrodyn_mals_fit.cpp \
        src/us_hydrodyn_mals_fit_global.cpp \
        src/us_hydrodyn_mals_fit_gsm.cpp \
        src/us_hydrodyn_mals_fit_ga.cpp \
        src/us_hydrodyn_mals_gauss_mode.cpp \
        src/us_hydrodyn_mals_nth.cpp \
        src/us_hydrodyn_mals_options.cpp \
        src/us_hydrodyn_mals_simulate.cpp \
        src/us_hydrodyn_mals_svd.cpp \
        src/us_hydrodyn_mals_scale_trend.cpp \
        src/us_hydrodyn_mals_saxs.cpp \
        src/us_hydrodyn_mals_saxs_baseline_best.cpp \
        src/us_hydrodyn_mals_saxs_conc_csv_frames.cpp \
        src/us_hydrodyn_mals_saxs_gui.cpp \
        src/us_hydrodyn_mals_saxs_stack.cpp \
        src/us_hydrodyn_mals_saxs_modes.cpp \
        src/us_hydrodyn_mals_saxs_modes_guinier.cpp \
        src/us_hydrodyn_mals_saxs_modes_bb.cpp \
        src/us_hydrodyn_mals_saxs_modes_scroll.cpp \
        src/us_hydrodyn_mals_saxs_modes_scale.cpp \
        src/us_hydrodyn_mals_saxs_modes_wheel.cpp \
        src/us_hydrodyn_mals_saxs_makeiq.cpp \
        src/us_hydrodyn_mals_saxs_plots.cpp \
        src/us_hydrodyn_mals_saxs_ciq.cpp \
        src/us_hydrodyn_mals_saxs_cistarq.cpp \
        src/us_hydrodyn_mals_saxs_dctr.cpp \
        src/us_hydrodyn_mals_saxs_gg.cpp \
        src/us_hydrodyn_mals_saxs_p3d.cpp \
        src/us_hydrodyn_mals_saxs_parameters.cpp \
        src/us_hydrodyn_mals_saxs_util.cpp \
        src/us_hydrodyn_mals_saxs_movie.cpp \
        src/us_hydrodyn_mals_saxs_conc.cpp \
        src/us_hydrodyn_mals_saxs_conc_load.cpp \
        src/us_hydrodyn_mals_saxs_fit.cpp \
        src/us_hydrodyn_mals_saxs_fit_global.cpp \
        src/us_hydrodyn_mals_saxs_fit_gsm.cpp \
        src/us_hydrodyn_mals_saxs_fit_ga.cpp \
        src/us_hydrodyn_mals_saxs_gauss_mode.cpp \
        src/us_hydrodyn_mals_saxs_nth.cpp \
        src/us_hydrodyn_mals_saxs_options.cpp \
        src/us_hydrodyn_mals_saxs_simulate.cpp \
        src/us_hydrodyn_mals_saxs_svd.cpp \
        src/us_hydrodyn_mals_saxs_scale_trend.cpp \
        src/us_hydrodyn_misc.cpp \
        src/us_hydrodyn_other.cpp \
        src/us_hydrodyn_overlap.cpp \
        src/us_hydrodyn_overlap_reduction.cpp \
        src/us_hydrodyn_pat.cpp \
        src/us_hydrodyn_pdb_parsing.cpp \
        src/us_hydrodyn_pdb_tool.cpp \
        src/us_hydrodyn_pdb_tool_merge.cpp \
        src/us_hydrodyn_pdb_tool_renum.cpp \
        src/us_hydrodyn_pdb_tool_selres.cpp \
        src/us_hydrodyn_pdb_tool_sort.cpp \
        src/us_hydrodyn_pdb_tool_util.cpp \
        src/us_hydrodyn_pdb_visualization.cpp \
        src/us_hydrodyn_results.cpp \
        src/us_hydrodyn_sas_options_saxs.cpp \
        src/us_hydrodyn_sas_options_sans.cpp \
        src/us_hydrodyn_sas_options_curve.cpp \
        src/us_hydrodyn_sas_options_bead_model.cpp \
        src/us_hydrodyn_sas_options_hydration.cpp \
        src/us_hydrodyn_sas_options_guinier.cpp \
        src/us_hydrodyn_sas_options_xsr.cpp \
        src/us_hydrodyn_sas_options_misc.cpp \
        src/us_hydrodyn_sas_options_experimental.cpp \
        src/us_hydrodyn_save.cpp \
        src/us_hydrodyn_saxs.cpp \
        src/us_hydrodyn_saxs_conc.cpp \
        src/us_hydrodyn_saxs_cormap.cpp \
        src/us_hydrodyn_saxs_external.cpp \
        src/us_hydrodyn_saxs_guinier.cpp \
        src/us_hydrodyn_saxs_fits.cpp \
        src/us_hydrodyn_saxs_ift.cpp \
        src/us_hydrodyn_saxs_iqq.cpp \
        src/us_hydrodyn_saxs_iqq_bead_model.cpp \
        src/us_hydrodyn_saxs_iqq_load_csv.cpp \
        src/us_hydrodyn_saxs_iqq_residuals.cpp \
        src/us_hydrodyn_saxs_iqq_sh.cpp \
        src/us_hydrodyn_saxs_load_csv.cpp \
        src/us_hydrodyn_saxs_loads.cpp \
        src/us_hydrodyn_saxs_legend.cpp \
        src/us_hydrodyn_saxs_mw.cpp \
        src/us_hydrodyn_saxs_options.cpp \
        src/us_hydrodyn_saxs_plots.cpp \
        src/us_hydrodyn_saxs_pr.cpp \
        src/us_hydrodyn_saxs_residuals.cpp \
        src/us_hydrodyn_saxs_screen.cpp \
        src/us_hydrodyn_saxs_search.cpp \
        src/us_hydrodyn_saxs_buffer.cpp \
        src/us_hydrodyn_saxs_buffer_conc.cpp \
        src/us_hydrodyn_saxs_buffer_conc_load.cpp \
        src/us_hydrodyn_saxs_buffer_nth.cpp \
        src/us_hydrodyn_saxs_hplc.cpp \
        src/us_hydrodyn_saxs_hplc_baseline_best.cpp \
        src/us_hydrodyn_saxs_hplc_conc_csv_frames.cpp \
        src/us_hydrodyn_saxs_hplc_gui.cpp \
        src/us_hydrodyn_saxs_hplc_stack.cpp \
        src/us_hydrodyn_saxs_hplc_modes.cpp \
        src/us_hydrodyn_saxs_hplc_modes_bb.cpp \
        src/us_hydrodyn_saxs_hplc_modes_wheel.cpp \
        src/us_hydrodyn_saxs_hplc_makeiq.cpp \
        src/us_hydrodyn_saxs_hplc_parameters.cpp \
        src/us_hydrodyn_saxs_hplc_plots.cpp \
        src/us_hydrodyn_saxs_hplc_ciq.cpp \
        src/us_hydrodyn_saxs_hplc_dctr.cpp \
        src/us_hydrodyn_saxs_hplc_gg.cpp \
        src/us_hydrodyn_saxs_hplc_p3d.cpp \
        src/us_hydrodyn_saxs_hplc_util.cpp \
        src/us_hydrodyn_saxs_hplc_movie.cpp \
        src/us_hydrodyn_saxs_hplc_conc.cpp \
        src/us_hydrodyn_saxs_hplc_conc_load.cpp \
        src/us_hydrodyn_saxs_hplc_fit.cpp \
        src/us_hydrodyn_saxs_hplc_fit_global.cpp \
        src/us_hydrodyn_saxs_hplc_fit_gsm.cpp \
        src/us_hydrodyn_saxs_hplc_fit_ga.cpp \
        src/us_hydrodyn_saxs_hplc_gauss_mode.cpp \
        src/us_hydrodyn_saxs_hplc_nth.cpp \
        src/us_hydrodyn_saxs_hplc_options.cpp \
        src/us_hydrodyn_saxs_hplc_simulate.cpp \
        src/us_hydrodyn_saxs_hplc_svd.cpp \
        src/us_hydrodyn_saxs_hplc_scale_trend.cpp \
        src/us_hydrodyn_saxs_1d.cpp \
        src/us_hydrodyn_saxs_2d.cpp \
        src/us_hydrodyn_script.cpp \
        src/us_hydrodyn_settings.cpp \
        src/us_hydrodyn_supc.cpp \
        src/us_hydrodyn_util.cpp \
        src/us_hydrodyn_vdw_overlap.cpp \
        src/us_hydrodyn_xsr.cpp \
        src/us_hydrodyn_write.cpp \
        src/us_hydrodyn_zeno.cpp \
#  us_imgviewer.cpp \
#  us_infopanel.cpp \
        src/us_json.cpp \
        src/us_mqt.cpp \
        src/us_license.cpp \
        src/us_lm.cpp \
        src/us_lud.cpp \
#  us_lncr2.cpp \
        src/us_math.cpp \
        src/us_matrix.cpp \
#  us_meniscus.cpp \
#  us_meniscus_dlg.cpp \
#  us_merge.cpp \
#  us_mfem.cpp \
#  us_minimize.cpp \
#  us_model_editor.cpp \
#  us_modelselection.cpp \
#  us_montecarlo.cpp \
#  us_montecarloreport.cpp \
#  us_montecarlostats.cpp \
#  us_nucleotide.cpp \
        src/us_pdb_util.cpp \
        src/us_pm.cpp \
        src/us_pm_best.cpp \
        src/us_pm_best_sphere.cpp \
        src/us_pm_best_cylinder.cpp \
        src/us_pm_best_spheroid.cpp \
        src/us_pm_best_ellipsoid.cpp \
        src/us_pm_best_torus.cpp \
        src/us_pm_best_torus_segment.cpp \
        src/us_pm_fitness.cpp \
        src/us_pm_ga.cpp \
        src/us_pm_objects.cpp \
        src/us_pm_test.cpp \
#        src/us_polyfit.cpp \
        src/us_plot_zoom.cpp \
#  us_pixmap.cpp \
#  us_plotdata.cpp \
#  us_printfilter.cpp \
#  us_pseudo3d_combine.cpp \
#  us_pseudo3d_thread.cpp \
#  us_pseudoabs.cpp \
#  us_radial_correction.cpp \
        src/us_register.cpp \
#  us_reorder.cpp \
#  us_resplot.cpp \
#  us_rotor_stretch.cpp \
#  us_rundetails.cpp \
#  us_sa2d.cpp \
#  us_sa2d_control.cpp \
#  us_sa2dbase.cpp \
#  us_sassoc.cpp \
        src/us_saxs_gp.cpp \
        src/us_saxs_util.cpp \
        src/us_saxs_util_asab1.cpp \
        src/us_saxs_util_a2sb.cpp \
        src/us_saxs_util_best.cpp \
        src/us_saxs_util_c2check.cpp \
        src/us_saxs_util_crysol.cpp \
        src/us_saxs_util_cuda.cpp \
        src/us_saxs_util_dammin.cpp \
        src/us_saxs_util_dmd.cpp \
        src/us_saxs_util_extern.cpp \
        src/us_saxs_util_guinier.cpp \
        src/us_saxs_util_gui_only.cpp \
        src/us_saxs_util_hydrate.cpp \
        src/us_saxs_util_hydrate_align.cpp \
        src/us_saxs_util_ift.cpp \
        src/us_saxs_util_iqq.cpp \
        src/us_saxs_util_iqq_1d.cpp \
        src/us_saxs_util_iqq_pdb.cpp \
        src/us_saxs_util_iqq_bead_model.cpp \
        src/us_saxs_util_loads.cpp \
        src/us_saxs_util_nsa.cpp \
        src/us_saxs_util_nsa_ga.cpp \
        src/us_saxs_util_nsa_gsm.cpp \
        src/us_saxs_util_nsa_sga.cpp \
        src/us_saxs_util_pat.cpp \
        src/us_saxs_util_pm.cpp \
        src/us_saxs_util_sgp.cpp \
        src/us_saxs_util_sgp_phys.cpp \
        src/us_saxs_util_ssbond.cpp \
        src/us_saxs_util_static.cpp \
#  us_select_channel.cpp \
#  us_selectmodel.cpp \
#  us_selectmodel10.cpp \
#  us_selectmodel13.cpp \
#  us_selectmodel3.cpp \
#  us_selectplot.cpp \
        src/us_sh.cpp \
#  us_showdetails.cpp \
#  us_simulationparameters.cpp \
#  us_smdat.cpp \
#  us_spectrum.cpp \
#  us_stiffbase.cpp \
        src/us_surfracer.cpp \
        src/us_svd.cpp \
        src/us_efa.cpp \
        src/us_tar.cpp \
        src/us_timer.cpp \
#  us_thread.cpp \
        src/us_util.cpp \
#  us_vbar.cpp \
        src/us_vector.cpp \
#  us_velocreport.cpp \
#  us_velocmodelctrl.cpp \
#  us_vhwdat.cpp \
#  us_vhwsim.cpp \
#  us_vhwenhanced.cpp \
#  us_viewmwl.cpp \
        src/us_vvv.cpp \
#  us_widgets.cpp \
#  us_win.cpp \
        src/us_write_config.cpp \
        src/us_zeno_cxx.cpp \
        src/us_zeno_cxx_nf.cpp \
        src/qwt/scrollbar.cpp \
        src/qwt/scrollzoomer.cpp \
        src/us_gui_settings.cpp \
        src/us_hydrodyn_saxs_guinier_frames.cpp \
        src/us_plot_util.cpp \
        src/us3i_color.cpp \
        src/us3i_editor.cpp \
        src/us3i_editor_gui.cpp \
        src/us3i_gui_settings.cpp \
        src/us3i_settings.cpp \
        src/us3i_widgets.cpp \
        src/us3i_widgets_dialog.cpp \
        src/us3i_images.cpp \
        src/us3i_global.cpp \
        src/us3i_gui_util.cpp \
        src/us3i_util.cpp \
        src/us3i_gzip.cpp \
        src/us3i_plot.cpp \
        include/us_revision.h \
        shd_mpi/shs_use.cpp
#  us_db_rst_channel.cpp \
#  us_db_rst_equil.cpp \
#  us_db_rst_veloc.cpp \
#  us_db_rst_cell.cpp \
#  us_db_laser.cpp \
#  us_globallaser.cpp \
#  us_kirkwood.cpp \
#  us_laser.cpp \

HEADERS *= \
#  3dplot/mesh2mainwindowbase.h \
#  3dplot/mesh2mainwindow.h \
#  3dplot/functions.h \
#  3dplot/colormapreader.h \
#  3dplot/lightingdlgbase.h \
#  3dplot/lightingdlg.h \
#  femreader.h \
#  D_calc.h \
#?  bluearrow.xpm \
#?  greenarrow.xpm \
#?  redarrow.xpm \
        include/us.h \
#  us_2dplot.h \
#  us_3dsolutes.h \
        include/us_admin.h \
        include/us_arch.h \
        include/us_average.h \
#  us_archive.h \
#  us_astfem_math.h \
#  us_astfem_rsa.h \
#  us_astfem_sim.h \
#  us_average.h \
#  us_buffer.h \
#  us_calendar.h \
#  us_clipdata.h \
        include/us_container.h \
        include/us_container_grpy.h \
        include/us_cmdline_app.h \
#  us_cofdistro.h \
#  us_cofs.h \
        include/us_color.h \
#  us_colorgradient.h \
#  us_combine.h \
        include/us_config_gui.h \
#  us_constraint_control.h \
#  us_create_global.h \
        include/us_csv.h \
#  us_data_io.h \
#  us_database.h \
#  us_db.h \
#  us_db_admin.h \
#  us_db_veloc.h \
#  us_db_equil.h \
#  us_db_rst_equilproject.h \
#  us_db_rst_montecarlo.h \
#  us_db_rtv_date.h \
#  us_db_rtv_description.h \
#  us_db_rtv_edittype.h \
#  us_db_rtv_image.h \
#  us_db_rtv_investigator.h \
#  us_db_rtv_request.h \
#  us_db_rtv_requeststatus.h \
#  us_db_rtv_runrequest.h \
#  us_db_rtv_sample.h \
#  us_db_runrequest.h \
#  us_db_t.h \
#  us_db_tbl_buffer.h \
#  us_db_tbl_cell.h \
#  us_db_tbl_expdata.h \
#  us_db_tbl_investigator.h \
#  us_db_tbl_nucleotide.h \
#  us_db_tbl_vbar.h \
#  us_db_template.h \
#  us_db_widgets.h \
#  us_dcdrdat.h \
#  us_dcdtdat.h \
#  us_diagnostics.h \
        include/us_dirhist.h \
#  us_dma60.h \
#  us_dtactrl.h \
#  us_edeabs.h \
#  us_edeflo.h \
#  us_edeint.h \
#  us_edit.h \
#  us_editdb.h \
        include/us_editor.h \
#  us_editwavelength.h \
#  us_edscan.h \
#  us_edvabs.h \
#  us_edvflo.h \
#  us_edvint.h \
#  us_encryption.h \
#  us_enter_dna.h \
#  us_eqfitter.h \
#  us_eqmodelctrl.h \
#  us_equilprojectreport.h \
#  us_equilreport.h \
#  us_equilsim.h \
#  us_equilspeed.h \
#  us_equiltime.h \
#  us_exclude_profile.h \
#  us_export.h \
        include/us_extern.h \
#  us_extinctfitter.h \
#  us_extinction.h \
#  us_fefit.h \
#  us_fematch.h \
#  us_fematch_ra.h \
#  us_femglobal.h \
        include/us_ffd.h \
        include/us_file_util.h \
#  us_finite1.h \
#  us_finite2.h \
        include/us_font.h \
#  us_ga_initialize.h \
#  us_ga_model_editor.h \
#  us_global.h \
#  us_globalequil.h \
        include/us_gzip.h \
#  us_htmledit.h \
#  us_hydro.h \
        include/us_hydrodyn.h \
        include/us_hydrodyn_addatom.h \
        include/us_hydrodyn_addhybrid.h \
        include/us_hydrodyn_addresidue.h \
        include/us_hydrodyn_addsaxs.h \
        include/us_hydrodyn_advanced_config.h \
        include/us_hydrodyn_anaflex_options.h \
        include/us_hydrodyn_asa.h \
        include/us_hydrodyn_asab1.h \
        include/us_hydrodyn_batch.h \
        include/us_hydrodyn_batch_movie_opts.h \
        include/us_hydrodyn_bd.h \
        include/us_hydrodyn_bd_options.h \
        include/us_hydrodyn_bd_load_results_opts.h \
        include/us_hydrodyn_bead_output.h \
        include/us_hydrodyn_best.h \
        include/us_hydrodyn_cluster.h \
        include/us_hydrodyn_cluster_additional.h \
        include/us_hydrodyn_cluster_advanced.h \
        include/us_hydrodyn_cluster_bfnb.h \
        include/us_hydrodyn_cluster_best.h \
        include/us_hydrodyn_cluster_bfnb_nsa.h \
        include/us_hydrodyn_cluster_config.h \
        include/us_hydrodyn_cluster_config_server.h \
        include/us_hydrodyn_cluster_dammin.h \
        include/us_hydrodyn_cluster_dmd.h \
        include/us_hydrodyn_cluster_oned.h \
        include/us_hydrodyn_cluster_results.h \
        include/us_hydrodyn_cluster_submit.h \
        include/us_hydrodyn_cluster_status.h \
        include/us_hydrodyn_comparative.h \
        include/us_hydrodyn_csv_viewer.h \
        include/us_hydrodyn_dammin_opts.h \
        include/us_hydrodyn_dad.h \
        include/us_hydrodyn_dad_ciq.h \
        include/us_hydrodyn_dad_cistarq.h \
        include/us_hydrodyn_dad_conc_csv_frames.h \
        include/us_hydrodyn_dad_dctr.h \
        include/us_hydrodyn_dad_p3d.h \
        include/us_hydrodyn_dad_parameters.h \
        include/us_hydrodyn_dad_conc.h \
        include/us_hydrodyn_dad_conc_load.h \
        include/us_hydrodyn_dad_fit.h \
        include/us_hydrodyn_dad_fit_global.h \
        include/us_hydrodyn_dad_gauss_mode.h \
        include/us_hydrodyn_dad_movie.h \
        include/us_hydrodyn_dad_nth.h \
        include/us_hydrodyn_dad_options.h \
        include/us_hydrodyn_dad_scale_trend.h \
        include/us_hydrodyn_dad_svd.h \
        include/us_hydrodyn_dad_baseline_best.h \
        include/us_hydrodyn_dad_simulate.h \
        include/us_hydrodyn_dmd_options.h \
        include/us_hydrodyn_file.h \
        include/us_hydrodyn_file2.h \
        include/us_fractal_dimension.h \
        include/us_hydrodyn_fractal_dimension_options.h \
        include/us_hydrodyn_grid.h \
        include/us_hydrodyn_grid_atob.h \
        include/us_hydrodyn_hydro.h \
        include/us_hydrodyn_hydro_zeno.h \
        include/us_hydrodyn_mals.h \
        include/us_hydrodyn_mals_ciq.h \
        include/us_hydrodyn_mals_cistarq.h \
        include/us_hydrodyn_mals_conc_csv_frames.h \
        include/us_hydrodyn_mals_dctr.h \
        include/us_hydrodyn_mals_p3d.h \
        include/us_hydrodyn_mals_parameters.h \
        include/us_hydrodyn_mals_conc.h \
        include/us_hydrodyn_mals_conc_load.h \
        include/us_hydrodyn_mals_fit.h \
        include/us_hydrodyn_mals_fit_global.h \
        include/us_hydrodyn_mals_gauss_mode.h \
        include/us_hydrodyn_mals_movie.h \
        include/us_hydrodyn_mals_nth.h \
        include/us_hydrodyn_mals_options.h \
        include/us_hydrodyn_mals_scale_trend.h \
        include/us_hydrodyn_mals_svd.h \
        include/us_hydrodyn_mals_baseline_best.h \
        include/us_hydrodyn_mals_simulate.h \
        include/us_hydrodyn_mals_saxs.h \
        include/us_hydrodyn_mals_saxs_ciq.h \
        include/us_hydrodyn_mals_saxs_cistarq.h \
        include/us_hydrodyn_mals_saxs_conc_csv_frames.h \
        include/us_hydrodyn_mals_saxs_dctr.h \
        include/us_hydrodyn_mals_saxs_p3d.h \
        include/us_hydrodyn_mals_saxs_parameters.h \
        include/us_hydrodyn_mals_saxs_conc.h \
        include/us_hydrodyn_mals_saxs_conc_load.h \
        include/us_hydrodyn_mals_saxs_fit.h \
        include/us_hydrodyn_mals_saxs_fit_global.h \
        include/us_hydrodyn_mals_saxs_gauss_mode.h \
        include/us_hydrodyn_mals_saxs_movie.h \
        include/us_hydrodyn_mals_saxs_nth.h \
        include/us_hydrodyn_mals_saxs_options.h \
        include/us_hydrodyn_mals_saxs_scale_trend.h \
        include/us_hydrodyn_mals_saxs_svd.h \
        include/us_hydrodyn_mals_saxs_baseline_best.h \
        include/us_hydrodyn_mals_saxs_simulate.h \
        include/us_hydrodyn_misc.h \
        include/us_hydrodyn_overlap.h \
        include/us_hydrodyn_overlap_reduction.h \
        include/us_hydrodyn_pdbdefs.h \
        include/us_hydrodyn_pat.h \
        include/us_hydrodyn_pdb_parsing.h \
        include/us_hydrodyn_pdb_tool.h \
        include/us_hydrodyn_pdb_tool_merge.h \
        include/us_hydrodyn_pdb_tool_renum.h \
        include/us_hydrodyn_pdb_tool_selres.h \
        include/us_hydrodyn_pdb_tool_sort.h \
        include/us_hydrodyn_pdb_visualization.h \
        include/us_hydrodyn_results.h \
        include/us_hydrodyn_sas_options_saxs.h \
        include/us_hydrodyn_sas_options_sans.h \
        include/us_hydrodyn_sas_options_curve.h \
        include/us_hydrodyn_sas_options_bead_model.h \
        include/us_hydrodyn_sas_options_hydration.h \
        include/us_hydrodyn_sas_options_guinier.h \
        include/us_hydrodyn_sas_options_xsr.h \
        include/us_hydrodyn_sas_options_misc.h \
        include/us_hydrodyn_sas_options_experimental.h \
        include/us_hydrodyn_save.h \
        include/us_hydrodyn_saxs.h \
        include/us_hydrodyn_saxs_conc.h \
        include/us_hydrodyn_saxs_load_csv.h \
        include/us_hydrodyn_saxs_mw.h \
        include/us_hydrodyn_saxs_options.h \
        include/us_hydrodyn_saxs_ift.h \
        include/us_hydrodyn_saxs_iqq_load_csv.h \
        include/us_hydrodyn_saxs_iqq_residuals.h \
        include/us_hydrodyn_saxs_residuals.h \
        include/us_hydrodyn_saxs_screen.h \
        include/us_hydrodyn_saxs_search.h \
        include/us_hydrodyn_saxs_buffer.h \
        include/us_hydrodyn_saxs_buffer_conc.h \
        include/us_hydrodyn_saxs_buffer_conc_load.h \
        include/us_hydrodyn_saxs_buffer_nth.h \
        include/us_hydrodyn_saxs_hplc.h \
        include/us_hydrodyn_saxs_hplc_ciq.h \
        include/us_hydrodyn_saxs_hplc_conc_csv_frames.h \
        include/us_hydrodyn_saxs_hplc_dctr.h \
        include/us_hydrodyn_saxs_hplc_p3d.h \
        include/us_hydrodyn_saxs_hplc_conc.h \
        include/us_hydrodyn_saxs_hplc_conc_load.h \
        include/us_hydrodyn_saxs_hplc_fit.h \
        include/us_hydrodyn_saxs_hplc_fit_global.h \
        include/us_hydrodyn_saxs_hplc_gauss_mode.h \
        include/us_hydrodyn_saxs_hplc_movie.h \
        include/us_hydrodyn_saxs_hplc_nth.h \
        include/us_hydrodyn_saxs_hplc_options.h \
        include/us_hydrodyn_saxs_hplc_parameters.h \
        include/us_hydrodyn_saxs_hplc_scale_trend.h \
        include/us_hydrodyn_saxs_hplc_svd.h \
        include/us_hydrodyn_saxs_hplc_baseline_best.h \
        include/us_hydrodyn_saxs_hplc_simulate.h \
        include/us_hydrodyn_saxs_1d.h \
        include/us_hydrodyn_saxs_2d.h \
        include/us_hydrodyn_supc.h \
        include/us_hydrodyn_vdw_overlap.h \
        include/us_hydrodyn_xsr.h \
        include/us_hydrodyn_zeno.h \
        include/us_fable.h \
#  us_imgviewer.h \
#  us_infopanel.h \
        include/us_json.h \
        include/us_mqt.h \
        include/us_license.h \
        include/us_lm.h \
        include/us_lud.h \
#  us_lncr2.h \
        include/us_math.h \
        include/us_matrix.h \
#  us_meniscus.h \
#  us_meniscus_dlg.h \
#  us_merge.h \
#  us_mfem.h \
#  us_minimize.h \
#  us_model_editor.h \
#  us_modelselection.h \
#  us_montecarlo.h \
#  us_montecarloreport.h \
#  us_montecarlostats.h \
#  us_nucleotide.h \
        include/us_pdb_util.h \
        include/us_pm.h \
        include/us_plot_zoom.h \
#  us_plotdata.h \
#  us_printfilter.h \
#  us_pseudo3d_combine.h \
#  us_pseudo3d_thread.h \
#  us_pseudoabs.h \
#  us_radial_correction.h \
        include/us_register.h \
#  us_reorder.h \
#  us_resplot.h \
#  us_rotor_stretch.h \
        include/us_revision.h \
#  us_rundetails.h \
#  us_sa2d.h \
#  us_sa2d_control.h \
#  us_sa2dbase.h \
#  us_sassoc.h \
        include/us_saxs_gp.h \
        include/us_saxs_util.h \
        include/us_saxs_util_nsa.h \
#  us_select_channel.h \
#  us_selectmodel.h \
#  us_selectmodel10.h \
#  us_selectmodel13.h \
#  us_selectmodel3.h \
#  us_selectplot.h \
        include/us_sh.h \
#  us_showdetails.h \
#  us_simulationparameters.h \
#  us_smdat.h \
#  us_spectrum.h \
#  us_stiffbase.h \
        include/us_surfracer.h \
        include/us_svd.h \
        include/us_efa.h \
        include/us_eigen.h \
#        include/us_polyfit.h \
        include/us_tar.h \
        include/us_timer.h \
        include/us_thread.h \
        include/us_tnt_jama.h \
        include/us_unicode.h \
        include/us_util.h \
#  us_vbar.h \
        include/us_vector.h \
#  us_velocreport.h \
#  us_velocmodelctrl.h \
        include/us_version.h \
#  us_vhwdat.h \
#  us_vhwsim.h \
#  us_vhwenhanced.h \
#  us_viewmwl.h \
        include/us_vvv.h \
#  us_widgets.h \
#  us_win.h \
        include/us_write_config.h \
        include/us_zeno_cxx.h \
        include/us_zeno_cxx_nf.h \
        include/us3_extern.h \
        include/us3_defines.h \
        include/us_gui_settings.h \
        include/qwt/scrollbar.h \
        include/qwt/scrollzoomer.h \
        include/us_hydrodyn_saxs_guinier_frames.h \
        include/us_hydrodyn_saxs_cormap.h \
        include/us_plot_util.h \
        include/us3i_color.h \
        include/us3i_extern.h \
        include/us3i_editor.h \
        include/us3i_editor_gui.h \
        include/us3i_gui_settings.h \
        include/us3i_settings.h \
        include/us3i_widgets.h \
        include/us3i_widgets_dialog.h \
        include/us3i_images.h \
        include/us3i_global.h \
        include/us3i_gui_util.h \
        include/us3i_util.h \
        include/us3i_gzip.h \
        include/us3i_plot.h \
        include/us3i_pixmaps.h \
        shd_mpi/shs_data.h

# IMAGES = \
#   include/editcopy.xpm \
#   include/editcut.xpm \
#   include/editpaste.xpm \
#   include/editredo.xpm \
#   include/editundo.xpm \
#   include/filenew.xpm \
#   include/fileopen.xpm \
#   include/fileprint.xpm \
#   include/filesave.xpm \
#   include/textbold.xpm \
#   include/textcenter.xpm \
#   include/textitalic.xpm \
#   include/textjustify.xpm \
#   include/textleft.xpm \
#   include/textright.xpm \
#   include/textunder.xpm

QT += opengl network
