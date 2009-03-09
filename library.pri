# Profile include file for libraries

TEMPLATE     = lib
CONFIG       += debug qt thread warn
VERSION      = 10.0
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj
DEFINES     += INTEL
QT          += sql

unix {
#  QMAKE_CXXFLAGS_DEBUG += -pg
  QWTPATH       = /usr/local/qwt-5.1.1
  DEPENDPATH   += ../gui ../utils ../db $$QWTPATH/include
  INCLUDEPATH  += ../gui ../utils ../db $$QWTPATH/include
  DEFINES      += LINUX
  DESTDIR      = ../lib
}

win32 {
  QWTPATH                 = C:/qwt/qwt-5.1.1
  QCAPATH                 = C:/qca-2.0.1
  DEPENDPATH             += ../gui ../utils
  INCLUDEPATH            += ../gui ../utils $$QWTPATH/src $$QCAPATH/include/QtCrypto
  LIBS                   += $$QCAPATH/lib/qcad2.lib $$QWTPATH/lib/qwtd5.lib 
  QMAKE_LFLAGS           += /IMPLIB:../lib/$${TARGET}.lib /MACHINE:X86 /INCREMENTAL:NO 
  QMAKE_CXXFLAGS_DEBUG   += /wd4996
  QMAKE_CXXFLAGS_RELEASE += /wd4996
  DEFINES                += US_MAKE_DLL
  DESTDIR                 = ../bin
}


