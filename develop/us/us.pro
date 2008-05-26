!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

TARGET       = us
SOURCES      = us.cpp
TRANSLATIONS = us.ts

win32 {
  RC_FILE    = us.rc 
}
