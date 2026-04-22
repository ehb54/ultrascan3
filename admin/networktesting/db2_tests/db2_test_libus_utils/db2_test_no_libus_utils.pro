TEMPLATE = app
TARGET   = db2_test_libus_utils
CONFIG  += console c++11
QT      += core network

INCLUDEPATH += ../../../utils
INCLUDEPATH += /mariadb-client-openssl111/include/mariadb
INCLUDEPATH += /openssl-1.1.1/include

LIBS += -L../../../lib -lus_utils -lus_gui -lxml2

QMAKE_RPATHDIR += '$ORIGIN/../../lib'
QMAKE_RPATHDIR += /qt-5.15.17/lib
QMAKE_LFLAGS   += -Wl,--enable-new-dtags -Wl,--as-needed

SOURCES += main.cpp 

