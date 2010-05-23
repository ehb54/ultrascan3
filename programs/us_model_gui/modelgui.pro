include( ../../gui.pri )

TARGET       = us_model
DESTDIR      = ../../bin

# Input
SOURCES     += main.cpp                \
               us_properties.cpp       \
               us_associations_gui.cpp \
               us_model_editor_new2.cpp

HEADERS     += us_modelgui.h           \
               us_properties.h         \
               us_associations_gui.h   \
               us_model_editor_new2.h

