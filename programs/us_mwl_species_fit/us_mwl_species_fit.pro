include( ../../gui.pri )

PROGRAM_LIBS  = analysis
include( ../../programlibs/link.pri )

QT += datavisualization
QT += printsupport
QT           += sql

TARGET        = us_mwl_species_fit

SOURCES       = us_mwl_species_fit_main.cpp
