/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_search.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_search.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_search.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Search[] = {

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
      25,   24,   24,   24, 0x08,
      38,   36,   24,   24, 0x08,
      59,   24,   24,   24, 0x08,
      73,   24,   24,   24, 0x08,
      87,   24,   24,   24, 0x08,
     104,   24,   24,   24, 0x08,
     117,   24,   24,   24, 0x08,
     125,   24,   24,   24, 0x08,
     139,   24,   24,   24, 0x08,
     150,   24,   24,   24, 0x08,
     157,   24,   24,   24, 0x08,
     173,   24,   24,   24, 0x08,
     187,   24,   24,   24, 0x08,
     194,   24,   24,   24, 0x08,
     203,   24,   24,   24, 0x08,
     210,   24,   24,   24, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Search[] = {
    "US_Hydrodyn_Saxs_Search\0\0setupGUI()\0"
    ",\0table_value(int,int)\0save_to_csv()\0"
    "replot_saxs()\0save_saxs_plot()\0"
    "set_target()\0start()\0run_current()\0"
    "run_best()\0stop()\0clear_display()\0"
    "update_font()\0save()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Search::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Search *_t = static_cast<US_Hydrodyn_Saxs_Search *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->table_value((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->save_to_csv(); break;
        case 3: _t->replot_saxs(); break;
        case 4: _t->save_saxs_plot(); break;
        case 5: _t->set_target(); break;
        case 6: _t->start(); break;
        case 7: _t->run_current(); break;
        case 8: _t->run_best(); break;
        case 9: _t->stop(); break;
        case 10: _t->clear_display(); break;
        case 11: _t->update_font(); break;
        case 12: _t->save(); break;
        case 13: _t->cancel(); break;
        case 14: _t->help(); break;
        case 15: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Search::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Search::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Search,
      qt_meta_data_US_Hydrodyn_Saxs_Search, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Search::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Search::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Search::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Search))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Search*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Search::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
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
