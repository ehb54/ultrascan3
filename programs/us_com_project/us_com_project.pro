include( ../../gui.pri )

TARGET        = us_comproject
QT           += opengl
QT           += sql
QT           += xml

HEADERS       = us_com_project_gui.h \
                ../us_experiment/us_experiment_gui.h \
                ../us_xpn_viewer/us_xpn_viewer_gui.h \
                ../us_xpn_viewer/us_xpn_run_raw.h \
                ../us_xpn_viewer/us_xpn_run_auc.h
                
                
SOURCES       = us_com_project_main.cpp \
                us_com_project_gui.cpp \                
                ../us_experiment/us_experiment_gui.cpp \
                ../us_experiment/us_exp_utils.cpp \ 
                ../us_experiment/us_proto_ranges.cpp \
                ../us_experiment/us_run_protocol.cpp \
                ../us_xpn_viewer/us_xpn_viewer_gui.cpp \
                ../us_xpn_viewer/us_xpn_run_raw.cpp \
                ../us_xpn_viewer/us_xpn_run_auc.cpp
                
                
