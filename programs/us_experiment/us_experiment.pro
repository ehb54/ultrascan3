include( ../../gui.pri )

TARGET        = us_experiment
QT           += xml
QT           += sql

HEADERS       = us_experiment_main.h   \
                us_run_protocol.h

SOURCES       = us_experiment_main.cpp   \
                us_exp_utils.cpp         \
                us_run_protocol.cpp
