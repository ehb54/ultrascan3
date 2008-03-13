!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG = us_fe_nnls_t_mpi

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}

SOURCES   += ../src/us_fe_nnls_t_mpi.cpp
SOURCES   += ../src/us_mfem_test.cpp
SOURCES   += ../src/us_math.cpp
SOURCES   += ../src/us_ga.cpp
SOURCES   += ../src/us_ga_stacks.cpp
SOURCES   += ../src/us_ga_round.cpp
SOURCES   += ../src/us_ga_random_normal.cpp
SOURCES   += ../src/us_ga_s_estimate.cpp
SOURCES   += ../src/us_ga_gsm.cpp

HEADERS    = ../include/us_fe_nnls_t.h
HEADERS   += ../include/us_mfem_test.h
HEADERS   += ../include/us_math.h
HEADERS   += ../include/us_ga.h
HEADERS   += ../include/us_ga_stacks.h
HEADERS   += ../include/us_ga_round.h 
HEADERS   += ../include/us_ga_random_normal.h
HEADERS   += ../include/us_ga_s_estimate.h
HEADERS   += ../include/us_ga_gsm.h

