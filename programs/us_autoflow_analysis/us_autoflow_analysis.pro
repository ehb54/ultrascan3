include( ../../gui.pri )

PROGRAM_LIBS  = gmp
include( ../../programlibs/link.pri )

TARGET        = us_autoflow_analysis
QT           += xml
QT           += sql
QT           += svg opengl printsupport
QT           += datavisualization

win32:LIBS   += -lpsapi

SOURCES       = us_autoflow_analysis_main.cpp
