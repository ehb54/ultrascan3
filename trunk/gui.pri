# Profile include file for applications

!include( local.pri ) error( "local.pri is missing.  Copy from local.pri.template and update variables as appropriate" )

TEMPLATE     = app
DESTDIR      = ../../bin
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

CONFIG       += $$DEBUGORRELEASE qt thread warn

unix {
  LIBS       += -lus_utils -lus_gui -L../../lib
  LIBS       += -lcrypto
  LIBS       += -lqwt -L$$QWTPATH/lib
  LIBS       += -lmysqlclient -L$$MYSQLDIR 
  LIBS       += -lqwtplot3d-qt4 
  DEFINES    += INTEL LINUX

  DEPENDPATH   += ../../gui ../../utils $$QWTPATH/include ..
  INCLUDEPATH  += ../../gui ../../utils $$QWTPATH/include ..
  INCLUDEPATH  += $$MYSQLPATH $$QWT3D/include
}

win32 {
  LIBS       += ../../lib/libus_utils.lib
  LIBS       += ../../lib/libus_gui.lib
  LIBS       += $$QWTLIB
  DEFINES     += INTEL

  QMAKE_LFLAGS           += /MACHINE:X86 /INCREMENTAL:NO 
  QMAKE_CXXFLAGS_DEBUG   += /wd4996
  QMAKE_CXXFLAGS_RELEASE += /wd4996

  DEPENDPATH   += ../../gui ../../utils $$QWTPATH/src ..
  INCLUDEPATH  += ../../gui ../../utils $$QWTPATH/src ..
  INCLUDEPATH  += $$MYSQLPATH/include ../$$QWT3D/include
}

macx {
  CONFIG     += x86 x86_64
  LIBS       += -L../../lib -lus_utils -lus_gui 
  LIBS       += $$QWTLIB 
  LIBS       += -L../../lib -lmysqlclient -framework QtOpenGL
  DEFINES    += MAC OSX 
  INCLUDEPATH += ../../mysqldir/include
}

