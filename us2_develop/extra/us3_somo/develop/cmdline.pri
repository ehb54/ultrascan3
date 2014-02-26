
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
#!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

TEMPLATE        = app
INCLUDEPATH     = $(QWTDIR)/include
DEPENDPATH     += ../src ../include
SOURCES         = main.cpp 

DEFINES         += CMDLINE
DEFINES         += NO_DB
DEFINES         += QT4

unix {
 CONFIG                += qt warn thread release
 DEFINES               += UNIX
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor
}

win32 {
  message ("Configuring for the Microsoft Windows Platform...")
  TEMPLATE             = lib
  TARGET               = us_somo
  QWTPATH              = C:/qwt-5.2.3
  CONFIG              += qt thread warn exceptions dll release
  DEFINES             += QT_DLL QWT_USE_DLL US_MAKE_DLL US_MAKE_GUI_DLL
  LIBS                += $$QWTPATH/lib/libqwt5.a
  INCLUDEPATH         += $$QWTPATH/src
  INCLUDEPATH         += C:/us3/qwtplot3d-qt4/include
  LIBS                += C:/us3/lib/libqwtplot3d-qt4.a
  INCLUDEPATH         += ../src
  DEFINES             += MINGW
  DESTDIR              = ../../bin
  LIBS                += ../../bin/libus_somo10.a
}

macx {

  BUILDBASE   = /Users/eb/us3/ultrascan3
##  QWTPATH     = $$BUILDBASE/qwt-5.2.3
  QWTPATH     = /src/qwt-5.2.3
  QWTLIB      = -L$$QWTPATH/lib -lqwt
##  SINGLEDIR    = $$BUILDBASE/qtsingleapplication-2.6_1-opensource/src
##  CONFIG      += x86_64 x86 app_bundle
  CONFIG      += x86_64
  DEFINES     += MAC OSX
  INCLUDEPATH += /usr/include
  INCLUDEPATH += /System/Library/Frameworks/OpenGL.framework/Versions/A/Headers
  INCLUDEPATH += $$QWTPATH/src
  INCLUDEPATH += /Users/eb/us3/qwtplot3d-qt4/include
  INCLUDEPATH += /usr/X11R6/include
  INCLUDEPATH += /Library/Frameworks/QtCore.framework/Versions/4/Headers
  INCLUDEPATH += /Library/Frameworks/QtGui.framework//Versions/4/Headers
  INCLUDEPATH += /Library/Frameworks/QtOpenGL.framework/Versions/4/Headers
##  INCLUDEPATH += /Library/Frameworks/QtSvg.framework/Versions/4/Headers
##  INCLUDEPATH += /Library/Frameworks/QtXml.framework/Versions/4/Headers
  LIBS        += -L/System/Library/Frameworks/OpenGL.framework/Libraries
  LIBS        += -L/Users/eb/us3/lib
  LIBS        += -lssl -lcrypto -lqwtplot3d-qt4
  LIBS        += -framework QtOpenGL

##  MYSQLPATH    = $$BUILDBASE/mysql
##  MYSQLDIR     = $$MYSQLPATH/lib
##  LIBS        += -lmysqlclient -L$$MYSQLDIR/lib
  X11LIB       = -L/usr/X11R6/lib -lXau -lX11
##  INCLUDEPATH += $$MYSQLDIR/include
}

#macx { RC_FILE = us_somo.icns }
#The following line was inserted by qt3to4
QT += qt3support 
