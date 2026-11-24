include( ../../gui.pri )

PROGRAM_LIBS  = analysis
include( ../../programlibs/link.pri )

QT           += xml svg opengl sql printsupport
QT           += datavisualization

TARGET        = us_2dsa

win32:LIBS   += -lpsapi

SOURCES       = us_2dsa_main.cpp
