!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG = us_combine

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}
