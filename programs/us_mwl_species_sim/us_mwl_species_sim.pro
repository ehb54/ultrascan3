include( ../../gui.pri )

PROGRAM_LIBS  = analysis
include( ../../programlibs/link.pri )

QT           += xml
TARGET        = us_mwl_species_sim

SOURCES       = us_mwl_species_sim_main.cpp
