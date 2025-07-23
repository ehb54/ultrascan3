include( ../../gui.pri )

CONFIG	     += qt debug
TARGET        = us_modelbuilder
QT           += core xml
INCLUDEPATH  += ../../programs

HEADERS       = us_modelbuilder.h ../../programs/us_astfem_sim/us_simulationparameters.h

SOURCES       = us_modelbuilder.cpp ../../programs/us_astfem_sim/us_simulationparameters.cpp
