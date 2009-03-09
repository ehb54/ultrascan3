# Profile include file for applications

TEMPLATE     = app
DESTDIR      = ../../bin
DEFINES     += INTEL
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj


CONFIG       += debug qt thread warn

unix {
#  QMAKE_CXXFLAGS_DEBUG += -pg
#  QMAKE_LFLAGS_DEBUG += -pg
  QWTPATH     = /usr/local/qwt-5.1.1
  LIBS       += -lus_utils -lus_gui -lus_db -L../../lib
  LIBS       += -lqca
  LIBS       += -lqwt -L$$QWTPATH/lib
  DEFINES    += LINUX

  DEPENDPATH   += ../../gui ../../utils ../../db $$QWTPATH/include ..
  INCLUDEPATH  += ../../gui ../../utils ../../db $$QWTPATH/include ..
}

win32 {
  QWTPATH     = C:/qwt/qwt-5.1.1
  LIBS       += ../../lib/libus_utils.lib
  LIBS       += ../../lib/libus_db.lib
  LIBS       += ../../lib/libus_gui.lib
  #  Fix this for release
  LIBS       += $$QWTPATH/lib/qwtd5.lib

  QMAKE_LFLAGS           += /MACHINE:X86 /INCREMENTAL:NO 
  QMAKE_CXXFLAGS_DEBUG   += /wd4996
  QMAKE_CXXFLAGS_RELEASE += /wd4996

  DEPENDPATH   += ../../gui ../../utils ../../db $$QWTPATH/src ..
  INCLUDEPATH  += ../../gui ../../utils ../../db $$QWTPATH/src ..
}


