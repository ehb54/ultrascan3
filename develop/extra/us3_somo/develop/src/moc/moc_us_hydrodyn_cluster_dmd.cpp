/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_dmd.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_dmd.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_dmd.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Dmd[] = {

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
      25,   24,   24,   24, 0x08,
      38,   36,   24,   24, 0x08,
      59,   24,   24,   24, 0x08,
      76,   24,   24,   24, 0x08,
     101,   24,   24,   24, 0x08,
     114,   24,   24,   24, 0x08,
     121,   24,   24,   24, 0x08,
     129,   24,   24,   24, 0x08,
     141,   24,   24,   24, 0x08,
     147,   24,   24,   24, 0x08,
     161,   24,   24,   24, 0x08,
     168,   24,   24,   24, 0x08,
     176,   24,   24,   24, 0x08,
     187,   24,   24,   24, 0x08,
     203,   24,   24,   24, 0x08,
     217,   24,   24,   24, 0x08,
     224,   24,   24,   24, 0x08,
     229,   24,   24,   24, 0x08,
     238,   24,   24,   24, 0x08,
     245,   24,   24,   24, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Dmd[] = {
    "US_Hydrodyn_Cluster_Dmd\0\0setupGUI()\0"
    ",\0table_value(int,int)\0update_enables()\0"
    "row_header_released(int)\0select_all()\0"
    "copy()\0paste()\0paste_all()\0dup()\0"
    "delete_rows()\0load()\0reset()\0save_csv()\0"
    "clear_display()\0update_font()\0save()\0"
    "ok()\0cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster_Dmd::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Dmd *_t = static_cast<US_Hydrodyn_Cluster_Dmd *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->table_value((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->update_enables(); break;
        case 3: _t->row_header_released((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->select_all(); break;
        case 5: _t->copy(); break;
        case 6: _t->paste(); break;
        case 7: _t->paste_all(); break;
        case 8: _t->dup(); break;
        case 9: _t->delete_rows(); break;
        case 10: _t->load(); break;
        case 11: _t->reset(); break;
        case 12: _t->save_csv(); break;
        case 13: _t->clear_display(); break;
        case 14: _t->update_font(); break;
        case 15: _t->save(); break;
        case 16: _t->ok(); break;
        case 17: _t->cancel(); break;
        case 18: _t->help(); break;
        case 19: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Dmd::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Dmd::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Dmd,
      qt_meta_data_US_Hydrodyn_Cluster_Dmd, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Dmd::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Dmd::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Dmd::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Dmd))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Dmd*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Dmd::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
