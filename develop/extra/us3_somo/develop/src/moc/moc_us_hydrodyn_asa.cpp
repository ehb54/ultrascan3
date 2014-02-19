/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_asa.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_asa.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_asa.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_ASA[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x08,
      28,   16,   16,   16, 0x08,
      56,   16,   16,   16, 0x08,
      92,   16,   16,   16, 0x08,
     121,   16,   16,   16, 0x08,
     158,   16,   16,   16, 0x08,
     192,   16,   16,   16, 0x08,
     234,   16,   16,   16, 0x08,
     260,   16,   16,   16, 0x08,
     296,   16,   16,   16, 0x08,
     329,   16,   16,   16, 0x08,
     361,   16,   16,   16, 0x08,
     388,   16,   16,   16, 0x08,
     411,   16,   16,   16, 0x08,
     433,   16,   16,   16, 0x08,
     450,   16,   16,   16, 0x08,
     460,   16,   16,   16, 0x08,
     469,   16,   16,   16, 0x08,
     476,   16,   16,   16, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_ASA[] = {
    "US_Hydrodyn_ASA\0\0setupGUI()\0"
    "update_probe_radius(double)\0"
    "update_probe_recheck_radius(double)\0"
    "update_asa_threshold(double)\0"
    "update_asa_threshold_percent(double)\0"
    "update_asa_grid_threshold(double)\0"
    "update_asa_grid_threshold_percent(double)\0"
    "update_asab1_step(double)\0"
    "update_hydrate_probe_radius(double)\0"
    "update_hydrate_threshold(double)\0"
    "update_vvv_probe_radius(double)\0"
    "update_vvv_grid_dR(double)\0"
    "select_asa_method(int)\0set_asa_calculation()\0"
    "set_bead_check()\0set_vvv()\0cancel()\0"
    "help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_ASA::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_ASA *_t = static_cast<US_Hydrodyn_ASA *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_probe_radius((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->update_probe_recheck_radius((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->update_asa_threshold((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->update_asa_threshold_percent((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->update_asa_grid_threshold((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->update_asa_grid_threshold_percent((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->update_asab1_step((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 8: _t->update_hydrate_probe_radius((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: _t->update_hydrate_threshold((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 10: _t->update_vvv_probe_radius((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 11: _t->update_vvv_grid_dR((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 12: _t->select_asa_method((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->set_asa_calculation(); break;
        case 14: _t->set_bead_check(); break;
        case 15: _t->set_vvv(); break;
        case 16: _t->cancel(); break;
        case 17: _t->help(); break;
        case 18: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_ASA::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_ASA::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_ASA,
      qt_meta_data_US_Hydrodyn_ASA, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_ASA::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_ASA::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_ASA::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_ASA))
        return static_cast<void*>(const_cast< US_Hydrodyn_ASA*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_ASA::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
