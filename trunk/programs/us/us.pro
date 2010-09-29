include( ../../gui.pri )
include( $${SINGLEDIR}/qtsingleapplication.pri )

TARGET        = us
TRANSLATIONS += $${TARGET}_de_DE.ts

revision.target           = us_revision.h
revision.commands         = sh revision.sh
revision.depends          = FORCE
unix:QMAKE_EXTRA_TARGETS += revision

HEADERS      += us.h            \
                us_revision.h

SOURCES      += us.cpp          \
                us_win_data.cpp


