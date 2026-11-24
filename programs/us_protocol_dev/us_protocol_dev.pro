include( ../../gui.pri )

PROGRAM_LIBS  = gmp
include( ../../programlibs/link.pri )

TARGET        = us_protocoldev
QT           += opengl
QT           += sql
QT           += xml
QT           += network
QT           += svg
QT           += printsupport
QT           += datavisualization

HEADERS       = us_protocol_dev_gui.h

SOURCES       = us_protocol_dev_main.cpp \
                us_protocol_dev_gui.cpp
