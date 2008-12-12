include( ../../gui.pri )

QT           += sql svg
TARGET        = us_config
TRANSLATIONS += $${TARGET}_de_DE.ts

HEADERS      += us_config.h   \
                us_admin.h    \
                us_color.h    \
                us_database.h \
                us_font.h

SOURCES      += us_config.cpp   \
                us_admin.cpp    \
                us_color.cpp    \
                us_database.cpp \
                us_font.cpp
