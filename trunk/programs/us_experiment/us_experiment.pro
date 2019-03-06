include( ../../gui.pri )

TARGET        = us_experiment
QT           += xml
QT           += sql

HEADERS       = us_experiment_gui_optima.h \
                ../us_analysis_profile/us_analysis_profile.h \
                ../us_analysis_profile/us_anapro_parms.h     \
                us_run_protocol.h      

SOURCES       = us_experiment_main.cpp       \
                us_experiment_gui_optima.cpp \
                us_exp_utils.cpp             \
                us_proto_ranges.cpp          \
                ../us_analysis_profile/us_analysis_profile.cpp \
                ../us_analysis_profile/us_anapro_utils.cpp     \
                ../us_analysis_profile/us_anapro_parms.cpp     \
                us_run_protocol.cpp

