!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

contains(UNAME,x86_64) {
    TARGET  = ../../bin64/us
} else {
    TARGET  = ../../bin/us
}

win32 { RC_FILE = us.rc }

SOURCES      = us.cpp 
TRANSLATIONS = us.ts

