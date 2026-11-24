include( ../../gui.pri )

PROGRAM_LIBS  = analysis
include( ../../programlibs/link.pri )

TARGET        = us_analysis_profile
QT           += xml
QT           += sql

SOURCES       = us_anapro_main.cpp
