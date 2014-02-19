/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_sas_options_curve.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_sas_options_curve.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_sas_options_curve.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_SasOptionsCurve[] = {

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
      29,   28,   28,   28, 0x08,
      40,   28,   28,   28, 0x08,
      66,   28,   28,   28, 0x08,
      93,   28,   28,   28, 0x08,
     118,   28,   28,   28, 0x08,
     145,   28,   28,   28, 0x08,
     168,   28,   28,   28, 0x08,
     189,   28,   28,   28, 0x08,
     212,   28,   28,   28, 0x08,
     234,   28,   28,   28, 0x08,
     243,   28,   28,   28, 0x08,
     250,   28,   28,   28, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_SasOptionsCurve[] = {
    "US_Hydrodyn_SasOptionsCurve\0\0setupGUI()\0"
    "update_wavelength(double)\0"
    "update_start_angle(double)\0"
    "update_end_angle(double)\0"
    "update_delta_angle(double)\0"
    "update_start_q(double)\0update_end_q(double)\0"
    "update_delta_q(double)\0set_normalize_by_mw()\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_SasOptionsCurve::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_SasOptionsCurve *_t = static_cast<US_Hydrodyn_SasOptionsCurve *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_wavelength((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->update_start_angle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->update_end_angle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->update_delta_angle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->update_start_q((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->update_end_q((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->update_delta_q((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 8: _t->set_normalize_by_mw(); break;
        case 9: _t->cancel(); break;
        case 10: _t->help(); break;
        case 11: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_SasOptionsCurve::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_SasOptionsCurve::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_SasOptionsCurve,
      qt_meta_data_US_Hydrodyn_SasOptionsCurve, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_SasOptionsCurve::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_SasOptionsCurve::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_SasOptionsCurve::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_SasOptionsCurve))
        return static_cast<void*>(const_cast< US_Hydrodyn_SasOptionsCurve*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_SasOptionsCurve::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
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
