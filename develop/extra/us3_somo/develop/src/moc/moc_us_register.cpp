/****************************************************************************
** Meta object code from reading C++ file 'us_register.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_register.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_register.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Register[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x08,
      35,   12,   12,   12, 0x08,
      53,   12,   48,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_US_Register[] = {
    "US_Register\0\0license_info(QString)\0"
    "us_license()\0bool\0read()\0"
};

void US_Register::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Register *_t = static_cast<US_Register *>(_o);
        switch (_id) {
        case 0: _t->license_info((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->us_license(); break;
        case 2: { bool _r = _t->read();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Register::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Register::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Register,
      qt_meta_data_US_Register, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Register::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Register::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Register::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Register))
        return static_cast<void*>(const_cast< US_Register*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Register::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
