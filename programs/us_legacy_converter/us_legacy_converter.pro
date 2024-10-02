include( ../../gui.pri )

TARGET        = us_legacy_converter
# QT           += xml printsupport

HEADERS       =  \
                 ../us_convert/us_convert.h \
                 us_legacy_converter.h

SOURCES       =  \
                 ../us_convert/us_convert.cpp \
                 us_legacy_converter.cpp
