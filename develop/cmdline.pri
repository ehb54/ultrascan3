# cmdline.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the cmdline version of UltraScan main programs.
# The only thing the perent needs to supply is TARGET and
# possibly HEADERS

##########################################################################################
#
# NOTICE !!! DO NOT EDIT ANY CMDLINE FLAGS OR LIBRARIES INTO THIS FILE!
#
# These properly belong in the correct qmake.conf file for cross-platform transportability
# for example $QTDIR/mkspecs/linux-g++-64-cmdline/qmake.conf
#
##########################################################################################

# Messages -- sets UNAME
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

TEMPLATE        = app
INCLUDEPATH     = $(QWTDIR)/include $(QWT3DDIR)/include 
DEPENDPATH     += ../src ../include
SOURCES         = main.cpp 

DEFINES         += CMDLINE

unix {
 CONFIG                += qt warn thread release
 DEFINES               += UNIX
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor
}

win32 {
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


