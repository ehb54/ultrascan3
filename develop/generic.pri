# Generic.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the UltraScan main programs.

# Messages
!include ( local.pri ) error( "local.pri missing or corrupt.  Aborting..." )

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
        LIBS    += -L$$QWTPATH/lib64 -lqwt
        DESTDIR  = $$US3SOMOPATH/bin64
     } else {
        LIBS    += -L$$QWTPATH/lib -lqwt
     }
    
     LIBS +=  -L$$US3SOMOPATH/lib -lus_somo
    }
    LIBS += -L$$US3PATH/lib -l$$QWT3DLIBNAME
}

win32 {
  VER          = 10
  DEFINES      += MINGW

  LIBS         += $$QWTLIB
  LIBS         += $$MINGWPATH/lib/libws2_32.a $$MINGWPATH/lib/libadvapi32.a
  LIBS         += $$MINGWPATH/lib/libgdi32.a $$MINGWPATH/lib/libuser32.a
  LIBS         += $$US3SOMOPATH/bin/libus_somo$${VER}.a
}

macx {
  DESTDIR      = $$US3SOMOPATH/bin
  LIBS        += -L$$US3SOMOPATH/lib -lus_somo
}

# macx { RC_FILE = us_somo.icns }

