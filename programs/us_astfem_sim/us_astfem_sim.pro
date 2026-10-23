include( ../../gui.pri )

TARGET        = us_astfem_sim
QT+=xml
HEADERS       = us_astfem_sim.h             \
                us_clipdata.h

SOURCES       = main.cpp                    \
                us_astfem_sim.cpp           \
                us_clipdata.cpp

