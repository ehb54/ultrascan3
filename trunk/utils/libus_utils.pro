include ( ../library.pri )

QT          -= gui
QT          += network
unix: TARGET = us_utils
win32:TARGET = libus_utils
TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = us_crypto.h    \
               us_global.h    \
               us_http_post.h \
               us_license_t.h \
               us_settings.h

SOURCES      = us_crypto.cpp    \
               us_global.cpp    \
               us_http_post.cpp \
               us_license_t.cpp \
               us_settings.cpp

