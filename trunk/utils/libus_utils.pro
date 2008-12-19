include ( ../library.pri )

QT          -= gui
QT          += network
TARGET       = us_utils
TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = us_global.h    \
               us_http_post.h \
               us_license_t.h \
               us_settings.h

SOURCES      = us_global.cpp    \
               us_http_post.cpp \
               us_license_t.cpp \
               us_settings.cpp

