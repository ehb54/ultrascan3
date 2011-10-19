!include ( ../cmdline.pri ) error( "../cmdline.pri missing.  Aborting..." )

DEFINES         += USE_MPI
SOURCES		+= ../src/us_saxs_util.cpp
SOURCES		+= ../src/us_saxs_util_extern.cpp
SOURCES		+= ../src/us_saxs_util_iqq.cpp
SOURCES		+= ../src/us_saxs_util_iqq_pdb.cpp
SOURCES		+= ../src/us_saxs_util_loads.cpp
SOURCES		+= ../src/us_saxs_util_mpi.cpp
SOURCES		+= ../src/us_tar.cpp
SOURCES		+= ../src/us_gzip.cpp
SOURCES		+= ../src/us_math.cpp

HEADERS         += ../include/us.h
HEADERS         += ../include/us_math.h
HEADERS         += ../include/us_tar.h
HEADERS         += ../include/us_gzip.h
HEADERS         += ../include/us_hydrodyn_pdbdefs.h

TARGET = us_saxs_cmds_mpi

unix {
 contains(UNAME,x86_64) {
    TARGET = $(ULTRASCAN)/bin64/us_saxs_cmds_mpi
 } else {
    TARGET = $(ULTRASCAN)/bin/us_saxs_cmds_mpi
 }
}

win32 {
  message ("MPI Not valid for WIN32")
}

macx {  
  message ("MPI Not valid for mac") 
}
