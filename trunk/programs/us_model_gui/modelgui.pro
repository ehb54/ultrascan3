include( ../../gui.pri )

TARGET       = us_model_gui
DESTDIR      = ../../bin

# Input
SOURCES     += main.cpp                \
               us_associations_gui.cpp \
               us_model_gui.cpp        \
               us_properties.cpp       

HEADERS     += us_associations_gui.h   \
               us_model_gui.h          \
               us_properties.h

