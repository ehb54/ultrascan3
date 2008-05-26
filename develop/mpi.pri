# mpi.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the mpi version of UltraScan main programs.
# The only thing the perent needs to supply is TARGET and
# possibly HEADERS

##########################################################################################
#
# NOTICE !!! DO NOT EDIT ANY MPI FLAGS OR LIBRARIES INTO THIS FILE!
#
# These properly belong in the correct qmake.conf file for cross-platform transportability
# for example $QTDIR/mkspecs/linux-g++-64-mpi/qmake.conf
#
##########################################################################################

# Messages -- sets UNAME
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

TEMPLATE        = app
INCLUDEPATH     = $(QWTDIR)/include $(QWT3DDIR)/include 
DEPENDPATH     += ../src ../include
SOURCES         = main.cpp 

unix {
 CONFIG                += qt warn thread release
 DEFINES               += UNIX
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor

 contains(UNAME,x86_64) {
    LIBS   += -L$(QWTDIR)/lib64/ -L$(QWT3DDIR)/lib64 -L$(ULTRASCAN)/lib64
 } else {
    LIBS   += -L$(QWTDIR)/lib -L$(QWT3DDIR)/lib -L$(ULTRASCAN)/lib
 }
}

win32 {
  message ("MPI Not valid for WIN32")
}

macx {  
  message ("MPI Not valid for mac") 
}


