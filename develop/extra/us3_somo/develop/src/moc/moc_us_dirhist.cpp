/****************************************************************************
** Meta object code from reading C++ file 'us_dirhist.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_dirhist.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_dirhist.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Dirhist[] = {

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
      12,   11,   11,   11, 0x08,
      37,   33,   11,   11, 0x08,
      80,   56,   11,   11, 0x08,
     116,   11,   11,   11, 0x08,
     122,   11,   11,   11, 0x08,
     127,   11,   11,   11, 0x08,
     136,   11,   11,   11, 0x08,
     143,   11,   11,   11, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Dirhist[] = {
    "US_Dirhist\0\0t_selectionChanged()\0col\0"
    "t_sort_column(int)\0row,col,button,mousePos\0"
    "t_doubleClicked(int,int,int,QPoint)\0"
    "del()\0ok()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Dirhist::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Dirhist *_t = static_cast<US_Dirhist *>(_o);
        switch (_id) {
        case 0: _t->t_selectionChanged(); break;
        case 1: _t->t_sort_column((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->t_doubleClicked((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< const QPoint(*)>(_a[4]))); break;
        case 3: _t->del(); break;
        case 4: _t->ok(); break;
        case 5: _t->cancel(); break;
        case 6: _t->help(); break;
        case 7: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Dirhist::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Dirhist::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Dirhist,
      qt_meta_data_US_Dirhist, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Dirhist::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Dirhist::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Dirhist::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Dirhist))
        return static_cast<void*>(const_cast< US_Dirhist*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Dirhist::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
