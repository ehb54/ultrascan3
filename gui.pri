# Profile include file for applications

include( local.pri )

TEMPLATE     = app
DESTDIR      = ../../bin
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj
VER          = 10
QT          += widgets openglwidgets


CONFIG       += $$DEBUGORRELEASE qt thread warn widgets

unix {
  LIBS       += -L../../lib -lus_gui -lus_utils
  LIBS       += -lcrypto
  LIBS       += $$QWTLIB
  ##LIBS       += -lqwtplot3d-qt4 -lGLU
  LIBS       += -lqwtplot3d -lGLU
  LIBS       += -L$$MYSQLDIR -lmysqlclient
  DEFINES    += INTEL LINUX

  DEPENDPATH   += ../../gui ../../utils $$QWTPATH/include ..
  INCLUDEPATH  += $$MYSQLPATH ../$$QWT3D/include
  INCLUDEPATH  += ../../gui ../../utils $$QWTPATH/include ..
}

win32 {
  DEPENDPATH   += ../../gui ../../utils $$QWTPATH/src ..

  INCLUDEPATH  += ../../gui ../../utils $$QWTPATH/src ..
  INCLUDEPATH  += $$MYSQLPATH/include ../$$QWT3D/include
  INCLUDEPATH  += $$OPENSSL/include
  INCLUDEPATH  += $$QTPATH/include

  LIBS       += $$QWTLIB
  LIBS       += $$MYSQLLIB
  LIBS       += -L$$OPENSSL/lib -lssl -lcrypto
  LIBS       += $$MINGWDIR/lib/libws2_32.a $$MINGWDIR/lib/libadvapi32.a
  LIBS       += $$MINGWDIR/lib/libgdi32.a $$MINGWDIR/lib/libuser32.a
  LIBS       += -L../../lib -lus_gui$${VER} -lus_utils$${VER}

  DEFINES     += INTEL

  #QMAKE_LFLAGS           += /MACHINE:X86 /INCREMENTAL:NO 
  #QMAKE_CXXFLAGS_DEBUG   += /wd4996
  #QMAKE_CXXFLAGS_RELEASE += /wd4996
}

macx {
  CONFIG     += x86_64
  LIBS       += -L../../lib -lus_gui -lus_utils
  LIBS       += $$QWTLIB 
  LIBS       += -L../../lib -lmysqlclient -framework QtOpenGL
  DEFINES    += MAC OSX 
  INCLUDEPATH += $$MYSQLPATH/include
}

