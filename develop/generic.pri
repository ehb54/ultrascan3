# Generic.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the UltraScan main programs.
# The only thing the perent needs to supply is TARGET and
# possibly HEADERS

# Messages
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

TEMPLATE        = app
INCLUDEPATH     = $(QWTDIR)/include $(QWT3DDIR)/include 
DEPENDPATH     += ../src ../include
SOURCES         = main.cpp 
DESTDIR         = ../../bin

unix {
 UNAME                  = $$system(uname -a)
 CONFIG                += qt warn thread release
 DEFINES               += UNIX QT_NO_STL
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor
 # make sure CBFDIR is set if HAS_CBF is defined
  DEFINES               += HAS_CBF
  INCLUDEPATH           += $(CBFDIR)/include
  LIBS                  += -L$(CBFDIR)/lib -lcbf

 contains(UNAME,x86_64) {
    LIBS    += -L$(QWTDIR)/lib64/ -lqwt -L$(QWT3DDIR)/lib64 -lqwtplot3d -L$(ULTRASCAN)/lib64 -lus
    DESTDIR  = ../../bin64
 } else {
    LIBS    += -L$(QWTDIR)/lib -lqwt -L$(QWT3DDIR)/lib -lqwtplot3d -L$(ULTRASCAN)/lib -lus
 }
}

win32 {
  #TEMPLATE             = vcapp          # Visual C application (creates .vcproj file)
  CONFIG              += qt warn thread release
  #CONFIG              += qt warn thread debug
  QMAKE_CXXFLAGS      += /EHsc          # Assume extern C functions never throw exceptions
  QMAKE_CXXFLAGS      += /Fd$(IntDir)\  # Put intermediate directories in separate location
  QMAKE_LFLAGS_DEBUG  += /NODEFAULTLIB:"msvcrt.lib"
  QMAKE_LFLAGS_RELEASE =                # Remove //DELAYLOAD: statements
  LIBS                += ../../bin/libus99.lib 
  DESTDIR              = ..\..\bin\
}

macx { RC_FILE = ultrascan.icns }

