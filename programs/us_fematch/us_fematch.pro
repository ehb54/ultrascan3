include( ../../gui.pri )

PROGRAM_LIBS  = analysis
include( ../../programlibs/link.pri )

QT           += xml svg opengl sql printsupport
QT           += datavisualization

TARGET        = us_fematch

SOURCES       = us_fematch_main.cpp
