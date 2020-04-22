include( ../../gui.pri )

QT           += opengl sql xml network

TARGET        = us_xpn_viewer


HEADERS       = us_xpn_viewer_gui.h    \
                us_xpn_run_auc.h   \
                us_xpn_run_raw.h \
                us_link.h

SOURCES       = us_xpn_viewer_main.cpp  \
                us_xpn_viewer_gui.cpp  \
                us_xpn_run_auc.cpp \
                us_xpn_run_raw.cpp \
                us_link.cpp
                

