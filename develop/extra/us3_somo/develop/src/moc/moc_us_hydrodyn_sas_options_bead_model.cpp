/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_sas_options_bead_model.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_sas_options_bead_model.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_sas_options_bead_model.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_SasOptionsBeadModel[] = {

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
      33,   32,   32,   32, 0x08,
      44,   32,   32,   32, 0x08,
      85,   32,   32,   32, 0x08,
     126,   32,   32,   32, 0x08,
     152,   32,   32,   32, 0x08,
     189,   32,   32,   32, 0x08,
     228,   32,   32,   32, 0x08,
     260,   32,   32,   32, 0x08,
     288,   32,   32,   32, 0x08,
     324,   32,   32,   32, 0x08,
     360,   32,   32,   32, 0x08,
     394,   32,   32,   32, 0x08,
     434,   32,   32,   32, 0x08,
     467,   32,   32,   32, 0x08,
     510,   32,   32,   32, 0x08,
     519,   32,   32,   32, 0x08,
     526,   32,   32,   32, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_SasOptionsBeadModel[] = {
    "US_Hydrodyn_SasOptionsBeadModel\0\0"
    "setupGUI()\0set_compute_saxs_coeff_for_bead_models()\0"
    "set_compute_sans_coeff_for_bead_models()\0"
    "set_bead_model_rayleigh()\0"
    "set_bead_models_use_bead_radius_ev()\0"
    "set_bead_models_rho0_in_scat_factors()\0"
    "update_dummy_saxs_name(QString)\0"
    "set_dummy_atom_pdbs_in_nm()\0"
    "set_iq_global_avg_for_bead_models()\0"
    "set_bead_models_use_quick_fitting()\0"
    "set_bead_models_use_gsm_fitting()\0"
    "set_apply_loaded_sf_repeatedly_to_pdb()\0"
    "set_bead_models_use_var_len_sf()\0"
    "update_bead_models_var_len_sf_max(QString)\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_SasOptionsBeadModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_SasOptionsBeadModel *_t = static_cast<US_Hydrodyn_SasOptionsBeadModel *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->set_compute_saxs_coeff_for_bead_models(); break;
        case 2: _t->set_compute_sans_coeff_for_bead_models(); break;
        case 3: _t->set_bead_model_rayleigh(); break;
        case 4: _t->set_bead_models_use_bead_radius_ev(); break;
        case 5: _t->set_bead_models_rho0_in_scat_factors(); break;
        case 6: _t->update_dummy_saxs_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->set_dummy_atom_pdbs_in_nm(); break;
        case 8: _t->set_iq_global_avg_for_bead_models(); break;
        case 9: _t->set_bead_models_use_quick_fitting(); break;
        case 10: _t->set_bead_models_use_gsm_fitting(); break;
        case 11: _t->set_apply_loaded_sf_repeatedly_to_pdb(); break;
        case 12: _t->set_bead_models_use_var_len_sf(); break;
        case 13: _t->update_bead_models_var_len_sf_max((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->cancel(); break;
        case 15: _t->help(); break;
        case 16: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_SasOptionsBeadModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_SasOptionsBeadModel::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_SasOptionsBeadModel,
      qt_meta_data_US_Hydrodyn_SasOptionsBeadModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_SasOptionsBeadModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_SasOptionsBeadModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_SasOptionsBeadModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_SasOptionsBeadModel))
        return static_cast<void*>(const_cast< US_Hydrodyn_SasOptionsBeadModel*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_SasOptionsBeadModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
