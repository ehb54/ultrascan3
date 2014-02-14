include( ../../gui.pri )

QT           += xml
win32 {
CONFIG       += console
}

TARGET        = us_ramp

HEADERS       = us_ramp.h               \
                us_ramp_gui.h           \
                us_mwlramp.h            \
                us_get_dbrun_ra.h       \
                us_experiment_ra.h      \
                us_experiment_gui_ra.h  \
                us_intensity_ra.h       \
                us_selectbox_ra.h       \
                us_select_triples_ra.h

SOURCES       = us_ramp.cpp               \
                us_ramp_gui.cpp           \
                us_mwlramp.cpp            \
                us_get_dbrun_ra.cpp       \
                us_experiment_ra.cpp      \
                us_experiment_gui_ra.cpp  \
                us_intensity_ra.cpp       \
                us_selectbox_ra.cpp       \
                us_select_triples_ra.cpp

