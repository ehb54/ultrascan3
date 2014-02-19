/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_file2.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_file2.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_file2.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_File2[] = {

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
      19,   18,   18,   18, 0x08,
      40,   18,   18,   18, 0x08,
      52,   18,   18,   18, 0x08,
      63,   18,   18,   18, 0x08,
      75,   18,   18,   18, 0x08,
      82,   18,   18,   18, 0x08,
     101,   18,   18,   18, 0x08,
     113,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_File2[] = {
    "US_Hydrodyn_File2\0\0update_base(QString)\0"
    "overwrite()\0auto_inc()\0try_again()\0"
    "help()\0do_overwrite_all()\0do_cancel()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_File2::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_File2 *_t = static_cast<US_Hydrodyn_File2 *>(_o);
        switch (_id) {
        case 0: _t->update_base((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->overwrite(); break;
        case 2: _t->auto_inc(); break;
        case 3: _t->try_again(); break;
        case 4: _t->help(); break;
        case 5: _t->do_overwrite_all(); break;
        case 6: _t->do_cancel(); break;
        case 7: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_File2::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_File2::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_File2,
      qt_meta_data_US_Hydrodyn_File2, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_File2::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_File2::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_File2::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_File2))
        return static_cast<void*>(const_cast< US_Hydrodyn_File2*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_File2::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
