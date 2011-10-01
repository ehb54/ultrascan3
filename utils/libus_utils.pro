include ( ../library.pri )

QT          -= gui
QT          += network
unix: TARGET = us_utils

macx {
      TARGET = us_utils
      LIBS  += -L$${MYSQLDIR} -lmysqlclient
      LIBS  += -L../lib
}

win32 {
      TARGET   = us_utils
      DEFINES += US_MAKE_UTIL_DLL
}

TRANSLATIONS = $${TARGET}_DE_de.ts


# Consolidate us_femglobal and us_femglobal_new when development is
# progressed to a point where us_femglobal can be removed

HEADERS      = us_analyte.h       \
               us_astfem_math.h   \
               us_astfem_rsa.h    \
               us_buffer.h        \
               us_constants.h     \
               us_crc.h           \
               us_dataIO2.h       \
               us_datafiles.h     \
               us_db2.h           \
               us_global.h        \
               us_gzip.h          \
               us_hardware.h      \
               us_hydrosim.h      \
               us_http_post.h     \
               us_lamm_astfvm.h   \
               us_license_t.h     \
               us_math2.h         \
               us_matrix.h        \
               us_memory.h        \
               us_model.h         \
               us_noise.h         \
               us_project.h       \
               us_settings.h      \
               us_simparms.h      \
               us_solute.h        \
               us_solution.h      \
               us_solution_vals.h \
               us_solve_sim.h     \
               us_stiffbase.h     \
               us_tar.h           \
               us_util.h          \
               us_vector.h        \
               us_rotor.h

SOURCES      = us_analyte.cpp       \
               us_astfem_math.cpp   \
               us_astfem_rsa.cpp    \
               us_buffer.cpp        \
               us_constants.cpp     \
               us_crc.cpp           \
               us_dataIO2.cpp       \
               us_datafiles.cpp     \
               us_db2.cpp           \
               us_global.cpp        \
               us_gzip.cpp          \
               us_hardware.cpp      \
               us_http_post.cpp     \
               us_hydrosim.cpp      \
               us_lamm_astfvm.cpp   \
               us_license_t.cpp     \
               us_math2.cpp         \
               us_matrix.cpp        \
               us_memory.cpp        \
               us_model.cpp         \
               us_noise.cpp         \
               us_project.cpp       \
               us_settings.cpp      \
               us_simparms.cpp      \
               us_solute.cpp        \
               us_solution.cpp      \
               us_solution_vals.cpp \
               us_solve_sim.cpp     \
               us_stiffbase.cpp     \
               us_tar.cpp           \
               us_util.cpp          \
               us_vector.cpp        \
               us_rotor.cpp

# Only include us_crypto when not on a supercomputer

!contains( DEFINES, NO_DB ) {
   HEADERS      += us_crypto.h
   SOURCES      += us_crypto.cpp
}

