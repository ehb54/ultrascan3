/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_xsr.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_xsr.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_xsr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Xsr[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x08,
      28,   16,   16,   16, 0x08,
      36,   16,   16,   16, 0x08,
      43,   16,   16,   16, 0x08,
      59,   16,   16,   16, 0x08,
      73,   16,   16,   16, 0x08,
      80,   16,   16,   16, 0x08,
      89,   16,   16,   16, 0x08,
      96,   16,   16,   16, 0x08,
     113,   16,   16,   16, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Xsr[] = {
    "US_Hydrodyn_Xsr\0\0setupGUI()\0start()\0"
    "stop()\0clear_display()\0update_font()\0"
    "save()\0cancel()\0help()\0update_enables()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Xsr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Xsr *_t = static_cast<US_Hydrodyn_Xsr *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->start(); break;
        case 2: _t->stop(); break;
        case 3: _t->clear_display(); break;
        case 4: _t->update_font(); break;
        case 5: _t->save(); break;
        case 6: _t->cancel(); break;
        case 7: _t->help(); break;
        case 8: _t->update_enables(); break;
        case 9: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Xsr::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Xsr::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Xsr,
      qt_meta_data_US_Hydrodyn_Xsr, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Xsr::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Xsr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Xsr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Xsr))
        return static_cast<void*>(const_cast< US_Hydrodyn_Xsr*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Xsr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
