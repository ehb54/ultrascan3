include( ../../library.pri )
include( $${SINGLEDIR}/qtsingleapplication.pri )

# Not a gui program

TEMPLATE      = app

TARGET        = us_helpdaemon

DEPENDPATH   += ../../utils 
INCLUDEPATH  += ../../utils 
DESTDIR       = ../../bin

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

SOURCES      += us_helpdaemon.cpp 
HEADERS      += us_helpdaemon.h

unix:  DEFINES      += LINUX
win32: QMAKE_LFLAGS += /MACHINE:X86 /INCREMENTAL:NO 

