include( ../../gui.pri )

PROGRAM_LIBS  = results
include( ../../programlibs/link.pri )

TARGET        = us_buoyancy
QT           += xml

SOURCES       = us_buoyancy_main.cpp
