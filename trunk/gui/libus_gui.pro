TEMPLATE     = lib

QWTPATH      = /usr/local/qwt-5.1.1

CONFIG      += debug qt thread warn
TARGET       = us_gui
DEPENDPATH   += ../utils $$QWTPATH/include
INCLUDEPATH  += ../utils $$QWTPATH/include

DESTDIR      = ../lib

TRANSLATIONS = us_gui.ts
VERSION      = 10.0

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

HEADERS      = us_crypto.h       \
               us_gui_settings.h \
               us_help.h         \
               us_license.h      \
               us_passwd.h       \
               us_widgets.h

SOURCES      = us_crypto.cpp       \
               us_gui_settings.cpp \ 
               us_help.cpp         \
               us_license.cpp      \
               us_passwd.cpp       \
               us_widgets.cpp

DEFINES      += LINUX INTEL
