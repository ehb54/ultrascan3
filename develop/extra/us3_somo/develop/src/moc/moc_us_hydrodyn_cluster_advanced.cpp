/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_advanced.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_advanced.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_advanced.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Advanced[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      30,   29,   29,   29, 0x08,
      43,   41,   29,   29, 0x08,
      64,   29,   29,   29, 0x08,
      80,   29,   29,   29, 0x08,
      94,   29,   29,   29, 0x08,
     101,   29,   29,   29, 0x08,
     106,   29,   29,   29, 0x08,
     115,   29,   29,   29, 0x08,
     122,   29,   29,   29, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Advanced[] = {
    "US_Hydrodyn_Cluster_Advanced\0\0setupGUI()\0"
    ",\0table_value(int,int)\0clear_display()\0"
    "update_font()\0save()\0ok()\0cancel()\0"
    "help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster_Advanced::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Advanced *_t = static_cast<US_Hydrodyn_Cluster_Advanced *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->table_value((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->clear_display(); break;
        case 3: _t->update_font(); break;
        case 4: _t->save(); break;
        case 5: _t->ok(); break;
        case 6: _t->cancel(); break;
        case 7: _t->help(); break;
        case 8: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Advanced::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Advanced::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Advanced,
      qt_meta_data_US_Hydrodyn_Cluster_Advanced, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Advanced::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Advanced::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Advanced::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Advanced))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Advanced*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Advanced::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
