include ( ../library.pri )

TARGET       = us_gui
TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = us_crypto.h       \
               us_gui_settings.h \
               us_help.h         \
               us_license.h      \
               us_passwd.h       \
               us_widgets.h

SOURCES      = us_crypto.cpp       \
               us_gui_settings.cpp \ 
               us_help.cpp         \
               us_license.cpp      \
               us_passwd.cpp       \
               us_widgets.cpp

