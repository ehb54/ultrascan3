/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_grid.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_grid.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_grid.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Grid[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x08,
      29,   17,   17,   17, 0x08,
      54,   17,   17,   17, 0x08,
      73,   17,   17,   17, 0x08,
      87,   17,   17,   17, 0x08,
     102,   17,   17,   17, 0x08,
     119,   17,   17,   17, 0x08,
     131,   17,   17,   17, 0x08,
     157,   17,   17,   17, 0x08,
     194,   17,   17,   17, 0x08,
     205,   17,   17,   17, 0x08,
     214,   17,   17,   17, 0x08,
     221,   17,   17,   17, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Grid[] = {
    "US_Hydrodyn_Grid\0\0setupGUI()\0"
    "update_cube_side(double)\0select_center(int)\0"
    "set_hydrate()\0set_tangency()\0"
    "set_enable_asa()\0set_cubic()\0"
    "set_create_nmr_bead_pdb()\0"
    "set_equalize_radii_constant_volume()\0"
    "overlaps()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Grid::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Grid *_t = static_cast<US_Hydrodyn_Grid *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_cube_side((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->select_center((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->set_hydrate(); break;
        case 4: _t->set_tangency(); break;
        case 5: _t->set_enable_asa(); break;
        case 6: _t->set_cubic(); break;
        case 7: _t->set_create_nmr_bead_pdb(); break;
        case 8: _t->set_equalize_radii_constant_volume(); break;
        case 9: _t->overlaps(); break;
        case 10: _t->cancel(); break;
        case 11: _t->help(); break;
        case 12: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Grid::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Grid::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Grid,
      qt_meta_data_US_Hydrodyn_Grid, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Grid::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Grid::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Grid::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Grid))
        return static_cast<void*>(const_cast< US_Hydrodyn_Grid*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Grid::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
