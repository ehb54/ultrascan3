include( ../../gui.pri )

CONFIG	     += qt debug
TARGET        = us_modelbuilder
QT           += xml

HEADERS       = us_modelbuilder.h ../us_astfem_sim/us_simulationparameters.h RegularGrid.h points2.h

SOURCES       = us_modelbuilder.cpp ../us_astfem_sim/us_simulationparameters.cpp RegularGrid.cpp points2.cpp
