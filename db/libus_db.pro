TEMPLATE     = lib

CONFIG      += debug qt thread warn
TARGET       = us_db
DEPENDPATH  += ../utils ../gui
INCLUDEPATH += ../utils ../gui
DESTDIR      = ../lib

# gui is needed for now because us_crypto can pop
# up a window asking for teh master password
#QT          -= gui
QT          += sql

TRANSLATIONS = us_utils.ts
VERSION      = 10.0

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

HEADERS      = us_db.h    

SOURCES      = us_db.cpp

DEFINES      += LINUX INTEL
