include( ../../gui.pri )

QT           += xml
win32 {
CONFIG       += console
}

TARGET        = us_ramp

HEADERS       = us_ramp.h           \
                us_ramp_gui.h       \
                us_mwlramp.h        \
#                 us_convertio.h         \
                us_get_dbrun.h         \
                us_experiment.h        \
                us_experiment_gui.h    \
                us_intensity.h         \
                us_selectbox.h         \
                us_select_triples.h

SOURCES       = us_ramp.cpp         \
                us_ramp_gui.cpp     \
                us_mwlramp.cpp      \
#                 us_convertio.cpp       \
                us_get_dbrun.cpp       \
                us_experiment.cpp      \
                us_experiment_gui.cpp  \
                us_intensity.cpp       \
                us_selectbox.cpp       \
                us_select_triples.cpp

