# Messages
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

revision.target           = include/us_revision.h
revision.commands         = sh revision.sh
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

INCLUDEPATH = $(QWTDIR)/include $(QTDIR)/include
DEPENDPATH += src include
DEFINES += NO_DB
# temporary fix (us2 code was using qt2 qpdevmnt which I think need to be replaced by qprintdevicemetrics)
DEFINES += NO_EDITOR_PRINT
DEFINES += QT4

unix {
  TARGET                  = us_somo
  QMAKE_CXXFLAGS_WARN_ON += -Wno-non-virtual-dtor
  DEFINES                += UNIX
  #CONFIG                 += qt thread warn release 
  CONFIG                 += qt thread warn debug

  INCLUDEPATH    +=  $(QWTDIR)/src

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
  message ("Configuring for the Microsoft Windows Platform...")
  TEMPLATE             = vclib
  TARGET               = libus_somo
  #CONFIG              += qt thread warn exceptions dll release
  CONFIG              += qt thread warn exceptions dll debug
  QMAKE_CXXFLAGS      += /EHsc          # Assume extern C functions never throw exceptions
  QMAKE_CXXFLAGS      += /Fd$(IntDir)\  # Put intermediate directories in separate location
  QMAKE_LFLAGS_DEBUG  += /NODEFAULTLIB:"msvcrt.lib"
  QMAKE_LFLAGS_RELEASE = 
  DEFINES             += QT_DLL -GX QWT_USE_DLL US_MAKE_DLL
  LIBS                += $(QWTDIR)/lib/qwt.lib
  LIBS                += opengl32.lib glu32.lib glaux.lib
  DESTDIR              = ../bin
}

# Do not remake cpp and h files from ui files
#FORMS = 3dplot/mesh2mainwindowbase.ui 3dplot/lightingdlgbase.ui

unix: { 

SOURCES += \
  us_admin.cpp \
  us_color.cpp \
  us_config.cpp \
  us_config_gui.cpp \
#  us_database.cpp \
#  us_db.cpp \
#  us_db_admin.cpp \
#  us_db_t.cpp \
#  us_db_tbl_investigator.cpp \
#  us_db_tbl_vbar.cpp \
  us_gzip.cpp \
  us_editor.cpp \
  us_encryption.cpp \
  us_file_util.cpp \
  us_font.cpp \
  us_global.cpp \
  us_help.cpp \
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
  us_hydrodyn_bd.cpp \
  us_hydrodyn_bd_core.cpp \
  us_hydrodyn_bd_load_results_opts.cpp \
  us_hydrodyn_bd_options.cpp \
  us_hydrodyn_bead_output.cpp \
  us_hydrodyn_cluster.cpp \
  us_hydrodyn_cluster_advanced.cpp \
  us_hydrodyn_cluster_config.cpp \
  us_hydrodyn_cluster_config_server.cpp \
  us_hydrodyn_cluster_dmd.cpp \
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
  us_hydrodyn_pdb_visualization.cpp \
  us_hydrodyn_results.cpp \
  us_hydrodyn_sas_options_saxs.cpp \
  us_hydrodyn_sas_options_sans.cpp \
  us_hydrodyn_sas_options_guinier.cpp \
  us_hydrodyn_sas_options_hydration.cpp \
  us_hydrodyn_sas_options_misc.cpp \
  us_hydrodyn_sas_options_bead_model.cpp \
  us_hydrodyn_sas_options_experimental.cpp \
  us_hydrodyn_sas_options_curve.cpp \
  us_hydrodyn_save.cpp \
  us_hydrodyn_saxs.cpp \
  us_hydrodyn_saxs_external.cpp \
  us_hydrodyn_saxs_fits.cpp \
  us_hydrodyn_saxs_iqq.cpp \
  us_hydrodyn_saxs_iqq_bead_model.cpp \
  us_hydrodyn_saxs_iqq_load_csv.cpp \
  us_hydrodyn_saxs_iqq_residuals.cpp \
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
  us_hydrodyn_supc.cpp \
  us_hydrodyn_zeno.cpp \
  us_surfracer.cpp \
#  us_imgviewer.cpp \
  us_license.cpp \
  us_math.cpp \
  us_matrix.cpp \
  us_register.cpp \
  us_saxs_gp.cpp \
  us_saxs_util.cpp \
  us_saxs_util_a2sb.cpp \
  us_saxs_util_cuda.cpp \
  us_saxs_util_dmd.cpp \
  us_saxs_util_extern.cpp \
  us_saxs_util_hydrate.cpp \
  us_saxs_util_hydrate_align.cpp \
  us_saxs_util_ift.cpp \
  us_saxs_util_iqq.cpp \
  us_saxs_util_iqq_pdb.cpp \
  us_saxs_util_iqq_bead_model.cpp \
  us_saxs_util_loads.cpp \
  us_saxs_util_nsa.cpp \
  us_saxs_util_nsa_ga.cpp \
  us_saxs_util_nsa_gsm.cpp \
  us_saxs_util_nsa_sga.cpp \
  us_saxs_util_sgp.cpp \
  us_saxs_util_sgp_phys.cpp \
  us_tar.cpp \
  us_timer.cpp \
  us_thread.cpp \
  us_util.cpp \
#  us_vbar.cpp \
  us_write_config.cpp \
  us_gui_settings.cpp \
  src/qwt/scrollbar.cpp \
  src/qwt/scrollzoomer.cpp

HEADERS += \
  us_admin.h \
  us_color.h \
  us_config_gui.h \
#  us_database.h \
#  us_db.h \
#  us_db_admin.h \
#  us_db_t.h \
#  us_db_tbl_investigator.h \
#  us_db_tbl_vbar.h \
  us_editor.h \
  us_encryption.h \
  us_extern.h \
  us_file_util.h \
  us_font.h \
  us_global.h \
  us_gzip.h \
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
  us_hydrodyn_cluster.h \
  us_hydrodyn_cluster_advanced.h \
  us_hydrodyn_cluster_config.h \
  us_hydrodyn_cluster_config_server.h \
  us_hydrodyn_cluster_dmd.h \
  us_hydrodyn_cluster_results.h \
  us_hydrodyn_cluster_submit.h \
  us_hydrodyn_cluster_status.h \
  us_hydrodyn_comparative.h \
  us_hydrodyn_csv_viewer.h \
  us_hydrodyn_dammin_opts.h \
  us_hydrodyn_dmd_options.h \
  us_hydrodyn_file.h \
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
  us_hydrodyn_pdb_visualization.h \
  us_hydrodyn_results.h \
  us_hydrodyn_sas_options_saxs.h \
  us_hydrodyn_sas_options_sans.h \
  us_hydrodyn_sas_options_guinier.h \
  us_hydrodyn_sas_options_hydration.h \
  us_hydrodyn_sas_options_misc.h \
  us_hydrodyn_sas_options_bead_model.h \
  us_hydrodyn_sas_options_experimental.h \
  us_hydrodyn_sas_options_curve.h \
  us_hydrodyn_save.h \
  us_hydrodyn_saxs.h \
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
  us_hydrodyn_supc.h \
  us_hydrodyn_zeno.h \
  us_fable.h \
  us_saxs_gp.h \
  us_saxs_util.h \
  us_saxs_util_nsa.h \
  us_surfracer.h \
#  us_imgviewer.h \
  us_license.h \
  us_math.h \
  us_matrix.h \
  us_register.h \
  us_tar.h \
  us_thread.h \
  us_timer.h \
  us_tnt_jama.h \
  us_util.h \
  us_version.h \
#  us_vbar.h \
  us_write_config.h \
  us_gui_settings.h \
  us3_extern.h \
  us3_defines.h \
  qwt/scrollbar.h \
  qwt/scrollzoomer.h

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
}

#The following line was inserted by qt3to4
QT += qt3support 
