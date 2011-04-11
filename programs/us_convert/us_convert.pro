include( ../../gui.pri )

QT           += xml

TARGET        = us_convert

HEADERS       = us_convert_gui.h       \
                us_convert.h           \
                us_convertio.h         \
                us_experiment.h        \
                us_experiment_gui.h    \
                us_selectbox.h         \
                us_intensity.h         \
                us_get_dbrun.h

SOURCES       = us_convert_gui.cpp     \
                us_convert.cpp         \
                us_convertio.cpp       \
                us_experiment.cpp      \
                us_experiment_gui.cpp  \
                us_selectbox.cpp       \
                us_intensity.cpp       \
                us_get_dbrun.cpp

