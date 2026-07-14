TEMPLATE = app
TARGET   = db2_test_no_libus_utils
CONFIG  += console c++11
QT      += core network

DEFINES += NO_DB

INCLUDEPATH += /mariadb-client-openssl111/include/mariadb
INCLUDEPATH += /openssl-1.1.1/include
LIBS += -Wl,--start-group \
/mariadb-client-openssl111/lib/mariadb/libmariadb.a \
/usr/lib/libarchive.a \
-lz -lzstd \
-Wl,--end-group

SOURCES += main.cpp us_db2.cpp

HEADERS += us_db2.h
