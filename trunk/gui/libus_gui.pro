include ( ../library.pri )

unix:   TARGET = us_gui

win32 {
        DEFINES += QWT_DLL US_MAKE_GUI_DLL
        TARGET   = libus_gui
        LIBS    += ../lib/libus_utils.lib 
}

macx {
        LIBS   += -L../lib/ -lus_utils 
}

QT          += network svg

TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = \
               us_analysis_base2.h      \
               us_analyte_gui.h         \
               us_associations_gui.h    \
               us_buffer_gui.h          \
               us_colorgradIO.h         \
               us_data_loader.h         \
               us_editor.h              \
               us_editor_gui.h          \
               us_gui_settings.h        \
               us_help.h                \
               us_images.h              \
               us_investigator.h        \
               us_license.h             \
               us_loadable_noise.h      \
               us_model_gui.h           \
               us_model_loader.h        \
               us_noise_loader.h        \
               us_passwd.h              \
               us_plot.h                \
               us_plot3d.h              \
               us_predict1.h            \
               us_properties.h          \
               us_resids_bitmap.h       \
               us_run_details2.h        \
               us_sassoc.h              \
               us_table.h               \
               us_widgets.h             \
               us_widgets_dialog.h      \
               us_solution_gui.h        \
               us_project_gui.h         \
               us_abstractrotor_gui.h   \
               us_rotor_gui.h

SOURCES      = \
               us_analysis_base2.cpp      \
               us_analyte_gui.cpp         \
               us_associations_gui.cpp    \
               us_buffer_gui.cpp          \
               us_colorgradIO.cpp         \
               us_data_loader.cpp         \
               us_editor.cpp              \
               us_editor_gui.cpp          \
               us_gui_settings.cpp        \ 
               us_help.cpp                \
               us_images.cpp              \
               us_investigator.cpp        \
               us_license.cpp             \
               us_loadable_noise.cpp      \
               us_model_gui.cpp           \
               us_model_loader.cpp        \
               us_noise_loader.cpp        \
               us_passwd.cpp              \
               us_plot.cpp                \
               us_plot3d.cpp              \
               us_predict1.cpp            \
               us_properties.cpp          \
               us_resids_bitmap.cpp       \
               us_run_details2.cpp        \
               us_sassoc.cpp              \
               us_table.cpp               \
               us_widgets.cpp             \
               us_widgets_dialog.cpp      \
               us_solution_gui.cpp        \
               us_project_gui.cpp         \
               us_abstractrotor_gui.cpp   \
               us_rotor_gui.cpp

RESOURCES     = images.qrc
