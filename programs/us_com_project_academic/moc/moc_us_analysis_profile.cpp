/****************************************************************************
** Meta object code from reading C++ file 'us_analysis_profile.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../us_analysis_profile/us_analysis_profile.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_analysis_profile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_US_AnaprofPanGen_t {
    QByteArrayData data[16];
    char stringdata0[308];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_AnaprofPanGen_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_AnaprofPanGen_t qt_meta_stringdata_US_AnaprofPanGen = {
    {
QT_MOC_LITERAL(0, 0, 16), // "US_AnaprofPanGen"
QT_MOC_LITERAL(1, 17, 25), // "set_tabs_buttons_inactive"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 32), // "set_tabs_buttons_active_readonly"
QT_MOC_LITERAL(4, 77, 23), // "set_tabs_buttons_active"
QT_MOC_LITERAL(5, 101, 20), // "build_general_layout"
QT_MOC_LITERAL(6, 122, 19), // "apro_button_clicked"
QT_MOC_LITERAL(7, 142, 19), // "prot_button_clicked"
QT_MOC_LITERAL(8, 162, 17), // "apro_text_changed"
QT_MOC_LITERAL(9, 180, 17), // "prot_text_changed"
QT_MOC_LITERAL(10, 198, 18), // "lcrat_text_changed"
QT_MOC_LITERAL(11, 217, 18), // "lctol_text_changed"
QT_MOC_LITERAL(12, 236, 18), // "ldvol_text_changed"
QT_MOC_LITERAL(13, 255, 18), // "lvtol_text_changed"
QT_MOC_LITERAL(14, 274, 18), // "daend_text_changed"
QT_MOC_LITERAL(15, 293, 14) // "applied_to_all"

    },
    "US_AnaprofPanGen\0set_tabs_buttons_inactive\0"
    "\0set_tabs_buttons_active_readonly\0"
    "set_tabs_buttons_active\0build_general_layout\0"
    "apro_button_clicked\0prot_button_clicked\0"
    "apro_text_changed\0prot_text_changed\0"
    "lcrat_text_changed\0lctol_text_changed\0"
    "ldvol_text_changed\0lvtol_text_changed\0"
    "daend_text_changed\0applied_to_all"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_AnaprofPanGen[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x06 /* Public */,
       3,    0,   85,    2, 0x06 /* Public */,
       4,    0,   86,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   87,    2, 0x08 /* Private */,
       6,    0,   88,    2, 0x08 /* Private */,
       7,    0,   89,    2, 0x08 /* Private */,
       8,    0,   90,    2, 0x08 /* Private */,
       9,    0,   91,    2, 0x08 /* Private */,
      10,    0,   92,    2, 0x08 /* Private */,
      11,    0,   93,    2, 0x08 /* Private */,
      12,    0,   94,    2, 0x08 /* Private */,
      13,    0,   95,    2, 0x08 /* Private */,
      14,    0,   96,    2, 0x08 /* Private */,
      15,    0,   97,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void US_AnaprofPanGen::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_AnaprofPanGen *_t = static_cast<US_AnaprofPanGen *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->set_tabs_buttons_inactive(); break;
        case 1: _t->set_tabs_buttons_active_readonly(); break;
        case 2: _t->set_tabs_buttons_active(); break;
        case 3: _t->build_general_layout(); break;
        case 4: _t->apro_button_clicked(); break;
        case 5: _t->prot_button_clicked(); break;
        case 6: _t->apro_text_changed(); break;
        case 7: _t->prot_text_changed(); break;
        case 8: _t->lcrat_text_changed(); break;
        case 9: _t->lctol_text_changed(); break;
        case 10: _t->ldvol_text_changed(); break;
        case 11: _t->lvtol_text_changed(); break;
        case 12: _t->daend_text_changed(); break;
        case 13: _t->applied_to_all(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_AnaprofPanGen::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_AnaprofPanGen::set_tabs_buttons_inactive)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (US_AnaprofPanGen::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_AnaprofPanGen::set_tabs_buttons_active_readonly)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (US_AnaprofPanGen::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_AnaprofPanGen::set_tabs_buttons_active)) {
                *result = 2;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject US_AnaprofPanGen::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_AnaprofPanGen.data,
      qt_meta_data_US_AnaprofPanGen,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_AnaprofPanGen::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_AnaprofPanGen::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_AnaprofPanGen.stringdata0))
        return static_cast<void*>(const_cast< US_AnaprofPanGen*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_AnaprofPanGen::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void US_AnaprofPanGen::set_tabs_buttons_inactive()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void US_AnaprofPanGen::set_tabs_buttons_active_readonly()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void US_AnaprofPanGen::set_tabs_buttons_active()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}
struct qt_meta_stringdata_US_AnaprofPan2DSA_t {
    QByteArrayData data[26];
    char stringdata0[407];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_AnaprofPan2DSA_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_AnaprofPan2DSA_t qt_meta_stringdata_US_AnaprofPan2DSA = {
    {
QT_MOC_LITERAL(0, 0, 17), // "US_AnaprofPan2DSA"
QT_MOC_LITERAL(1, 18, 16), // "channel_selected"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 12), // "next_channel"
QT_MOC_LITERAL(4, 49, 12), // "smin_changed"
QT_MOC_LITERAL(5, 62, 12), // "smax_changed"
QT_MOC_LITERAL(6, 75, 16), // "sgpoints_changed"
QT_MOC_LITERAL(7, 92, 12), // "kmin_changed"
QT_MOC_LITERAL(8, 105, 12), // "kmax_changed"
QT_MOC_LITERAL(9, 118, 16), // "kgpoints_changed"
QT_MOC_LITERAL(10, 135, 17), // "grid_reps_changed"
QT_MOC_LITERAL(11, 153, 17), // "cust_grid_clicked"
QT_MOC_LITERAL(12, 171, 17), // "cust_grid_changed"
QT_MOC_LITERAL(13, 189, 17), // "vary_vbar_checked"
QT_MOC_LITERAL(14, 207, 14), // "constk_changed"
QT_MOC_LITERAL(15, 222, 17), // "apply_all_clicked"
QT_MOC_LITERAL(16, 240, 16), // "job1_run_checked"
QT_MOC_LITERAL(17, 257, 16), // "job2_run_checked"
QT_MOC_LITERAL(18, 274, 16), // "mgpoints_changed"
QT_MOC_LITERAL(19, 291, 15), // "mfrange_changed"
QT_MOC_LITERAL(20, 307, 16), // "job3_run_checked"
QT_MOC_LITERAL(21, 324, 16), // "autopick_checked"
QT_MOC_LITERAL(22, 341, 16), // "job4_run_checked"
QT_MOC_LITERAL(23, 358, 15), // "rfiters_changed"
QT_MOC_LITERAL(24, 374, 16), // "job5_run_checked"
QT_MOC_LITERAL(25, 391, 15) // "mciters_changed"

    },
    "US_AnaprofPan2DSA\0channel_selected\0\0"
    "next_channel\0smin_changed\0smax_changed\0"
    "sgpoints_changed\0kmin_changed\0"
    "kmax_changed\0kgpoints_changed\0"
    "grid_reps_changed\0cust_grid_clicked\0"
    "cust_grid_changed\0vary_vbar_checked\0"
    "constk_changed\0apply_all_clicked\0"
    "job1_run_checked\0job2_run_checked\0"
    "mgpoints_changed\0mfrange_changed\0"
    "job3_run_checked\0autopick_checked\0"
    "job4_run_checked\0rfiters_changed\0"
    "job5_run_checked\0mciters_changed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_AnaprofPan2DSA[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,  134,    2, 0x08 /* Private */,
       3,    0,  137,    2, 0x08 /* Private */,
       4,    0,  138,    2, 0x08 /* Private */,
       5,    0,  139,    2, 0x08 /* Private */,
       6,    0,  140,    2, 0x08 /* Private */,
       7,    0,  141,    2, 0x08 /* Private */,
       8,    0,  142,    2, 0x08 /* Private */,
       9,    0,  143,    2, 0x08 /* Private */,
      10,    0,  144,    2, 0x08 /* Private */,
      11,    0,  145,    2, 0x08 /* Private */,
      12,    0,  146,    2, 0x08 /* Private */,
      13,    1,  147,    2, 0x08 /* Private */,
      14,    0,  150,    2, 0x08 /* Private */,
      15,    0,  151,    2, 0x08 /* Private */,
      16,    1,  152,    2, 0x08 /* Private */,
      17,    1,  155,    2, 0x08 /* Private */,
      18,    0,  158,    2, 0x08 /* Private */,
      19,    0,  159,    2, 0x08 /* Private */,
      20,    1,  160,    2, 0x08 /* Private */,
      21,    1,  163,    2, 0x08 /* Private */,
      22,    1,  166,    2, 0x08 /* Private */,
      23,    0,  169,    2, 0x08 /* Private */,
      24,    1,  170,    2, 0x08 /* Private */,
      25,    0,  173,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,

       0        // eod
};

void US_AnaprofPan2DSA::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_AnaprofPan2DSA *_t = static_cast<US_AnaprofPan2DSA *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->channel_selected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->next_channel(); break;
        case 2: _t->smin_changed(); break;
        case 3: _t->smax_changed(); break;
        case 4: _t->sgpoints_changed(); break;
        case 5: _t->kmin_changed(); break;
        case 6: _t->kmax_changed(); break;
        case 7: _t->kgpoints_changed(); break;
        case 8: _t->grid_reps_changed(); break;
        case 9: _t->cust_grid_clicked(); break;
        case 10: _t->cust_grid_changed(); break;
        case 11: _t->vary_vbar_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->constk_changed(); break;
        case 13: _t->apply_all_clicked(); break;
        case 14: _t->job1_run_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->job2_run_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->mgpoints_changed(); break;
        case 17: _t->mfrange_changed(); break;
        case 18: _t->job3_run_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 19: _t->autopick_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 20: _t->job4_run_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 21: _t->rfiters_changed(); break;
        case 22: _t->job5_run_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 23: _t->mciters_changed(); break;
        default: ;
        }
    }
}

const QMetaObject US_AnaprofPan2DSA::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_AnaprofPan2DSA.data,
      qt_meta_data_US_AnaprofPan2DSA,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_AnaprofPan2DSA::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_AnaprofPan2DSA::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_AnaprofPan2DSA.stringdata0))
        return static_cast<void*>(const_cast< US_AnaprofPan2DSA*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_AnaprofPan2DSA::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 24;
    }
    return _id;
}
struct qt_meta_stringdata_US_AnaprofPanPCSA_t {
    QByteArrayData data[24];
    char stringdata0[359];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_AnaprofPanPCSA_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_AnaprofPanPCSA_t qt_meta_stringdata_US_AnaprofPanPCSA = {
    {
QT_MOC_LITERAL(0, 0, 17), // "US_AnaprofPanPCSA"
QT_MOC_LITERAL(1, 18, 14), // "nopcsa_checked"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 16), // "channel_selected"
QT_MOC_LITERAL(4, 51, 12), // "next_channel"
QT_MOC_LITERAL(5, 64, 17), // "curvtype_selected"
QT_MOC_LITERAL(6, 82, 17), // "apply_all_clicked"
QT_MOC_LITERAL(7, 100, 14), // "xaxis_selected"
QT_MOC_LITERAL(8, 115, 12), // "xmin_changed"
QT_MOC_LITERAL(9, 128, 12), // "xmax_changed"
QT_MOC_LITERAL(10, 141, 14), // "yaxis_selected"
QT_MOC_LITERAL(11, 156, 12), // "ymin_changed"
QT_MOC_LITERAL(12, 169, 12), // "ymax_changed"
QT_MOC_LITERAL(13, 182, 14), // "zaxis_selected"
QT_MOC_LITERAL(14, 197, 14), // "zvalue_changed"
QT_MOC_LITERAL(15, 212, 16), // "varcount_changed"
QT_MOC_LITERAL(16, 229, 16), // "grfiters_changed"
QT_MOC_LITERAL(17, 246, 16), // "crpoints_changed"
QT_MOC_LITERAL(18, 263, 15), // "tinoise_checked"
QT_MOC_LITERAL(19, 279, 15), // "rinoise_checked"
QT_MOC_LITERAL(20, 295, 16), // "tregspec_checked"
QT_MOC_LITERAL(21, 312, 16), // "tregauto_checked"
QT_MOC_LITERAL(22, 329, 13), // "alpha_changed"
QT_MOC_LITERAL(23, 343, 15) // "mciters_changed"

    },
    "US_AnaprofPanPCSA\0nopcsa_checked\0\0"
    "channel_selected\0next_channel\0"
    "curvtype_selected\0apply_all_clicked\0"
    "xaxis_selected\0xmin_changed\0xmax_changed\0"
    "yaxis_selected\0ymin_changed\0ymax_changed\0"
    "zaxis_selected\0zvalue_changed\0"
    "varcount_changed\0grfiters_changed\0"
    "crpoints_changed\0tinoise_checked\0"
    "rinoise_checked\0tregspec_checked\0"
    "tregauto_checked\0alpha_changed\0"
    "mciters_changed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_AnaprofPanPCSA[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,  124,    2, 0x08 /* Private */,
       3,    1,  127,    2, 0x08 /* Private */,
       4,    0,  130,    2, 0x08 /* Private */,
       5,    1,  131,    2, 0x08 /* Private */,
       6,    0,  134,    2, 0x08 /* Private */,
       7,    1,  135,    2, 0x08 /* Private */,
       8,    0,  138,    2, 0x08 /* Private */,
       9,    0,  139,    2, 0x08 /* Private */,
      10,    1,  140,    2, 0x08 /* Private */,
      11,    0,  143,    2, 0x08 /* Private */,
      12,    0,  144,    2, 0x08 /* Private */,
      13,    1,  145,    2, 0x08 /* Private */,
      14,    0,  148,    2, 0x08 /* Private */,
      15,    0,  149,    2, 0x08 /* Private */,
      16,    0,  150,    2, 0x08 /* Private */,
      17,    0,  151,    2, 0x08 /* Private */,
      18,    1,  152,    2, 0x08 /* Private */,
      19,    1,  155,    2, 0x08 /* Private */,
      20,    1,  158,    2, 0x08 /* Private */,
      21,    1,  161,    2, 0x08 /* Private */,
      22,    0,  164,    2, 0x08 /* Private */,
      23,    0,  165,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void US_AnaprofPanPCSA::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_AnaprofPanPCSA *_t = static_cast<US_AnaprofPanPCSA *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->nopcsa_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->channel_selected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->next_channel(); break;
        case 3: _t->curvtype_selected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->apply_all_clicked(); break;
        case 5: _t->xaxis_selected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->xmin_changed(); break;
        case 7: _t->xmax_changed(); break;
        case 8: _t->yaxis_selected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->ymin_changed(); break;
        case 10: _t->ymax_changed(); break;
        case 11: _t->zaxis_selected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->zvalue_changed(); break;
        case 13: _t->varcount_changed(); break;
        case 14: _t->grfiters_changed(); break;
        case 15: _t->crpoints_changed(); break;
        case 16: _t->tinoise_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->rinoise_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: _t->tregspec_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 19: _t->tregauto_checked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 20: _t->alpha_changed(); break;
        case 21: _t->mciters_changed(); break;
        default: ;
        }
    }
}

const QMetaObject US_AnaprofPanPCSA::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_AnaprofPanPCSA.data,
      qt_meta_data_US_AnaprofPanPCSA,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_AnaprofPanPCSA::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_AnaprofPanPCSA::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_AnaprofPanPCSA.stringdata0))
        return static_cast<void*>(const_cast< US_AnaprofPanPCSA*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_AnaprofPanPCSA::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 22;
    }
    return _id;
}
struct qt_meta_stringdata_US_AnalysisProfileGui_t {
    QByteArrayData data[14];
    char stringdata0[187];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_AnalysisProfileGui_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_AnalysisProfileGui_t qt_meta_stringdata_US_AnalysisProfileGui = {
    {
QT_MOC_LITERAL(0, 0, 21), // "US_AnalysisProfileGui"
QT_MOC_LITERAL(1, 22, 16), // "us_exp_is_closed"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 5), // "reset"
QT_MOC_LITERAL(4, 46, 8), // "newPanel"
QT_MOC_LITERAL(5, 55, 10), // "statUpdate"
QT_MOC_LITERAL(6, 66, 7), // "panelUp"
QT_MOC_LITERAL(7, 74, 9), // "panelDown"
QT_MOC_LITERAL(8, 84, 4), // "help"
QT_MOC_LITERAL(9, 89, 20), // "disable_tabs_buttons"
QT_MOC_LITERAL(10, 110, 28), // "enable_tabs_buttons_readonly"
QT_MOC_LITERAL(11, 139, 19), // "enable_tabs_buttons"
QT_MOC_LITERAL(12, 159, 13), // "close_program"
QT_MOC_LITERAL(13, 173, 13) // "apply_profile"

    },
    "US_AnalysisProfileGui\0us_exp_is_closed\0"
    "\0reset\0newPanel\0statUpdate\0panelUp\0"
    "panelDown\0help\0disable_tabs_buttons\0"
    "enable_tabs_buttons_readonly\0"
    "enable_tabs_buttons\0close_program\0"
    "apply_profile"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_AnalysisProfileGui[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   75,    2, 0x08 /* Private */,
       4,    1,   76,    2, 0x08 /* Private */,
       5,    0,   79,    2, 0x08 /* Private */,
       6,    0,   80,    2, 0x08 /* Private */,
       7,    0,   81,    2, 0x08 /* Private */,
       8,    0,   82,    2, 0x08 /* Private */,
       9,    0,   83,    2, 0x08 /* Private */,
      10,    0,   84,    2, 0x08 /* Private */,
      11,    0,   85,    2, 0x08 /* Private */,
      12,    0,   86,    2, 0x0a /* Public */,
      13,    0,   87,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void US_AnalysisProfileGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_AnalysisProfileGui *_t = static_cast<US_AnalysisProfileGui *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->us_exp_is_closed(); break;
        case 1: _t->reset(); break;
        case 2: _t->newPanel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->statUpdate(); break;
        case 4: _t->panelUp(); break;
        case 5: _t->panelDown(); break;
        case 6: _t->help(); break;
        case 7: _t->disable_tabs_buttons(); break;
        case 8: _t->enable_tabs_buttons_readonly(); break;
        case 9: _t->enable_tabs_buttons(); break;
        case 10: _t->close_program(); break;
        case 11: _t->apply_profile(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_AnalysisProfileGui::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_AnalysisProfileGui::us_exp_is_closed)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject US_AnalysisProfileGui::staticMetaObject = {
    { &US_Widgets::staticMetaObject, qt_meta_stringdata_US_AnalysisProfileGui.data,
      qt_meta_data_US_AnalysisProfileGui,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_AnalysisProfileGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_AnalysisProfileGui::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_AnalysisProfileGui.stringdata0))
        return static_cast<void*>(const_cast< US_AnalysisProfileGui*>(this));
    return US_Widgets::qt_metacast(_clname);
}

int US_AnalysisProfileGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_Widgets::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void US_AnalysisProfileGui::us_exp_is_closed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
