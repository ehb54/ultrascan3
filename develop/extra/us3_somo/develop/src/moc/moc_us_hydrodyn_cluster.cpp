/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x08,
      32,   20,   20,   20, 0x08,
      45,   20,   20,   20, 0x08,
      60,   20,   20,   20, 0x08,
      88,   20,   20,   20, 0x08,
     101,   20,   20,   20, 0x08,
     114,   20,   20,   20, 0x08,
     129,   20,   20,   20, 0x08,
     144,   20,   20,   20, 0x08,
     154,   20,   20,   20, 0x08,
     167,   20,   20,   20, 0x08,
     173,   20,   20,   20, 0x08,
     186,   20,   20,   20, 0x08,
     197,   20,   20,   20, 0x08,
     213,   20,   20,   20, 0x08,
     227,   20,   20,   20, 0x08,
     234,   20,   20,   20, 0x08,
     243,   20,   20,   20, 0x08,
     252,   20,   20,   20, 0x08,
     259,   20,   20,   20, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster[] = {
    "US_Hydrodyn_Cluster\0\0setupGUI()\0"
    "add_target()\0clear_target()\0"
    "update_output_name(QString)\0create_pkg()\0"
    "submit_pkg()\0check_status()\0load_results()\0"
    "for_mpi()\0split_grid()\0dmd()\0additional()\0"
    "advanced()\0clear_display()\0update_font()\0"
    "save()\0cancel()\0config()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster *_t = static_cast<US_Hydrodyn_Cluster *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->add_target(); break;
        case 2: _t->clear_target(); break;
        case 3: _t->update_output_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->create_pkg(); break;
        case 5: _t->submit_pkg(); break;
        case 6: _t->check_status(); break;
        case 7: _t->load_results(); break;
        case 8: _t->for_mpi(); break;
        case 9: _t->split_grid(); break;
        case 10: _t->dmd(); break;
        case 11: _t->additional(); break;
        case 12: _t->advanced(); break;
        case 13: _t->clear_display(); break;
        case 14: _t->update_font(); break;
        case 15: _t->save(); break;
        case 16: _t->cancel(); break;
        case 17: _t->config(); break;
        case 18: _t->help(); break;
        case 19: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster,
      qt_meta_data_US_Hydrodyn_Cluster, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
