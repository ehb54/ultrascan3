/****************************************************************************
** Meta object code from reading C++ file 'us_license.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_license.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_license.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_License[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      29,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      19,   11,   11,   11, 0x0a,
      28,   11,   11,   11, 0x0a,
      35,   11,   11,   11, 0x0a,
      44,   11,   11,   11, 0x0a,
      54,   11,   11,   11, 0x0a,
      64,   11,   11,   11, 0x0a,
      90,   11,   11,   11, 0x0a,
     115,   11,   11,   11, 0x0a,
     143,   11,   11,   11, 0x0a,
     167,   11,   11,   11, 0x0a,
     188,   11,   11,   11, 0x0a,
     206,   11,   11,   11, 0x0a,
     226,   11,   11,   11, 0x0a,
     248,   11,   11,   11, 0x0a,
     270,   11,   11,   11, 0x0a,
     290,   11,   11,   11, 0x0a,
     314,   11,   11,   11, 0x0a,
     335,   11,   11,   11, 0x0a,
     362,   11,   11,   11, 0x0a,
     382,   11,   11,   11, 0x0a,
     400,   11,   11,   11, 0x0a,
     418,   11,   11,   11, 0x0a,
     434,   11,   11,   11, 0x0a,
     450,   11,   11,   11, 0x0a,
     465,   11,   11,   11, 0x0a,
     481,   11,   11,   11, 0x0a,
     497,   11,   11,   11, 0x0a,
     510,   11,   11,   11, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_License[] = {
    "US_License\0\0help()\0cancel()\0save()\0"
    "import()\0request()\0display()\0"
    "update_firstname(QString)\0"
    "update_lastname(QString)\0"
    "update_institution(QString)\0"
    "update_address(QString)\0update_city(QString)\0"
    "update_state(int)\0update_zip(QString)\0"
    "update_phone(QString)\0update_email(QString)\0"
    "update_version(int)\0update_licensetype(int)\0"
    "update_code(QString)\0update_expiration(QString)\0"
    "update_opteron_rb()\0update_intel_rb()\0"
    "update_sparc_rb()\0update_mac_rb()\0"
    "update_sgi_rb()\0update_os(int)\0"
    "captureStdout()\0captureStderr()\0"
    "endProcess()\0closeEvent(QCloseEvent*)\0"
};

void US_License::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_License *_t = static_cast<US_License *>(_o);
        switch (_id) {
        case 0: _t->help(); break;
        case 1: _t->cancel(); break;
        case 2: _t->save(); break;
        case 3: _t->import(); break;
        case 4: _t->request(); break;
        case 5: _t->display(); break;
        case 6: _t->update_firstname((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_lastname((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_institution((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_address((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->update_city((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->update_state((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->update_zip((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->update_phone((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->update_email((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: _t->update_version((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->update_licensetype((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->update_code((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 18: _t->update_expiration((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 19: _t->update_opteron_rb(); break;
        case 20: _t->update_intel_rb(); break;
        case 21: _t->update_sparc_rb(); break;
        case 22: _t->update_mac_rb(); break;
        case 23: _t->update_sgi_rb(); break;
        case 24: _t->update_os((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: _t->captureStdout(); break;
        case 26: _t->captureStderr(); break;
        case 27: _t->endProcess(); break;
        case 28: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_License::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_License::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_License,
      qt_meta_data_US_License, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_License::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_License::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_License::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_License))
        return static_cast<void*>(const_cast< US_License*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_License::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 29)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 29;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
