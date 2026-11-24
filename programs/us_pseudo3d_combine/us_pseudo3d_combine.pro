include( ../../gui.pri )

PROGRAM_LIBS  = results
include( ../../programlibs/link.pri )

QT           += xml

TARGET        = us_pseudo3d_combine

SOURCES       = us_pseudo3d_combine_main.cpp
