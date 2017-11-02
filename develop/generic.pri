# Generic.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the UltraScan main programs.

# Messages
include ( local.pri )

TEMPLATE        = app
DEPENDPATH     += $$US3SOMOPATH/src $$US3SOMOPATH/include
INCLUDEPATH    += $$US3SOMOPATH/include
SOURCES         = main.cpp 
DESTDIR         = $$US3SOMOPATH/bin

DEFINES += NO_DB
# temporary fix (us2 code was using qt2 qpdevmnt which I think need to be replaced by qprintdevicemetrics)
DEFINES += NO_EDITOR_PRINT

# OSX also reports UNIX
contains( DEFINES, "OSX" ) {
} else {
    unix {
     UNAME                  = $$system(uname -a)
     DEFINES               += UNIX
     QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor
    
     contains(UNAME,x86_64) {
        LIBS    += -L$$QWTPATH/lib -lqwt
        DESTDIR  = $$US3SOMOPATH/bin64
     } else {
        LIBS    += -L$$QWTPATH/lib -lqwt
     }
    
     LIBS +=  -L$$US3SOMOPATH/lib -lus_somo
    }
    LIBS += -L$$US3PATH/lib -l$$QWT3DLIBNAME
}

win32 {

  VER = 10
  DEFINES      += MINGW

  # QWT3D is right for libraries, but gui apps need ../$$QWT3D
  # due to us3 directory structure

  QWT3D       = ../qwtplot3d
  ##OPENSSL     = C:/openssl
  ##OPENSSL     = C:/mingw64/opt
  OPENSSL     = C:/utils/openssl
  MYSQLPATH   = C:/utils/mysql
  MYSQLDIR    = $$MYSQLPATH/lib
  QTMYSQLPATH = C:/utils/Qt/5.4.1/plugins/sqldrivers
  QTPATH      = C:/utils/Qt/5.4.1
  QMAKESPEC   = $$QTPATH/mkspecs/win32-g++
  QTMAKESPEC  = $$QMAKESPEC
  QWTPATH     = C:/utils/Qwt/6.1.2
  SINGLEDIR   = C:/utils/Qt/5.4.1/addons/qtsingleapplication-2.6_1-opensource/src/
  MINGWDIR    = C:/mingw64/x86_64-w64-mingw32
  
  contains( DEBUGORRELEASE, debug ) {
    QWTLIB      = $$QWTPATH/lib/libqwtd.a
    MYSQLLIB    = $$MYSQLDIR/libmysqld.lib
  } else {
    QWTLIB      = $$QWTPATH/lib/libqwt.a
    MYSQLLIB    = $$MYSQLDIR/libmysql.lib
    INCLUDEPATH += c:/mingw64/opt/include
  }
  ##LIBS        += $$MYSQLLIB
  LIBS        += -L$$MYSQLDIR -lmysql
  LIBS        += -lpsapi

  #  __LCC__ is needed on W32 to make mysql headers include the right W32 includes
  ##DEFINES    += __LCC__
  DEFINES    += __WIN64__
  LIBS         += $$US3SOMOPATH/bin/libus_somo$${VER}.a
}

macx {
  DESTDIR      = $$US3SOMOPATH/bin
  LIBS        += -L$$US3SOMOPATH/lib -lus_somo
}

# macx { RC_FILE = us_somo.icns }

