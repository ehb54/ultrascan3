/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_hydro_zeno.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_hydro_zeno.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_hydro_zeno.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Hydro_Zeno[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x08,
      35,   23,   23,   23, 0x08,
      55,   51,   23,   23, 0x08,
      87,   23,   23,   23, 0x08,
     107,   51,   23,   23, 0x08,
     143,   23,   23,   23, 0x08,
     162,   51,   23,   23, 0x08,
     197,   51,   23,   23, 0x08,
     236,   23,   23,   23, 0x08,
     245,   23,   23,   23, 0x08,
     252,   23,   23,   23, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Hydro_Zeno[] = {
    "US_Hydrodyn_Hydro_Zeno\0\0setupGUI()\0"
    "set_zeno_zeno()\0str\0update_zeno_zeno_steps(QString)\0"
    "set_zeno_interior()\0"
    "update_zeno_interior_steps(QString)\0"
    "set_zeno_surface()\0"
    "update_zeno_surface_steps(QString)\0"
    "update_zeno_surface_thickness(QString)\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Hydro_Zeno::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Hydro_Zeno *_t = static_cast<US_Hydrodyn_Hydro_Zeno *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->set_zeno_zeno(); break;
        case 2: _t->update_zeno_zeno_steps((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->set_zeno_interior(); break;
        case 4: _t->update_zeno_interior_steps((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->set_zeno_surface(); break;
        case 6: _t->update_zeno_surface_steps((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_zeno_surface_thickness((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->cancel(); break;
        case 9: _t->help(); break;
        case 10: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Hydro_Zeno::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Hydro_Zeno::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Hydro_Zeno,
      qt_meta_data_US_Hydrodyn_Hydro_Zeno, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Hydro_Zeno::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Hydro_Zeno::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Hydro_Zeno::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Hydro_Zeno))
        return static_cast<void*>(const_cast< US_Hydrodyn_Hydro_Zeno*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Hydro_Zeno::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
