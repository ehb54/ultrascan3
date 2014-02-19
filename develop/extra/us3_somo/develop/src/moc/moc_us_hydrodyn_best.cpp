/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_best.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_best.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_best.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Best[] = {

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
      18,   17,   17,   17, 0x08,
      25,   17,   17,   17, 0x08,
      40,   17,   17,   17, 0x08,
      55,   17,   17,   17, 0x08,
      68,   17,   17,   17, 0x08,
      84,   17,   17,   17, 0x08,
      98,   17,   17,   17, 0x08,
     105,   17,   17,   17, 0x08,
     119,   17,   17,   17, 0x08,
     126,   17,   17,   17, 0x08,
     152,  135,   17,   17, 0x08,
     172,   17,   17,   17, 0x28,
     196,  188,   17,   17, 0x08,
     213,   17,   17,   17, 0x28,
     226,  188,   17,   17, 0x08,
     246,   17,   17,   17, 0x28,
     262,  188,   17,   17, 0x08,
     283,   17,   17,   17, 0x28,
     300,   17,   17,   17, 0x08,
     316,   17,   17,   17, 0x08,
     335,   17,   17,   17, 0x08,
     359,  355,   17,   17, 0x08,
     382,   17,   17,   17, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Best[] = {
    "US_Hydrodyn_Best\0\0load()\0join_results()\0"
    "save_results()\0hide_input()\0clear_display()\0"
    "update_font()\0save()\0hide_editor()\0"
    "help()\0cancel()\0do_recompute_tau\0"
    "data_selected(bool)\0data_selected()\0"
    "do_data\0cb_changed(bool)\0cb_changed()\0"
    "cb_changed_ln(bool)\0cb_changed_ln()\0"
    "cb_changed_exp(bool)\0cb_changed_exp()\0"
    "toggle_points()\0toggle_points_ln()\0"
    "toggle_points_exp()\0str\0set_last_file(QString)\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Best::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Best *_t = static_cast<US_Hydrodyn_Best *>(_o);
        switch (_id) {
        case 0: _t->load(); break;
        case 1: _t->join_results(); break;
        case 2: _t->save_results(); break;
        case 3: _t->hide_input(); break;
        case 4: _t->clear_display(); break;
        case 5: _t->update_font(); break;
        case 6: _t->save(); break;
        case 7: _t->hide_editor(); break;
        case 8: _t->help(); break;
        case 9: _t->cancel(); break;
        case 10: _t->data_selected((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->data_selected(); break;
        case 12: _t->cb_changed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->cb_changed(); break;
        case 14: _t->cb_changed_ln((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->cb_changed_ln(); break;
        case 16: _t->cb_changed_exp((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->cb_changed_exp(); break;
        case 18: _t->toggle_points(); break;
        case 19: _t->toggle_points_ln(); break;
        case 20: _t->toggle_points_exp(); break;
        case 21: _t->set_last_file((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 22: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Best::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Best::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Best,
      qt_meta_data_US_Hydrodyn_Best, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Best::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Best::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Best::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Best))
        return static_cast<void*>(const_cast< US_Hydrodyn_Best*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Best::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
