include( /opt/qt-4.5.0/addons/qtsingleapplication-2.5_1-commercial/src/qtsingleapplication.pri )

# Not a gui program

TEMPLATE      = app

CONFIG       += debug qt thread warn
TARGET        = us_helpdaemon

DEPENDPATH   += ../../utils 
INCLUDEPATH  += ../../utils 
DESTDIR       = ../../bin

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

SOURCES      += us_helpdaemon.cpp 
HEADERS      += us_helpdaemon.h

DEFINES      += LINUX
