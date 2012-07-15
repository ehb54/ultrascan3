!include ( ../cmdline.pri ) error( "../cmdline.pri missing.  Aborting..." )
LIBS += -L$(TACC_CUDA_LIB) -lcudart us_cuda.o

DEFINES         += USE_MPI
DEFINES         += CUDA
SOURCES		+= ../src/us_saxs_gp.cpp
SOURCES		+= ../src/us_saxs_util.cpp
SOURCES		+= ../src/us_saxs_util_a2sb.cpp
SOURCES		+= ../src/us_saxs_util_asab1.cpp
SOURCES		+= ../src/us_saxs_util_cuda.cpp
SOURCES		+= ../src/us_saxs_util_dmd.cpp
SOURCES		+= ../src/us_saxs_util_extern.cpp
SOURCES		+= ../src/us_saxs_util_hydrate.cpp
SOURCES		+= ../src/us_saxs_util_hydrate_align.cpp
SOURCES		+= ../src/us_saxs_util_ift.cpp
SOURCES		+= ../src/us_saxs_util_iqq.cpp
SOURCES		+= ../src/us_saxs_util_iqq_pdb.cpp
SOURCES		+= ../src/us_saxs_util_iqq_bead_model.cpp
SOURCES		+= ../src/us_saxs_util_loads.cpp
SOURCES		+= ../src/us_saxs_util_nsa.cpp
SOURCES		+= ../src/us_saxs_util_nsa_ga.cpp
SOURCES		+= ../src/us_saxs_util_nsa_ga_mpi.cpp
SOURCES		+= ../src/us_saxs_util_nsa_gsm.cpp
SOURCES		+= ../src/us_saxs_util_nsa_sga.cpp
SOURCES		+= ../src/us_saxs_util_sgp.cpp
SOURCES		+= ../src/us_saxs_util_sgp_phys.cpp
SOURCES		+= ../src/us_saxs_util_mpi.cpp
SOURCES		+= ../src/us_tar.cpp
SOURCES		+= ../src/us_gzip.cpp
SOURCES		+= ../src/us_timer.cpp
SOURCES		+= ../src/us_math.cpp
SOURCES		+= ../src/us_file_util.cpp
SOURCES		+= ../src/us_hydrodyn_pat.cpp
SOURCES		+= us_semaphore.cpp
SOURCES		+= ../src/us_cmdline_app.cpp
SOURCES		+= ../src/us_saxs_util_dammin.cpp
SOURCES		+= ../src/us_saxs_util_crysol.cpp

HEADERS         += ../include/us.h
HEADERS         += ../include/us_math.h
HEADERS         += ../include/us_tar.h
HEADERS         += ../include/us_gzip.h
HEADERS         += ../include/us_hydrodyn_pdbdefs.h
HEADERS         += ../include/us_saxs_gp.h
HEADERS         += ../include/us_saxs_util.h
HEADERS		+= ../include/us_saxs_util_asab1.h
HEADERS         += ../include/us_tnt_jama.h
HEADERS         += ../include/us_file_util.h
HEADERS         += ../include/us_timer.h
HEADERS		+= us_semaphore.h
HEADERS         += ../include/us_cmdline_app.h

TARGET = us_saxs_cmds_mpi_cuda

unix {
 contains(UNAME,x86_64) {
    TARGET = $(ULTRASCAN)/bin64/us_saxs_cmds_mpi_cuda
 } else {
    TARGET = $(ULTRASCAN)/bin/us_saxs_cmds_mpi_cuda
 }
}

win32 {
  message ("MPI Not valid for WIN32")
}

macx {  
  message ("MPI Not valid for mac") 
}
