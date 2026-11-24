include( ../../gui.pri )

PROGRAM_LIBS  = results
include( ../../programlibs/link.pri )

TARGET        = us_abde
QT           += xml

HEADERS       = us_abde_main.h

SOURCES       = us_abde_main.cpp
