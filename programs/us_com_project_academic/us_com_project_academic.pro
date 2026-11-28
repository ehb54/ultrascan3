include( ../../gui.pri )

PROGRAM_LIBS  = comproject
include( ../../programlibs/link.pri )

TARGET        = us_comproject_academic
QT           += opengl
QT           += sql
QT           += xml
QT           += network
QT           += svg
QT           += printsupport
QT           += datavisualization

SOURCES       = us_com_project_academic_main.cpp
