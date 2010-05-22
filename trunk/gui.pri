# Profile include file for applications

!include( local.pri ) error( "local.pri is missing.  Copy from local.pri.template and update variables as appropriate" )

TEMPLATE     = app
DESTDIR      = ../../bin
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj


CONFIG       += $$DEBUGORRELEASE qt thread warn

unix {
  LIBS       += -lus_utils -lus_gui -lus_db -L../../lib
  LIBS       += -lqca
  LIBS       += -lqwt -L$$QWTPATH/lib
  LIBS       += -lmysqlclient -L$$MYSQLDIR 
  DEFINES    += INTEL LINUX

  DEPENDPATH   += ../../gui ../../utils ../../db $$QWTPATH/include ..
  INCLUDEPATH  += ../../gui ../../utils ../../db $$QWTPATH/include ..
}

win32 {
  LIBS       += ../../lib/libus_utils.lib
  LIBS       += ../../lib/libus_db.lib
  LIBS       += ../../lib/libus_gui.lib
  LIBS       += $$QWTLIB
  DEFINES     += INTEL

  QMAKE_LFLAGS           += /MACHINE:X86 /INCREMENTAL:NO 
  QMAKE_CXXFLAGS_DEBUG   += /wd4996
  QMAKE_CXXFLAGS_RELEASE += /wd4996

  DEPENDPATH   += ../../gui ../../utils ../../db $$QWTPATH/src ..
  INCLUDEPATH  += ../../gui ../../utils ../../db $$QWTPATH/src ..
}

macx {
  CONFIG     += i386 ppc
  LIBS       += -L../../lib -lus_utils -lus_gui -lus_db
  LIBS       += $$QWTLIB $$QCALIB
  LIBS       += -L$$MYSQLDIR -lmysqlclient
  DEFINES    += MAC OSX 
}

