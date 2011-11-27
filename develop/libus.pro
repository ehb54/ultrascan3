# Messages
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

revision.target           = include/us_revision.h
revision.commands         = sh revision.sh
revision.depends          = FORCE
QMAKE_EXTRA_UNIX_TARGETS += revision

TEMPLATE       = lib
TRANSLATIONS   = lib.ts
VERSION        = 9.9
MOC_DIR        = src/moc
OBJECTS_DIR    = src/obj

#RC_FILE        = ../icon.rc

# enabled threading in fitting algorithm:
DEFINES += THREAD

# Automatic hardware platform and operating system configurations:

INCLUDEPATH = $(QWTDIR)/include $(QTDIR)/include $(QWT3DDIR)/include 
DEPENDPATH += src include

unix {
  TARGET                  = us
  QMAKE_CXXFLAGS_WARN_ON += -Wno-non-virtual-dtor
  DEFINES                += UNIX
  #CONFIG                 += qt thread warn release 
  CONFIG                 += qt thread warn debug

  contains(UNAME,x86_64) {
    LIBS    += -L$(QWTDIR)/lib64/ -lqwt -L$(QWT3DDIR)/lib64 -lqwtplot3d
    DEFINES += BIN64
    DESTDIR  = ../lib64
  } else {
    LIBS += -L$(QWTDIR)/lib -lqwt -L$(QWT3DDIR)/lib -lqwtplot3d
    DESTDIR  = ../lib
  }
}

win32 {
  message ("Configuring for the Microsoft Windows Platform...")
  TEMPLATE             = lib
  TARGET               = libus
  CONFIG              += qt thread warn exceptions dll release
  #CONFIG              += qt thread warn exceptions dll debug
  QMAKE_CXXFLAGS      += /EHsc          # Assume extern C functions never throw exceptions
  QMAKE_CXXFLAGS      += /Fd$(IntDir)\  # Put intermediate directories in separate location
  QMAKE_LFLAGS_DEBUG  += /NODEFAULTLIB:"msvcrt.lib"
  QMAKE_LFLAGS_RELEASE = /NODEFAULTLIB:"msvcrtd.lib"
  DEFINES             += QT_DLL -GX QWT_USE_DLL US_MAKE_DLL
  LIBS                += $(QWTDIR)/lib/qwt.lib $(QWT3DDIR)/lib/qwtplot3d.lib 
  LIBS                += opengl32.lib glu32.lib glaux.lib
  DESTDIR              = ../bin
}

# Do not remake cpp and h files from ui files
#FORMS = 3dplot/mesh2mainwindowbase.ui 3dplot/lightingdlgbase.ui

unix: { 
SOURCES += \
  src/us_beowulf.cpp \
  src/us_gridcontrol_t.cpp
}

SOURCES += \
  3dplot/mesh2mainwindowbase.cpp \
  3dplot/mesh2mainwindow.cpp \
  3dplot/colormapreader.cpp \
  3dplot/lightingdlgbase.cpp \
  3dplot/lightingdlg.cpp \
  3dplot/mesh.cpp \
  3dplot/D_calc.cpp \
  src/us_2dplot.cpp \
  src/us_3dsolutes.cpp \
  src/us_admin.cpp \
  src/us_archive.cpp \
  src/us_astfem_math.cpp \
  src/us_astfem_rsa.cpp \
  src/us_astfem_sim.cpp \
  src/us_average.cpp \
  src/us_buffer.cpp \
  src/us_calendar.cpp \ 
  src/us_clipdata.cpp \ 
  src/us_cofdistro.cpp \
  src/us_cofs.cpp \ 
  src/us_color.cpp \
  src/us_colorgradient.cpp \
  src/us_combine.cpp \
  src/us_config.cpp \
  src/us_config_gui.cpp \
  src/us_constraint_control.cpp \
  src/us_create_global.cpp \
  src/us_data_io.cpp \
  src/us_database.cpp \
  src/us_db.cpp \
  src/us_db_admin.cpp \
  src/us_db_veloc.cpp \
  src/us_db_equil.cpp \
  src/us_db_rst_equilproject.cpp \
  src/us_db_rst_montecarlo.cpp \
  src/us_db_rtv_date.cpp \
  src/us_db_rtv_description.cpp \
  src/us_db_rtv_edittype.cpp \
  src/us_db_rtv_image.cpp \
  src/us_db_rtv_investigator.cpp \
  src/us_db_rtv_request.cpp \
  src/us_db_rtv_requeststatus.cpp \
  src/us_db_rtv_runrequest.cpp \
  src/us_db_rtv_sample.cpp \
  src/us_db_runrequest.cpp \
  src/us_db_t.cpp \
  src/us_db_tbl_buffer.cpp \
  src/us_db_tbl_cell.cpp \
  src/us_db_tbl_expdata.cpp \
  src/us_db_tbl_investigator.cpp \
  src/us_db_tbl_nucleotide.cpp \
  src/us_db_tbl_vbar.cpp \
  src/us_db_template.cpp \
  src/us_db_widgets.cpp \
  src/us_dcdrdat.cpp \
  src/us_dcdtdat.cpp \
  src/us_diagnostics.cpp \
  src/us_dma60.cpp \
  src/us_dtactrl.cpp \
  src/us_edeabs.cpp \
  src/us_edeflo.cpp \
  src/us_edeint.cpp \
  src/us_edit.cpp \
  src/us_editdb.cpp \
  src/us_editor.cpp \
  src/us_editwavelength.cpp \
  src/us_edscan.cpp \
  src/us_edvabs.cpp \
  src/us_edvflo.cpp \
  src/us_edvint.cpp \
  src/us_encryption.cpp \
  src/us_enter_dna.cpp \
  src/us_eqfitter.cpp \
  src/us_eqmodelctrl.cpp \
  src/us_equilprojectreport.cpp \
  src/us_equilreport.cpp \
  src/us_equilsim.cpp \
  src/us_equilspeed.cpp \
  src/us_equiltime.cpp \
  src/us_exclude_profile.cpp \
  src/us_export.cpp \
  src/us_extinctfitter.cpp \
  src/us_extinction.cpp \
  src/us_fefit.cpp \
  src/us_fematch.cpp \
  src/us_fematch_ra.cpp \
  src/us_femglobal.cpp \
  src/us_file_util.cpp \
  src/us_finite1.cpp \
  src/us_finite2.cpp \
  src/us_font.cpp \
  src/us_ga_initialize.cpp \
  src/us_ga_model_editor.cpp \
  src/us_global.cpp \
  src/us_globalequil.cpp \
  src/us_help.cpp \
  src/us_gzip.cpp \
  src/us_htmledit.cpp \
  src/us_hydro.cpp \
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
  src/us_hydrodyn_bd.cpp \
  src/us_hydrodyn_bd_core.cpp \
  src/us_hydrodyn_bd_options.cpp \
  src/us_hydrodyn_bd_load_results_opts.cpp \
  src/us_hydrodyn_cluster.cpp \
  src/us_hydrodyn_cluster_advanced.cpp \
  src/us_hydrodyn_cluster_config.cpp \
  src/us_hydrodyn_cluster_config_server.cpp \
  src/us_hydrodyn_cluster_dmd.cpp \
  src/us_hydrodyn_cluster_results.cpp \
  src/us_hydrodyn_cluster_submit.cpp \
  src/us_hydrodyn_cluster_status.cpp \
  src/us_hydrodyn_comparative.cpp \
  src/us_hydrodyn_core.cpp \
  src/us_hydrodyn_csv_viewer.cpp \
  src/us_hydrodyn_dammin_opts.cpp \
  src/us_hydrodyn_dmd_core.cpp \
  src/us_hydrodyn_dmd_options.cpp \
  src/us_hydrodyn_file.cpp \
  src/us_hydrodyn_grid.cpp \
  src/us_hydrodyn_grid_atob.cpp \
  src/us_hydrodyn_hydrate.cpp \
  src/us_hydrodyn_hydrate_align.cpp \
  src/us_hydrodyn_hydro.cpp \
  src/us_hydrodyn_misc.cpp \
  src/us_hydrodyn_other.cpp \
  src/us_hydrodyn_overlap.cpp \
  src/us_hydrodyn_overlap_reduction.cpp \
  src/us_hydrodyn_pat.cpp \
  src/us_hydrodyn_pdb_parsing.cpp \
  src/us_hydrodyn_pdb_tool.cpp \
  src/us_hydrodyn_pdb_tool_merge.cpp \
  src/us_hydrodyn_pdb_visualization.cpp \
  src/us_hydrodyn_results.cpp \
  src/us_hydrodyn_save.cpp \
  src/us_hydrodyn_saxs.cpp \
  src/us_hydrodyn_saxs_external.cpp \
  src/us_hydrodyn_saxs_fits.cpp \
  src/us_hydrodyn_saxs_iqq.cpp \
  src/us_hydrodyn_saxs_iqq_bead_model.cpp \
  src/us_hydrodyn_saxs_iqq_load_csv.cpp \
  src/us_hydrodyn_saxs_iqq_residuals.cpp \
  src/us_hydrodyn_saxs_load_csv.cpp \
  src/us_hydrodyn_saxs_loads.cpp \
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
  src/us_hydrodyn_supc.cpp \
  src/us_imgviewer.cpp \
  src/us_infopanel.cpp \
  src/us_license.cpp \
  src/us_lncr2.cpp \
  src/us_math.cpp \
  src/us_matrix.cpp \
  src/us_meniscus.cpp \
  src/us_meniscus_dlg.cpp \
  src/us_merge.cpp \
  src/us_mfem.cpp \
  src/us_minimize.cpp \
  src/us_model_editor.cpp \
  src/us_modelselection.cpp \
  src/us_montecarlo.cpp \
  src/us_montecarloreport.cpp \
  src/us_montecarlostats.cpp \
  src/us_nucleotide.cpp \
  src/us_pixmap.cpp \
  src/us_plotdata.cpp \
  src/us_printfilter.cpp \
  src/us_pseudo3d_combine.cpp \
  src/us_pseudo3d_thread.cpp \
  src/us_pseudoabs.cpp \
  src/us_radial_correction.cpp \
  src/us_register.cpp \
  src/us_reorder.cpp \
  src/us_resplot.cpp \
  src/us_rotor_stretch.cpp \
  src/us_rundetails.cpp \
  src/us_sa2d.cpp \
  src/us_sa2d_control.cpp \
  src/us_sa2dbase.cpp \
  src/us_sassoc.cpp \
  src/us_saxs_gp.cpp \
  src/us_saxs_util.cpp \
  src/us_saxs_util_a2sb.cpp \
  src/us_saxs_util_dmd.cpp \
  src/us_saxs_util_extern.cpp \
  src/us_saxs_util_hydrate.cpp \
  src/us_saxs_util_hydrate_align.cpp \
  src/us_saxs_util_ift.cpp \
  src/us_saxs_util_iqq.cpp \
  src/us_saxs_util_iqq_pdb.cpp \
  src/us_saxs_util_iqq_bead_model.cpp \
  src/us_saxs_util_loads.cpp \
  src/us_saxs_util_nsa.cpp \
  src/us_saxs_util_nsa_ga.cpp \
  src/us_saxs_util_nsa_gsm.cpp \
  src/us_saxs_util_nsa_sga.cpp \
  src/us_saxs_util_sgp.cpp \
  src/us_saxs_util_sgp_phys.cpp \
  src/us_select_channel.cpp \
  src/us_selectmodel.cpp \
  src/us_selectmodel10.cpp \
  src/us_selectmodel13.cpp \
  src/us_selectmodel3.cpp \
  src/us_selectplot.cpp \
  src/us_showdetails.cpp \
  src/us_simulationparameters.cpp \
  src/us_smdat.cpp \
  src/us_spectrum.cpp \
  src/us_stiffbase.cpp \
  src/us_surfracer.cpp \
  src/us_tar.cpp \
  src/us_timer.cpp \
  src/us_thread.cpp \
  src/us_util.cpp \
  src/us_vbar.cpp \
  src/us_velocreport.cpp \
  src/us_velocmodelctrl.cpp \
  src/us_vhwdat.cpp \
  src/us_vhwsim.cpp \
  src/us_vhwenhanced.cpp \
  src/us_viewmwl.cpp \
  src/us_widgets.cpp \
  src/us_win.cpp \
  src/us_write_config.cpp \
  src/qwt/scrollbar.cpp \
  src/qwt/scrollzoomer.cpp

#  src/us_db_rst_channel.cpp \
#  src/us_db_rst_equil.cpp \
#  src/us_db_rst_veloc.cpp \
#  src/us_db_rst_cell.cpp \
#  src/us_db_laser.cpp \
#  src/us_globallaser.cpp \
#  src/us_kirkwood.cpp \
#  src/us_laser.cpp \

unix:{ 
  HEADERS += \
  include/us_beowulf.h \
  include/us_gridcontrol_t.h
}
    
HEADERS += \
  3dplot/mesh2mainwindowbase.h \
  3dplot/mesh2mainwindow.h \
  3dplot/functions.h \
  3dplot/colormapreader.h \
  3dplot/lightingdlgbase.h \
  3dplot/lightingdlg.h \
  3dplot/femreader.h \
  3dplot/D_calc.h \
  include/bluearrow.xpm \
  include/greenarrow.xpm \
  include/redarrow.xpm \
  include/us.h \
  include/us_2dplot.h \
  include/us_3dsolutes.h \
  include/us_admin.h \
  include/us_archive.h \
  include/us_astfem_math.h \
  include/us_astfem_rsa.h \
  include/us_astfem_sim.h \
  include/us_average.h \
  include/us_buffer.h \
  include/us_calendar.h \
  include/us_clipdata.h \
  include/us_cofdistro.h \
  include/us_cofs.h \
  include/us_color.h \
  include/us_colorgradient.h \
  include/us_combine.h \
  include/us_config_gui.h \
  include/us_constraint_control.h \
  include/us_create_global.h \
  include/us_data_io.h \
  include/us_database.h \
  include/us_db.h \
  include/us_db_admin.h \
  include/us_db_veloc.h \
  include/us_db_equil.h \
  include/us_db_rst_equilproject.h \
  include/us_db_rst_montecarlo.h \
  include/us_db_rtv_date.h \
  include/us_db_rtv_description.h \
  include/us_db_rtv_edittype.h \
  include/us_db_rtv_image.h \
  include/us_db_rtv_investigator.h \
  include/us_db_rtv_request.h \
  include/us_db_rtv_requeststatus.h \
  include/us_db_rtv_runrequest.h \
  include/us_db_rtv_sample.h \
  include/us_db_runrequest.h \
  include/us_db_t.h \
  include/us_db_tbl_buffer.h \
  include/us_db_tbl_cell.h \
  include/us_db_tbl_expdata.h \
  include/us_db_tbl_investigator.h \
  include/us_db_tbl_nucleotide.h \
  include/us_db_tbl_vbar.h \
  include/us_db_template.h \
  include/us_db_widgets.h \
  include/us_dcdrdat.h \
  include/us_dcdtdat.h \
  include/us_diagnostics.h \
  include/us_dma60.h \
  include/us_dtactrl.h \
  include/us_edeabs.h \
  include/us_edeflo.h \
  include/us_edeint.h \
  include/us_edit.h \
  include/us_editdb.h \
  include/us_editor.h \
  include/us_editwavelength.h \
  include/us_edscan.h \
  include/us_edvabs.h \
  include/us_edvflo.h \
  include/us_edvint.h \
  include/us_encryption.h \
  include/us_enter_dna.h \
  include/us_eqfitter.h \
  include/us_eqmodelctrl.h \
  include/us_equilprojectreport.h \
  include/us_equilreport.h \
  include/us_equilsim.h \
  include/us_equilspeed.h \
  include/us_equiltime.h \
  include/us_exclude_profile.h \
  include/us_export.h \
  include/us_extern.h \
  include/us_extinctfitter.h \
  include/us_extinction.h \
  include/us_fefit.h \
  include/us_fematch.h \
  include/us_fematch_ra.h \
  include/us_femglobal.h \
  include/us_file_util.h \
  include/us_finite1.h \
  include/us_finite2.h \
  include/us_font.h \
  include/us_ga_initialize.h \
  include/us_ga_model_editor.h \
  include/us_global.h \
  include/us_globalequil.h \
  include/us_gzip.h \
  include/us_htmledit.h \
  include/us_hydro.h \
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
  include/us_hydrodyn_cluster.h \
  include/us_hydrodyn_cluster_advanced.h \
  include/us_hydrodyn_cluster_config.h \
  include/us_hydrodyn_cluster_config_server.h \
  include/us_hydrodyn_cluster_dmd.h \
  include/us_hydrodyn_cluster_results.h \
  include/us_hydrodyn_cluster_submit.h \
  include/us_hydrodyn_cluster_status.h \
  include/us_hydrodyn_comparative.h \
  include/us_hydrodyn_csv_viewer.h \
  include/us_hydrodyn_dammin_opts.h \
  include/us_hydrodyn_dmd_options.h \
  include/us_hydrodyn_file.h \
  include/us_hydrodyn_grid.h \
  include/us_hydrodyn_grid_atob.h \
  include/us_hydrodyn_hydro.h \
  include/us_hydrodyn_misc.h \
  include/us_hydrodyn_overlap.h \
  include/us_hydrodyn_overlap_reduction.h \
  include/us_hydrodyn_pdbdefs.h \
  include/us_hydrodyn_pat.h \
  include/us_hydrodyn_pdb_parsing.h \
  include/us_hydrodyn_pdb_tool.h \
  include/us_hydrodyn_pdb_tool_merge.h \
  include/us_hydrodyn_pdb_visualization.h \
  include/us_hydrodyn_results.h \
  include/us_hydrodyn_save.h \
  include/us_hydrodyn_saxs.h \
  include/us_hydrodyn_saxs_load_csv.h \
  include/us_hydrodyn_saxs_mw.h \
  include/us_hydrodyn_saxs_options.h \
  include/us_hydrodyn_saxs_iqq_load_csv.h \
  include/us_hydrodyn_saxs_iqq_residuals.h \
  include/us_hydrodyn_saxs_residuals.h \
  include/us_hydrodyn_saxs_screen.h \
  include/us_hydrodyn_saxs_search.h \
  include/us_hydrodyn_saxs_buffer.h \
  include/us_hydrodyn_saxs_buffer_conc.h \
  include/us_hydrodyn_saxs_buffer_conc_load.h \
  include/us_hydrodyn_supc.h \
  include/us_imgviewer.h \
  include/us_infopanel.h \
  include/us_license.h \
  include/us_lncr2.h \
  include/us_math.h \
  include/us_matrix.h \
  include/us_meniscus.h \
  include/us_meniscus_dlg.h \
  include/us_merge.h \
  include/us_mfem.h \
  include/us_minimize.h \
  include/us_model_editor.h \
  include/us_modelselection.h \
  include/us_montecarlo.h \
  include/us_montecarloreport.h \
  include/us_montecarlostats.h \
  include/us_nucleotide.h \
  include/us_pixmap.h \
  include/us_plotdata.h \
  include/us_printfilter.h \
  include/us_pseudo3d_combine.h \
  include/us_pseudo3d_thread.h \
  include/us_pseudoabs.h \
  include/us_radial_correction.h \
  include/us_register.h \
  include/us_reorder.h \
  include/us_resplot.h \
  include/us_rotor_stretch.h \
  include/us_revision.h \
  include/us_rundetails.h \
  include/us_sa2d.h \
  include/us_sa2d_control.h \
  include/us_sa2dbase.h \
  include/us_sassoc.h \
  include/us_saxs_gp.h \
  include/us_saxs_util.h \
  include/us_saxs_util_nsa.h \
  include/us_select_channel.h \
  include/us_selectmodel.h \
  include/us_selectmodel10.h \
  include/us_selectmodel13.h \
  include/us_selectmodel3.h \
  include/us_selectplot.h \
  include/us_showdetails.h \
  include/us_simulationparameters.h \
  include/us_smdat.h \
  include/us_spectrum.h \
  include/us_stiffbase.h \
  include/us_surfracer.h \
  include/us_tar.h \
  include/us_timer.h \
  include/us_thread.h \
  include/us_tnt_jama.h \
  include/us_util.h \
  include/us_vbar.h \
  include/us_velocreport.h \
  include/us_velocmodelctrl.h \
  include/us_version.h \
  include/us_vhwdat.h \
  include/us_vhwsim.h \
  include/us_vhwenhanced.h \
  include/us_viewmwl.h \
  include/us_widgets.h \
  include/us_win.h \
  include/us_write_config.h \
  include/qwt/scrollbar.h \
  include/qwt/scrollzoomer.h

#  include/us_db_rst_channel.h \
#  include/us_db_rst_equil.h \
#  include/us_db_rst_veloc.h \
#  include/us_db_rst_cell.h \
#  include/us_db_laser.h \
#  include/us_globallaser.h \
#  include/us_kirkwood.h \
#  include/us_laser.h \

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

