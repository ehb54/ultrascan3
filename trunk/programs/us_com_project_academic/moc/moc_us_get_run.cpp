/****************************************************************************
** Meta object code from reading C++ file 'us_get_run.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../us_convert/us_get_run.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_get_run.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_US_GetRun_t {
    QByteArrayData data[14];
    char stringdata0[144];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_GetRun_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_GetRun_t qt_meta_stringdata_US_GetRun = {
    {
QT_MOC_LITERAL(0, 0, 9), // "US_GetRun"
QT_MOC_LITERAL(1, 10, 12), // "dkdb_changed"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 2), // "DB"
QT_MOC_LITERAL(4, 27, 7), // "load_db"
QT_MOC_LITERAL(5, 35, 9), // "load_disk"
QT_MOC_LITERAL(6, 45, 16), // "sel_investigator"
QT_MOC_LITERAL(7, 62, 19), // "assign_investigator"
QT_MOC_LITERAL(8, 82, 6), // "select"
QT_MOC_LITERAL(9, 89, 9), // "deleteRun"
QT_MOC_LITERAL(10, 99, 13), // "populate_list"
QT_MOC_LITERAL(11, 113, 14), // "update_disk_db"
QT_MOC_LITERAL(12, 128, 10), // "limit_data"
QT_MOC_LITERAL(13, 139, 4) // "help"

    },
    "US_GetRun\0dkdb_changed\0\0DB\0load_db\0"
    "load_disk\0sel_investigator\0"
    "assign_investigator\0select\0deleteRun\0"
    "populate_list\0update_disk_db\0limit_data\0"
    "help"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_GetRun[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   72,    2, 0x08 /* Private */,
       5,    0,   73,    2, 0x08 /* Private */,
       6,    0,   74,    2, 0x08 /* Private */,
       7,    1,   75,    2, 0x08 /* Private */,
       8,    0,   78,    2, 0x08 /* Private */,
       9,    0,   79,    2, 0x08 /* Private */,
      10,    0,   80,    2, 0x08 /* Private */,
      11,    1,   81,    2, 0x08 /* Private */,
      12,    1,   84,    2, 0x08 /* Private */,
      13,    0,   87,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void US_GetRun::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_GetRun *_t = static_cast<US_GetRun *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->dkdb_changed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->load_db(); break;
        case 2: _t->load_disk(); break;
        case 3: _t->sel_investigator(); break;
        case 4: _t->assign_investigator((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->select(); break;
        case 6: _t->deleteRun(); break;
        case 7: _t->populate_list(); break;
        case 8: _t->update_disk_db((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->limit_data((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->help(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_GetRun::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_GetRun::dkdb_changed)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject US_GetRun::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_GetRun.data,
      qt_meta_data_US_GetRun,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_GetRun::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_GetRun::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_GetRun.stringdata0))
        return static_cast<void*>(const_cast< US_GetRun*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_GetRun::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void US_GetRun::dkdb_changed(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
