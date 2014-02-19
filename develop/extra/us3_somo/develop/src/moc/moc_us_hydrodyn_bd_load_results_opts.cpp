/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_bd_load_results_opts.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_bd_load_results_opts.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_bd_load_results_opts.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_BD_Load_Results_Opts[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      38,   34,   33,   33, 0x08,
      67,   34,   33,   33, 0x08,
      99,   34,   33,   33, 0x08,
     127,   34,   33,   33, 0x08,
     161,   34,   33,   33, 0x08,
     193,   33,   33,   33, 0x08,
     213,   33,   33,   33, 0x08,
     231,   33,   33,   33, 0x08,
     254,   33,   33,   33, 0x08,
     279,   33,   33,   33, 0x08,
     313,   33,   33,   33, 0x08,
     322,   33,   33,   33, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_BD_Load_Results_Opts[] = {
    "US_Hydrodyn_BD_Load_Results_Opts\0\0str\0"
    "update_solvent_name(QString)\0"
    "update_solvent_acronym(QString)\0"
    "update_temperature(QString)\0"
    "update_solvent_viscosity(QString)\0"
    "update_solvent_density(QString)\0"
    "update_psv(QString)\0set_to_browflex()\0"
    "set_solvent_defaults()\0set_check_fix_overlaps()\0"
    "check_solvent_browflex_defaults()\0"
    "cancel()\0help()\0"
};

void US_Hydrodyn_BD_Load_Results_Opts::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_BD_Load_Results_Opts *_t = static_cast<US_Hydrodyn_BD_Load_Results_Opts *>(_o);
        switch (_id) {
        case 0: _t->update_solvent_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update_solvent_acronym((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_temperature((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_solvent_viscosity((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_solvent_density((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_psv((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->set_to_browflex(); break;
        case 7: _t->set_solvent_defaults(); break;
        case 8: _t->set_check_fix_overlaps(); break;
        case 9: _t->check_solvent_browflex_defaults(); break;
        case 10: _t->cancel(); break;
        case 11: _t->help(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_BD_Load_Results_Opts::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_BD_Load_Results_Opts::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_BD_Load_Results_Opts,
      qt_meta_data_US_Hydrodyn_BD_Load_Results_Opts, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_BD_Load_Results_Opts::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_BD_Load_Results_Opts::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_BD_Load_Results_Opts::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_BD_Load_Results_Opts))
        return static_cast<void*>(const_cast< US_Hydrodyn_BD_Load_Results_Opts*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_BD_Load_Results_Opts::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
