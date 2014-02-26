# Generic.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the UltraScan main programs.

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
# UNAME                  = $$system(uname -a)
 CONFIG                += qt warn thread release
 DEFINES               += UNIX
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor
 MYSQLPATH              = /usr/lib/mysql

 contains(UNAME,x86_64) {
    LIBS    += -L$(QWTDIR)/lib64/ -L$(QWTDIR)/lib/ -lqwt -L$(ULTRASCAN)/lib64 -lus_somo
    DESTDIR  = ../../bin64
 } else {
    LIBS    += -L$(QWTDIR)/lib -lqwt -L$(ULTRASCAN)/lib -lus_somo
 }
}

win32 {
  US3PATH         = C:/Users/Admin/Documents/ultrascan3
  MINGWDIR        = C:/mingw
  QWT3DPATH       = $$US3PATH/qwtplot3d-qt4
  VER             = 10

  CONFIG         += qt thread warn release

  QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_STL
  QMAKESPEC       = win32-g++-4.6

  DEFINES        += MINGW

  INCLUDEPATH    += $$QWTPATH/src ..
  INCLUDEPATH    += $$MYSQLPATH/include ../$$QWT3D/include
  INCLUDEPATH    += $$OPENSSL/include
  INCLUDEPATH    += $$QTPATH/include
  INCLUDEPATH    += $$QWT3DPATH/include
  INCLUDEPATH    += ../src

  LIBS         += $$QWTLIB
  LIBS         += $$MINGWDIR/lib/libws2_32.a $$MINGWDIR/lib/libadvapi32.a
  LIBS         += $$MINGWDIR/lib/libgdi32.a $$MINGWDIR/lib/libuser32.a
  LIBS         += $$US3PATH/somo/bin/liblibus_somo$${VER}.a
}

macx {
  BUILDBASE   = /Users/eb/us3/ultrascan3
  QWTPATH     = /src/qwt-5.2.3
  QWTLIB      = -L$$QWTPATH/lib -lqwt
  DEFINES     += MAC OSX

  CONFIG      += x86_64

  INCLUDEPATH += /usr/include
  INCLUDEPATH += /System/Library/Frameworks/OpenGL.framework/Versions/A/Headers
  INCLUDEPATH += $$QWTPATH/src
  INCLUDEPATH += /Users/eb/us3/qwtplot3d-qt4/include
  INCLUDEPATH += /usr/X11R6/include
  INCLUDEPATH += /Library/Frameworks/QtCore.framework/Versions/4/Headers
  INCLUDEPATH += /Library/Frameworks/QtGui.framework//Versions/4/Headers
  INCLUDEPATH += /Library/Frameworks/QtOpenGL.framework/Versions/4/Headers

  LIBS        += -L/System/Library/Frameworks/OpenGL.framework/Libraries
  LIBS        += -L/Users/eb/us3/lib
  LIBS        += -lssl -lcrypto -lqwtplot3d-qt4
  LIBS        += -framework QtOpenGL
  LIBS        += -L/Users/eb/us2a/develop/extra/us3_somo/lib

#  X11LIB       = -L/usr/X11R6/lib -lXau -lX11
}

# macx { RC_FILE = us_somo.icns }

#The following line was inserted by qt3to4
QT += qt3support 
