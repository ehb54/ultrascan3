include( ../../gui.pri )

PROGRAM_LIBS  = gmp
include( ../../programlibs/link.pri )

TARGET        = us_esigner_gmp
QT           += xml
QT           += sql
QT           += svg opengl
QT           += printsupport

SOURCES       = us_esigner_gmp_main.cpp
