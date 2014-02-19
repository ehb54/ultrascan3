/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_ciq.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_hplc_ciq.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_hplc_ciq.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Hplc_Ciq[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x08,
      40,   26,   26,   26, 0x08,
      61,   26,   26,   26, 0x08,
      77,   26,   26,   26, 0x08,
     104,   26,   26,   26, 0x08,
     132,   26,   26,   26, 0x08,
     160,   26,   26,   26, 0x08,
     174,   26,   26,   26, 0x08,
     197,   26,   26,   26, 0x08,
     221,   26,   26,   26, 0x08,
     246,   26,   26,   26, 0x08,
     262,   26,   26,   26, 0x08,
     273,   26,   26,   26, 0x08,
     290,   26,   26,   26, 0x08,
     299,   26,   26,   26, 0x08,
     306,   26,   26,   26, 0x08,
     313,   26,   26,   26, 0x08,
     325,   26,   26,   26, 0x08,
     330,   26,   26,   26, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Ciq[] = {
    "US_Hydrodyn_Saxs_Hplc_Ciq\0\0set_add_bl()\0"
    "set_save_as_pct_iq()\0set_sd_source()\0"
    "set_sd_zero_avg_local_sd()\0"
    "set_sd_zero_keep_as_zeros()\0"
    "set_sd_zero_set_to_pt1pct()\0zeros_found()\0"
    "set_zero_drop_points()\0set_zero_avg_local_sd()\0"
    "set_zero_keep_as_zeros()\0set_normalize()\0"
    "set_I0se()\0update_enables()\0global()\0"
    "help()\0quit()\0create_ng()\0go()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Hplc_Ciq::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Hplc_Ciq *_t = static_cast<US_Hydrodyn_Saxs_Hplc_Ciq *>(_o);
        switch (_id) {
        case 0: _t->set_add_bl(); break;
        case 1: _t->set_save_as_pct_iq(); break;
        case 2: _t->set_sd_source(); break;
        case 3: _t->set_sd_zero_avg_local_sd(); break;
        case 4: _t->set_sd_zero_keep_as_zeros(); break;
        case 5: _t->set_sd_zero_set_to_pt1pct(); break;
        case 6: _t->zeros_found(); break;
        case 7: _t->set_zero_drop_points(); break;
        case 8: _t->set_zero_avg_local_sd(); break;
        case 9: _t->set_zero_keep_as_zeros(); break;
        case 10: _t->set_normalize(); break;
        case 11: _t->set_I0se(); break;
        case 12: _t->update_enables(); break;
        case 13: _t->global(); break;
        case 14: _t->help(); break;
        case 15: _t->quit(); break;
        case 16: _t->create_ng(); break;
        case 17: _t->go(); break;
        case 18: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Hplc_Ciq::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Hplc_Ciq::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Ciq,
      qt_meta_data_US_Hydrodyn_Saxs_Hplc_Ciq, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Hplc_Ciq::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Hplc_Ciq::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Hplc_Ciq::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Ciq))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Hplc_Ciq*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Hplc_Ciq::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
