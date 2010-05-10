include ( ../library.pri )

QT          -= gui
QT          += network
unix: TARGET = us_utils

macx {
      TARGET = us_utils
      LIBS  += -L../lib -lus_gui -lus_db
}

win32:TARGET = libus_utils
TRANSLATIONS = $${TARGET}_DE_de.ts


# Consolidate us_femglobal and us_femglobal_new when development is 
# progressed to a point where us_femglobal can be removed 

HEADERS      = us_analyte.h     \
               us_astfem_math.h \
               us_astfem_rsa.h  \
               us_buffer.h      \
               us_constants.h   \
               us_crc.h         \
               us_crypto.h      \
               us_dataIO.h      \
               us_global.h      \
               us_femglobal.h   \
               us_femglobal_new.h   \
               us_hardware.h    \ 
               us_http_post.h   \
               us_license_t.h   \
               us_math.h        \
               us_matrix.h      \
               us_settings.h    \
               us_stiffbase.h   \
               us_util.h

SOURCES      = us_analyte.cpp     \
               us_astfem_math.cpp \
               us_astfem_rsa.cpp  \
               us_buffer.cpp      \
               us_constants.cpp   \
               us_crc.cpp         \
               us_crypto.cpp      \
               us_dataIO.cpp      \
               us_femglobal_new.cpp   \
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

