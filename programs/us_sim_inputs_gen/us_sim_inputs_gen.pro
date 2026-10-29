# us_sim_inputs_gen is a headless command-line tool: no GUI libraries.
include( ../../local.pri )

TEMPLATE     = app
TARGET       = us_sim_inputs_gen
DESTDIR      = ../../bin
MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj
VER          = 10

QT          -= gui
QT          += xml
CONFIG      += $$DEBUGORRELEASE qt thread warn console
CONFIG      -= app_bundle

DEPENDPATH  += ../../utils ..
INCLUDEPATH += ../../utils ..

unix:!macx {
  LIBS      += -L../../lib -lus_utils
  LIBS      += -lcrypto
  DEFINES   += INTEL LINUX
}

win32 {
  LIBS      += -L../../lib -lus_utils$${VER}
  LIBS      += -L$$OPENSSL/lib -lssl -lcrypto
  INCLUDEPATH += $$OPENSSL/include $$QTPATH/include
  DEFINES   += INTEL
}

macx {
  LIBS      += -L../../lib -lus_utils
  DEFINES   += MAC OSX
}

# libus_utils uses MySQL only for its database support (US_DB2), and does not
# carry that dependency on every platform, so link the client library here
# too. NO_DB builds compile US_DB2 without MySQL and need none.
!contains( DEFINES, NO_DB ) {
  unix:!macx {
    LIBS      += -L$$MYSQLDIR -lmysqlclient
    INCLUDEPATH += $$MYSQLPATH
  }
  win32 {
    LIBS      += $$MYSQLLIB
    INCLUDEPATH += $$MYSQLPATH/include
  }
  macx {
    LIBS      += -lmysqlclient
    INCLUDEPATH += $$MYSQLPATH/include
  }
}

HEADERS      = us_sim_inputs_gen_cli.h
SOURCES      = us_sim_inputs_gen.cpp
