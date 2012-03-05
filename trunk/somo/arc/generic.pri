# Generic.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the UltraScan main programs.
# The only thing the perent needs to supply is TARGET and
# possibly HEADERS

# Messages
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

QWTDIR          = /opt/qwt-qt4

TEMPLATE        = app
INCLUDEPATH     = $(QWTDIR)/src $(QWTDIR)/include
DEPENDPATH     += ../src ../include
SOURCES         = main.cpp 
DESTDIR         = ../../bin
DEFINES += NO_DB
# temporary fix (us2 code was using qt2 qpdevmnt which I think need to be replaced by qprintdevicemetrics)
DEFINES += NO_EDITOR_PRINT
DEFINES += QT4

unix {
 UNAME                  = $$system(uname -a)
 CONFIG                += qt warn thread release
 DEFINES               += UNIX
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor

 contains(UNAME,x86_64) {
    LIBS    += -L$(QWTDIR)/lib64/ -lqwt -L$(ULTRASCAN)/somo/lib64 -lus_somo
    DESTDIR  = ../../bin64
 } else {
    LIBS    += -L$(QWTDIR)/lib -lqwt -L$(ULTRASCAN)/somo/lib -lus_somo
 }
}

win32 {
  TEMPLATE             = vcapp          # Visual C application (creates .vcproj file)
  CONFIG              += qt warn thread release
  #CONFIG              += qt warn thread debug
  QMAKE_CXXFLAGS      += /EHsc          # Assume extern C functions never throw exceptions
  QMAKE_CXXFLAGS      += /Fd$(IntDir)\  # Put intermediate directories in separate location
  QMAKE_LFLAGS_DEBUG  += /NODEFAULTLIB:"msvcrt.lib"
  QMAKE_LFLAGS_RELEASE =                # Remove //DELAYLOAD: statements
  LIBS                += ../../bin/libus_somo10.lib 
  DESTDIR              = ..\..\bin\
}

macx { RC_FILE = ultrascan.icns }

#The following line was inserted by qt3to4
QT += qt3support 
