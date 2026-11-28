include( ../../gui.pri )

PROGRAM_LIBS  = data
include( ../../programlibs/link.pri )

TARGET        = us_edit
QT           += xml

SOURCES       = us_edit_main.cpp
