US3PATH   = $$PWD
US3SOMOPATH = $$US3PATH/us_somo

INCLUDEPATH += $$US3SOMOPATH/develop/include

QWT3DLIBNAME =

count (QWT3DLIBNAME, 0 ) {
        QWT3DLIBNAME = qwtplot3d
}

INCLUDEPATH += $$QWT3DPATH/include
INCLUDEPATH += $$QWTPATH/include $$QWTPATH/src

QT += network
QT += svg
QT += widgets
QT += printsupport
QT += multimedia
LIBS *= -lz

include ($$US3SOMOPATH/develop/uname.pri)
