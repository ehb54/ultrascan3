/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_sas_options_sans.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_sas_options_sans.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_sas_options_sans.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_SasOptionsSans[] = {

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
      28,   27,   27,   27, 0x08,
      39,   27,   27,   27, 0x08,
      65,   27,   27,   27, 0x08,
      91,   27,   27,   27, 0x08,
     124,   27,   27,   27, 0x08,
     157,   27,   27,   27, 0x08,
     181,   27,   27,   27, 0x08,
     213,   27,   27,   27, 0x08,
     243,   27,   27,   27, 0x08,
     270,   27,   27,   27, 0x08,
     298,   27,   27,   27, 0x08,
     327,   27,   27,   27, 0x08,
     356,   27,   27,   27, 0x08,
     386,   27,   27,   27, 0x08,
     412,   27,   27,   27, 0x08,
     449,   27,   27,   27, 0x08,
     470,   27,   27,   27, 0x08,
     493,   27,   27,   27, 0x08,
     532,   27,   27,   27, 0x08,
     578,   27,   27,   27, 0x08,
     625,   27,   27,   27, 0x08,
     634,   27,   27,   27, 0x08,
     641,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_SasOptionsSans[] = {
    "US_Hydrodyn_SasOptionsSans\0\0setupGUI()\0"
    "update_h_scat_len(double)\0"
    "update_d_scat_len(double)\0"
    "update_h2o_scat_len_dens(double)\0"
    "update_d2o_scat_len_dens(double)\0"
    "update_d2o_conc(double)\0"
    "update_frac_of_exch_pep(double)\0"
    "update_perdeuteration(double)\0"
    "set_sans_iq_native_debye()\0"
    "set_sans_iq_native_hybrid()\0"
    "set_sans_iq_native_hybrid2()\0"
    "set_sans_iq_native_hybrid3()\0"
    "set_sans_iq_hybrid_adaptive()\0"
    "set_sans_iq_native_fast()\0"
    "set_sans_iq_native_fast_compute_pr()\0"
    "set_sans_iq_cryson()\0set_cryson_manual_hs()\0"
    "update_cryson_sh_max_harmonics(double)\0"
    "update_cryson_sh_fibonacci_grid_order(double)\0"
    "update_cryson_hydration_shell_contrast(double)\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_SasOptionsSans::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_SasOptionsSans *_t = static_cast<US_Hydrodyn_SasOptionsSans *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_h_scat_len((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->update_d_scat_len((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->update_h2o_scat_len_dens((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->update_d2o_scat_len_dens((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->update_d2o_conc((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->update_frac_of_exch_pep((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->update_perdeuteration((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 8: _t->set_sans_iq_native_debye(); break;
        case 9: _t->set_sans_iq_native_hybrid(); break;
        case 10: _t->set_sans_iq_native_hybrid2(); break;
        case 11: _t->set_sans_iq_native_hybrid3(); break;
        case 12: _t->set_sans_iq_hybrid_adaptive(); break;
        case 13: _t->set_sans_iq_native_fast(); break;
        case 14: _t->set_sans_iq_native_fast_compute_pr(); break;
        case 15: _t->set_sans_iq_cryson(); break;
        case 16: _t->set_cryson_manual_hs(); break;
        case 17: _t->update_cryson_sh_max_harmonics((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 18: _t->update_cryson_sh_fibonacci_grid_order((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 19: _t->update_cryson_hydration_shell_contrast((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 20: _t->cancel(); break;
        case 21: _t->help(); break;
        case 22: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_SasOptionsSans::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_SasOptionsSans::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_SasOptionsSans,
      qt_meta_data_US_Hydrodyn_SasOptionsSans, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_SasOptionsSans::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_SasOptionsSans::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_SasOptionsSans::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_SasOptionsSans))
        return static_cast<void*>(const_cast< US_Hydrodyn_SasOptionsSans*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_SasOptionsSans::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
