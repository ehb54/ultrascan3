!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/us_admin
} else {
    TARGET  = ../../bin/us_admin
}

