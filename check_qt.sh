#!/bin/bash
export OPENSSL_LIBS='-L/usr/lib64 -lssl -lcrypto'
./configure -prefix /opt/qt-5.13.2 -release -opensource -confirm-license \
 -platform linux-g++-64 \
 -nomake tests -nomake examples \
 -opengl desktop \
 -qt-xcb \
 -plugin-sql-mysql -plugin-sql-psql -openssl-linked \
 -system-proxies \
 -D QT_SHAREDMEMORY -D QT_SYSTEMSEMAPHORE \
 -no-icu \
 "$@"
