# Profile include file for applications

TEMPLATE     = app
DESTDIR      = ../../bin
DEFINES     += INTEL
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

unix:  QWTPATH = /usr/local/qwt-5.1.1
win32: QWTPATH = C:/Qwt-5.1.1

CONFIG       += debug qt thread warn

DEPENDPATH   += ../../gui ../../utils ../../db $$QWTPATH/include ..
INCLUDEPATH  += ../../gui ../../utils ../../db $$QWTPATH/include ..

unix {
  LIBS       += -lus_utils -lus_gui -lus_db -L../../lib
  LIBS       += -lqca
  LIBS       += -lqwt -L$$QWTPATH/lib
  DEFINES    += LINUX
}

win32 {
  LIBS       += ../../lib/libus_utils.lib
  LIBS       += ../../lib/libus_db.lib
  LIBS       += ../../lib/libus_gui.lib
  QMAKE_LFLAGS += /MACHINE:X86 /INCREMENTAL:NO 

  QMAKE_CXXFLAGS_DEBUG   += /wd4996
  QMAKE_CXXFLAGS_RELEASE += /wd4996
}
