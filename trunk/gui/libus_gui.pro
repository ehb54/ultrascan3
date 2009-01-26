include ( ../library.pri )

unix:   TARGET = us_gui

win32 {
        TARGET = libus_gui
        LIBS  += ../lib/libus_utils.lib ../lib/libus_db.lib
}

QT          += network 

TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = us_gui_settings.h \
               us_help.h         \
               us_license.h      \
               us_passwd.h       \
               us_widgets.h

SOURCES      = us_gui_settings.cpp \ 
               us_help.cpp         \
               us_license.cpp      \
               us_passwd.cpp       \
               us_widgets.cpp

