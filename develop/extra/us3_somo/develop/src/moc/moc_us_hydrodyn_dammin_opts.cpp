/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_dammin_opts.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_dammin_opts.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_dammin_opts.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Dammin_Opts[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   24,   24,   24, 0x08,
      45,   24,   24,   24, 0x08,
      64,   24,   24,   24, 0x08,
      88,   24,   24,   24, 0x08,
     111,   24,   24,   24, 0x08,
     126,   24,   24,   24, 0x08,
     144,   24,   24,   24, 0x08,
     153,   24,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Dammin_Opts[] = {
    "US_Hydrodyn_Dammin_Opts\0\0update_psv(QString)\0"
    "update_mw(QString)\0update_partial(QString)\0"
    "set_write_bead_model()\0set_remember()\0"
    "set_use_partial()\0cancel()\0help()\0"
};

void US_Hydrodyn_Dammin_Opts::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Dammin_Opts *_t = static_cast<US_Hydrodyn_Dammin_Opts *>(_o);
        switch (_id) {
        case 0: _t->update_psv((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update_mw((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_partial((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->set_write_bead_model(); break;
        case 4: _t->set_remember(); break;
        case 5: _t->set_use_partial(); break;
        case 6: _t->cancel(); break;
        case 7: _t->help(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Dammin_Opts::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Dammin_Opts::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Dammin_Opts,
      qt_meta_data_US_Hydrodyn_Dammin_Opts, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Dammin_Opts::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Dammin_Opts::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Dammin_Opts::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Dammin_Opts))
        return static_cast<void*>(const_cast< US_Hydrodyn_Dammin_Opts*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Dammin_Opts::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
