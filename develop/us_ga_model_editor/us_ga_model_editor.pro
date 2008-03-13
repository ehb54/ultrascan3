!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

PROG = us_ga_model_editor

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/$$PROG
} else {
    TARGET  = ../../bin/$$PROG
}
