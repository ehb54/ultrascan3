!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG     = us_check
SOURCES += us_check.cpp
HEADERS  = us_check.h

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}
