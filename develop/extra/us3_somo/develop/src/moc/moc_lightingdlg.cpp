/****************************************************************************
** Meta object code from reading C++ file 'lightingdlg.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../3dplot/lightingdlg.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'lightingdlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Plot[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_Plot[] = {
    "Plot\0"
};

void Plot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Plot::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Plot::staticMetaObject = {
    { &SPlot::staticMetaObject, qt_meta_stringdata_Plot,
      qt_meta_data_Plot, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Plot::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Plot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Plot::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Plot))
        return static_cast<void*>(const_cast< Plot*>(this));
    return SPlot::qt_metacast(_clname);
}

int Plot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SPlot::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_LightingDlg[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x0a,
      30,   12,   12,   12, 0x0a,
      47,   12,   12,   12, 0x0a,
      60,   12,   12,   12, 0x0a,
      73,   12,   12,   12, 0x0a,
      86,   12,   12,   12, 0x0a,
     100,   94,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_LightingDlg[] = {
    "LightingDlg\0\0setDistance(int)\0"
    "setEmission(int)\0setDiff(int)\0"
    "setSpec(int)\0setShin(int)\0reset()\0"
    "x,y,z\0setRotation(double,double,double)\0"
};

void LightingDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LightingDlg *_t = static_cast<LightingDlg *>(_o);
        switch (_id) {
        case 0: _t->setDistance((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->setEmission((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->setDiff((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setSpec((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->setShin((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->reset(); break;
        case 6: _t->setRotation((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData LightingDlg::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LightingDlg::staticMetaObject = {
    { &lightingdlgbaseBase::staticMetaObject, qt_meta_stringdata_LightingDlg,
      qt_meta_data_LightingDlg, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LightingDlg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LightingDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LightingDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LightingDlg))
        return static_cast<void*>(const_cast< LightingDlg*>(this));
    return lightingdlgbaseBase::qt_metacast(_clname);
}

int LightingDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = lightingdlgbaseBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
