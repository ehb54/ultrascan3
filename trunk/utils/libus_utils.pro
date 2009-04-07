include ( ../library.pri )

QT          -= gui
QT          += network
unix: TARGET = us_utils
win32:TARGET = libus_utils
TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = us_astfem_math.h \
               us_astfem_rsa.h  \
               us_constants.h   \
               us_crypto.h      \
               us_global.h      \
               us_femglobal.h   \
               us_hardware.h    \ 
               us_http_post.h   \
               us_license_t.h   \
               us_math.h        \
               us_matrix.h      \
               us_settings.h    \
               us_stiffbase.h   \
               us_util.h

SOURCES      = us_astfem_math.cpp \
               us_astfem_rsa.cpp  \
               us_constants.cpp   \
               us_crypto.cpp      \
               us_femglobal.cpp   \
               us_hardware.cpp    \
               us_global.cpp      \
               us_http_post.cpp   \
               us_license_t.cpp   \
               us_math.cpp        \
               us_matrix.cpp      \
               us_settings.cpp    \
               us_stiffbase.cpp   \
               us_util.cpp

