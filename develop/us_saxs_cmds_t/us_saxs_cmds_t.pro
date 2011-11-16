!include ( ../cmdline.pri ) error( "../cmdline.pri missing.  Aborting..." )

SOURCES		+= ../src/us_saxs_gp.cpp
SOURCES		+= ../src/us_saxs_util.cpp
SOURCES		+= ../src/us_saxs_util_asab1.cpp
SOURCES		+= ../src/us_saxs_util_dmd.cpp
SOURCES		+= ../src/us_saxs_util_extern.cpp
SOURCES		+= ../src/us_saxs_util_hydrate.cpp
SOURCES		+= ../src/us_saxs_util_hydrate_align.cpp
SOURCES		+= ../src/us_saxs_util_loads.cpp
SOURCES		+= ../src/us_saxs_util_iqq.cpp
SOURCES		+= ../src/us_saxs_util_iqq_pdb.cpp
SOURCES		+= ../src/us_saxs_util_iqq_bead_model.cpp
SOURCES		+= ../src/us_saxs_util_sgp.cpp
SOURCES		+= ../src/us_tar.cpp
SOURCES		+= ../src/us_gzip.cpp
SOURCES		+= ../src/us_math.cpp
SOURCES		+= ../src/us_file_util.cpp

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

TARGET = us_saxs_cmds_t

unix {
 contains(UNAME,x86_64) {
    TARGET = $(ULTRASCAN)/bin64/us_saxs_cmds_t
 } else {
    TARGET = $(ULTRASCAN)/bin/us_saxs_cmds_t
 }
}
