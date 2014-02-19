/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_batch_movie_opts.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_batch_movie_opts.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_batch_movie_opts.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Batch_Movie_Opts[] = {

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
      34,   30,   29,   29, 0x08,
      56,   30,   29,   29, 0x08,
      76,   30,   29,   29, 0x08,
      97,   30,   29,   29, 0x08,
     117,   30,   29,   29, 0x08,
     139,   30,   29,   29, 0x08,
     163,   30,   29,   29, 0x08,
     188,   30,   29,   29, 0x08,
     213,   30,   29,   29, 0x08,
     242,   29,   29,   29, 0x08,
     259,   29,   29,   29, 0x08,
     276,   29,   29,   29, 0x08,
     291,   29,   29,   29, 0x08,
     304,   29,   29,   29, 0x08,
     327,   29,   29,   29, 0x08,
     336,   29,   29,   29, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Batch_Movie_Opts[] = {
    "US_Hydrodyn_Batch_Movie_Opts\0\0str\0"
    "update_title(QString)\0update_dir(QString)\0"
    "update_file(QString)\0update_fps(QString)\0"
    "update_scale(QString)\0update_tc_unit(QString)\0"
    "update_tc_start(QString)\0"
    "update_tc_delta(QString)\0"
    "update_tc_pointsize(QString)\0"
    "update_dir_msg()\0update_enables()\0"
    "set_clean_up()\0set_use_tc()\0"
    "set_black_background()\0cancel()\0help()\0"
};

void US_Hydrodyn_Batch_Movie_Opts::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Batch_Movie_Opts *_t = static_cast<US_Hydrodyn_Batch_Movie_Opts *>(_o);
        switch (_id) {
        case 0: _t->update_title((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update_dir((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_file((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_fps((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_scale((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_tc_unit((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_tc_start((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_tc_delta((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_tc_pointsize((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_dir_msg(); break;
        case 10: _t->update_enables(); break;
        case 11: _t->set_clean_up(); break;
        case 12: _t->set_use_tc(); break;
        case 13: _t->set_black_background(); break;
        case 14: _t->cancel(); break;
        case 15: _t->help(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Batch_Movie_Opts::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Batch_Movie_Opts::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Batch_Movie_Opts,
      qt_meta_data_US_Hydrodyn_Batch_Movie_Opts, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Batch_Movie_Opts::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Batch_Movie_Opts::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Batch_Movie_Opts::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Batch_Movie_Opts))
        return static_cast<void*>(const_cast< US_Hydrodyn_Batch_Movie_Opts*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Batch_Movie_Opts::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
