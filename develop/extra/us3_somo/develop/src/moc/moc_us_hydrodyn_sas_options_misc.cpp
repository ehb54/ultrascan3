/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_sas_options_misc.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_sas_options_misc.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_sas_options_misc.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_SasOptionsMisc[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      39,   27,   27,   27, 0x08,
      63,   27,   27,   27, 0x08,
      89,   27,   27,   27, 0x08,
     113,   27,   27,   27, 0x08,
     135,   27,   27,   27, 0x08,
     148,   27,   27,   27, 0x08,
     167,   27,   27,   27, 0x08,
     193,   27,   27,   27, 0x08,
     217,   27,   27,   27, 0x08,
     235,   27,   27,   27, 0x08,
     274,   27,   27,   27, 0x08,
     295,   27,   27,   27, 0x08,
     315,   27,   27,   27, 0x08,
     340,   27,   27,   27, 0x08,
     369,   27,   27,   27, 0x08,
     398,   27,   27,   27, 0x08,
     428,   27,   27,   27, 0x08,
     451,   27,   27,   27, 0x08,
     484,   27,   27,   27, 0x08,
     513,   27,   27,   27, 0x08,
     544,   27,   27,   27, 0x08,
     575,   27,   27,   27, 0x08,
     592,   27,   27,   27, 0x08,
     601,   27,   27,   27, 0x08,
     608,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_SasOptionsMisc[] = {
    "US_Hydrodyn_SasOptionsMisc\0\0setupGUI()\0"
    "default_atom_filename()\0"
    "default_hybrid_filename()\0"
    "default_saxs_filename()\0default_ff_filename()\0"
    "set_iq_ask()\0set_iq_scale_ask()\0"
    "set_iqq_ask_target_grid()\0"
    "set_iq_scale_angstrom()\0set_iq_scale_nm()\0"
    "set_iqq_expt_data_contains_variances()\0"
    "set_iqq_kratky_fit()\0set_ignore_errors()\0"
    "set_disable_iq_scaling()\0"
    "set_iqq_scale_chi2_fitting()\0"
    "update_swh_excl_vol(QString)\0"
    "update_scale_excl_vol(double)\0"
    "set_use_iq_target_ev()\0"
    "set_set_iq_target_ev_from_vbar()\0"
    "update_iq_target_ev(QString)\0"
    "update_iqq_scale_minq(QString)\0"
    "update_iqq_scale_maxq(QString)\0"
    "clear_mw_cache()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_SasOptionsMisc::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_SasOptionsMisc *_t = static_cast<US_Hydrodyn_SasOptionsMisc *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->default_atom_filename(); break;
        case 2: _t->default_hybrid_filename(); break;
        case 3: _t->default_saxs_filename(); break;
        case 4: _t->default_ff_filename(); break;
        case 5: _t->set_iq_ask(); break;
        case 6: _t->set_iq_scale_ask(); break;
        case 7: _t->set_iqq_ask_target_grid(); break;
        case 8: _t->set_iq_scale_angstrom(); break;
        case 9: _t->set_iq_scale_nm(); break;
        case 10: _t->set_iqq_expt_data_contains_variances(); break;
        case 11: _t->set_iqq_kratky_fit(); break;
        case 12: _t->set_ignore_errors(); break;
        case 13: _t->set_disable_iq_scaling(); break;
        case 14: _t->set_iqq_scale_chi2_fitting(); break;
        case 15: _t->update_swh_excl_vol((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: _t->update_scale_excl_vol((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 17: _t->set_use_iq_target_ev(); break;
        case 18: _t->set_set_iq_target_ev_from_vbar(); break;
        case 19: _t->update_iq_target_ev((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: _t->update_iqq_scale_minq((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 21: _t->update_iqq_scale_maxq((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 22: _t->clear_mw_cache(); break;
        case 23: _t->cancel(); break;
        case 24: _t->help(); break;
        case 25: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_SasOptionsMisc::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_SasOptionsMisc::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_SasOptionsMisc,
      qt_meta_data_US_Hydrodyn_SasOptionsMisc, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_SasOptionsMisc::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_SasOptionsMisc::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_SasOptionsMisc::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_SasOptionsMisc))
        return static_cast<void*>(const_cast< US_Hydrodyn_SasOptionsMisc*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_SasOptionsMisc::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
