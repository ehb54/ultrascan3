include( ../../gui.pri )

TARGET        = us_experiment
QT           += xml
QT           += sql
QT           += network
QT           += printsupport

HEADERS       = us_experiment_gui_optima.h \
                ../us_analysis_profile/us_analysis_profile.h \
                ../us_esigner_gmp/us_esigner_gmp.h \
                ../us_convert/us_convert.h \
                ../us_convert/us_experiment.h \   
                ../us_convert/us_experiment_gui.h \
                ../us_convert/us_convert_gui.h \
                ../us_convert/us_convertio.h \
                 ../us_convert/us_get_run.h \
                ../us_convert/us_intensity.h \
                ../us_convert/us_selectbox.h \
                ../us_convert/us_select_triples.h \

SOURCES       = us_experiment_main.cpp       \
                us_experiment_gui_optima.cpp \
                us_exp_utils.cpp             \
                us_proto_ranges.cpp          \
                ../us_analysis_profile/us_analysis_profile.cpp \
                ../us_analysis_profile/us_anapro_utils.cpp  \
                ../us_esigner_gmp/us_esigner_gmp.cpp \
                ../us_convert/us_convert.cpp \
                ../us_convert/us_experiment.cpp \   
                ../us_convert/us_experiment_gui.cpp \
                ../us_convert/us_convert_gui.cpp \
                ../us_convert/us_convertio.cpp \
                ../us_convert/us_get_run.cpp \
                ../us_convert/us_intensity.cpp \
                ../us_convert/us_selectbox.cpp \
                ../us_convert/us_select_triples.cpp
                
                
                

