/****************************************************************************
** Meta object code from reading C++ file 'us_cmdline_app.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.13.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../include/us_cmdline_app.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_cmdline_app.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.13.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_US_Cmdline_App_t {
    QByteArrayData data[8];
    char stringdata0[92];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_Cmdline_App_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_Cmdline_App_t qt_meta_stringdata_US_Cmdline_App = {
    {
QT_MOC_LITERAL(0, 0, 14), // "US_Cmdline_App"
QT_MOC_LITERAL(1, 15, 14), // "readFromStdout"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 14), // "readFromStderr"
QT_MOC_LITERAL(4, 46, 8), // "finished"
QT_MOC_LITERAL(5, 55, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(6, 76, 7), // "started"
QT_MOC_LITERAL(7, 84, 7) // "timeout"

    },
    "US_Cmdline_App\0readFromStdout\0\0"
    "readFromStderr\0finished\0QProcess::ExitStatus\0"
    "started\0timeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_Cmdline_App[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    2,   41,    2, 0x08 /* Private */,
       6,    0,   46,    2, 0x08 /* Private */,
       7,    0,   47,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 5,    2,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void US_Cmdline_App::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<US_Cmdline_App *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->readFromStdout(); break;
        case 1: _t->readFromStderr(); break;
        case 2: _t->finished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 3: _t->started(); break;
        case 4: _t->timeout(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject US_Cmdline_App::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_US_Cmdline_App.data,
    qt_meta_data_US_Cmdline_App,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *US_Cmdline_App::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_Cmdline_App::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_US_Cmdline_App.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int US_Cmdline_App::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
