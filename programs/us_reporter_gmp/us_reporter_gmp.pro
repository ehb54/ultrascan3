include( ../../gui.pri )

PROGRAM_LIBS  = gmp
include( ../../programlibs/link.pri )

TARGET        = us_reporter_gmp
QT           += xml
QT           += sql
QT           += svg opengl
QT           += printsupport
QT           += datavisualization

SOURCES       = us_reporter_gmp_main.cpp
