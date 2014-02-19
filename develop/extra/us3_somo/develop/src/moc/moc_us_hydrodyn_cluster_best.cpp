/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_best.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_best.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_best.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Best[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,   25,   25,   25, 0x08,
      56,   25,   25,   25, 0x08,
      93,   25,   25,   25, 0x08,
     129,   25,   25,   25, 0x08,
     159,   25,   25,   25, 0x08,
     189,   25,   25,   25, 0x08,
     216,   25,   25,   25, 0x08,
     243,   25,   25,   25, 0x08,
     272,   25,   25,   25, 0x08,
     299,   25,   25,   25, 0x08,
     316,   25,   25,   25, 0x08,
     333,   25,   25,   25, 0x08,
     366,   25,   25,   25, 0x08,
     382,   25,   25,   25, 0x08,
     398,   25,   25,   25, 0x08,
     415,   25,   25,   25, 0x08,
     448,   25,   25,   25, 0x08,
     483,   25,   25,   25, 0x08,
     490,   25,   25,   25, 0x08,
     497,   25,   25,   25, 0x08,
     504,   25,   25,   25, 0x08,
     513,   25,   25,   25, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Best[] = {
    "US_Hydrodyn_Cluster_Best\0\0"
    "update_bestmsrprober(QString)\0"
    "update_bestmsrfinenessangle(QString)\0"
    "update_bestmsrmaxtriangles(QString)\0"
    "update_bestrcoalnmin(QString)\0"
    "update_bestrcoalnmax(QString)\0"
    "update_bestrcoaln(QString)\0"
    "update_bestbestmw(QString)\0"
    "update_bestbestwatr(QString)\0"
    "update_bestexpand(QString)\0set_bestbestvc()\0"
    "hide_opt_label()\0update_bestmsrcoalescer(QString)\0"
    "set_bestbestv()\0set_bestbestp()\0"
    "set_bestbestna()\0update_bestmsrradiifile(QString)\0"
    "update_bestmsrpatternfile(QString)\0"
    "save()\0load()\0help()\0cancel()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster_Best::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Best *_t = static_cast<US_Hydrodyn_Cluster_Best *>(_o);
        switch (_id) {
        case 0: _t->update_bestmsrprober((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update_bestmsrfinenessangle((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_bestmsrmaxtriangles((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_bestrcoalnmin((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_bestrcoalnmax((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_bestrcoaln((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_bestbestmw((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_bestbestwatr((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_bestexpand((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->set_bestbestvc(); break;
        case 10: _t->hide_opt_label(); break;
        case 11: _t->update_bestmsrcoalescer((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->set_bestbestv(); break;
        case 13: _t->set_bestbestp(); break;
        case 14: _t->set_bestbestna(); break;
        case 15: _t->update_bestmsrradiifile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: _t->update_bestmsrpatternfile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: _t->save(); break;
        case 18: _t->load(); break;
        case 19: _t->help(); break;
        case 20: _t->cancel(); break;
        case 21: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Best::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Best::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Best,
      qt_meta_data_US_Hydrodyn_Cluster_Best, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Best::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Best::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Best::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Best))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Best*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Best::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
