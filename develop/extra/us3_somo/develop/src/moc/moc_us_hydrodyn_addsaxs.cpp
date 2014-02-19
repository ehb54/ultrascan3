/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_addsaxs.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_addsaxs.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_addsaxs.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_AddSaxs[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      18,   11,   11,   11, 0x08,
      25,   11,   11,   11, 0x08,
      39,   11,   11,   11, 0x08,
      50,   11,   11,   11, 0x08,
      76,   11,   11,   11, 0x08,
      95,   11,   11,   11, 0x08,
     114,   11,   11,   11, 0x08,
     133,   11,   11,   11, 0x08,
     152,   11,   11,   11, 0x08,
     171,   11,   11,   11, 0x08,
     190,   11,   11,   11, 0x08,
     209,   11,   11,   11, 0x08,
     228,   11,   11,   11, 0x08,
     246,   11,   11,   11, 0x08,
     269,   11,   11,   11, 0x08,
     286,   11,   11,   11, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_AddSaxs[] = {
    "US_AddSaxs\0\0add()\0help()\0select_file()\0"
    "setupGUI()\0update_saxs_name(QString)\0"
    "update_a1(QString)\0update_a2(QString)\0"
    "update_a3(QString)\0update_a4(QString)\0"
    "update_b1(QString)\0update_b2(QString)\0"
    "update_b3(QString)\0update_b4(QString)\0"
    "update_c(QString)\0update_volume(QString)\0"
    "select_saxs(int)\0closeEvent(QCloseEvent*)\0"
};

void US_AddSaxs::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_AddSaxs *_t = static_cast<US_AddSaxs *>(_o);
        switch (_id) {
        case 0: _t->add(); break;
        case 1: _t->help(); break;
        case 2: _t->select_file(); break;
        case 3: _t->setupGUI(); break;
        case 4: _t->update_saxs_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_a1((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_a2((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_a3((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_a4((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_b1((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->update_b2((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->update_b3((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->update_b4((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->update_c((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->update_volume((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: _t->select_saxs((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_AddSaxs::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_AddSaxs::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_US_AddSaxs,
      qt_meta_data_US_AddSaxs, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_AddSaxs::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_AddSaxs::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_AddSaxs::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_AddSaxs))
        return static_cast<void*>(const_cast< US_AddSaxs*>(this));
    return QWidget::qt_metacast(_clname);
}

int US_AddSaxs::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
