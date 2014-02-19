/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_options.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_options.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_options.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_SaxsOptions[] = {

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
      25,   24,   24,   24, 0x08,
      36,   24,   24,   24, 0x08,
      55,   24,   24,   24, 0x08,
      74,   24,   24,   24, 0x08,
      94,   24,   24,   24, 0x08,
     119,   24,   24,   24, 0x08,
     143,   24,   24,   24, 0x08,
     165,   24,   24,   24, 0x08,
     183,   24,   24,   24, 0x08,
     202,   24,   24,   24, 0x08,
     229,   24,   24,   24, 0x08,
     238,   24,   24,   24, 0x08,
     245,   24,   24,   24, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_SaxsOptions[] = {
    "US_Hydrodyn_SaxsOptions\0\0setupGUI()\0"
    "sas_options_saxs()\0sas_options_sans()\0"
    "sas_options_curve()\0sas_options_bead_model()\0"
    "sas_options_hydration()\0sas_options_guinier()\0"
    "sas_options_xsr()\0sas_options_misc()\0"
    "sas_options_experimental()\0cancel()\0"
    "help()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_SaxsOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_SaxsOptions *_t = static_cast<US_Hydrodyn_SaxsOptions *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->sas_options_saxs(); break;
        case 2: _t->sas_options_sans(); break;
        case 3: _t->sas_options_curve(); break;
        case 4: _t->sas_options_bead_model(); break;
        case 5: _t->sas_options_hydration(); break;
        case 6: _t->sas_options_guinier(); break;
        case 7: _t->sas_options_xsr(); break;
        case 8: _t->sas_options_misc(); break;
        case 9: _t->sas_options_experimental(); break;
        case 10: _t->cancel(); break;
        case 11: _t->help(); break;
        case 12: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_SaxsOptions::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_SaxsOptions::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_SaxsOptions,
      qt_meta_data_US_Hydrodyn_SaxsOptions, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_SaxsOptions::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_SaxsOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_SaxsOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_SaxsOptions))
        return static_cast<void*>(const_cast< US_Hydrodyn_SaxsOptions*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_SaxsOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
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
