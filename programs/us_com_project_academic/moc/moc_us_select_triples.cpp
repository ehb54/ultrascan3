/****************************************************************************
** Meta object code from reading C++ file 'us_select_triples.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../us_convert/us_select_triples.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_select_triples.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_US_SelectTriples_t {
    QByteArrayData data[9];
    char stringdata0[87];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_SelectTriples_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_SelectTriples_t qt_meta_stringdata_US_SelectTriples = {
    {
QT_MOC_LITERAL(0, 0, 16), // "US_SelectTriples"
QT_MOC_LITERAL(1, 17, 15), // "new_triple_list"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 14), // "add_selections"
QT_MOC_LITERAL(4, 49, 14), // "rmv_selections"
QT_MOC_LITERAL(5, 64, 6), // "cancel"
QT_MOC_LITERAL(6, 71, 4), // "done"
QT_MOC_LITERAL(7, 76, 5), // "reset"
QT_MOC_LITERAL(8, 82, 4) // "help"

    },
    "US_SelectTriples\0new_triple_list\0\0"
    "add_selections\0rmv_selections\0cancel\0"
    "done\0reset\0help"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_SelectTriples[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   52,    2, 0x08 /* Private */,
       4,    0,   53,    2, 0x08 /* Private */,
       5,    0,   54,    2, 0x08 /* Private */,
       6,    0,   55,    2, 0x08 /* Private */,
       7,    0,   56,    2, 0x08 /* Private */,
       8,    0,   57,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QStringList,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void US_SelectTriples::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_SelectTriples *_t = static_cast<US_SelectTriples *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->new_triple_list((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 1: _t->add_selections(); break;
        case 2: _t->rmv_selections(); break;
        case 3: _t->cancel(); break;
        case 4: _t->done(); break;
        case 5: _t->reset(); break;
        case 6: _t->help(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_SelectTriples::*_t)(QStringList );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_SelectTriples::new_triple_list)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject US_SelectTriples::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_SelectTriples.data,
      qt_meta_data_US_SelectTriples,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_SelectTriples::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_SelectTriples::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_SelectTriples.stringdata0))
        return static_cast<void*>(const_cast< US_SelectTriples*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_SelectTriples::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void US_SelectTriples::new_triple_list(QStringList _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
