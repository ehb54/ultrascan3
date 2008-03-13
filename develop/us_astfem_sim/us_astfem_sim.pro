!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/us_astfem_sim
} else {
    TARGET  = ../../bin/us_astfem_sim
}

