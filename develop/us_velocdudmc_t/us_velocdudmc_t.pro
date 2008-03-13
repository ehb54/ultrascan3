!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG       = us_velocdudmc_t
SOURCES		+= ../src/us_velocdudmc_t.cpp
HEADERS		 = ../include/us_velocdudmc_t.h

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}

