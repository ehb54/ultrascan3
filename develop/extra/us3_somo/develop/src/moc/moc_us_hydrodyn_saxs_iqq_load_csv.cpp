/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_saxs_iqq_load_csv.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_saxs_iqq_load_csv.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_saxs_iqq_load_csv.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Saxs_Iqq_Load_Csv[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      31,   30,   30,   30, 0x08,
      49,   30,   30,   30, 0x08,
      66,   30,   30,   30, 0x08,
      87,   30,   30,   30, 0x08,
     109,   30,   30,   30, 0x08,
     127,   30,   30,   30, 0x08,
     156,   30,   30,   30, 0x08,
     181,   30,   30,   30, 0x08,
     196,   30,   30,   30, 0x08,
     215,   30,   30,   30, 0x08,
     228,   30,   30,   30, 0x08,
     244,   30,   30,   30, 0x08,
     256,   30,   30,   30, 0x08,
     270,   30,   30,   30, 0x08,
     286,   30,   30,   30, 0x08,
     291,   30,   30,   30, 0x08,
     300,   30,   30,   30, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Saxs_Iqq_Load_Csv[] = {
    "US_Hydrodyn_Saxs_Iqq_Load_Csv\0\0"
    "update_selected()\0set_create_avg()\0"
    "set_create_std_dev()\0set_only_plot_stats()\0"
    "set_save_to_csv()\0update_csv_filename(QString)\0"
    "set_save_original_data()\0set_run_nnls()\0"
    "set_run_best_fit()\0select_all()\0"
    "select_target()\0transpose()\0save_as_dat()\0"
    "save_selected()\0ok()\0cancel()\0help()\0"
};

void US_Hydrodyn_Saxs_Iqq_Load_Csv::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Saxs_Iqq_Load_Csv *_t = static_cast<US_Hydrodyn_Saxs_Iqq_Load_Csv *>(_o);
        switch (_id) {
        case 0: _t->update_selected(); break;
        case 1: _t->set_create_avg(); break;
        case 2: _t->set_create_std_dev(); break;
        case 3: _t->set_only_plot_stats(); break;
        case 4: _t->set_save_to_csv(); break;
        case 5: _t->update_csv_filename((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->set_save_original_data(); break;
        case 7: _t->set_run_nnls(); break;
        case 8: _t->set_run_best_fit(); break;
        case 9: _t->select_all(); break;
        case 10: _t->select_target(); break;
        case 11: _t->transpose(); break;
        case 12: _t->save_as_dat(); break;
        case 13: _t->save_selected(); break;
        case 14: _t->ok(); break;
        case 15: _t->cancel(); break;
        case 16: _t->help(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Saxs_Iqq_Load_Csv::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Saxs_Iqq_Load_Csv::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Saxs_Iqq_Load_Csv,
      qt_meta_data_US_Hydrodyn_Saxs_Iqq_Load_Csv, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Saxs_Iqq_Load_Csv::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Saxs_Iqq_Load_Csv::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Saxs_Iqq_Load_Csv::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Saxs_Iqq_Load_Csv))
        return static_cast<void*>(const_cast< US_Hydrodyn_Saxs_Iqq_Load_Csv*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Saxs_Iqq_Load_Csv::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
