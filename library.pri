# Profile include file for libraries
#  Copy file to library.pri and change QWTPATH, SINGLEDIR to match your installation
#  Also see gui.pri.template

include( local.pri )

TEMPLATE     = lib
CONFIG      += $$DEBUGORRELEASE qt thread warn
VERSION      = 10.0
VER          = 10
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj
QT          += xml
DESTDIR       = ../lib
DLLDESTDIR = ../bin

unix {
  contains( DEFINES, NO_DB ) {
    # NO_DB is for the supercomputer back end with no gui, mysql, or qca
    DEPENDPATH   += ../utils  
    INCLUDEPATH  += ../utils  
  }
  else {
    DEPENDPATH   += ../gui ../utils  $$QWTPATH/include
    INCLUDEPATH  += ../gui ../utils  $$QWTPATH/include $$QWT3D/include
    INCLUDEPATH  += $$MYSQLPATH
  }
  DEFINES      += INTEL LINUX
  LIBS         += -larchive
  
}

win32 {
  DEPENDPATH             += ../gui ../utils

  INCLUDEPATH            += ../gui ../utils
  INCLUDEPATH            += $$QWTPATH/src 
  INCLUDEPATH            += $$MYSQLPATH/include
  INCLUDEPATH            += $$OPENSSL/include
  INCLUDEPATH            += ../qwtplot3d/include

  LIBS                   += $$QWTLIB
  LIBS                   += $$MYSQLLIB
  LIBS                   += -L$$OPENSSL/lib -lssl -lcrypto
  LIBS                   += $$MINGWDIR/lib/libws2_32.a $$MINGWDIR/lib/libadvapi32.a
  LIBS                   += $$MINGWDIR/lib/libgdi32.a $$MINGWDIR/lib/libuser32.a
  LIBS                   += $$MINGWDIR/lib/libkernel32.a $$MINGWDIR/lib/libpsapi.a
  LIBS                   += -larchive

  #QMAKE_LFLAGS           += /IMPLIB:../lib/$${TARGET}.a /MACHINE:X86 /INCREMENTAL:NO 
  #QMAKE_CXXFLAGS_DEBUG   += /wd4996
  #QMAKE_CXXFLAGS_RELEASE += /wd4996

  DEFINES                += INTEL US_MAKE_DLL
}

macx {
  CONFIG       += x86_64
  DEPENDPATH   += ../gui ../utils $$QWTPATH/include
  INCLUDEPATH  += ../gui ../utils $$QWTPATH/include ../mysql/include
  DEFINES      += MAC OSX
  LIBS         += $$QWTLIB
  LIBS         += -larchive
}

