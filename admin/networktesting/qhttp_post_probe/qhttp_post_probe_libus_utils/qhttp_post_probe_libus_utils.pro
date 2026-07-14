TEMPLATE = app
TARGET   = qhttp_post_probe_libus_utils
CONFIG  += console c++11 no_link_prl
QT      += core network
QT      -= gui

# Where the utils headers live (adjust if different)
INCLUDEPATH += ../../../utils

SOURCES += main.cpp

# Link to your shared utils lib
LIBS += -L../../../lib -lus_utils -lus_gui -lxml2

# Keep the runtime loader on *your* libs first, without LD_LIBRARY_PATH
QMAKE_RPATHDIR += '$ORIGIN/../../lib'
QMAKE_RPATHDIR += /qt-5.15.17/lib
QMAKE_LFLAGS   += -Wl,--enable-new-dtags -Wl,--as-needed

# Be extra sure nothing drags in system OpenSSL/MariaDB/etc
QMAKE_LIBDIR -= /usr/lib64 /lib64
LIBS         -= -L/usr/lib64 -L/lib64 -lssl -lcrypto -lmariadb -lmysqlclient -larchive
