include( ../../gui.pri )

QT           += xml
win32 {
CONFIG       += console
}

TARGET        = us_convert

HEADERS       = us_convert.h           \
                us_convert_gui.h       \
                us_convertio.h         \
                us_get_run.h           \
                us_experiment.h        \
                us_experiment_gui.h    \
                us_intensity.h         \
                us_selectbox.h         \
                us_select_triples.h

SOURCES       = us_convert.cpp         \
                us_convert_gui.cpp     \
                us_convertio.cpp       \
                us_get_run.cpp         \
                us_experiment.cpp      \
                us_experiment_gui.cpp  \
                us_intensity.cpp       \
                us_selectbox.cpp       \
                us_select_triples.cpp

