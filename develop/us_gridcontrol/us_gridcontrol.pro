!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG = us_gridcontrol

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}
