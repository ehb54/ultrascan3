# Messages
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

revision.target           = include/us_revision.h
revision.depends          = FORCE
QMAKE_EXTRA_TARGETS       += revision

TEMPLATE       = lib
TRANSLATIONS   = lib.ts
VERSION        = 10
MOC_DIR        = src/moc
OBJECTS_DIR    = src/obj

#RC_FILE        = ../icon.rc

# enabled threading in fitting algorithm:
DEFINES += THREAD

# Automatic hardware platform and operating system configurations:

INCLUDEPATH = $(QWTDIR)/src $(QTDIR)/include
DEPENDPATH += src include
DEFINES += NO_DB
# temporary fix (us2 code was using qt2 qpdevmnt which I think need to be replaced by qprintdevicemetrics)
DEFINES += NO_EDITOR_PRINT
DEFINES += QT4

unix {
  revision.commands         = sh revision.sh
  TARGET                  = us_somo
  QMAKE_CXXFLAGS_WARN_ON += -Wno-non-virtual-dtor
  DEFINES                += UNIX
  #CONFIG                 += qt thread warn release 
  CONFIG                 += qt thread warn debug

  INCLUDEPATH    +=  $(QWTDIR)/include

  contains(UNAME,x86_64) {
    LIBS    += -L$(QWTDIR)/lib64/ -L$(QWTDIR)/lib/ -lqwt 
    DEFINES += BIN64
    DESTDIR  = ../lib64
  } else {
    LIBS += -L$(QWTDIR)/lib -lqwt
    DESTDIR  = ../lib
  }
}

win32 {
  MINGWDIR        = C:/mingw
  QTPATH          = C:/Qt/4.8.4
  QWTPATH         = C:/qwt-5.2.3
  US3PATH         = C:/Users/Admin/Documents/ultrascan3
  QWT3DPATH       = $$US3PATH/qwtplot3d-qt4
  DESTDIR         = ../bin

  QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_STL
  QMAKESPEC       = win32-g++-4.6

  CONFIG         += qt warn_on opengl thread zlib release
  CONFIG         += dll exceptions

  DEFINES        += US_MAKE_DLL US_MAKE_GUI_DLL
  DEFINES        += MINGW

  contains( DEBUGORRELEASE, debug ) {
    QWTLIB        = $$QWTPATH/lib/libqwtd5.a
  } else {
    QWTLIB        = $$QWTPATH/lib/libqwt5.a
  }

  INCLUDEPATH    += src
  INCLUDEPATH    += $$QWT3DPATH/include

  LIBS           += -lQtOpenGL4
  LIBS           += $$QWTLIB
  LIBS           += $$MINGWDIR/lib/libkernel32.a
  LIBS           += $$MINGWDIR/lib/libws2_32.a $$MINGWDIR/lib/libadvapi32.a
  LIBS           += $$MINGWDIR/lib/libgdi32.a $$MINGWDIR/lib/libuser32.a

  LIBS           += -L$$QWTPATH/lib -lqwt5
  LIBS           += $$US3PATH/lib/libqwtplot3d-qt4.a
  LIBS           += -L$$US3PATH/lib -lqwtplot3d-qt4

}

macx {
  revision.commands         = sh revision.sh

  BUILDBASE   = /Users/eb/us3/ultrascan3
  QWTPATH     = /src/qwt-5.2.3
  QWTLIB      = -L$$QWTPATH/lib -lqwt

  CONFIG      += x86_64
  DEFINES     += MAC OSX

  INCLUDEPATH += /usr/include
  INCLUDEPATH += /System/Library/Frameworks/OpenGL.framework/Versions/A/Headers
  INCLUDEPATH += $$QWTPATH/src
  INCLUDEPATH += /Users/eb/us3/qwtplot3d-qt4/include
  INCLUDEPATH += /usr/X11R6/include
  INCLUDEPATH += /Library/Frameworks/QtCore.framework/Versions/4/Headers
  INCLUDEPATH += /Library/Frameworks/QtGui.framework//Versions/4/Headers
  INCLUDEPATH += /Library/Frameworks/QtOpenGL.framework/Versions/4/Headers
  INCLUDEPATH += /Library/Frameworks/QtSvg.framework/Versions/4/Headers
  INCLUDEPATH += /Library/Frameworks/QtXml.framework/Versions/4/Headers

  LIBS        += -L/System/Library/Frameworks/OpenGL.framework/Libraries
  LIBS        += -L/Users/eb/us3/lib
  LIBS        += -lssl -lcrypto -lqwtplot3d-qt4
  LIBS        += -framework QtOpenGL

#  X11LIB       = -L/usr/X11R6/lib -lXau -lX11
}


# Do not remake cpp and h files from ui files
#FORMS = 3dplot/mesh2mainwindowbase.ui 3dplot/lightingdlgbase.ui


SOURCES += \
  us_admin.cpp \
  3dplot/mesh2mainwindowbase.cpp \
  3dplot/mesh2mainwindow.cpp \
  3dplot/colormapreader.cpp \
  3dplot/lightingdlgbase.cpp \
  3dplot/lightingdlg.cpp \
  3dplot/mesh.cpp \
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
  us_cmdline_app.cpp \
#  us_cofdistro.cpp \
#  us_cofs.cpp \ 
   us_color.cpp \
#  us_colorgradient.cpp \
#  us_combine.cpp \
  us_config.cpp \
  us_config_gui.cpp \
#  us_constraint_control.cpp \
#  us_create_global.cpp \
  us_csv.cpp \
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
  us_dirhist.cpp \
#  us_dma60.cpp \
#  us_dtactrl.cpp \
#  us_edeabs.cpp \
#  us_edeflo.cpp \
#  us_edeint.cpp \
#  us_edit.cpp \
#  us_editdb.cpp \
  us_editor.cpp \
#  us_editwavelength.cpp \
#  us_edscan.cpp \
#  us_edvabs.cpp \
#  us_edvflo.cpp \
#  us_edvint.cpp \
  us_encryption.cpp \
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
  us_file_util.cpp \
#  us_finite1.cpp \
#  us_finite2.cpp \
  us_font.cpp \
#  us_ga_initialize.cpp \
#  us_ga_model_editor.cpp \
  us_global.cpp \
#  us_globalequil.cpp \
  us_help.cpp \
  us_gzip.cpp \
#  us_htmledit.cpp \
#  us_hydro.cpp \
  us_hydrodyn.cpp \
  us_hydrodyn_addatom.cpp \
  us_hydrodyn_addhybrid.cpp \
  us_hydrodyn_addresidue.cpp \
  us_hydrodyn_addsaxs.cpp \
  us_hydrodyn_advanced_config.cpp \
  us_hydrodyn_anaflex_core.cpp \
  us_hydrodyn_anaflex_options.cpp \
  us_hydrodyn_asa.cpp \
  us_hydrodyn_asab1.cpp \
  us_hydrodyn_batch.cpp \
  us_hydrodyn_batch_movie_opts.cpp \
  us_hydrodyn_bead_output.cpp \
  us_hydrodyn_best.cpp \
  us_hydrodyn_bd.cpp \
  us_hydrodyn_bd_core.cpp \
  us_hydrodyn_bd_options.cpp \
  us_hydrodyn_bd_load_results_opts.cpp \
  us_hydrodyn_cluster.cpp \
  us_hydrodyn_cluster_advanced.cpp \
  us_hydrodyn_cluster_additional.cpp \
  us_hydrodyn_cluster_best.cpp \
  us_hydrodyn_cluster_bfnb.cpp \
  us_hydrodyn_cluster_bfnb_nsa.cpp \
  us_hydrodyn_cluster_config.cpp \
  us_hydrodyn_cluster_config_server.cpp \
  us_hydrodyn_cluster_dmd.cpp \
  us_hydrodyn_cluster_dammin.cpp \
  us_hydrodyn_cluster_oned.cpp \
  us_hydrodyn_cluster_results.cpp \
  us_hydrodyn_cluster_submit.cpp \
  us_hydrodyn_cluster_status.cpp \
  us_hydrodyn_comparative.cpp \
  us_hydrodyn_core.cpp \
  us_hydrodyn_csv_viewer.cpp \
  us_hydrodyn_dammin_opts.cpp \
  us_hydrodyn_dmd_core.cpp \
  us_hydrodyn_dmd_options.cpp \
  us_hydrodyn_file.cpp \
  us_hydrodyn_file2.cpp \
  us_hydrodyn_grid.cpp \
  us_hydrodyn_grid_atob.cpp \
  us_hydrodyn_hydrate.cpp \
  us_hydrodyn_hydrate_align.cpp \
  us_hydrodyn_hydro.cpp \
  us_hydrodyn_hydro_zeno.cpp \
  us_hydrodyn_misc.cpp \
  us_hydrodyn_other.cpp \
  us_hydrodyn_overlap.cpp \
  us_hydrodyn_overlap_reduction.cpp \
  us_hydrodyn_pat.cpp \
  us_hydrodyn_pdb_parsing.cpp \
  us_hydrodyn_pdb_tool.cpp \
  us_hydrodyn_pdb_tool_merge.cpp \
  us_hydrodyn_pdb_tool_renum.cpp \
  us_hydrodyn_pdb_tool_selres.cpp \
  us_hydrodyn_pdb_tool_sort.cpp \
  us_hydrodyn_pdb_tool_util.cpp \
  us_hydrodyn_pdb_visualization.cpp \
  us_hydrodyn_results.cpp \
  us_hydrodyn_sas_options_saxs.cpp \
  us_hydrodyn_sas_options_sans.cpp \
  us_hydrodyn_sas_options_curve.cpp \
  us_hydrodyn_sas_options_bead_model.cpp \
  us_hydrodyn_sas_options_hydration.cpp \
  us_hydrodyn_sas_options_guinier.cpp \
  us_hydrodyn_sas_options_xsr.cpp \
  us_hydrodyn_sas_options_misc.cpp \
  us_hydrodyn_sas_options_experimental.cpp \
  us_hydrodyn_save.cpp \
  us_hydrodyn_saxs.cpp \
  us_hydrodyn_saxs_conc.cpp \
  us_hydrodyn_saxs_external.cpp \
  us_hydrodyn_saxs_guinier.cpp \
  us_hydrodyn_saxs_fits.cpp \
  us_hydrodyn_saxs_iqq.cpp \
  us_hydrodyn_saxs_iqq_bead_model.cpp \
  us_hydrodyn_saxs_iqq_load_csv.cpp \
  us_hydrodyn_saxs_iqq_residuals.cpp \
  us_hydrodyn_saxs_iqq_sh.cpp \
  us_hydrodyn_saxs_load_csv.cpp \
  us_hydrodyn_saxs_loads.cpp \
  us_hydrodyn_saxs_legend.cpp \
  us_hydrodyn_saxs_mw.cpp \
  us_hydrodyn_saxs_options.cpp \
  us_hydrodyn_saxs_plots.cpp \
  us_hydrodyn_saxs_pr.cpp \
  us_hydrodyn_saxs_residuals.cpp \
  us_hydrodyn_saxs_screen.cpp \
  us_hydrodyn_saxs_search.cpp \
  us_hydrodyn_saxs_buffer.cpp \
  us_hydrodyn_saxs_buffer_conc.cpp \
  us_hydrodyn_saxs_buffer_conc_load.cpp \
  us_hydrodyn_saxs_hplc.cpp \
  us_hydrodyn_saxs_hplc_stack.cpp \
  us_hydrodyn_saxs_hplc_makeiq.cpp \
  us_hydrodyn_saxs_hplc_plots.cpp \
  us_hydrodyn_saxs_hplc_ciq.cpp \
  us_hydrodyn_saxs_hplc_dctr.cpp \
  us_hydrodyn_saxs_hplc_gg.cpp \
  us_hydrodyn_saxs_hplc_p3d.cpp \
  us_hydrodyn_saxs_hplc_util.cpp \
  us_hydrodyn_saxs_hplc_movie.cpp \
  us_hydrodyn_saxs_hplc_conc.cpp \
  us_hydrodyn_saxs_hplc_conc_load.cpp \
  us_hydrodyn_saxs_hplc_fit.cpp \
  us_hydrodyn_saxs_hplc_fit_global.cpp \
  us_hydrodyn_saxs_hplc_fit_gsm.cpp \
  us_hydrodyn_saxs_hplc_fit_ga.cpp \
  us_hydrodyn_saxs_hplc_nth.cpp \
  us_hydrodyn_saxs_hplc_options.cpp \
  us_hydrodyn_saxs_hplc_svd.cpp \
  us_hydrodyn_saxs_1d.cpp \
  us_hydrodyn_saxs_2d.cpp \
  us_hydrodyn_supc.cpp \
  us_hydrodyn_xsr.cpp \
  us_hydrodyn_zeno.cpp \
#  us_imgviewer.cpp \
#  us_infopanel.cpp \
  us_json.cpp \
  us_mqt.cpp \
  us_license.cpp \
  us_lm.cpp \
#  us_lncr2.cpp \
  us_math.cpp \
  us_matrix.cpp \
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
  us_pdb_util.cpp \
  us_pm.cpp \
  us_pm_best.cpp \
  us_pm_best_sphere.cpp \
  us_pm_best_cylinder.cpp \
  us_pm_best_spheroid.cpp \
  us_pm_best_ellipsoid.cpp \
  us_pm_best_torus.cpp \
  us_pm_best_torus_segment.cpp \
  us_pm_fitness.cpp \
  us_pm_ga.cpp \
  us_pm_objects.cpp \
  us_pm_test.cpp \
#  us_pixmap.cpp \
#  us_plotdata.cpp \
#  us_printfilter.cpp \
#  us_pseudo3d_combine.cpp \
#  us_pseudo3d_thread.cpp \
#  us_pseudoabs.cpp \
#  us_radial_correction.cpp \
  us_register.cpp \
#  us_reorder.cpp \
#  us_resplot.cpp \
#  us_rotor_stretch.cpp \
#  us_rundetails.cpp \
#  us_sa2d.cpp \
#  us_sa2d_control.cpp \
#  us_sa2dbase.cpp \
#  us_sassoc.cpp \
  us_saxs_gp.cpp \
  us_saxs_util.cpp \
  us_saxs_util_asab1.cpp \
  us_saxs_util_a2sb.cpp \
  us_saxs_util_best.cpp \
  us_saxs_util_c2check.cpp \
  us_saxs_util_crysol.cpp \
  us_saxs_util_cuda.cpp \
  us_saxs_util_dammin.cpp \
  us_saxs_util_dmd.cpp \
  us_saxs_util_extern.cpp \
  us_saxs_util_guinier.cpp \
  us_saxs_util_hydrate.cpp \
  us_saxs_util_hydrate_align.cpp \
  us_saxs_util_ift.cpp \
  us_saxs_util_iqq.cpp \
  us_saxs_util_iqq_1d.cpp \
  us_saxs_util_iqq_pdb.cpp \
  us_saxs_util_iqq_bead_model.cpp \
  us_saxs_util_loads.cpp \
  us_saxs_util_nsa.cpp \
  us_saxs_util_nsa_ga.cpp \
  us_saxs_util_nsa_gsm.cpp \
  us_saxs_util_nsa_sga.cpp \
  us_saxs_util_pat.cpp \
  us_saxs_util_pm.cpp \
  us_saxs_util_sgp.cpp \
  us_saxs_util_sgp_phys.cpp \
  us_saxs_util_static.cpp \
#  us_select_channel.cpp \
#  us_selectmodel.cpp \
#  us_selectmodel10.cpp \
#  us_selectmodel13.cpp \
#  us_selectmodel3.cpp \
#  us_selectplot.cpp \
  us_sh.cpp \
#  us_showdetails.cpp \
#  us_simulationparameters.cpp \
#  us_smdat.cpp \
#  us_spectrum.cpp \
#  us_stiffbase.cpp \
  us_surfracer.cpp \
  us_svd.cpp \
  us_tar.cpp \
  us_timer.cpp \
#  us_thread.cpp \
  us_util.cpp \
#  us_vbar.cpp \
  us_vector.cpp \
#  us_velocreport.cpp \
#  us_velocmodelctrl.cpp \
#  us_vhwdat.cpp \
#  us_vhwsim.cpp \
#  us_vhwenhanced.cpp \
#  us_viewmwl.cpp \
  us_vvv.cpp \
#  us_widgets.cpp \
#  us_win.cpp \
  us_write_config.cpp \
  qwt/scrollbar.cpp \
  qwt/scrollzoomer.cpp \
  us_gui_settings.cpp \
  shd_mpi/shs_use.cpp
#  us_db_rst_channel.cpp \
#  us_db_rst_equil.cpp \
#  us_db_rst_veloc.cpp \
#  us_db_rst_cell.cpp \
#  us_db_laser.cpp \
#  us_globallaser.cpp \
#  us_kirkwood.cpp \
#  us_laser.cpp \

HEADERS += \
  3dplot/mesh2mainwindowbase.h \
  3dplot/mesh2mainwindow.h \
  3dplot/functions.h \
  3dplot/colormapreader.h \
  3dplot/lightingdlgbase.h \
  3dplot/lightingdlg.h \
#  femreader.h \
#  D_calc.h \
#?  bluearrow.xpm \
#?  greenarrow.xpm \
#?  redarrow.xpm \
  us.h \
#  us_2dplot.h \
#  us_3dsolutes.h \
  us_admin.h \
#  us_archive.h \
#  us_astfem_math.h \
#  us_astfem_rsa.h \
#  us_astfem_sim.h \
#  us_average.h \
#  us_buffer.h \
#  us_calendar.h \
#  us_clipdata.h \
  us_cmdline_app.h \
#  us_cofdistro.h \
#  us_cofs.h \
  us_color.h \
#  us_colorgradient.h \
#  us_combine.h \
  us_config_gui.h \
#  us_constraint_control.h \
#  us_create_global.h \
  us_csv.h \
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
  us_dirhist.h \
#  us_dma60.h \
#  us_dtactrl.h \
#  us_edeabs.h \
#  us_edeflo.h \
#  us_edeint.h \
#  us_edit.h \
#  us_editdb.h \
  us_editor.h \
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
  us_extern.h \
#  us_extinctfitter.h \
#  us_extinction.h \
#  us_fefit.h \
#  us_fematch.h \
#  us_fematch_ra.h \
#  us_femglobal.h \
  us_file_util.h \
#  us_finite1.h \
#  us_finite2.h \
  us_font.h \
#  us_ga_initialize.h \
#  us_ga_model_editor.h \
#  us_global.h \
#  us_globalequil.h \
  us_gzip.h \
#  us_htmledit.h \
#  us_hydro.h \
  us_hydrodyn.h \
  us_hydrodyn_addatom.h \
  us_hydrodyn_addhybrid.h \
  us_hydrodyn_addresidue.h \
  us_hydrodyn_addsaxs.h \
  us_hydrodyn_advanced_config.h \
  us_hydrodyn_anaflex_options.h \
  us_hydrodyn_asa.h \
  us_hydrodyn_asab1.h \
  us_hydrodyn_batch.h \
  us_hydrodyn_batch_movie_opts.h \
  us_hydrodyn_bd.h \
  us_hydrodyn_bd_options.h \
  us_hydrodyn_bd_load_results_opts.h \
  us_hydrodyn_bead_output.h \
  us_hydrodyn_best.h \
  us_hydrodyn_cluster.h \
  us_hydrodyn_cluster_additional.h \
  us_hydrodyn_cluster_advanced.h \
  us_hydrodyn_cluster_bfnb.h \
  us_hydrodyn_cluster_best.h \
  us_hydrodyn_cluster_bfnb_nsa.h \
  us_hydrodyn_cluster_config.h \
  us_hydrodyn_cluster_config_server.h \
  us_hydrodyn_cluster_dammin.h \
  us_hydrodyn_cluster_dmd.h \
  us_hydrodyn_cluster_oned.h \
  us_hydrodyn_cluster_results.h \
  us_hydrodyn_cluster_submit.h \
  us_hydrodyn_cluster_status.h \
  us_hydrodyn_comparative.h \
  us_hydrodyn_csv_viewer.h \
  us_hydrodyn_dammin_opts.h \
  us_hydrodyn_dmd_options.h \
  us_hydrodyn_file.h \
  us_hydrodyn_file2.h \
  us_hydrodyn_grid.h \
  us_hydrodyn_grid_atob.h \
  us_hydrodyn_hydro.h \
  us_hydrodyn_hydro_zeno.h \
  us_hydrodyn_misc.h \
  us_hydrodyn_overlap.h \
  us_hydrodyn_overlap_reduction.h \
  us_hydrodyn_pdbdefs.h \
  us_hydrodyn_pat.h \
  us_hydrodyn_pdb_parsing.h \
  us_hydrodyn_pdb_tool.h \
  us_hydrodyn_pdb_tool_merge.h \
  us_hydrodyn_pdb_tool_renum.h \
  us_hydrodyn_pdb_tool_selres.h \
  us_hydrodyn_pdb_tool_sort.h \
  us_hydrodyn_pdb_visualization.h \
  us_hydrodyn_results.h \
  us_hydrodyn_sas_options_saxs.h \
  us_hydrodyn_sas_options_sans.h \
  us_hydrodyn_sas_options_curve.h \
  us_hydrodyn_sas_options_bead_model.h \
  us_hydrodyn_sas_options_hydration.h \
  us_hydrodyn_sas_options_guinier.h \
  us_hydrodyn_sas_options_xsr.h \
  us_hydrodyn_sas_options_misc.h \
  us_hydrodyn_sas_options_experimental.h \
  us_hydrodyn_save.h \
  us_hydrodyn_saxs.h \
  us_hydrodyn_saxs_conc.h \
  us_hydrodyn_saxs_load_csv.h \
  us_hydrodyn_saxs_mw.h \
  us_hydrodyn_saxs_options.h \
  us_hydrodyn_saxs_iqq_load_csv.h \
  us_hydrodyn_saxs_iqq_residuals.h \
  us_hydrodyn_saxs_residuals.h \
  us_hydrodyn_saxs_screen.h \
  us_hydrodyn_saxs_search.h \
  us_hydrodyn_saxs_buffer.h \
  us_hydrodyn_saxs_buffer_conc.h \
  us_hydrodyn_saxs_buffer_conc_load.h \
  us_hydrodyn_saxs_hplc.h \
  us_hydrodyn_saxs_hplc_ciq.h \
  us_hydrodyn_saxs_hplc_dctr.h \
  us_hydrodyn_saxs_hplc_p3d.h \
  us_hydrodyn_saxs_hplc_conc.h \
  us_hydrodyn_saxs_hplc_conc_load.h \
  us_hydrodyn_saxs_hplc_fit.h \
  us_hydrodyn_saxs_hplc_fit_global.h \
  us_hydrodyn_saxs_hplc_movie.h \
  us_hydrodyn_saxs_hplc_nth.h \
  us_hydrodyn_saxs_hplc_options.h \
  us_hydrodyn_saxs_hplc_svd.h \
  us_hydrodyn_saxs_1d.h \
  us_hydrodyn_saxs_2d.h \
  us_hydrodyn_supc.h \
  us_hydrodyn_xsr.h \
  us_hydrodyn_zeno.h \
  us_fable.h \
#  us_imgviewer.h \
#  us_infopanel.h \
  us_json.h \
  us_mqt.h \
  us_license.h \
  us_lm.h \
#  us_lncr2.h \
  us_math.h \
  us_matrix.h \
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
  us_pdb_util.h \
  us_pm.h \
#  us_pixmap.h \
#  us_plotdata.h \
#  us_printfilter.h \
#  us_pseudo3d_combine.h \
#  us_pseudo3d_thread.h \
#  us_pseudoabs.h \
#  us_radial_correction.h \
  us_register.h \
#  us_reorder.h \
#  us_resplot.h \
#  us_rotor_stretch.h \
  us_revision.h \
#  us_rundetails.h \
#  us_sa2d.h \
#  us_sa2d_control.h \
#  us_sa2dbase.h \
#  us_sassoc.h \
  us_saxs_gp.h \
  us_saxs_util.h \
  us_saxs_util_nsa.h \
#  us_select_channel.h \
#  us_selectmodel.h \
#  us_selectmodel10.h \
#  us_selectmodel13.h \
#  us_selectmodel3.h \
#  us_selectplot.h \
  us_sh.h \
#  us_showdetails.h \
#  us_simulationparameters.h \
#  us_smdat.h \
#  us_spectrum.h \
#  us_stiffbase.h \
  us_surfracer.h \
  us_svd.h \
  us_tar.h \
  us_timer.h \
  us_thread.h \
  us_tnt_jama.h \
  us_util.h \
#  us_vbar.h \
  us_vector.h \
#  us_velocreport.h \
#  us_velocmodelctrl.h \
  us_version.h \
#  us_vhwdat.h \
#  us_vhwsim.h \
#  us_vhwenhanced.h \
#  us_viewmwl.h \
  us_vvv.h \
#  us_widgets.h \
#  us_win.h \
  us_write_config.h \
  us3_extern.h \
  us3_defines.h \
  us_gui_settings.h \
  qwt/scrollbar.h \
  qwt/scrollzoomer.h \
  shd_mpi/shs_data.h

IMAGES = \
  include/editcopy.xpm \
  include/editcut.xpm \
  include/editpaste.xpm \
  include/editredo.xpm \
  include/editundo.xpm \
  include/filenew.xpm \
  include/fileopen.xpm \
  include/fileprint.xpm \
  include/filesave.xpm \
  include/textbold.xpm \
  include/textcenter.xpm \
  include/textitalic.xpm \
  include/textjustify.xpm \
  include/textleft.xpm \
  include/textright.xpm \
  include/textunder.xpm

#The following line was inserted by qt3to4
QT += qt3support 
#The following line was inserted by qt3to4
QT +=  opengl 
