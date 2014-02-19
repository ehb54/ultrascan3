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
 UNAME                  = $$system(uname -a)
 CONFIG                += qt warn thread release
 DEFINES               += UNIX
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor
 MYSQLPATH              = /usr/lib/mysql

 contains(UNAME,x86_64) {
    LIBS    += -L$(QWTDIR)/lib64/ -L$(QWTDIR)/lib/ -lqwt -L$(ULTRASCAN)/somo/lib64 -lus_somo
    DESTDIR  = ../../bin64
 } else {
    LIBS    += -L$(QWTDIR)/lib -lqwt -L$(ULTRASCAN)/somo/lib -lus_somo
 }
}

win32 {
  TEMPLATE     =app
  MINGWDIR     =c:/mingw
  MYSQLPATH    =c:/mysql-5.5
  QTMYSQLPATH  =$(QTDIR)/src/plugins/sqldrivers/mysql/release
  MYSQLLIB     =$$MYSQLPATH/lib/libmysql.a
  OPENSSL      =c:/openssl
  VER          =10
  CONFIG      += qt thread warn release
  INCLUDEPATH  += $$QWTPATH/src ..
  INCLUDEPATH  += $$MYSQLPATH/include ../$$QWT3D/include
  INCLUDEPATH  += $$OPENSSL/include
  INCLUDEPATH  += $$QTPATH/include
  LIBS         += $$QWTLIB
  LIBS         += $$MYSQLLIB
  LIBS         += $$QTMYSQLPATH/libqsqlmysql4.a
  LIBS         += $$OPENSSL/lib/libeay32.a
  LIBS         += $$MINGWDIR/lib/libws2_32.a $$MINGWDIR/lib/libadvapi32.a
  LIBS         += $$MINGWDIR/lib/libgdi32.a $$MINGWDIR/lib/libuser32.a
  LIBS         += ../../bin/libus_somo$${VER}.a
}

macx { RC_FILE = ultrascan.icns }

#The following line was inserted by qt3to4
QT += qt3support 
