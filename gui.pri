# Profile include file for applications

TEMPLATE     = app

QWTPATH      = /usr/local/qwt-5.1.1

CONFIG       += debug qt thread warn

DEPENDPATH   += ../../gui ../../utils ../../db $$QWTPATH/include ..
INCLUDEPATH  += ../../gui ../../utils ../../db $$QWTPATH/include ..

LIBS         += -lus_utils -lus_gui -lus_db -L../../lib
LIBS         += -lqca
LIBS         += -lqwt -L$$QWTPATH/lib

DESTDIR      = ../../bin

DEFINES     += LINUX INTEL

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

