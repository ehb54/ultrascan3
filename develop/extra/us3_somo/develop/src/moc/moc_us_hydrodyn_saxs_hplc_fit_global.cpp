/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_fit_global.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_hplc_fit_global.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_hplc_fit_global.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Hplc_Fit_Global[] = {

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
      34,   33,   33,   33, 0x08,
      51,   33,   33,   33, 0x08,
      61,   33,   33,   33, 0x08,
      68,   33,   33,   33, 0x08,
      73,   33,   33,   33, 0x08,
      82,   33,   33,   33, 0x08,
      91,   33,   33,   33, 0x08,
     100,   33,   33,   33, 0x08,
     105,   33,   33,   33, 0x08,
     112,   33,   33,   33, 0x08,
     119,   33,   33,   33, 0x08,
     128,   33,   33,   33, 0x08,
     135,   33,   33,   33, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Fit_Global[] = {
    "US_Hydrodyn_Saxs_Hplc_Fit_Global\0\0"
    "update_enables()\0restore()\0undo()\0"
    "lm()\0gsm_sd()\0gsm_ih()\0gsm_cg()\0ga()\0"
    "grid()\0stop()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Hplc_Fit_Global::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Hplc_Fit_Global *_t = static_cast<US_Hydrodyn_Saxs_Hplc_Fit_Global *>(_o);
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

const QMetaObjectExtraData US_Hydrodyn_Saxs_Hplc_Fit_Global::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Hplc_Fit_Global::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Fit_Global,
      qt_meta_data_US_Hydrodyn_Saxs_Hplc_Fit_Global, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Hplc_Fit_Global::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Hplc_Fit_Global::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Hplc_Fit_Global::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Fit_Global))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Hplc_Fit_Global*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Hplc_Fit_Global::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
