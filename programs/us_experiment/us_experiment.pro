include( ../../gui.pri )

TARGET        = us_experiment
QT           += xml
QT           += sql
QT           += network
QT           += printsupport

HEADERS       = us_experiment_gui_optima.h \
                ../us_analysis_profile/us_analysis_profile.h \
                ../us_esigner_gmp/us_esigner_gmp.h \
                ../us_convert/us_convert.h

SOURCES       = us_experiment_main.cpp       \
                us_experiment_gui_optima.cpp \
                us_exp_utils.cpp             \
                us_proto_ranges.cpp          \
                ../us_analysis_profile/us_analysis_profile.cpp \
                ../us_analysis_profile/us_anapro_utils.cpp  \
                ../us_esigner_gmp/us_esigner_gmp.cpp \
                ../us_convert/us_convert.cpp
                
                

