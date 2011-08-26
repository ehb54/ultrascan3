# Profile include file for applications

!include( local.pri ) error( "local.pri is missing.  Copy from local.pri.template and update variables as appropriate" )

TEMPLATE     = app
DESTDIR      = ../../bin
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj
VER          = 10

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
  DEPENDPATH   += ../../gui ../../utils $$QWTPATH/src ..

  INCLUDEPATH  += ../../gui ../../utils $$QWTPATH/src ..
  INCLUDEPATH  += $$MYSQLPATH/include ../$$QWT3D/include
  INCLUDEPATH  += $$OPENSSL/include
  INCLUDEPATH  += $$QTPATH/include

  LIBS       += $$QWTLIB
  LIBS       += $$MYSQLLIB
  LIBS       += $$QTMYSQLPATH/libqsqlmysql4.a
  LIBS       += $$OPENSSL/lib/libeay32.a
  LIBS       += $$MINGWDIR/lib/libws2_32.a $$MINGWDIR/lib/libadvapi32.a
  LIBS       += $$MINGWDIR/lib/libgdi32.a $$MINGWDIR/lib/libuser32.a
  LIBS       += -L../../lib -lus_utils$${VER} -lus_gui$${VER}

  DEFINES     += INTEL

  #QMAKE_LFLAGS           += /MACHINE:X86 /INCREMENTAL:NO 
  #QMAKE_CXXFLAGS_DEBUG   += /wd4996
  #QMAKE_CXXFLAGS_RELEASE += /wd4996
}

macx {
  CONFIG     += x86 x86_64
  LIBS       += -L../../lib -lus_utils -lus_gui 
  LIBS       += $$QWTLIB 
  LIBS       += -L../../lib -lmysqlclient -framework QtOpenGL
  DEFINES    += MAC OSX 
  INCLUDEPATH += ../../mysqldir/include
}

