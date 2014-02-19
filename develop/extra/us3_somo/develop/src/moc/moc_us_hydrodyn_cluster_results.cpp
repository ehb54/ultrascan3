/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_results.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_results.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_results.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Results[] = {

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
      29,   28,   28,   28, 0x08,
      40,   28,   28,   28, 0x08,
      57,   28,   28,   28, 0x08,
      70,   28,   28,   28, 0x08,
      78,   28,   28,   28, 0x08,
      93,   28,   28,   28, 0x08,
     109,   28,   28,   28, 0x08,
     123,   28,   28,   28, 0x08,
     130,   28,   28,   28, 0x08,
     139,   28,   28,   28, 0x08,
     146,   28,   28,   28, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Results[] = {
    "US_Hydrodyn_Cluster_Results\0\0setupGUI()\0"
    "update_enables()\0select_all()\0purge()\0"
    "load_results()\0clear_display()\0"
    "update_font()\0save()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster_Results::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Results *_t = static_cast<US_Hydrodyn_Cluster_Results *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_enables(); break;
        case 2: _t->select_all(); break;
        case 3: _t->purge(); break;
        case 4: _t->load_results(); break;
        case 5: _t->clear_display(); break;
        case 6: _t->update_font(); break;
        case 7: _t->save(); break;
        case 8: _t->cancel(); break;
        case 9: _t->help(); break;
        case 10: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Results::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Results::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Results,
      qt_meta_data_US_Hydrodyn_Cluster_Results, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Results::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Results::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Results::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Results))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Results*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Results::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
