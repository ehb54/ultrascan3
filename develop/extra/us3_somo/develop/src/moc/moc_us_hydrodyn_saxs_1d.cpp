/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_1d.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_1d.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_1d.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_1d[] = {

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
      21,   20,   20,   20, 0x08,
      32,   20,   20,   20, 0x08,
      55,   20,   20,   20, 0x08,
      89,   20,   20,   20, 0x08,
     120,   20,   20,   20, 0x08,
     158,   20,   20,   20, 0x08,
     191,   20,   20,   20, 0x08,
     222,   20,   20,   20, 0x08,
     243,   20,   20,   20, 0x08,
     266,   20,   20,   20, 0x08,
     295,   20,   20,   20, 0x08,
     321,   20,   20,   20, 0x08,
     347,   20,   20,   20, 0x08,
     379,   20,   20,   20, 0x08,
     402,   20,   20,   20, 0x08,
     427,   20,  422,   20, 0x08,
     451,   20,   20,   20, 0x08,
     467,   20,   20,   20, 0x08,
     474,   20,   20,   20, 0x08,
     482,   20,   20,   20, 0x08,
     489,   20,   20,   20, 0x08,
     505,   20,   20,   20, 0x08,
     519,   20,   20,   20, 0x08,
     526,   20,   20,   20, 0x08,
     538,   20,   20,   20, 0x08,
     548,   20,   20,   20, 0x08,
     557,   20,   20,   20, 0x08,
     564,   20,   20,   20, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_1d[] = {
    "US_Hydrodyn_Saxs_1d\0\0setupGUI()\0"
    "update_lambda(QString)\0"
    "update_detector_distance(QString)\0"
    "update_detector_width(QString)\0"
    "update_detector_pixels_width(QString)\0"
    "update_sample_rotations(QString)\0"
    "update_axis_rotations(QString)\0"
    "update_rho0(QString)\0update_deltaR(QString)\0"
    "update_probe_radius(QString)\0"
    "update_threshold(QString)\0"
    "update_target_ev(QString)\0"
    "update_spec_multiplier(QString)\0"
    "set_random_rotations()\0set_planar_method()\0"
    "bool\0find_target_ev_thresh()\0"
    "set_target_ev()\0info()\0start()\0stop()\0"
    "clear_display()\0update_font()\0save()\0"
    "save_data()\0to_somo()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_1d::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_1d *_t = static_cast<US_Hydrodyn_Saxs_1d *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_lambda((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_detector_distance((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_detector_width((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_detector_pixels_width((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_sample_rotations((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_axis_rotations((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_rho0((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_deltaR((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_probe_radius((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->update_threshold((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->update_target_ev((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->update_spec_multiplier((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->set_random_rotations(); break;
        case 14: _t->set_planar_method(); break;
        case 15: { bool _r = _t->find_target_ev_thresh();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 16: _t->set_target_ev(); break;
        case 17: _t->info(); break;
        case 18: _t->start(); break;
        case 19: _t->stop(); break;
        case 20: _t->clear_display(); break;
        case 21: _t->update_font(); break;
        case 22: _t->save(); break;
        case 23: _t->save_data(); break;
        case 24: _t->to_somo(); break;
        case 25: _t->cancel(); break;
        case 26: _t->help(); break;
        case 27: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_1d::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_1d::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_1d,
      qt_meta_data_US_Hydrodyn_Saxs_1d, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_1d::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_1d::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_1d::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_1d))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_1d*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_1d::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
