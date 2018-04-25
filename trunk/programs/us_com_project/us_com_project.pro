include( ../../gui.pri )

TARGET        = us_comproject
QT           += xml
QT           += sql

HEADERS       = us_com_project_gui.h \
                ../us_experiment/us_experiment_gui.h                
                
SOURCES       = us_com_project_main.cpp \
                us_com_project_gui.cpp \                
                ../us_experiment/us_experiment_gui.cpp \
                ../us_experiment/us_exp_utils.cpp \ 
                ../us_experiment/us_proto_ranges.cpp \
                ../us_experiment/us_run_protocol.cpp 
                
                
