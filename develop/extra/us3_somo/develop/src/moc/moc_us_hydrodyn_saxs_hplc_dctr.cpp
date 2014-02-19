/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_dctr.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_hplc_dctr.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_hplc_dctr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Hplc_Dctr[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      37,   27,   27,   27, 0x08,
      46,   27,   27,   27, 0x08,
      68,   27,   27,   27, 0x08,
      90,   27,   27,   27, 0x08,
      97,   27,   27,   27, 0x08,
     104,   27,   27,   27, 0x08,
     111,   27,   27,   27, 0x08,
     118,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Dctr[] = {
    "US_Hydrodyn_Saxs_Hplc_Dctr\0\0set_uv()\0"
    "set_ri()\0ri_conv_text(QString)\0"
    "uv_conv_text(QString)\0help()\0quit()\0"
    "keep()\0save()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Hplc_Dctr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Hplc_Dctr *_t = static_cast<US_Hydrodyn_Saxs_Hplc_Dctr *>(_o);
        switch (_id) {
        case 0: _t->set_uv(); break;
        case 1: _t->set_ri(); break;
        case 2: _t->ri_conv_text((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->uv_conv_text((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->help(); break;
        case 5: _t->quit(); break;
        case 6: _t->keep(); break;
        case 7: _t->save(); break;
        case 8: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Hplc_Dctr::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Hplc_Dctr::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Dctr,
      qt_meta_data_US_Hydrodyn_Saxs_Hplc_Dctr, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Hplc_Dctr::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Hplc_Dctr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Hplc_Dctr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Dctr))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Hplc_Dctr*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Hplc_Dctr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
