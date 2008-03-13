!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG = us_db_rst_montecarlo

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}
