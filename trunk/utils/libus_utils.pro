TEMPLATE     = lib

CONFIG      += debug qt thread warn
QT          -= gui
TARGET       = us_utils
DESTDIR      = ../lib

TRANSLATIONS = us_utils.ts
VERSION      = 10.0

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

HEADERS      = us_global.h    \
               us_license_t.h \
               us_settings.h

SOURCES      = us_global.cpp    \
               us_license_t.cpp \
               us_settings.cpp

DEFINES      += LINUX INTEL
