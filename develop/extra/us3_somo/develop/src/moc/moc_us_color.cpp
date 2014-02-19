/****************************************************************************
** Meta object code from reading C++ file 'us_color.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_color.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_color.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Color[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      29,    9,    9,    9, 0x0a,
      41,    9,    9,    9, 0x0a,
      48,    9,    9,    9, 0x0a,
      62,    9,    9,    9, 0x0a,
      78,    9,    9,    9, 0x0a,
     101,    9,    9,    9, 0x0a,
     126,    9,    9,    9, 0x0a,
     134,    9,    9,    9, 0x0a,
     142,    9,    9,    9, 0x0a,
     149,    9,    9,    9, 0x0a,
     159,    9,    9,    9, 0x0a,
     180,    9,    9,    9, 0x0a,
     199,    9,    9,    9, 0x0a,
     213,    9,    9,    9, 0x0a,
     227,    9,    9,    9, 0x0a,
     241,    9,    9,    9, 0x0a,
     255,    9,    9,    9, 0x0a,
     269,    9,    9,    9, 0x0a,
     283,    9,    9,    9, 0x0a,
     299,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_US_Color[] = {
    "US_Color\0\0marginChanged(int)\0setup_GUI()\0"
    "help()\0set_default()\0sel_margin(int)\0"
    "update_widgets(double)\0update_save_str(QString)\0"
    "apply()\0reset()\0quit()\0save_as()\0"
    "selected_scheme(int)\0selected_item(int)\0"
    "pick_color1()\0pick_color2()\0pick_color3()\0"
    "pick_color4()\0pick_color5()\0pick_color6()\0"
    "delete_scheme()\0closeEvent(QCloseEvent*)\0"
};

void US_Color::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Color *_t = static_cast<US_Color *>(_o);
        switch (_id) {
        case 0: _t->marginChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->setup_GUI(); break;
        case 2: _t->help(); break;
        case 3: _t->set_default(); break;
        case 4: _t->sel_margin((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->update_widgets((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->update_save_str((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->apply(); break;
        case 8: _t->reset(); break;
        case 9: _t->quit(); break;
        case 10: _t->save_as(); break;
        case 11: _t->selected_scheme((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->selected_item((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->pick_color1(); break;
        case 14: _t->pick_color2(); break;
        case 15: _t->pick_color3(); break;
        case 16: _t->pick_color4(); break;
        case 17: _t->pick_color5(); break;
        case 18: _t->pick_color6(); break;
        case 19: _t->delete_scheme(); break;
        case 20: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Color::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Color::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Color,
      qt_meta_data_US_Color, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Color::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Color::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Color::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Color))
        return static_cast<void*>(const_cast< US_Color*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Color::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void US_Color::marginChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
