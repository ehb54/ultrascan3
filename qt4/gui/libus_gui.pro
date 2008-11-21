TEMPLATE     = lib

CONFIG      += debug qt thread warn
TARGET       = us_gui
DEPENDPATH   += ../utils 
INCLUDEPATH  += ../utils 

DESTDIR      = ../../lib

TRANSLATIONS = us_gui.ts
VERSION      = 10.0

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

HEADERS      = us_gui_settings.h \
               us_help.h         \
               us_license.h      \
               us_widgets.h

SOURCES      = us_gui_settings.cpp \ 
               us_help.cpp         \
               us_license.cpp      \
               us_widgets.cpp

DEFINES      += LINUX INTEL
