/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_bd.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_bd.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_bd.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_BD[] = {

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
      16,   15,   15,   15, 0x08,
      27,   15,   15,   15, 0x08,
      36,   15,   15,   15, 0x08,
      43,   15,   15,   15, 0x08,
      56,   15,   15,   15, 0x08,
      74,   15,   15,   15, 0x08,
      81,   15,   15,   15, 0x08,
      94,   15,   15,   15, 0x08,
     104,   15,   15,   15, 0x08,
     114,   15,   15,   15, 0x08,
     123,   15,   15,   15, 0x08,
     141,   15,   15,   15, 0x08,
     159,   15,   15,   15, 0x08,
     174,   15,   15,   15, 0x08,
     189,   15,   15,   15, 0x08,
     203,   15,   15,   15, 0x08,
     226,   15,   15,   15, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_BD[] = {
    "US_Hydrodyn_BD\0\0setupGUI()\0cancel()\0"
    "help()\0bd_options()\0anaflex_options()\0"
    "stop()\0bd_prepare()\0bd_load()\0bd_edit()\0"
    "bd_run()\0bd_load_results()\0anaflex_prepare()\0"
    "anaflex_load()\0anaflex_edit()\0"
    "anaflex_run()\0anaflex_load_results()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_BD::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_BD *_t = static_cast<US_Hydrodyn_BD *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->cancel(); break;
        case 2: _t->help(); break;
        case 3: _t->bd_options(); break;
        case 4: _t->anaflex_options(); break;
        case 5: _t->stop(); break;
        case 6: _t->bd_prepare(); break;
        case 7: _t->bd_load(); break;
        case 8: _t->bd_edit(); break;
        case 9: _t->bd_run(); break;
        case 10: _t->bd_load_results(); break;
        case 11: _t->anaflex_prepare(); break;
        case 12: _t->anaflex_load(); break;
        case 13: _t->anaflex_edit(); break;
        case 14: _t->anaflex_run(); break;
        case 15: _t->anaflex_load_results(); break;
        case 16: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_BD::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_BD::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_BD,
      qt_meta_data_US_Hydrodyn_BD, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_BD::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_BD::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_BD::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_BD))
        return static_cast<void*>(const_cast< US_Hydrodyn_BD*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_BD::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
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
