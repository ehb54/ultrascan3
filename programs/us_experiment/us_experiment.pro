include( ../../gui.pri )

PROGRAM_LIBS  = experiment
include( ../../programlibs/link.pri )

TARGET        = us_experiment
QT           += xml
QT           += sql
QT           += network
QT           += printsupport

SOURCES       = us_experiment_main.cpp
