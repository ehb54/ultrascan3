include( ../../gui.pri )

PROGRAM_LIBS  = data
include( ../../programlibs/link.pri )

QT           += opengl sql xml network
QT           += datavisualization
QT           += printsupport

TARGET        = us_xpn_viewer

SOURCES       = us_xpn_viewer_main.cpp
