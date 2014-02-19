/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_2d.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_2d.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_2d.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_2d[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x08,
      32,   20,   20,   20, 0x08,
      55,   20,   20,   20, 0x08,
      89,   20,   20,   20, 0x08,
     121,   20,   20,   20, 0x08,
     152,   20,   20,   20, 0x08,
     191,   20,   20,   20, 0x08,
     229,   20,   20,   20, 0x08,
     271,   20,   20,   20, 0x08,
     312,   20,   20,   20, 0x08,
     345,   20,   20,   20, 0x08,
     366,   20,   20,   20, 0x08,
     373,   20,   20,   20, 0x08,
     381,   20,   20,   20, 0x08,
     388,   20,   20,   20, 0x08,
     400,   20,   20,   20, 0x08,
     416,   20,   20,   20, 0x08,
     430,   20,   20,   20, 0x08,
     437,   20,   20,   20, 0x08,
     446,   20,   20,   20, 0x08,
     453,   20,   20,   20, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_2d[] = {
    "US_Hydrodyn_Saxs_2d\0\0setupGUI()\0"
    "update_lambda(QString)\0"
    "update_detector_distance(QString)\0"
    "update_detector_height(QString)\0"
    "update_detector_width(QString)\0"
    "update_detector_pixels_height(QString)\0"
    "update_detector_pixels_width(QString)\0"
    "update_beam_center_pixels_height(QString)\0"
    "update_beam_center_pixels_width(QString)\0"
    "update_sample_rotations(QString)\0"
    "adjust_wheel(double)\0info()\0start()\0"
    "stop()\0integrate()\0clear_display()\0"
    "update_font()\0save()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_2d::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_2d *_t = static_cast<US_Hydrodyn_Saxs_2d *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_lambda((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_detector_distance((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_detector_height((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_detector_width((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_detector_pixels_height((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_detector_pixels_width((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_beam_center_pixels_height((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_beam_center_pixels_width((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_sample_rotations((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->adjust_wheel((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 11: _t->info(); break;
        case 12: _t->start(); break;
        case 13: _t->stop(); break;
        case 14: _t->integrate(); break;
        case 15: _t->clear_display(); break;
        case 16: _t->update_font(); break;
        case 17: _t->save(); break;
        case 18: _t->cancel(); break;
        case 19: _t->help(); break;
        case 20: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_2d::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_2d::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_2d,
      qt_meta_data_US_Hydrodyn_Saxs_2d, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_2d::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_2d::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_2d::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_2d))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_2d*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_2d::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
