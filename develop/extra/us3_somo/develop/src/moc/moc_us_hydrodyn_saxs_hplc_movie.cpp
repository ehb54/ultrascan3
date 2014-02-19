/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_movie.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_hplc_movie.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_hplc_movie.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Hplc_Movie[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      29,   28,   28,   28, 0x08,
      37,   28,   28,   28, 0x08,
      44,   28,   28,   28, 0x08,
      53,   28,   28,   28, 0x08,
      61,   28,   28,   28, 0x08,
      70,   28,   28,   28, 0x08,
      77,   28,   28,   28, 0x08,
      83,   28,   28,   28, 0x08,
     100,   28,   28,   28, 0x08,
     115,   28,   28,   28, 0x08,
     126,   28,   28,   28, 0x08,
     137,   28,   28,   28, 0x08,
     144,   28,   28,   28, 0x08,
     153,   28,   28,   28, 0x08,
     164,   28,   28,   28, 0x08,
     181,   28,   28,   28, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Movie[] = {
    "US_Hydrodyn_Saxs_Hplc_Movie\0\0front()\0"
    "prev()\0slower()\0start()\0faster()\0"
    "next()\0end()\0set_show_gauss()\0"
    "set_show_ref()\0set_mono()\0set_save()\0"
    "help()\0cancel()\0setupGUI()\0update_enables()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Saxs_Hplc_Movie::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Hplc_Movie *_t = static_cast<US_Hydrodyn_Saxs_Hplc_Movie *>(_o);
        switch (_id) {
        case 0: _t->front(); break;
        case 1: _t->prev(); break;
        case 2: _t->slower(); break;
        case 3: _t->start(); break;
        case 4: _t->faster(); break;
        case 5: _t->next(); break;
        case 6: _t->end(); break;
        case 7: _t->set_show_gauss(); break;
        case 8: _t->set_show_ref(); break;
        case 9: _t->set_mono(); break;
        case 10: _t->set_save(); break;
        case 11: _t->help(); break;
        case 12: _t->cancel(); break;
        case 13: _t->setupGUI(); break;
        case 14: _t->update_enables(); break;
        case 15: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Hplc_Movie::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Hplc_Movie::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Movie,
      qt_meta_data_US_Hydrodyn_Saxs_Hplc_Movie, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Hplc_Movie::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Hplc_Movie::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Hplc_Movie::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Hplc_Movie))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Hplc_Movie*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Hplc_Movie::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
