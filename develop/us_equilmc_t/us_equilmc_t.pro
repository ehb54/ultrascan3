!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG     = us_equilmc_t
SOURCES += ../src/us_equilmc_t.cpp
HEADERS  = ../include/us_equilmc_t.h 

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}

