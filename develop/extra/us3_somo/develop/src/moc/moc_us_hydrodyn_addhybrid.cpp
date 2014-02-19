/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_addhybrid.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_addhybrid.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_addhybrid.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_AddHybridization[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x08,
      27,   20,   20,   20, 0x08,
      34,   20,   20,   20, 0x08,
      48,   20,   20,   20, 0x08,
      67,   20,   20,   20, 0x08,
      78,   20,   20,   20, 0x08,
      99,   20,   20,   20, 0x08,
     118,   20,   20,   20, 0x08,
     141,   20,   20,   20, 0x08,
     166,   20,   20,   20, 0x08,
     192,   20,   20,   20, 0x08,
     218,   20,   20,   20, 0x08,
     237,   20,   20,   20, 0x08,
     254,   20,   20,   20, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_AddHybridization[] = {
    "US_AddHybridization\0\0add()\0help()\0"
    "select_file()\0select_saxs_file()\0"
    "setupGUI()\0update_name(QString)\0"
    "update_mw(QString)\0update_radius(QString)\0"
    "update_scat_len(QString)\0"
    "update_exch_prot(QString)\0"
    "update_num_elect(QString)\0select_hybrid(int)\0"
    "select_saxs(int)\0closeEvent(QCloseEvent*)\0"
};

void US_AddHybridization::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_AddHybridization *_t = static_cast<US_AddHybridization *>(_o);
        switch (_id) {
        case 0: _t->add(); break;
        case 1: _t->help(); break;
        case 2: _t->select_file(); break;
        case 3: _t->select_saxs_file(); break;
        case 4: _t->setupGUI(); break;
        case 5: _t->update_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_mw((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_radius((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_scat_len((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_exch_prot((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->update_num_elect((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->select_hybrid((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->select_saxs((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_AddHybridization::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_AddHybridization::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_US_AddHybridization,
      qt_meta_data_US_AddHybridization, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_AddHybridization::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_AddHybridization::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_AddHybridization::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_AddHybridization))
        return static_cast<void*>(const_cast< US_AddHybridization*>(this));
    return QWidget::qt_metacast(_clname);
}

int US_AddHybridization::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
