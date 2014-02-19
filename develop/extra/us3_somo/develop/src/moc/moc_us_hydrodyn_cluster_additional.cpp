/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_additional.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_additional.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_additional.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Additional[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      32,   31,   31,   31, 0x08,
      43,   31,   31,   31, 0x08,
      54,   31,   31,   31, 0x08,
      61,   31,   31,   31, 0x08,
      76,   31,   31,   31, 0x08,
      87,   31,   31,   31, 0x08,
      98,   31,   31,   31, 0x08,
     105,   31,   31,   31, 0x08,
     116,   31,   31,   31, 0x08,
     123,   31,   31,   31, 0x08,
     133,   31,   31,   31, 0x08,
     139,   31,   31,   31, 0x08,
     152,   31,   31,   31, 0x08,
     161,   31,   31,   31, 0x08,
     174,   31,   31,   31, 0x08,
     183,   31,   31,   31, 0x08,
     196,   31,   31,   31, 0x08,
     205,   31,   31,   31, 0x08,
     210,   31,   31,   31, 0x08,
     219,   31,   31,   31, 0x08,
     226,   31,   31,   31, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Additional[] = {
    "US_Hydrodyn_Cluster_Additional\0\0"
    "setupGUI()\0set_bfnb()\0bfnb()\0"
    "set_bfnb_nsa()\0bfnb_nsa()\0set_best()\0"
    "best()\0set_oned()\0oned()\0set_csa()\0"
    "csa()\0set_dammin()\0dammin()\0set_dammif()\0"
    "dammif()\0set_gasbor()\0gasbor()\0ok()\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster_Additional::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Additional *_t = static_cast<US_Hydrodyn_Cluster_Additional *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->set_bfnb(); break;
        case 2: _t->bfnb(); break;
        case 3: _t->set_bfnb_nsa(); break;
        case 4: _t->bfnb_nsa(); break;
        case 5: _t->set_best(); break;
        case 6: _t->best(); break;
        case 7: _t->set_oned(); break;
        case 8: _t->oned(); break;
        case 9: _t->set_csa(); break;
        case 10: _t->csa(); break;
        case 11: _t->set_dammin(); break;
        case 12: _t->dammin(); break;
        case 13: _t->set_dammif(); break;
        case 14: _t->dammif(); break;
        case 15: _t->set_gasbor(); break;
        case 16: _t->gasbor(); break;
        case 17: _t->ok(); break;
        case 18: _t->cancel(); break;
        case 19: _t->help(); break;
        case 20: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Additional::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Additional::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Additional,
      qt_meta_data_US_Hydrodyn_Cluster_Additional, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Additional::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Additional::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Additional::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Additional))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Additional*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Additional::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
