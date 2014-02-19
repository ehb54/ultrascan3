/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_misc.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_misc.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_misc.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Misc[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      33,   17,   17,   17, 0x08,
      44,   17,   17,   17, 0x08,
      55,   17,   17,   17, 0x08,
      72,   17,   17,   17, 0x08,
      88,   86,   17,   17, 0x08,
     120,   17,   17,   17, 0x08,
     141,   17,   17,   17, 0x08,
     174,   17,   17,   17, 0x08,
     198,   17,   17,   17, 0x08,
     224,   17,   17,   17, 0x08,
     248,   17,   17,   17, 0x08,
     277,   17,   17,   17, 0x08,
     303,   17,   17,   17, 0x08,
     327,   17,   17,   17, 0x08,
     360,   17,   17,   17, 0x08,
     390,   17,   17,   17, 0x08,
     419,   17,   17,   17, 0x08,
     440,   17,   17,   17, 0x08,
     457,   17,   17,   17, 0x08,
     474,   17,   17,   17, 0x08,
     494,   17,   17,   17, 0x08,
     503,   17,   17,   17, 0x08,
     510,   17,   17,   17, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Misc[] = {
    "US_Hydrodyn_Misc\0\0vbar_changed()\0"
    "setupGUI()\0set_vbar()\0set_pb_rule_on()\0"
    "select_vbar()\0,\0update_vbar_signal(float,float)\0"
    "update_vbar(QString)\0"
    "update_vbar_temperature(QString)\0"
    "update_hydrovol(double)\0"
    "update_avg_radius(double)\0"
    "update_avg_mass(double)\0"
    "update_avg_hydration(double)\0"
    "update_avg_volume(double)\0"
    "update_avg_vbar(double)\0"
    "update_target_e_density(QString)\0"
    "update_target_volume(QString)\0"
    "set_set_target_on_load_pdb()\0"
    "set_equalize_radii()\0set_hydro_supc()\0"
    "set_hydro_zeno()\0set_export_msroll()\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Misc::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Misc *_t = static_cast<US_Hydrodyn_Misc *>(_o);
        switch (_id) {
        case 0: _t->vbar_changed(); break;
        case 1: _t->setupGUI(); break;
        case 2: _t->set_vbar(); break;
        case 3: _t->set_pb_rule_on(); break;
        case 4: _t->select_vbar(); break;
        case 5: _t->update_vbar_signal((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 6: _t->update_vbar((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_vbar_temperature((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_hydrovol((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: _t->update_avg_radius((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 10: _t->update_avg_mass((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 11: _t->update_avg_hydration((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 12: _t->update_avg_volume((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 13: _t->update_avg_vbar((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 14: _t->update_target_e_density((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: _t->update_target_volume((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: _t->set_set_target_on_load_pdb(); break;
        case 17: _t->set_equalize_radii(); break;
        case 18: _t->set_hydro_supc(); break;
        case 19: _t->set_hydro_zeno(); break;
        case 20: _t->set_export_msroll(); break;
        case 21: _t->cancel(); break;
        case 22: _t->help(); break;
        case 23: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Misc::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Misc::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Misc,
      qt_meta_data_US_Hydrodyn_Misc, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Misc::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Misc::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Misc::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Misc))
        return static_cast<void*>(const_cast< US_Hydrodyn_Misc*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Misc::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void US_Hydrodyn_Misc::vbar_changed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
