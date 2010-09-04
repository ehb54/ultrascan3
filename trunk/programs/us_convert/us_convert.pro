include( ../../gui.pri )

QT           += xml

TARGET        = us_convert

HEADERS       = us_convert.h           \
                us_process_convert.h   \
                us_convertio.h         \
                us_expinfo.h           \
                us_selectbox.h         \
                us_intensity.h         \
                us_get_dbrun.h

SOURCES       = us_convert.cpp         \
                us_process_convert.cpp \
                us_convertio.cpp       \
                us_expinfo.cpp         \
                us_selectbox.cpp       \
                us_intensity.cpp       \
                us_get_dbrun.cpp

