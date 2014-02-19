/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_pdb_tool_merge.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_pdb_tool_merge.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_pdb_tool_merge.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Pdb_Tool_Merge[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      28,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      41,   39,   27,   27, 0x08,
      62,   27,   27,   27, 0x08,
      79,   27,   27,   27, 0x08,
      90,   27,   27,   27, 0x08,
     110,   27,   27,   27, 0x08,
     128,   27,   27,   27, 0x08,
     144,   27,   27,   27, 0x08,
     160,   27,   27,   27, 0x08,
     175,   27,   27,   27, 0x08,
     190,   27,   27,   27, 0x08,
     203,   27,   27,   27, 0x08,
     211,   27,   27,   27, 0x08,
     218,   27,   27,   27, 0x08,
     229,   27,   27,   27, 0x08,
     240,   27,   27,   27, 0x08,
     254,   27,   27,   27, 0x08,
     266,   27,   27,   27, 0x08,
     275,   27,   27,   27, 0x08,
     283,   27,   27,   27, 0x08,
     291,   27,   27,   27, 0x08,
     298,   27,   27,   27, 0x08,
     314,   27,   27,   27, 0x08,
     328,   27,   27,   27, 0x08,
     335,   27,   27,   27, 0x08,
     344,   27,   27,   27, 0x08,
     355,   27,   27,   27, 0x08,
     362,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Merge[] = {
    "US_Hydrodyn_Pdb_Tool_Merge\0\0setupGUI()\0"
    ",\0table_value(int,int)\0update_enables()\0"
    "sel_auto()\0sel_from_to_merge()\0"
    "sel_from_to_fit()\0sel_to_to_fit()\0"
    "sel_to_to_cut()\0extra_chains()\0"
    "only_closest()\0delete_row()\0clear()\0"
    "load()\0validate()\0csv_save()\0chains_from()\0"
    "chains_to()\0target()\0start()\0trial()\0"
    "stop()\0clear_display()\0update_font()\0"
    "save()\0cancel()\0pdb_tool()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Pdb_Tool_Merge::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Pdb_Tool_Merge *_t = static_cast<US_Hydrodyn_Pdb_Tool_Merge *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->table_value((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->update_enables(); break;
        case 3: _t->sel_auto(); break;
        case 4: _t->sel_from_to_merge(); break;
        case 5: _t->sel_from_to_fit(); break;
        case 6: _t->sel_to_to_fit(); break;
        case 7: _t->sel_to_to_cut(); break;
        case 8: _t->extra_chains(); break;
        case 9: _t->only_closest(); break;
        case 10: _t->delete_row(); break;
        case 11: _t->clear(); break;
        case 12: _t->load(); break;
        case 13: _t->validate(); break;
        case 14: _t->csv_save(); break;
        case 15: _t->chains_from(); break;
        case 16: _t->chains_to(); break;
        case 17: _t->target(); break;
        case 18: _t->start(); break;
        case 19: _t->trial(); break;
        case 20: _t->stop(); break;
        case 21: _t->clear_display(); break;
        case 22: _t->update_font(); break;
        case 23: _t->save(); break;
        case 24: _t->cancel(); break;
        case 25: _t->pdb_tool(); break;
        case 26: _t->help(); break;
        case 27: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Pdb_Tool_Merge::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Pdb_Tool_Merge::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Merge,
      qt_meta_data_US_Hydrodyn_Pdb_Tool_Merge, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Pdb_Tool_Merge::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Pdb_Tool_Merge::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Pdb_Tool_Merge::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Merge))
        return static_cast<void*>(const_cast< US_Hydrodyn_Pdb_Tool_Merge*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Pdb_Tool_Merge::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
