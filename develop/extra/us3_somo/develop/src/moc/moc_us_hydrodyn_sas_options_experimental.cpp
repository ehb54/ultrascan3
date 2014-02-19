/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_sas_options_experimental.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_sas_options_experimental.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_sas_options_experimental.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_SasOptionsExperimental[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      28,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      36,   35,   35,   35, 0x08,
      47,   35,   35,   35, 0x08,
      77,   35,   35,   35, 0x08,
      97,   35,   35,   35, 0x08,
     126,   35,   35,   35, 0x08,
     148,   35,   35,   35, 0x08,
     172,   35,   35,   35, 0x08,
     200,   35,   35,   35, 0x08,
     218,   35,   35,   35, 0x08,
     239,   35,   35,   35, 0x08,
     261,   35,   35,   35, 0x08,
     282,   35,   35,   35, 0x08,
     295,   35,   35,   35, 0x08,
     321,   35,   35,   35, 0x08,
     338,   35,   35,   35, 0x08,
     365,   35,   35,   35, 0x08,
     406,   35,   35,   35, 0x08,
     434,   35,   35,   35, 0x08,
     460,   35,   35,   35, 0x08,
     493,   35,   35,   35, 0x08,
     523,   35,   35,   35, 0x08,
     537,   35,   35,   35, 0x08,
     551,   35,   35,   35, 0x08,
     568,   35,   35,   35, 0x08,
     603,   35,   35,   35, 0x08,
     620,   35,   35,   35, 0x08,
     629,   35,   35,   35, 0x08,
     636,   35,   35,   35, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_SasOptionsExperimental[] = {
    "US_Hydrodyn_SasOptionsExperimental\0\0"
    "setupGUI()\0set_iqq_scale_linear_offset()\0"
    "set_autocorrelate()\0set_hybrid_radius_excl_vol()\0"
    "set_subtract_radius()\0set_iqq_use_atomic_ff()\0"
    "set_iqq_use_saxs_excl_vol()\0"
    "set_use_somo_ff()\0set_iqq_scale_nnls()\0"
    "set_iqq_log_fitting()\0set_iqq_scale_play()\0"
    "set_alt_ff()\0set_five_term_gaussians()\0"
    "set_iq_exact_q()\0set_compute_exponentials()\0"
    "update_compute_exponential_terms(double)\0"
    "update_ev_exp_mult(QString)\0"
    "set_compute_chi2shannon()\0"
    "update_chi2shannon_dmax(QString)\0"
    "update_chi2shannon_k(QString)\0"
    "set_alt_sh1()\0set_alt_sh2()\0"
    "set_create_shd()\0set_multiply_iq_by_atomic_volume()\0"
    "create_somo_ff()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_SasOptionsExperimental::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_SasOptionsExperimental *_t = static_cast<US_Hydrodyn_SasOptionsExperimental *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->set_iqq_scale_linear_offset(); break;
        case 2: _t->set_autocorrelate(); break;
        case 3: _t->set_hybrid_radius_excl_vol(); break;
        case 4: _t->set_subtract_radius(); break;
        case 5: _t->set_iqq_use_atomic_ff(); break;
        case 6: _t->set_iqq_use_saxs_excl_vol(); break;
        case 7: _t->set_use_somo_ff(); break;
        case 8: _t->set_iqq_scale_nnls(); break;
        case 9: _t->set_iqq_log_fitting(); break;
        case 10: _t->set_iqq_scale_play(); break;
        case 11: _t->set_alt_ff(); break;
        case 12: _t->set_five_term_gaussians(); break;
        case 13: _t->set_iq_exact_q(); break;
        case 14: _t->set_compute_exponentials(); break;
        case 15: _t->update_compute_exponential_terms((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 16: _t->update_ev_exp_mult((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: _t->set_compute_chi2shannon(); break;
        case 18: _t->update_chi2shannon_dmax((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 19: _t->update_chi2shannon_k((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: _t->set_alt_sh1(); break;
        case 21: _t->set_alt_sh2(); break;
        case 22: _t->set_create_shd(); break;
        case 23: _t->set_multiply_iq_by_atomic_volume(); break;
        case 24: _t->create_somo_ff(); break;
        case 25: _t->cancel(); break;
        case 26: _t->help(); break;
        case 27: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_SasOptionsExperimental::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_SasOptionsExperimental::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_SasOptionsExperimental,
      qt_meta_data_US_Hydrodyn_SasOptionsExperimental, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_SasOptionsExperimental::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_SasOptionsExperimental::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_SasOptionsExperimental::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_SasOptionsExperimental))
        return static_cast<void*>(const_cast< US_Hydrodyn_SasOptionsExperimental*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_SasOptionsExperimental::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
