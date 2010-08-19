include( ../../gui.pri )

QT           += svg
TARGET        = us_config
TRANSLATIONS += $${TARGET}_de_DE.ts

HEADERS      += us_config.h   \
                us_admin.h    \
                us_color.h    \
                us_database.h \
                us_advanced.h \
                us_font.h

SOURCES      += us_config.cpp   \
                us_admin.cpp    \
                us_color.cpp    \
                us_database.cpp \
                us_advanced.cpp \
                us_font.cpp     \
                main1.inc
