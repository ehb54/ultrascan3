include( ../../gui.pri )

TARGET        = us_experiment
QT           += xml
QT           += sql

HEADERS       = us_experiment_gui_optima.h \
                us_run_protocol.h      

SOURCES       = us_experiment_main.cpp   \
                us_experiment_gui_optima.cpp \
                us_exp_utils.cpp         \
                us_proto_ranges.cpp      \
                us_run_protocol.cpp
