/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_sas_options_saxs.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_sas_options_saxs.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_sas_options_saxs.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_SasOptionsSaxs[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      29,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      39,   27,   27,   27, 0x08,
      70,   27,   27,   27, 0x08,
      97,   27,   27,   27, 0x08,
     121,   27,   27,   27, 0x08,
     149,   27,   27,   27, 0x08,
     178,   27,   27,   27, 0x08,
     207,   27,   27,   27, 0x08,
     237,   27,   27,   27, 0x08,
     263,   27,   27,   27, 0x08,
     300,   27,   27,   27, 0x08,
     321,   27,   27,   27, 0x08,
     340,   27,   27,   27, 0x08,
     361,   27,   27,   27, 0x08,
     390,   27,   27,   27, 0x08,
     421,   27,   27,   27, 0x08,
     453,   27,   27,   27, 0x08,
     485,   27,   27,   27, 0x08,
     524,   27,   27,   27, 0x08,
     571,   27,   27,   27, 0x08,
     618,   27,   27,   27, 0x08,
     642,   27,   27,   27, 0x08,
     674,   27,   27,   27, 0x08,
     690,   27,   27,   27, 0x08,
     713,   27,   27,   27, 0x08,
     729,   27,   27,   27, 0x08,
     746,   27,   27,   27, 0x08,
     755,   27,   27,   27, 0x08,
     762,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_SasOptionsSaxs[] = {
    "US_Hydrodyn_SasOptionsSaxs\0\0setupGUI()\0"
    "update_water_e_density(double)\0"
    "set_saxs_iq_native_debye()\0"
    "set_saxs_iq_native_sh()\0"
    "set_saxs_iq_native_hybrid()\0"
    "set_saxs_iq_native_hybrid2()\0"
    "set_saxs_iq_native_hybrid3()\0"
    "set_saxs_iq_hybrid_adaptive()\0"
    "set_saxs_iq_native_fast()\0"
    "set_saxs_iq_native_fast_compute_pr()\0"
    "set_saxs_iq_crysol()\0set_saxs_iq_foxs()\0"
    "set_saxs_iq_sastbx()\0update_fast_bin_size(double)\0"
    "update_fast_modulation(double)\0"
    "update_hybrid2_q_points(double)\0"
    "update_sh_max_harmonics(double)\0"
    "update_sh_fibonacci_grid_order(double)\0"
    "update_crysol_hydration_shell_contrast(double)\0"
    "set_crysol_default_load_difference_intensity()\0"
    "set_crysol_version_26()\0"
    "set_crysol_explicit_hydrogens()\0"
    "crysol_target()\0set_sastbx_method(int)\0"
    "set_ra(QString)\0set_vol(QString)\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_SasOptionsSaxs::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_SasOptionsSaxs *_t = static_cast<US_Hydrodyn_SasOptionsSaxs *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_water_e_density((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->set_saxs_iq_native_debye(); break;
        case 3: _t->set_saxs_iq_native_sh(); break;
        case 4: _t->set_saxs_iq_native_hybrid(); break;
        case 5: _t->set_saxs_iq_native_hybrid2(); break;
        case 6: _t->set_saxs_iq_native_hybrid3(); break;
        case 7: _t->set_saxs_iq_hybrid_adaptive(); break;
        case 8: _t->set_saxs_iq_native_fast(); break;
        case 9: _t->set_saxs_iq_native_fast_compute_pr(); break;
        case 10: _t->set_saxs_iq_crysol(); break;
        case 11: _t->set_saxs_iq_foxs(); break;
        case 12: _t->set_saxs_iq_sastbx(); break;
        case 13: _t->update_fast_bin_size((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 14: _t->update_fast_modulation((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 15: _t->update_hybrid2_q_points((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 16: _t->update_sh_max_harmonics((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 17: _t->update_sh_fibonacci_grid_order((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 18: _t->update_crysol_hydration_shell_contrast((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 19: _t->set_crysol_default_load_difference_intensity(); break;
        case 20: _t->set_crysol_version_26(); break;
        case 21: _t->set_crysol_explicit_hydrogens(); break;
        case 22: _t->crysol_target(); break;
        case 23: _t->set_sastbx_method((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: _t->set_ra((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 25: _t->set_vol((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 26: _t->cancel(); break;
        case 27: _t->help(); break;
        case 28: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_SasOptionsSaxs::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_SasOptionsSaxs::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_SasOptionsSaxs,
      qt_meta_data_US_Hydrodyn_SasOptionsSaxs, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_SasOptionsSaxs::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_SasOptionsSaxs::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_SasOptionsSaxs::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_SasOptionsSaxs))
        return static_cast<void*>(const_cast< US_Hydrodyn_SasOptionsSaxs*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_SasOptionsSaxs::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 29)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 29;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
