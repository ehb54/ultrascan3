!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG = us_db_veloc

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}
