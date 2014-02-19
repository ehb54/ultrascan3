/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_addatom.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_addatom.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_addatom.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_AddAtom[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      18,   11,   11,   11, 0x08,
      25,   11,   11,   11, 0x08,
      44,   11,   11,   11, 0x08,
      65,   11,   11,   11, 0x08,
      84,   11,   11,   11, 0x08,
      95,   11,   11,   11, 0x08,
     116,   11,   11,   11, 0x08,
     141,   11,   11,   11, 0x08,
     176,   11,   11,   11, 0x08,
     195,   11,   11,   11, 0x08,
     212,   11,   11,   11, 0x08,
     226,   11,   11,   11, 0x08,
     244,   11,   11,   11, 0x08,
     257,   11,   11,   11, 0x08,
     272,   11,   11,   11, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_AddAtom[] = {
    "US_AddAtom\0\0add()\0help()\0select_atom_file()\0"
    "select_hybrid_file()\0select_saxs_file()\0"
    "setupGUI()\0update_name(QString)\0"
    "update_excl_vol(QString)\0"
    "update_hybridization_name(QString)\0"
    "select_hybrid(int)\0select_atom(int)\0"
    "delete_atom()\0write_atom_file()\0"
    "sort_atoms()\0set_excl_vol()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_AddAtom::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_AddAtom *_t = static_cast<US_AddAtom *>(_o);
        switch (_id) {
        case 0: _t->add(); break;
        case 1: _t->help(); break;
        case 2: _t->select_atom_file(); break;
        case 3: _t->select_hybrid_file(); break;
        case 4: _t->select_saxs_file(); break;
        case 5: _t->setupGUI(); break;
        case 6: _t->update_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_excl_vol((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_hybridization_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->select_hybrid((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->select_atom((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->delete_atom(); break;
        case 12: _t->write_atom_file(); break;
        case 13: _t->sort_atoms(); break;
        case 14: _t->set_excl_vol(); break;
        case 15: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_AddAtom::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_AddAtom::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_US_AddAtom,
      qt_meta_data_US_AddAtom, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_AddAtom::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_AddAtom::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_AddAtom::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_AddAtom))
        return static_cast<void*>(const_cast< US_AddAtom*>(this));
    return QWidget::qt_metacast(_clname);
}

int US_AddAtom::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
