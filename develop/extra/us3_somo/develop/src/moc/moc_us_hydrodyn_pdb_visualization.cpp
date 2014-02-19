/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_pdb_visualization.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_pdb_visualization.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_pdb_visualization.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_PDB_Visualization[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      31,   30,   30,   30, 0x08,
      42,   30,   30,   30, 0x08,
      61,   30,   30,   30, 0x08,
      79,   30,   30,   30, 0x08,
     104,   30,   30,   30, 0x08,
     113,   30,   30,   30, 0x08,
     120,   30,   30,   30, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_PDB_Visualization[] = {
    "US_Hydrodyn_PDB_Visualization\0\0"
    "setupGUI()\0select_option(int)\0"
    "select_filename()\0update_filename(QString)\0"
    "cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_PDB_Visualization::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_PDB_Visualization *_t = static_cast<US_Hydrodyn_PDB_Visualization *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->select_option((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->select_filename(); break;
        case 3: _t->update_filename((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->cancel(); break;
        case 5: _t->help(); break;
        case 6: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_PDB_Visualization::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_PDB_Visualization::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_PDB_Visualization,
      qt_meta_data_US_Hydrodyn_PDB_Visualization, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_PDB_Visualization::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_PDB_Visualization::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_PDB_Visualization::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_PDB_Visualization))
        return static_cast<void*>(const_cast< US_Hydrodyn_PDB_Visualization*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_PDB_Visualization::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
