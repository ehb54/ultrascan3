
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
include ( ../local.pri )
include (somo.pri)

TEMPLATE        = app
DEPENDPATH     += $$US3SOMOPATH/src $$US3SOMOPATH/include

SOURCES         = main.cpp 

DEFINES         += CMDLINE
DEFINES         += NO_DB

DESTDIR         = $$US3PATH/bin

unix {
  contains(UNAME,x86_64) {
    DESTDIR         = $$US3PATH/bin
  }
}

win32 {
  error ("Command line not currently setup for the Microsoft Windows Platform...")
}

macx {
#  X11LIB       = -L/usr/X11R6/lib -lXau -lX11
}

#macx { RC_FILE = us_somo.icns }
