include( ../../gui.pri )

QT           += xml

TARGET        = us_convert

HEADERS       = us_convert.h           \
                us_process_convert.h   \
                us_convertio.h         \
                us_expinfo.h           \
                us_selectbox.h 

SOURCES       = us_convert.cpp         \
                us_process_convert.cpp \
                us_convertio.cpp       \
                us_expinfo.cpp         \
                us_selectbox.cpp

