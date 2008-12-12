include( /opt/qt-4.5.0/addons/qtsingleapplication-2.5_1-commercial/src/qtsingleapplication.pri )
include( ../../gui.pri )


TARGET        = us
TRANSLATIONS += $${TARGET}_de_DE.ts

HEADERS      += us.h

SOURCES      += us.cpp          \
                us_win_data.cpp


