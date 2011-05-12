# Profile include file for libraries
#  Copy file to library.pri and change QWTPATH, SINGLEDIR to match your installation
#  Also see gui.pri.template

!include( local.pri ) error( "local.pri is missing.  Copy from local.pri.template and update variables as appropriate" )

TEMPLATE     = lib
CONFIG      += $$DEBUGORRELEASE qt thread warn
VERSION      = 10.0
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj
QT          += xml

unix {
  contains( DEFINES, NO_DB ) {
    # NO_DB is for the supercomputer back end with no gui, mysql, or qca
    DEPENDPATH   += ../utils  
    INCLUDEPATH  += ../utils  
  }
  else {
    DEPENDPATH   += ../gui ../utils $$QWTPATH/include
    INCLUDEPATH  += ../gui ../utils $$QWTPATH/include
    INCLUDEPATH  += $$MYSQLPATH
  }
  DEFINES      += INTEL LINUX
  LIBS         += -luuid
  DESTDIR       = ../lib
}

win32 {
  DEPENDPATH             += ../gui ../utils
  INCLUDEPATH            += ../gui ../utils $$QWTPATH/src 
  INCLUDEPATH            += $$MYSQLPATH
  LIBS                   += $$QWTLIB
  QMAKE_LFLAGS           += /IMPLIB:../lib/$${TARGET}.lib /MACHINE:X86 /INCREMENTAL:NO 
  QMAKE_CXXFLAGS_DEBUG   += /wd4996
  QMAKE_CXXFLAGS_RELEASE += /wd4996
  DEFINES                += INTEL US_MAKE_DLL
  DESTDIR                 = ../bin
}

macx {
  CONFIG       += i386 ppc
  DEPENDPATH   += ../gui ../utils $$QWTPATH/include
  INCLUDEPATH  += ../gui ../utils $$QWTPATH/include 
  DEFINES      += MAC OSX
  LIBS         += -luuid $$QWTLIB
  DESTDIR       = ../lib
}

