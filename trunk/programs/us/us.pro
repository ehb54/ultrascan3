include( /opt/qt-4.5.0/addons/qtsingleapplication-2.5_1-commercial/src/qtsingleapplication.pri )
include( ../../gui.pri )

TARGET        = us
TRANSLATIONS += $${TARGET}_de_DE.ts

revision.target           = us_revision.h
revision.commands         = sh revision.sh
revision.depends          = FORCE
QMAKE_EXTRA_UNIX_TARGETS += revision

HEADERS      += us.h            \
                us_revision.h

SOURCES      += us.cpp          \
                us_win_data.cpp


