# us_import_run is a headless command-line tool: no GUI libraries.
include( ../../local.pri )

TEMPLATE     = app
TARGET       = us_import_run
DESTDIR      = ../../bin
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj
VER          = 10

QT          -= gui
QT          += xml
QT          += sql
QT          += network
CONFIG      += $$DEBUGORRELEASE qt thread warn console
CONFIG      -= app_bundle

DEPENDPATH  += ../../utils ..
INCLUDEPATH += ../../utils ..

unix:!macx {
  LIBS      += -L../../lib -lus_utils
  LIBS      += -lcrypto
  LIBS      += -L$$MYSQLDIR -lmysqlclient
  INCLUDEPATH += $$MYSQLPATH
  DEFINES   += INTEL LINUX
}

win32 {
  LIBS      += -L../../lib -lus_utils$${VER}
  LIBS      += $$MYSQLLIB
  LIBS      += -L$$OPENSSL/lib -lssl -lcrypto
  INCLUDEPATH += $$MYSQLPATH/include $$OPENSSL/include $$QTPATH/include
  DEFINES   += INTEL
}

macx {
  LIBS      += -L../../lib -lus_utils -lmysqlclient
  INCLUDEPATH += $$MYSQLPATH/include
  DEFINES   += MAC OSX
}

SOURCES      = us_import_run.cpp
