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
               us_dataIO2.h     \
               us_global.h      \
               us_hardware.h    \ 
               us_hydrosim.h    \ 
               us_http_post.h   \
               us_lamm_astfvm.h \
               us_license_t.h   \
               us_math2.h       \
               us_matrix.h      \
               us_model.h       \
               us_noise.h       \
               us_settings.h    \
               us_stiffbase.h   \
               us_simparms.h    \
               us_util.h

SOURCES      = us_analyte.cpp     \
               us_astfem_math.cpp \
               us_astfem_rsa.cpp  \
               us_buffer.cpp      \
               us_constants.cpp   \
               us_crc.cpp         \
               us_dataIO2.cpp     \
               us_hardware.cpp    \
               us_hydrosim.cpp    \
               us_global.cpp      \
               us_http_post.cpp   \
               us_lamm_astfvm.cpp \
               us_license_t.cpp   \
               us_math2.cpp       \
               us_model.cpp       \
               us_noise.cpp       \
               us_matrix.cpp      \
               us_settings.cpp    \
               us_simparms.cpp    \
               us_stiffbase.cpp   \
               us_util.cpp

# Only include us_crypto when not on a supercomputer

!contains( DEFINES, NO_DB ) {
   HEADERS      += us_crypto.h     
   SOURCES      += us_crypto.cpp    
}

