/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_overlap_reduction.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_overlap_reduction.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_overlap_reduction.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_OR[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x08,
      27,   15,   15,   15, 0x08,
      38,   15,   15,   15, 0x08,
      53,   15,   15,   15, 0x08,
      64,   15,   15,   15, 0x08,
      80,   15,   15,   15, 0x08,
      93,   15,   15,   15, 0x08,
     116,   15,   15,   15, 0x08,
     128,   15,   15,   15, 0x08,
     148,   15,   15,   15, 0x08,
     168,   15,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_OR[] = {
    "US_Hydrodyn_OR\0\0setupGUI()\0set_fuse()\0"
    "set_hierarch()\0set_sync()\0set_translate()\0"
    "set_remove()\0set_replicate_method()\0"
    "replicate()\0update_fuse(double)\0"
    "update_sync(double)\0update_hierarch(double)\0"
};

void US_Hydrodyn_OR::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_OR *_t = static_cast<US_Hydrodyn_OR *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->set_fuse(); break;
        case 2: _t->set_hierarch(); break;
        case 3: _t->set_sync(); break;
        case 4: _t->set_translate(); break;
        case 5: _t->set_remove(); break;
        case 6: _t->set_replicate_method(); break;
        case 7: _t->replicate(); break;
        case 8: _t->update_fuse((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: _t->update_sync((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 10: _t->update_hierarch((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_OR::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_OR::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_OR,
      qt_meta_data_US_Hydrodyn_OR, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_OR::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_OR::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_OR::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_OR))
        return static_cast<void*>(const_cast< US_Hydrodyn_OR*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_OR::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
