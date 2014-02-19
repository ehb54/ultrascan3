/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_buffer_conc.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_buffer_conc.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_buffer_conc.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Buffer_Conc[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      30,   29,   29,   29, 0x08,
      51,   47,   29,   29, 0x08,
      76,   29,   29,   29, 0x08,
      93,   29,   29,   29, 0x08,
     100,   29,   29,   29, 0x08,
     107,   29,   29,   29, 0x08,
     114,   29,   29,   29, 0x08,
     122,   29,   29,   29, 0x08,
     134,   29,   29,   29, 0x08,
     143,   29,   29,   29, 0x08,
     150,   29,   29,   29, 0x08,
     159,   29,   29,   29, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Buffer_Conc[] = {
    "US_Hydrodyn_Saxs_Buffer_Conc\0\0"
    "update_enables()\0row\0row_header_released(int)\0"
    "sort_column(int)\0load()\0save()\0copy()\0"
    "paste()\0paste_all()\0cancel()\0help()\0"
    "set_ok()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Buffer_Conc::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Buffer_Conc *_t = static_cast<US_Hydrodyn_Saxs_Buffer_Conc *>(_o);
        switch (_id) {
        case 0: _t->update_enables(); break;
        case 1: _t->row_header_released((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sort_column((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->load(); break;
        case 4: _t->save(); break;
        case 5: _t->copy(); break;
        case 6: _t->paste(); break;
        case 7: _t->paste_all(); break;
        case 8: _t->cancel(); break;
        case 9: _t->help(); break;
        case 10: _t->set_ok(); break;
        case 11: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Buffer_Conc::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Buffer_Conc::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Buffer_Conc,
      qt_meta_data_US_Hydrodyn_Saxs_Buffer_Conc, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Buffer_Conc::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Buffer_Conc::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Buffer_Conc::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Buffer_Conc))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Buffer_Conc*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Buffer_Conc::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
