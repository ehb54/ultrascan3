include( ../../gui.pri )

CONFIG	     += qt debug
TARGET        = us_modelbuilder
QT           += xml
INCLUDEPATH  += ../../programs

HEADERS       = us_modelbuilder.h ../../programs/us_astfem_sim/us_simulationparameters.h RegularGrid.h points2.h

SOURCES       = us_modelbuilder.cpp ../../programs/us_astfem_sim/us_simulationparameters.cpp RegularGrid.cpp points2.cpp
