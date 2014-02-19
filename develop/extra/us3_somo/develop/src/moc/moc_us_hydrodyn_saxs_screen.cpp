/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_screen.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_screen.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_screen.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Screen[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   24,   24,   24, 0x08,
      38,   36,   24,   24, 0x08,
      59,   24,   24,   24, 0x08,
      66,   24,   24,   24, 0x08,
      83,   24,   24,   24, 0x08,
     100,   24,   24,   24, 0x08,
     112,   24,   24,   24, 0x08,
     124,   24,   24,   24, 0x08,
     133,   24,   24,   24, 0x08,
     147,   24,   24,   24, 0x08,
     164,   24,   24,   24, 0x08,
     177,   24,   24,   24, 0x08,
     198,   24,   24,   24, 0x08,
     236,  220,   24,   24, 0x08,
     248,   24,   24,   24, 0x28,
     256,   24,   24,   24, 0x08,
     274,   24,   24,   24, 0x08,
     281,   24,   24,   24, 0x08,
     297,   24,   24,   24, 0x08,
     311,   24,   24,   24, 0x08,
     318,   24,   24,   24, 0x08,
     327,   24,   24,   24, 0x08,
     334,   24,   24,   24, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Screen[] = {
    "US_Hydrodyn_Saxs_Screen\0\0setupGUI()\0"
    ",\0table_value(int,int)\0push()\0"
    "clear_plot_row()\0clear_plot_all()\0"
    "save_plot()\0load_plot()\0replot()\0"
    "replot_saxs()\0save_saxs_plot()\0"
    "set_target()\0adjust_wheel(double)\0"
    "adjust_wheel2(double)\0already_running\0"
    "start(bool)\0start()\0run_all_targets()\0"
    "stop()\0clear_display()\0update_font()\0"
    "save()\0cancel()\0help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Screen::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Screen *_t = static_cast<US_Hydrodyn_Saxs_Screen *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->table_value((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->push(); break;
        case 3: _t->clear_plot_row(); break;
        case 4: _t->clear_plot_all(); break;
        case 5: _t->save_plot(); break;
        case 6: _t->load_plot(); break;
        case 7: _t->replot(); break;
        case 8: _t->replot_saxs(); break;
        case 9: _t->save_saxs_plot(); break;
        case 10: _t->set_target(); break;
        case 11: _t->adjust_wheel((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 12: _t->adjust_wheel2((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 13: _t->start((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->start(); break;
        case 15: _t->run_all_targets(); break;
        case 16: _t->stop(); break;
        case 17: _t->clear_display(); break;
        case 18: _t->update_font(); break;
        case 19: _t->save(); break;
        case 20: _t->cancel(); break;
        case 21: _t->help(); break;
        case 22: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Screen::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Screen::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Screen,
      qt_meta_data_US_Hydrodyn_Saxs_Screen, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Screen::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Screen::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Screen::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Screen))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Screen*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Screen::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
