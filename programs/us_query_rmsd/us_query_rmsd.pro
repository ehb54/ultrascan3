include( ../../gui.pri )

PROGRAM_LIBS  = analysis
include( ../../programlibs/link.pri )

QT           += xml svg opengl sql printsupport
QT           += datavisualization 

TARGET        = us_query_rmsd

HEADERS       = us_query_rmsd.h

SOURCES       = us_query_rmsd.cpp
