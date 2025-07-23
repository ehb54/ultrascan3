include ( ../library.pri )

QT          -= gui
QT          += network
QT          += sql
unix: TARGET = us_utils

macx {
      TARGET = us_utils
      LIBS  += -L$${MYSQLDIR} -lmysqlclient
      LIBS  += -L../lib
}

win32 {
      TARGET   = us_utils
      LIBS  += -L$${OPENSSL}/lib -lssl -lcrypto -lgdi32
      DEFINES += US_MAKE_UTIL_DLL
}

TRANSLATIONS = $${TARGET}_DE_de.ts


# Consolidate us_femglobal and us_femglobal_new when development is
# progressed to a point where us_femglobal can be removed

HEADERS      = us_ana_profile.h   \
               us_analyte.h       \
               us_archive.h       \
               us_astfem_math.h   \
               us_astfem_rsa.h    \
               us_buffer.h        \
               us_cfa_data.h      \
               us_constants.h     \
               us_crc.h           \
               us_csv_data.h      \
               us_dataIO.h        \
               us_datafiles.h     \
               us_db2.h           \
               us_dmga_constr.h   \
               us_eprofile.h      \
               us_global.h        \
               us_gzip.h          \
               us_hardware.h      \
               us_hydrosim.h      \
               us_http_post.h     \
               us_lamm_astfvm.h   \
               us_license_t.h     \
               us_link_ssl.h      \
               us_lm.h            \
               us_local_server.h  \
               us_math2.h         \
               us_matrix.h        \
               us_memory.h        \
               us_model.h         \
               us_noise.h         \
               us_pcsa_modelrec.h \
               us_project.h       \
               us_protocol_util.h \
               us_report.h        \
               us_report_gmp.h    \
               us_rotor.h         \
               us_run_protocol.h  \
               us_settings.h      \
               us_simparms.h      \
               us_solute.h        \
               us_solution.h      \
               us_solution_vals.h \
               us_solve_sim.h     \
               us_stiffbase.h     \
               us_tar.h           \
               us_time_state.h    \
               us_timer.h         \
               us_geturl.h        \
               us_util.h          \
               us_vector.h        \
               us_xpn_data.h      \
               us_zsolute.h

SOURCES      = us_ana_profile.cpp   \
               us_analyte.cpp       \
               us_archive.cpp       \
               us_astfem_math.cpp   \
               us_astfem_rsa.cpp    \
               us_buffer.cpp        \
               us_cfa_data.cpp      \
               us_constants.cpp     \
               us_crc.cpp           \
               us_csv_data.cpp      \
               us_dataIO.cpp        \
               us_datafiles.cpp     \
               us_db2.cpp           \
               us_dmga_constr.cpp   \
               us_eprofile.cpp      \
               us_global.cpp        \
               us_gzip.cpp          \
               us_hardware.cpp      \
               us_http_post.cpp     \
               us_hydrosim.cpp      \
               us_lamm_astfvm.cpp   \
               us_license_t.cpp     \
               us_link_ssl.cpp      \
               us_lm.cpp            \
               us_local_server.cpp  \
               us_math2.cpp         \
               us_matrix.cpp        \
               us_memory.cpp        \
               us_model.cpp         \
               us_noise.cpp         \
               us_pcsa_modelrec.cpp \
               us_project.cpp       \
               us_protocol_util.cpp \
               us_report.cpp        \
               us_report_gmp.cpp    \
               us_rotor.cpp         \
               us_run_protocol.cpp  \
               us_settings.cpp      \
               us_simparms.cpp      \
               us_solute.cpp        \
               us_solution.cpp      \
               us_solution_vals.cpp \
               us_solve_sim.cpp     \
               us_stiffbase.cpp     \
               us_tar.cpp           \
               us_time_state.cpp    \
               us_timer.cpp         \
               us_geturl.cpp        \
               us_util.cpp          \
               us_vector.cpp        \
               us_xpn_data.cpp      \
               us_zsolute.cpp

# Only include us_crypto when not on a supercomputer

!contains( DEFINES, NO_DB ) {
   HEADERS      += us_crypto.h
   SOURCES      += us_crypto.cpp
}
contains( DEFINES, NO_DB ) {
   LIBS         += -L$${MPIPATH}/lib
   INCLUDE      += -I$${MPIPATH}/include
}

