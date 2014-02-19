/****************************************************************************
** Meta object code from reading C++ file 'us_font.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_font.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_font.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Font[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x0a,
      18,    8,    8,    8, 0x0a,
      26,    8,    8,    8, 0x0a,
      33,    8,    8,    8, 0x0a,
      46,    8,    8,    8, 0x09,
      58,    8,    8,    8, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Font[] = {
    "US_Font\0\0cancel()\0check()\0help()\0"
    "selectFont()\0setup_GUI()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Font::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Font *_t = static_cast<US_Font *>(_o);
        switch (_id) {
        case 0: _t->cancel(); break;
        case 1: _t->check(); break;
        case 2: _t->help(); break;
        case 3: _t->selectFont(); break;
        case 4: _t->setup_GUI(); break;
        case 5: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Font::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Font::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Font,
      qt_meta_data_US_Font, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Font::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Font::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Font::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Font))
        return static_cast<void*>(const_cast< US_Font*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Font::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
