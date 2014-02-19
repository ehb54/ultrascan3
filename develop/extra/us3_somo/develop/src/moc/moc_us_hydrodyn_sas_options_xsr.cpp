/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_sas_options_xsr.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_sas_options_xsr.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_sas_options_xsr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_SasOptionsXsr[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x08,
      54,   26,   26,   26, 0x08,
      77,   26,   26,   26, 0x08,
     100,   26,   26,   26, 0x08,
     133,   26,   26,   26, 0x08,
     165,   26,   26,   26, 0x08,
     204,   26,   26,   26, 0x08,
     241,   26,   26,   26, 0x08,
     273,   26,   26,   26, 0x08,
     282,   26,   26,   26, 0x08,
     289,   26,   26,   26, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_SasOptionsXsr[] = {
    "US_Hydrodyn_SasOptionsXsr\0\0"
    "update_xsr_symmop(QString)\0"
    "update_xsr_nx(QString)\0update_xsr_ny(QString)\0"
    "update_xsr_griddistance(QString)\0"
    "update_xsr_ncomponents(QString)\0"
    "update_xsr_compactness_weight(QString)\0"
    "update_xsr_looseness_weight(QString)\0"
    "update_xsr_temperature(QString)\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_SasOptionsXsr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_SasOptionsXsr *_t = static_cast<US_Hydrodyn_SasOptionsXsr *>(_o);
        switch (_id) {
        case 0: _t->update_xsr_symmop((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update_xsr_nx((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_xsr_ny((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_xsr_griddistance((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_xsr_ncomponents((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_xsr_compactness_weight((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_xsr_looseness_weight((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_xsr_temperature((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->cancel(); break;
        case 9: _t->help(); break;
        case 10: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_SasOptionsXsr::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_SasOptionsXsr::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_SasOptionsXsr,
      qt_meta_data_US_Hydrodyn_SasOptionsXsr, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_SasOptionsXsr::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_SasOptionsXsr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_SasOptionsXsr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_SasOptionsXsr))
        return static_cast<void*>(const_cast< US_Hydrodyn_SasOptionsXsr*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_SasOptionsXsr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
