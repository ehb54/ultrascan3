/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_conc.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_conc.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_conc.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Conc[] = {

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
      23,   22,   22,   22, 0x08,
      44,   40,   22,   22, 0x08,
      69,   22,   22,   22, 0x08,
      86,   22,   22,   22, 0x08,
      93,   22,   22,   22, 0x08,
     100,   22,   22,   22, 0x08,
     120,   22,   22,   22, 0x08,
     127,   22,   22,   22, 0x08,
     135,   22,   22,   22, 0x08,
     147,   22,   22,   22, 0x08,
     156,   22,   22,   22, 0x08,
     163,   22,   22,   22, 0x08,
     172,   22,   22,   22, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Conc[] = {
    "US_Hydrodyn_Saxs_Conc\0\0update_enables()\0"
    "row\0row_header_released(int)\0"
    "sort_column(int)\0load()\0save()\0"
    "reset_to_defaults()\0copy()\0paste()\0"
    "paste_all()\0cancel()\0help()\0set_ok()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Conc::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Conc *_t = static_cast<US_Hydrodyn_Saxs_Conc *>(_o);
        switch (_id) {
        case 0: _t->update_enables(); break;
        case 1: _t->row_header_released((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sort_column((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->load(); break;
        case 4: _t->save(); break;
        case 5: _t->reset_to_defaults(); break;
        case 6: _t->copy(); break;
        case 7: _t->paste(); break;
        case 8: _t->paste_all(); break;
        case 9: _t->cancel(); break;
        case 10: _t->help(); break;
        case 11: _t->set_ok(); break;
        case 12: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Conc::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Conc::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Conc,
      qt_meta_data_US_Hydrodyn_Saxs_Conc, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Conc::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Conc::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Conc::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Conc))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Conc*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Conc::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
