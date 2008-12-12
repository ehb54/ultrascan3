# Profile include file for libraries

TEMPLATE     = lib

QWTPATH      = /usr/local/qwt-5.1.1

CONFIG       += debug qt thread warn
DEPENDPATH   += ../gui ../utils $$QWTPATH/include
INCLUDEPATH  += ../gui ../utils $$QWTPATH/include

DESTDIR      = ../lib

VERSION      = 10.0

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

DEFINES     += LINUX INTEL

