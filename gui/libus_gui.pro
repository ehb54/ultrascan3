include ( ../library.pri )

QT            += opengl
greaterThan( QT_VERSION, 4.99 ) {
QT            += printsupport
}

unix:   TARGET = us_gui

win32 {
        DEFINES += QWT_DLL US_MAKE_GUI_DLL
        TARGET   = us_gui
        LIBS    += -L../lib -lus_utils$${VER} -lqwtplot3d
        LIBS    += -L$${QWTPATH}/lib -lqwt
        QMAKE_LFLAGS += -shared 
        # We assume QMAKE_LFLAGS += Wl,--out-implib,../lib/lib$${TARGET}$${VER}.a
}

macx {
        LIBS   += -L../lib/ -lus_utils $${X11LIB}
        INCLUDES += -I/opt/X11/include
}

QT          += network svg

TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = \
               us_abstractrotor_gui.h   \
               us_analysis_base2.h      \
               us_analyte_gui.h         \
               us_associations_gui.h    \
               us_buffer_gui.h          \
               us_choice.h              \
               us_colorgradIO.h         \
               us_combined_plots_parms_gui.h \
               us_csv_loader.h \
               us_data_loader.h         \
               us_edit_spectrum.h       \
               us_editor.h              \
               us_editor_gui.h          \
               us_extinction_gui.h      \
               us_extinctfitter_gui.h   \
               us_failed_gmp_run_gui.h  \
               us_gui_settings.h        \
               us_gui_util.h            \
               us_help.h                \
               us_images.h              \
               us_investigator.h        \
               us_license.h             \
               us_load_auc.h            \
               us_loadable_noise.h      \
               us_minimize.h            \
               us_model_gui.h           \
               us_model_loader.h        \
               us_mwl_data.h            \
               us_new_spectrum.h        \
               us_noise_loader.h        \
               us_passwd.h              \
               us_plot.h                \
               us_plot3d.h              \
               us_predict1.h            \
               us_project_gui.h         \
               us_properties.h          \
               us_resids_bitmap.h       \
               us_report_gui.h          \
               us_report_general_gui.h  \
               us_rotor_gui.h           \
               us_run_details2.h        \
               us_sassoc.h              \
               us_scan_excl_gui.h       \
               us_select_edits.h        \
               us_select_item.h         \
               us_select_runs.h         \
               us_sim_params_gui.h      \
               us_solution_gui.h        \
               us_spectrodata.h         \
               us_table.h               \
               us_tmst_plot.h           \
               us_widgets.h             \
               us_widgets_dialog.h      \
               us_window_message.h

SOURCES      = \
               us_abstractrotor_gui.cpp   \
               us_analysis_base2.cpp      \
               us_analyte_gui.cpp         \
               us_associations_gui.cpp    \
               us_buffer_gui.cpp          \
               us_choice.cpp              \
               us_colorgradIO.cpp         \
               us_combined_plots_parms_gui.cpp \
               us_csv_loader.cpp \
               us_data_loader.cpp         \
               us_edit_spectrum.cpp       \
               us_editor.cpp              \
               us_editor_gui.cpp          \
               us_extinction_gui.cpp      \
               us_extinctfitter_gui.cpp   \
               us_failed_gmp_run_gui.cpp  \
               us_gui_settings.cpp        \ 
               us_gui_util.cpp            \ 
               us_help.cpp                \
               us_images.cpp              \
               us_investigator.cpp        \
               us_license.cpp             \
               us_load_auc.cpp            \
               us_loadable_noise.cpp      \
               us_minimize.cpp            \
               us_model_gui.cpp           \
               us_model_loader.cpp        \
               us_mwl_data.cpp            \
               us_new_spectrum.cpp        \
               us_noise_loader.cpp        \
               us_passwd.cpp              \
               us_plot.cpp                \
               us_plot3d.cpp              \
               us_predict1.cpp            \
               us_project_gui.cpp         \
               us_properties.cpp          \
               us_resids_bitmap.cpp       \
               us_report_gui.cpp          \
               us_report_general_gui.cpp  \
               us_rotor_gui.cpp           \
               us_run_details2.cpp        \
               us_sassoc.cpp              \
               us_scan_excl_gui.cpp       \
               us_select_item.cpp         \
               us_select_edits.cpp        \
               us_select_runs.cpp         \
               us_sim_params_gui.cpp      \
               us_solution_gui.cpp        \
               us_spectrodata.cpp         \
               us_table.cpp               \
               us_tmst_plot.cpp           \
               us_widgets.cpp             \
               us_widgets_dialog.cpp      \
               us_window_message.cpp

RESOURCES     = images.qrc

unix  {
        HEADERS += us_x11_utils.h
        SOURCES += us_x11_utils.c
}
mac   {
        HEADERS += us_mac_utils.h
        SOURCES += us_mac_utils.c
}
win32 {
        HEADERS += us_win_utils.h
        SOURCES += us_win_utils.c
}

