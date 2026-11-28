include( ../../gui.pri )

PROGRAM_LIBS  = results
include( ../../programlibs/link.pri )

QT           += xml svg

TARGET        = us_ddist_combine

SOURCES       = us_ddist_combine_main.cpp
