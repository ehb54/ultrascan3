/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_results.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_results.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_results.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Results[] = {

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
      21,   20,   20,   20, 0x0a,
      30,   20,   20,   20, 0x08,
      41,   20,   20,   20, 0x08,
      56,   20,   20,   20, 0x08,
      67,   20,   20,   20, 0x08,
      84,   20,   20,   20, 0x08,
      91,   20,   20,   20, 0x08,
     110,   20,   20,   20, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Results[] = {
    "US_Hydrodyn_Results\0\0cancel()\0setupGUI()\0"
    "load_results()\0load_asa()\0load_beadmodel()\0"
    "help()\0view_file(QString)\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Results::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Results *_t = static_cast<US_Hydrodyn_Results *>(_o);
        switch (_id) {
        case 0: _t->cancel(); break;
        case 1: _t->setupGUI(); break;
        case 2: _t->load_results(); break;
        case 3: _t->load_asa(); break;
        case 4: _t->load_beadmodel(); break;
        case 5: _t->help(); break;
        case 6: _t->view_file((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Results::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Results::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Results,
      qt_meta_data_US_Hydrodyn_Results, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Results::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Results::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Results::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Results))
        return static_cast<void*>(const_cast< US_Hydrodyn_Results*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Results::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
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
