include( ../../gui.pri )

QT           += opengl sql

TARGET        = us_xpn_viewer


HEADERS       = us_xpn_viewer.h    \
                us_xpn_run_auc.h   \
                us_xpn_run_raw.h

SOURCES       = us_xpn_viewer.cpp  \
                us_xpn_run_auc.cpp \
                us_xpn_run_raw.cpp

