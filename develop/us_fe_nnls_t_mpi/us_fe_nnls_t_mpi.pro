!include ( ../mpi.pri ) error( "../mpi.pri missing.  Aborting..." )

SOURCES		+= ../src/us_fe_nnls_t_mpi.cpp
SOURCES		+= ../src/us_mfem_test.cpp
SOURCES		+= ../src/us_math.cpp
SOURCES		+= ../src/us_ga.cpp
SOURCES		+= ../src/us_ga_stacks.cpp
SOURCES		+= ../src/us_ga_round.cpp
SOURCES		+= ../src/us_ga_random_normal.cpp
SOURCES		+= ../src/us_ga_s_estimate.cpp
SOURCES		+= ../src/us_ga_gsm.cpp
SOURCES		+= ../src/us_femglobal.cpp
SOURCES		+= ../src/us_astfem_rsa.cpp
SOURCES		+= ../src/us_stiffbase.cpp
SOURCES		+= ../src/us_ga_interacting.cpp

HEADERS		= ../include/us_fe_nnls_t.h
HEADERS		+= ../include/us_mfem_test.h
HEADERS		+= ../include/us_math.h
HEADERS         += ../include/us_ga.h
HEADERS         += ../include/us_ga_stacks.h
HEADERS         += ../include/us_ga_round.h 
HEADERS         += ../include/us_ga_random_normal.h
HEADERS         += ../include/us_ga_s_estimate.h
HEADERS         += ../include/us_ga_gsm.h
HEADERS		+= ../include/us_femglobal.h
HEADERS		+= ../include/us_astfem_rsa.h
HEADERS		+= ../include/us_stiffbase.h
HEADERS		+= ../include/us_ga_interacting.h

contains (UNAME,x86_64) {
	TARGET		= ../../bin64/us_fe_nnls_t_mpi
        DEFINES         += BIN64
} else {
	TARGET		= ../../bin/us_fe_nnls_t_mpi
}
