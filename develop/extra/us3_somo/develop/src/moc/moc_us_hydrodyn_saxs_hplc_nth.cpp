/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_nth.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_hplc_nth.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_hplc_nth.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Hplc_Nth[] = {

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
      27,   26,   26,   26, 0x08,
      38,   26,   26,   26, 0x08,
      48,   26,   26,   26, 0x08,
      64,   26,   26,   26, 0x08,
      79,   26,   26,   26, 0x08,
     103,   26,   26,   26, 0x08,
     108,   26,   26,   26, 0x08,
     115,   26,   26,   26, 0x08,
     122,   26,   26,   26, 0x08,
     139,   26,   26,   26, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Nth[] = {
    "US_Hydrodyn_Saxs_Hplc_Nth\0\0nth_only()\0"
    "nth_add()\0contains_only()\0contains_add()\0"
    "update_files_selected()\0go()\0quit()\0"
    "help()\0update_enables()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Hplc_Nth::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Hplc_Nth *_t = static_cast<US_Hydrodyn_Saxs_Hplc_Nth *>(_o);
        switch (_id) {
        case 0: _t->nth_only(); break;
        case 1: _t->nth_add(); break;
        case 2: _t->contains_only(); break;
        case 3: _t->contains_add(); break;
        case 4: _t->update_files_selected(); break;
        case 5: _t->go(); break;
        case 6: _t->quit(); break;
        case 7: _t->help(); break;
        case 8: _t->update_enables(); break;
        case 9: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Hplc_Nth::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Hplc_Nth::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Nth,
      qt_meta_data_US_Hydrodyn_Saxs_Hplc_Nth, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Hplc_Nth::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Hplc_Nth::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Hplc_Nth::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Nth))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Hplc_Nth*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Hplc_Nth::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
