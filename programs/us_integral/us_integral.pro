include( ../../gui.pri )

PROGRAM_LIBS  = results
include( ../../programlibs/link.pri )

QT           += xml

TARGET        = us_integral

SOURCES       = us_integral_main.cpp
