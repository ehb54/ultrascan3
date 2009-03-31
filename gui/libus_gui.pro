include ( ../library.pri )

unix:   TARGET = us_gui

win32 {
        TARGET = libus_gui
        LIBS  += ../lib/libus_utils.lib ../lib/libus_db.lib
}

QT          += network svg

TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = us_buffer.h              \
               us_editor.h              \
               us_investigator.h        \
               us_gui_settings.h        \
               us_help.h                \
               us_license.h             \
               us_model_editor.h        \
               us_model_selection.h     \
               us_passwd.h              \
               us_plot.h                \
               us_predict1.h            \
               us_sassoc.h              \
               us_selectmodel.h         \
               us_selectmodel3.h        \
               us_selectmodel10.h       \
               us_selectmodel13.h       \
               us_vbar.h                \
               us_widgets.h             \
               us_widgets_dialog.h

SOURCES      = us_buffer.cpp              \
               us_editor.cpp              \
               us_investigator.cpp        \
               us_gui_settings.cpp        \ 
               us_help.cpp                \
               us_license.cpp             \
               us_model_editor.cpp        \
               us_model_selection.cpp     \
               us_passwd.cpp              \
               us_plot.cpp                \
               us_predict1.cpp            \
               us_sassoc.cpp              \
               us_selectmodel.cpp         \
               us_selectmodel3.cpp        \
               us_selectmodel10.cpp       \
               us_selectmodel13.cpp       \
               us_vbar.cpp                \
               us_widgets.cpp             \
               us_widgets_dialog.cpp

