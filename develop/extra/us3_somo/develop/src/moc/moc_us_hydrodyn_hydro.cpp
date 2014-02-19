/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_hydro.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_hydro.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_hydro.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Hydro[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x08,
      34,   30,   18,   18, 0x08,
      63,   30,   18,   18, 0x08,
      95,   30,   18,   18, 0x08,
     123,   30,   18,   18, 0x08,
     157,   30,   18,   18, 0x08,
     189,   18,   18,   18, 0x08,
     209,   30,   18,   18, 0x08,
     232,   30,   18,   18, 0x08,
     253,   30,   18,   18, 0x08,
     277,   18,   18,   18, 0x08,
     306,   18,   18,   18, 0x08,
     332,   18,   18,   18, 0x08,
     362,   18,   18,   18, 0x08,
     390,   18,   18,   18, 0x08,
     410,   18,   18,   18, 0x08,
     437,   18,   18,   18, 0x08,
     460,   18,   18,   18, 0x08,
     477,   18,   18,   18, 0x08,
     493,   18,   18,   18, 0x08,
     518,   18,   18,   18, 0x08,
     527,   18,   18,   18, 0x08,
     534,   18,   18,   18, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Hydro[] = {
    "US_Hydrodyn_Hydro\0\0setupGUI()\0str\0"
    "update_solvent_name(QString)\0"
    "update_solvent_acronym(QString)\0"
    "update_temperature(QString)\0"
    "update_solvent_viscosity(QString)\0"
    "update_solvent_density(QString)\0"
    "update_unit(double)\0update_volume(QString)\0"
    "update_mass(QString)\0update_overlap(QString)\0"
    "select_reference_system(int)\0"
    "select_boundary_cond(int)\0"
    "select_volume_correction(int)\0"
    "select_mass_correction(int)\0"
    "select_overlap(int)\0select_bead_inclusion(int)\0"
    "set_solvent_defaults()\0set_rotational()\0"
    "set_viscosity()\0check_solvent_defaults()\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Hydro::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Hydro *_t = static_cast<US_Hydrodyn_Hydro *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_solvent_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_solvent_acronym((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_temperature((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_solvent_viscosity((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_solvent_density((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_unit((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->update_volume((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_mass((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_overlap((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->select_reference_system((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->select_boundary_cond((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->select_volume_correction((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->select_mass_correction((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->select_overlap((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->select_bead_inclusion((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->set_solvent_defaults(); break;
        case 17: _t->set_rotational(); break;
        case 18: _t->set_viscosity(); break;
        case 19: _t->check_solvent_defaults(); break;
        case 20: _t->cancel(); break;
        case 21: _t->help(); break;
        case 22: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Hydro::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Hydro::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Hydro,
      qt_meta_data_US_Hydrodyn_Hydro, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Hydro::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Hydro::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Hydro::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Hydro))
        return static_cast<void*>(const_cast< US_Hydrodyn_Hydro*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Hydro::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
