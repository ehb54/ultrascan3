include( ../../gui.pri )

QT           += xml

TARGET        = us_manage_data

HEADERS       = us_manage_data.h \
                ../us_convert/us_convertio.h \
                ../us_convert/us_expinfo.h \
                ../us_convert/us_selectbox.h

SOURCES       = us_manage_data.cpp \
                ../us_convert/us_convertio.cpp \
                ../us_convert/us_expinfo.cpp \
                ../us_convert/us_selectbox.cpp

INCLUDEPATH  += ../us_convert
