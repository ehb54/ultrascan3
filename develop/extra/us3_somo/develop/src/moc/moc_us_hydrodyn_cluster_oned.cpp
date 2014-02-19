/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_oned.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_oned.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_oned.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Oned[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,   25,   25,   25, 0x08,
      57,   25,   25,   25, 0x08,
      92,   25,   25,   25, 0x08,
     117,   25,   25,   25, 0x08,
     152,   25,   25,   25, 0x08,
     184,   25,   25,   25, 0x08,
     222,   25,   25,   25, 0x08,
     245,   25,   25,   25, 0x08,
     270,   25,   25,   25, 0x08,
     300,   25,   25,   25, 0x08,
     328,   25,   25,   25, 0x08,
     362,   25,   25,   25, 0x08,
     394,   25,   25,   25, 0x08,
     430,   25,   25,   25, 0x08,
     437,   25,   25,   25, 0x08,
     444,   25,   25,   25, 0x08,
     451,   25,   25,   25, 0x08,
     460,   25,   25,   25, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Oned[] = {
    "US_Hydrodyn_Cluster_Oned\0\0"
    "update_1drotationfile(QString)\0"
    "update_1drotationsuserand(QString)\0"
    "update_1dlambda(QString)\0"
    "update_1ddetectordistance(QString)\0"
    "update_1ddetectorwidth(QString)\0"
    "update_1ddetectorpixelswidth(QString)\0"
    "update_1drho0(QString)\0update_1ddeltar(QString)\0"
    "update_1dproberadius(QString)\0"
    "update_1dthreshold(QString)\0"
    "update_1dsamplerotations(QString)\0"
    "update_1daxisrotations(QString)\0"
    "update_1dintermediatesaves(QString)\0"
    "save()\0load()\0help()\0cancel()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster_Oned::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Oned *_t = static_cast<US_Hydrodyn_Cluster_Oned *>(_o);
        switch (_id) {
        case 0: _t->update_1drotationfile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update_1drotationsuserand((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_1dlambda((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_1ddetectordistance((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_1ddetectorwidth((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_1ddetectorpixelswidth((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_1drho0((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_1ddeltar((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_1dproberadius((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_1dthreshold((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->update_1dsamplerotations((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->update_1daxisrotations((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->update_1dintermediatesaves((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->save(); break;
        case 14: _t->load(); break;
        case 15: _t->help(); break;
        case 16: _t->cancel(); break;
        case 17: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Oned::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Oned::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Oned,
      qt_meta_data_US_Hydrodyn_Cluster_Oned, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Oned::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Oned::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Oned::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Oned))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Oned*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Oned::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
