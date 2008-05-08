!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG      = us_combinescans
SOURCES   = us_combinescans.cpp

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}

