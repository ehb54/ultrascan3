/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_pdb_tool_sort.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_pdb_tool_sort.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_pdb_tool_sort.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Pdb_Tool_Sort[] = {

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
      27,   26,   26,   26, 0x08,
      53,   26,   26,   26, 0x08,
      79,   26,   26,   26, 0x08,
     107,   26,   26,   26, 0x08,
     119,   26,   26,   26, 0x08,
     132,   26,   26,   26, 0x08,
     139,   26,   26,   26, 0x08,
     148,   26,   26,   26, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Sort[] = {
    "US_Hydrodyn_Pdb_Tool_Sort\0\0"
    "update_residuesa(QString)\0"
    "update_residuesb(QString)\0"
    "update_reportcount(QString)\0set_order()\0"
    "set_caonly()\0help()\0cancel()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Pdb_Tool_Sort::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Pdb_Tool_Sort *_t = static_cast<US_Hydrodyn_Pdb_Tool_Sort *>(_o);
        switch (_id) {
        case 0: _t->update_residuesa((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update_residuesb((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_reportcount((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->set_order(); break;
        case 4: _t->set_caonly(); break;
        case 5: _t->help(); break;
        case 6: _t->cancel(); break;
        case 7: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Pdb_Tool_Sort::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Pdb_Tool_Sort::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Sort,
      qt_meta_data_US_Hydrodyn_Pdb_Tool_Sort, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Pdb_Tool_Sort::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Pdb_Tool_Sort::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Pdb_Tool_Sort::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Sort))
        return static_cast<void*>(const_cast< US_Hydrodyn_Pdb_Tool_Sort*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Pdb_Tool_Sort::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
