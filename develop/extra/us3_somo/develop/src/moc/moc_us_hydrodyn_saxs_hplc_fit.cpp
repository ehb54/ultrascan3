/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_fit.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_hplc_fit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_hplc_fit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Hplc_Fit[] = {

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
      27,   26,   26,   26, 0x08,
      44,   26,   26,   26, 0x08,
      54,   26,   26,   26, 0x08,
      61,   26,   26,   26, 0x08,
      66,   26,   26,   26, 0x08,
      75,   26,   26,   26, 0x08,
      84,   26,   26,   26, 0x08,
      93,   26,   26,   26, 0x08,
      98,   26,   26,   26, 0x08,
     105,   26,   26,   26, 0x08,
     112,   26,   26,   26, 0x08,
     121,   26,   26,   26, 0x08,
     128,   26,   26,   26, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Fit[] = {
    "US_Hydrodyn_Saxs_Hplc_Fit\0\0update_enables()\0"
    "restore()\0undo()\0lm()\0gsm_sd()\0gsm_ih()\0"
    "gsm_cg()\0ga()\0grid()\0stop()\0cancel()\0"
    "help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Hplc_Fit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Hplc_Fit *_t = static_cast<US_Hydrodyn_Saxs_Hplc_Fit *>(_o);
        switch (_id) {
        case 0: _t->update_enables(); break;
        case 1: _t->restore(); break;
        case 2: _t->undo(); break;
        case 3: _t->lm(); break;
        case 4: _t->gsm_sd(); break;
        case 5: _t->gsm_ih(); break;
        case 6: _t->gsm_cg(); break;
        case 7: _t->ga(); break;
        case 8: _t->grid(); break;
        case 9: _t->stop(); break;
        case 10: _t->cancel(); break;
        case 11: _t->help(); break;
        case 12: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Hplc_Fit::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Hplc_Fit::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Fit,
      qt_meta_data_US_Hydrodyn_Saxs_Hplc_Fit, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Hplc_Fit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Hplc_Fit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Hplc_Fit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Fit))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Hplc_Fit*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Hplc_Fit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
