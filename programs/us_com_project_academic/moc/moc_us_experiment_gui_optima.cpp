/****************************************************************************
** Meta object code from reading C++ file 'us_experiment_gui_optima.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../us_experiment/us_experiment_gui_optima.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_experiment_gui_optima.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_US_ExperGuiGeneral_t {
    QByteArrayData data[15];
    char stringdata0[257];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiGeneral_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiGeneral_t qt_meta_stringdata_US_ExperGuiGeneral = {
    {
QT_MOC_LITERAL(0, 0, 18), // "US_ExperGuiGeneral"
QT_MOC_LITERAL(1, 19, 25), // "set_tabs_buttons_inactive"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 32), // "set_tabs_buttons_active_readonly"
QT_MOC_LITERAL(4, 79, 23), // "set_tabs_buttons_active"
QT_MOC_LITERAL(5, 103, 11), // "sel_project"
QT_MOC_LITERAL(6, 115, 12), // "project_info"
QT_MOC_LITERAL(7, 128, 11), // "US_Project&"
QT_MOC_LITERAL(8, 140, 16), // "sel_investigator"
QT_MOC_LITERAL(9, 157, 16), // "run_name_entered"
QT_MOC_LITERAL(10, 174, 13), // "load_protocol"
QT_MOC_LITERAL(11, 188, 16), // "changed_protocol"
QT_MOC_LITERAL(12, 205, 15), // "centerpieceInfo"
QT_MOC_LITERAL(13, 221, 19), // "check_empty_runname"
QT_MOC_LITERAL(14, 241, 15) // "update_protdata"

    },
    "US_ExperGuiGeneral\0set_tabs_buttons_inactive\0"
    "\0set_tabs_buttons_active_readonly\0"
    "set_tabs_buttons_active\0sel_project\0"
    "project_info\0US_Project&\0sel_investigator\0"
    "run_name_entered\0load_protocol\0"
    "changed_protocol\0centerpieceInfo\0"
    "check_empty_runname\0update_protdata"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiGeneral[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x06 /* Public */,
       3,    0,   75,    2, 0x06 /* Public */,
       4,    0,   76,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   77,    2, 0x08 /* Private */,
       6,    1,   78,    2, 0x08 /* Private */,
       8,    0,   81,    2, 0x08 /* Private */,
       9,    0,   82,    2, 0x08 /* Private */,
      10,    0,   83,    2, 0x08 /* Private */,
      11,    0,   84,    2, 0x08 /* Private */,
      12,    0,   85,    2, 0x08 /* Private */,
      13,    1,   86,    2, 0x08 /* Private */,
      14,    0,   89,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void US_ExperGuiGeneral::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiGeneral *_t = static_cast<US_ExperGuiGeneral *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->set_tabs_buttons_inactive(); break;
        case 1: _t->set_tabs_buttons_active_readonly(); break;
        case 2: _t->set_tabs_buttons_active(); break;
        case 3: _t->sel_project(); break;
        case 4: _t->project_info((*reinterpret_cast< US_Project(*)>(_a[1]))); break;
        case 5: _t->sel_investigator(); break;
        case 6: _t->run_name_entered(); break;
        case 7: _t->load_protocol(); break;
        case 8: _t->changed_protocol(); break;
        case 9: _t->centerpieceInfo(); break;
        case 10: _t->check_empty_runname((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->update_protdata(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_ExperGuiGeneral::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperGuiGeneral::set_tabs_buttons_inactive)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (US_ExperGuiGeneral::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperGuiGeneral::set_tabs_buttons_active_readonly)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (US_ExperGuiGeneral::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperGuiGeneral::set_tabs_buttons_active)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject US_ExperGuiGeneral::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiGeneral.data,
      qt_meta_data_US_ExperGuiGeneral,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiGeneral::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiGeneral::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiGeneral.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiGeneral*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiGeneral::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
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
void US_ExperGuiGeneral::set_tabs_buttons_inactive()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void US_ExperGuiGeneral::set_tabs_buttons_active_readonly()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void US_ExperGuiGeneral::set_tabs_buttons_active()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}
struct qt_meta_stringdata_US_ExperGuiRotor_t {
    QByteArrayData data[15];
    char stringdata0[226];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiRotor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiRotor_t qt_meta_stringdata_US_ExperGuiRotor = {
    {
QT_MOC_LITERAL(0, 0, 16), // "US_ExperGuiRotor"
QT_MOC_LITERAL(1, 17, 9), // "changeLab"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 11), // "changeRotor"
QT_MOC_LITERAL(4, 40, 11), // "changeCalib"
QT_MOC_LITERAL(5, 52, 14), // "changeOperator"
QT_MOC_LITERAL(6, 67, 13), // "changeExpType"
QT_MOC_LITERAL(7, 81, 12), // "changeOptima"
QT_MOC_LITERAL(8, 94, 8), // "advRotor"
QT_MOC_LITERAL(9, 103, 15), // "advRotorChanged"
QT_MOC_LITERAL(10, 119, 16), // "US_Rotor::Rotor&"
QT_MOC_LITERAL(11, 136, 27), // "US_Rotor::RotorCalibration&"
QT_MOC_LITERAL(12, 164, 13), // "abstractRotor"
QT_MOC_LITERAL(13, 178, 24), // "US_Rotor::AbstractRotor*"
QT_MOC_LITERAL(14, 203, 22) // "test_optima_connection"

    },
    "US_ExperGuiRotor\0changeLab\0\0changeRotor\0"
    "changeCalib\0changeOperator\0changeExpType\0"
    "changeOptima\0advRotor\0advRotorChanged\0"
    "US_Rotor::Rotor&\0US_Rotor::RotorCalibration&\0"
    "abstractRotor\0US_Rotor::AbstractRotor*\0"
    "test_optima_connection"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiRotor[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x08 /* Private */,
       3,    1,   67,    2, 0x08 /* Private */,
       4,    1,   70,    2, 0x08 /* Private */,
       5,    1,   73,    2, 0x08 /* Private */,
       6,    1,   76,    2, 0x08 /* Private */,
       7,    1,   79,    2, 0x08 /* Private */,
       8,    0,   82,    2, 0x08 /* Private */,
       9,    2,   83,    2, 0x08 /* Private */,
      12,    1,   88,    2, 0x08 /* Private */,
      14,    0,   91,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 11,    2,    2,
    0x80000000 | 13, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void US_ExperGuiRotor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiRotor *_t = static_cast<US_ExperGuiRotor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->changeLab((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->changeRotor((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->changeCalib((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->changeOperator((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->changeExpType((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->changeOptima((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->advRotor(); break;
        case 7: _t->advRotorChanged((*reinterpret_cast< US_Rotor::Rotor(*)>(_a[1])),(*reinterpret_cast< US_Rotor::RotorCalibration(*)>(_a[2]))); break;
        case 8: { US_Rotor::AbstractRotor* _r = _t->abstractRotor((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< US_Rotor::AbstractRotor**>(_a[0]) = _r; }  break;
        case 9: _t->test_optima_connection(); break;
        default: ;
        }
    }
}

const QMetaObject US_ExperGuiRotor::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiRotor.data,
      qt_meta_data_US_ExperGuiRotor,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiRotor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiRotor::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiRotor.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiRotor*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiRotor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
struct qt_meta_stringdata_US_ExperGuiSpeeds_t {
    QByteArrayData data[31];
    char stringdata0[539];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiSpeeds_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiSpeeds_t qt_meta_stringdata_US_ExperGuiSpeeds = {
    {
QT_MOC_LITERAL(0, 0, 17), // "US_ExperGuiSpeeds"
QT_MOC_LITERAL(1, 18, 18), // "speedp_description"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 13), // "ssChangeCount"
QT_MOC_LITERAL(4, 52, 13), // "ssChangeProfx"
QT_MOC_LITERAL(5, 66, 13), // "ssChangeSpeed"
QT_MOC_LITERAL(6, 80, 13), // "ssChangeAccel"
QT_MOC_LITERAL(7, 94, 13), // "ssChgDuratDay"
QT_MOC_LITERAL(8, 108, 13), // "ssChangeScInt"
QT_MOC_LITERAL(9, 122, 17), // "ssChgDuratTime_hh"
QT_MOC_LITERAL(10, 140, 17), // "ssChgDuratTime_mm"
QT_MOC_LITERAL(11, 158, 17), // "ssChgDuratTime_ss"
QT_MOC_LITERAL(12, 176, 13), // "ssChgDelayDay"
QT_MOC_LITERAL(13, 190, 17), // "ssChgDelayTime_hh"
QT_MOC_LITERAL(14, 208, 17), // "ssChgDelayTime_mm"
QT_MOC_LITERAL(15, 226, 17), // "ssChgDelayTime_ss"
QT_MOC_LITERAL(16, 244, 17), // "ssChgDelayDay_int"
QT_MOC_LITERAL(17, 262, 21), // "ssChgDelayTime_int_hh"
QT_MOC_LITERAL(18, 284, 21), // "ssChgDelayTime_int_mm"
QT_MOC_LITERAL(19, 306, 21), // "ssChgDelayTime_int_ss"
QT_MOC_LITERAL(20, 328, 22), // "ssChgDelayStageTime_hh"
QT_MOC_LITERAL(21, 351, 22), // "ssChgDelayStageTime_mm"
QT_MOC_LITERAL(22, 374, 17), // "ssChgScIntTime_hh"
QT_MOC_LITERAL(23, 392, 17), // "ssChgScIntTime_mm"
QT_MOC_LITERAL(24, 410, 17), // "ssChgScIntTime_ss"
QT_MOC_LITERAL(25, 428, 21), // "ssChgScIntTime_int_hh"
QT_MOC_LITERAL(26, 450, 21), // "ssChgScIntTime_int_mm"
QT_MOC_LITERAL(27, 472, 21), // "ssChgScIntTime_int_ss"
QT_MOC_LITERAL(28, 494, 11), // "adjustDelay"
QT_MOC_LITERAL(29, 506, 15), // "stageDelay_sync"
QT_MOC_LITERAL(30, 522, 16) // "syncdelayChecked"

    },
    "US_ExperGuiSpeeds\0speedp_description\0"
    "\0ssChangeCount\0ssChangeProfx\0ssChangeSpeed\0"
    "ssChangeAccel\0ssChgDuratDay\0ssChangeScInt\0"
    "ssChgDuratTime_hh\0ssChgDuratTime_mm\0"
    "ssChgDuratTime_ss\0ssChgDelayDay\0"
    "ssChgDelayTime_hh\0ssChgDelayTime_mm\0"
    "ssChgDelayTime_ss\0ssChgDelayDay_int\0"
    "ssChgDelayTime_int_hh\0ssChgDelayTime_int_mm\0"
    "ssChgDelayTime_int_ss\0ssChgDelayStageTime_hh\0"
    "ssChgDelayStageTime_mm\0ssChgScIntTime_hh\0"
    "ssChgScIntTime_mm\0ssChgScIntTime_ss\0"
    "ssChgScIntTime_int_hh\0ssChgScIntTime_int_mm\0"
    "ssChgScIntTime_int_ss\0adjustDelay\0"
    "stageDelay_sync\0syncdelayChecked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiSpeeds[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      29,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,  159,    2, 0x08 /* Private */,
       3,    1,  162,    2, 0x08 /* Private */,
       4,    1,  165,    2, 0x08 /* Private */,
       5,    1,  168,    2, 0x08 /* Private */,
       6,    1,  171,    2, 0x08 /* Private */,
       7,    1,  174,    2, 0x08 /* Private */,
       8,    2,  177,    2, 0x08 /* Private */,
       9,    1,  182,    2, 0x08 /* Private */,
      10,    1,  185,    2, 0x08 /* Private */,
      11,    1,  188,    2, 0x08 /* Private */,
      12,    1,  191,    2, 0x08 /* Private */,
      13,    1,  194,    2, 0x08 /* Private */,
      14,    1,  197,    2, 0x08 /* Private */,
      15,    1,  200,    2, 0x08 /* Private */,
      16,    1,  203,    2, 0x08 /* Private */,
      17,    1,  206,    2, 0x08 /* Private */,
      18,    1,  209,    2, 0x08 /* Private */,
      19,    1,  212,    2, 0x08 /* Private */,
      20,    1,  215,    2, 0x08 /* Private */,
      21,    1,  218,    2, 0x08 /* Private */,
      22,    1,  221,    2, 0x08 /* Private */,
      23,    1,  224,    2, 0x08 /* Private */,
      24,    1,  227,    2, 0x08 /* Private */,
      25,    1,  230,    2, 0x08 /* Private */,
      26,    1,  233,    2, 0x08 /* Private */,
      27,    1,  236,    2, 0x08 /* Private */,
      28,    0,  239,    2, 0x08 /* Private */,
      29,    0,  240,    2, 0x08 /* Private */,
      30,    1,  241,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::QString, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Double, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,

       0        // eod
};

void US_ExperGuiSpeeds::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiSpeeds *_t = static_cast<US_ExperGuiSpeeds *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: { QString _r = _t->speedp_description((*reinterpret_cast< const int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 1: _t->ssChangeCount((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->ssChangeProfx((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->ssChangeSpeed((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->ssChangeAccel((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->ssChgDuratDay((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->ssChangeScInt((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->ssChgDuratTime_hh((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->ssChgDuratTime_mm((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->ssChgDuratTime_ss((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->ssChgDelayDay((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->ssChgDelayTime_hh((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->ssChgDelayTime_mm((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->ssChgDelayTime_ss((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->ssChgDelayDay_int((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->ssChgDelayTime_int_hh((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->ssChgDelayTime_int_mm((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->ssChgDelayTime_int_ss((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->ssChgDelayStageTime_hh((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: _t->ssChgDelayStageTime_mm((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->ssChgScIntTime_hh((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->ssChgScIntTime_mm((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 22: _t->ssChgScIntTime_ss((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->ssChgScIntTime_int_hh((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: _t->ssChgScIntTime_int_mm((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: _t->ssChgScIntTime_int_ss((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 26: _t->adjustDelay(); break;
        case 27: _t->stageDelay_sync(); break;
        case 28: _t->syncdelayChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject US_ExperGuiSpeeds::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiSpeeds.data,
      qt_meta_data_US_ExperGuiSpeeds,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiSpeeds::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiSpeeds::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiSpeeds.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiSpeeds*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiSpeeds::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 29)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 29;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 29)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 29;
    }
    return _id;
}
struct qt_meta_stringdata_US_ExperGuiCells_t {
    QByteArrayData data[5];
    char stringdata0[66];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiCells_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiCells_t qt_meta_stringdata_US_ExperGuiCells = {
    {
QT_MOC_LITERAL(0, 0, 16), // "US_ExperGuiCells"
QT_MOC_LITERAL(1, 17, 18), // "centerpieceChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 14), // "windowsChanged"
QT_MOC_LITERAL(4, 52, 13) // "rebuild_Cells"

    },
    "US_ExperGuiCells\0centerpieceChanged\0"
    "\0windowsChanged\0rebuild_Cells"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiCells[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x08 /* Private */,
       3,    1,   32,    2, 0x08 /* Private */,
       4,    0,   35,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,

       0        // eod
};

void US_ExperGuiCells::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiCells *_t = static_cast<US_ExperGuiCells *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->centerpieceChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->windowsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->rebuild_Cells(); break;
        default: ;
        }
    }
}

const QMetaObject US_ExperGuiCells::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiCells.data,
      qt_meta_data_US_ExperGuiCells,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiCells::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiCells::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiCells.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiCells*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiCells::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
struct qt_meta_stringdata_US_ExperGuiSolutions_t {
    QByteArrayData data[14];
    char stringdata0[180];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiSolutions_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiSolutions_t qt_meta_stringdata_US_ExperGuiSolutions = {
    {
QT_MOC_LITERAL(0, 0, 20), // "US_ExperGuiSolutions"
QT_MOC_LITERAL(1, 21, 15), // "manageSolutions"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 15), // "detailSolutions"
QT_MOC_LITERAL(4, 54, 10), // "solutionID"
QT_MOC_LITERAL(5, 65, 8), // "QString&"
QT_MOC_LITERAL(6, 74, 12), // "solutionData"
QT_MOC_LITERAL(7, 87, 12), // "US_Solution&"
QT_MOC_LITERAL(8, 100, 12), // "allSolutions"
QT_MOC_LITERAL(9, 113, 11), // "addComments"
QT_MOC_LITERAL(10, 125, 14), // "commentStrings"
QT_MOC_LITERAL(11, 140, 12), // "QStringList&"
QT_MOC_LITERAL(12, 153, 13), // "rebuild_Solut"
QT_MOC_LITERAL(13, 167, 12) // "regenSolList"

    },
    "US_ExperGuiSolutions\0manageSolutions\0"
    "\0detailSolutions\0solutionID\0QString&\0"
    "solutionData\0US_Solution&\0allSolutions\0"
    "addComments\0commentStrings\0QStringList&\0"
    "rebuild_Solut\0regenSolList"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiSolutions[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x08 /* Private */,
       3,    0,   60,    2, 0x08 /* Private */,
       4,    2,   61,    2, 0x08 /* Private */,
       6,    2,   66,    2, 0x08 /* Private */,
       8,    0,   71,    2, 0x08 /* Private */,
       9,    0,   72,    2, 0x08 /* Private */,
      10,    4,   73,    2, 0x08 /* Private */,
      12,    0,   82,    2, 0x08 /* Private */,
      13,    0,   83,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QString, 0x80000000 | 5,    2,    2,
    QMetaType::Bool, QMetaType::QString, 0x80000000 | 7,    2,    2,
    QMetaType::Int,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 5, 0x80000000 | 11, QMetaType::Int,    2,    2,    2,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void US_ExperGuiSolutions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiSolutions *_t = static_cast<US_ExperGuiSolutions *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->manageSolutions(); break;
        case 1: _t->detailSolutions(); break;
        case 2: { bool _r = _t->solutionID((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 3: { bool _r = _t->solutionData((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< US_Solution(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 4: { int _r = _t->allSolutions();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: _t->addComments(); break;
        case 6: _t->commentStrings((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QStringList(*)>(_a[3])),(*reinterpret_cast< const int(*)>(_a[4]))); break;
        case 7: _t->rebuild_Solut(); break;
        case 8: _t->regenSolList(); break;
        default: ;
        }
    }
}

const QMetaObject US_ExperGuiSolutions::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiSolutions.data,
      qt_meta_data_US_ExperGuiSolutions,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiSolutions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiSolutions::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiSolutions.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiSolutions*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiSolutions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
struct qt_meta_stringdata_US_ExperGuiOptical_t {
    QByteArrayData data[4];
    char stringdata0[47];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiOptical_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiOptical_t qt_meta_stringdata_US_ExperGuiOptical = {
    {
QT_MOC_LITERAL(0, 0, 18), // "US_ExperGuiOptical"
QT_MOC_LITERAL(1, 19, 12), // "opsysChecked"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 13) // "rebuild_Optic"

    },
    "US_ExperGuiOptical\0opsysChecked\0\0"
    "rebuild_Optic"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiOptical[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x08 /* Private */,
       3,    0,   27,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,

       0        // eod
};

void US_ExperGuiOptical::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiOptical *_t = static_cast<US_ExperGuiOptical *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->opsysChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->rebuild_Optic(); break;
        default: ;
        }
    }
}

const QMetaObject US_ExperGuiOptical::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiOptical.data,
      qt_meta_data_US_ExperGuiOptical,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiOptical::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiOptical::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiOptical.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiOptical*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiOptical::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
struct qt_meta_stringdata_US_ExperGuiRanges_t {
    QByteArrayData data[9];
    char stringdata0[143];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiRanges_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiRanges_t qt_meta_stringdata_US_ExperGuiRanges = {
    {
QT_MOC_LITERAL(0, 0, 17), // "US_ExperGuiRanges"
QT_MOC_LITERAL(1, 18, 12), // "detailRanges"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 17), // "selectWavelengths"
QT_MOC_LITERAL(4, 50, 24), // "selectWavelengths_manual"
QT_MOC_LITERAL(5, 75, 17), // "Wavelengths_class"
QT_MOC_LITERAL(6, 93, 16), // "changedLowRadius"
QT_MOC_LITERAL(7, 110, 17), // "changedHighRadius"
QT_MOC_LITERAL(8, 128, 14) // "rebuild_Ranges"

    },
    "US_ExperGuiRanges\0detailRanges\0\0"
    "selectWavelengths\0selectWavelengths_manual\0"
    "Wavelengths_class\0changedLowRadius\0"
    "changedHighRadius\0rebuild_Ranges"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiRanges[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    0,   52,    2, 0x08 /* Private */,
       6,    1,   53,    2, 0x08 /* Private */,
       7,    1,   56,    2, 0x08 /* Private */,
       8,    0,   59,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void,

       0        // eod
};

void US_ExperGuiRanges::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiRanges *_t = static_cast<US_ExperGuiRanges *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->detailRanges(); break;
        case 1: _t->selectWavelengths(); break;
        case 2: _t->selectWavelengths_manual(); break;
        case 3: _t->Wavelengths_class(); break;
        case 4: _t->changedLowRadius((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->changedHighRadius((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->rebuild_Ranges(); break;
        default: ;
        }
    }
}

const QMetaObject US_ExperGuiRanges::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiRanges.data,
      qt_meta_data_US_ExperGuiRanges,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiRanges::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiRanges::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiRanges.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiRanges*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiRanges::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
struct qt_meta_stringdata_US_SelectWavelengths_t {
    QByteArrayData data[15];
    char stringdata0[163];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_SelectWavelengths_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_SelectWavelengths_t qt_meta_stringdata_US_SelectWavelengths = {
    {
QT_MOC_LITERAL(0, 0, 20), // "US_SelectWavelengths"
QT_MOC_LITERAL(1, 21, 14), // "add_selections"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 14), // "rmv_selections"
QT_MOC_LITERAL(4, 52, 15), // "add_all_selects"
QT_MOC_LITERAL(5, 68, 15), // "rmv_all_selects"
QT_MOC_LITERAL(6, 84, 12), // "new_wl_start"
QT_MOC_LITERAL(7, 97, 10), // "new_wl_end"
QT_MOC_LITERAL(8, 108, 11), // "new_wl_incr"
QT_MOC_LITERAL(9, 120, 12), // "new_wl_range"
QT_MOC_LITERAL(10, 133, 6), // "report"
QT_MOC_LITERAL(11, 140, 6), // "cancel"
QT_MOC_LITERAL(12, 147, 4), // "done"
QT_MOC_LITERAL(13, 152, 5), // "reset"
QT_MOC_LITERAL(14, 158, 4) // "help"

    },
    "US_SelectWavelengths\0add_selections\0"
    "\0rmv_selections\0add_all_selects\0"
    "rmv_all_selects\0new_wl_start\0new_wl_end\0"
    "new_wl_incr\0new_wl_range\0report\0cancel\0"
    "done\0reset\0help"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_SelectWavelengths[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    0,   80,    2, 0x08 /* Private */,
       4,    0,   81,    2, 0x08 /* Private */,
       5,    0,   82,    2, 0x08 /* Private */,
       6,    1,   83,    2, 0x08 /* Private */,
       7,    1,   86,    2, 0x08 /* Private */,
       8,    1,   89,    2, 0x08 /* Private */,
       9,    3,   92,    2, 0x08 /* Private */,
      10,    0,   99,    2, 0x08 /* Private */,
      11,    0,  100,    2, 0x08 /* Private */,
      12,    0,  101,    2, 0x08 /* Private */,
      13,    0,  102,    2, 0x08 /* Private */,
      14,    0,  103,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,    2,    2,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void US_SelectWavelengths::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_SelectWavelengths *_t = static_cast<US_SelectWavelengths *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->add_selections(); break;
        case 1: _t->rmv_selections(); break;
        case 2: _t->add_all_selects(); break;
        case 3: _t->rmv_all_selects(); break;
        case 4: _t->new_wl_start((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->new_wl_end((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->new_wl_incr((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->new_wl_range((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const int(*)>(_a[3]))); break;
        case 8: _t->report(); break;
        case 9: _t->cancel(); break;
        case 10: _t->done(); break;
        case 11: _t->reset(); break;
        case 12: _t->help(); break;
        default: ;
        }
    }
}

const QMetaObject US_SelectWavelengths::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_SelectWavelengths.data,
      qt_meta_data_US_SelectWavelengths,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_SelectWavelengths::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_SelectWavelengths::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_SelectWavelengths.stringdata0))
        return static_cast<void*>(const_cast< US_SelectWavelengths*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_SelectWavelengths::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
struct qt_meta_stringdata_US_SelectWavelengths_manual_t {
    QByteArrayData data[9];
    char stringdata0[92];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_SelectWavelengths_manual_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_SelectWavelengths_manual_t qt_meta_stringdata_US_SelectWavelengths_manual = {
    {
QT_MOC_LITERAL(0, 0, 27), // "US_SelectWavelengths_manual"
QT_MOC_LITERAL(1, 28, 11), // "wln_entered"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 11), // "wln_changed"
QT_MOC_LITERAL(4, 53, 15), // "text_to_numbers"
QT_MOC_LITERAL(5, 69, 4), // "done"
QT_MOC_LITERAL(6, 74, 5), // "reset"
QT_MOC_LITERAL(7, 80, 6), // "cancel"
QT_MOC_LITERAL(8, 87, 4) // "help"

    },
    "US_SelectWavelengths_manual\0wln_entered\0"
    "\0wln_changed\0text_to_numbers\0done\0"
    "reset\0cancel\0help"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_SelectWavelengths_manual[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    1,   50,    2, 0x08 /* Private */,
       4,    0,   53,    2, 0x08 /* Private */,
       5,    0,   54,    2, 0x08 /* Private */,
       6,    0,   55,    2, 0x08 /* Private */,
       7,    0,   56,    2, 0x08 /* Private */,
       8,    0,   57,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Bool,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void US_SelectWavelengths_manual::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_SelectWavelengths_manual *_t = static_cast<US_SelectWavelengths_manual *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: { bool _r = _t->wln_entered();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 1: _t->wln_changed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: { bool _r = _t->text_to_numbers();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 3: _t->done(); break;
        case 4: _t->reset(); break;
        case 5: _t->cancel(); break;
        case 6: _t->help(); break;
        default: ;
        }
    }
}

const QMetaObject US_SelectWavelengths_manual::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_SelectWavelengths_manual.data,
      qt_meta_data_US_SelectWavelengths_manual,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_SelectWavelengths_manual::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_SelectWavelengths_manual::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_SelectWavelengths_manual.stringdata0))
        return static_cast<void*>(const_cast< US_SelectWavelengths_manual*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_SelectWavelengths_manual::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
struct qt_meta_stringdata_US_ExperGuiUpload_t {
    QByteArrayData data[11];
    char stringdata0[176];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiUpload_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiUpload_t qt_meta_stringdata_US_ExperGuiUpload = {
    {
QT_MOC_LITERAL(0, 0, 17), // "US_ExperGuiUpload"
QT_MOC_LITERAL(1, 18, 16), // "expdef_submitted"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 22), // "QMap<QString,QString>&"
QT_MOC_LITERAL(4, 59, 16), // "protocol_details"
QT_MOC_LITERAL(5, 76, 16), // "detailExperiment"
QT_MOC_LITERAL(6, 93, 14), // "testConnection"
QT_MOC_LITERAL(7, 108, 24), // "submitExperiment_confirm"
QT_MOC_LITERAL(8, 133, 16), // "submitExperiment"
QT_MOC_LITERAL(9, 150, 15), // "saveRunProtocol"
QT_MOC_LITERAL(10, 166, 9) // "buildJson"

    },
    "US_ExperGuiUpload\0expdef_submitted\0\0"
    "QMap<QString,QString>&\0protocol_details\0"
    "detailExperiment\0testConnection\0"
    "submitExperiment_confirm\0submitExperiment\0"
    "saveRunProtocol\0buildJson"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiUpload[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   52,    2, 0x08 /* Private */,
       6,    0,   53,    2, 0x08 /* Private */,
       7,    0,   54,    2, 0x08 /* Private */,
       8,    0,   55,    2, 0x08 /* Private */,
       9,    0,   56,    2, 0x08 /* Private */,
      10,    0,   57,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::QString,

       0        // eod
};

void US_ExperGuiUpload::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiUpload *_t = static_cast<US_ExperGuiUpload *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->expdef_submitted((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 1: _t->detailExperiment(); break;
        case 2: _t->testConnection(); break;
        case 3: _t->submitExperiment_confirm(); break;
        case 4: _t->submitExperiment(); break;
        case 5: _t->saveRunProtocol(); break;
        case 6: { QString _r = _t->buildJson();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_ExperGuiUpload::*_t)(QMap<QString,QString> & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperGuiUpload::expdef_submitted)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject US_ExperGuiUpload::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiUpload.data,
      qt_meta_data_US_ExperGuiUpload,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiUpload::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiUpload::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiUpload.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiUpload*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiUpload::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void US_ExperGuiUpload::expdef_submitted(QMap<QString,QString> & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_US_ExperGuiAProfile_t {
    QByteArrayData data[6];
    char stringdata0[92];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGuiAProfile_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGuiAProfile_t qt_meta_stringdata_US_ExperGuiAProfile = {
    {
QT_MOC_LITERAL(0, 0, 19), // "US_ExperGuiAProfile"
QT_MOC_LITERAL(1, 20, 16), // "expdef_submitted"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 22), // "QMap<QString,QString>&"
QT_MOC_LITERAL(4, 61, 16), // "protocol_details"
QT_MOC_LITERAL(5, 78, 13) // "detailProfile"

    },
    "US_ExperGuiAProfile\0expdef_submitted\0"
    "\0QMap<QString,QString>&\0protocol_details\0"
    "detailProfile"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGuiAProfile[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   27,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void US_ExperGuiAProfile::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGuiAProfile *_t = static_cast<US_ExperGuiAProfile *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->expdef_submitted((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 1: _t->detailProfile(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_ExperGuiAProfile::*_t)(QMap<QString,QString> & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperGuiAProfile::expdef_submitted)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject US_ExperGuiAProfile::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGuiAProfile.data,
      qt_meta_data_US_ExperGuiAProfile,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGuiAProfile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGuiAProfile::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGuiAProfile.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGuiAProfile*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGuiAProfile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void US_ExperGuiAProfile::expdef_submitted(QMap<QString,QString> & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_US_ExperimentMain_t {
    QByteArrayData data[17];
    char stringdata0[241];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperimentMain_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperimentMain_t qt_meta_stringdata_US_ExperimentMain = {
    {
QT_MOC_LITERAL(0, 0, 17), // "US_ExperimentMain"
QT_MOC_LITERAL(1, 18, 16), // "us_exp_is_closed"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 14), // "to_live_update"
QT_MOC_LITERAL(4, 51, 22), // "QMap<QString,QString>&"
QT_MOC_LITERAL(5, 74, 16), // "protocol_details"
QT_MOC_LITERAL(6, 91, 5), // "reset"
QT_MOC_LITERAL(7, 97, 8), // "newPanel"
QT_MOC_LITERAL(8, 106, 10), // "statUpdate"
QT_MOC_LITERAL(9, 117, 7), // "panelUp"
QT_MOC_LITERAL(10, 125, 9), // "panelDown"
QT_MOC_LITERAL(11, 135, 4), // "help"
QT_MOC_LITERAL(12, 140, 20), // "disable_tabs_buttons"
QT_MOC_LITERAL(13, 161, 28), // "enable_tabs_buttons_readonly"
QT_MOC_LITERAL(14, 190, 19), // "enable_tabs_buttons"
QT_MOC_LITERAL(15, 210, 13), // "close_program"
QT_MOC_LITERAL(16, 224, 16) // "optima_submitted"

    },
    "US_ExperimentMain\0us_exp_is_closed\0\0"
    "to_live_update\0QMap<QString,QString>&\0"
    "protocol_details\0reset\0newPanel\0"
    "statUpdate\0panelUp\0panelDown\0help\0"
    "disable_tabs_buttons\0enable_tabs_buttons_readonly\0"
    "enable_tabs_buttons\0close_program\0"
    "optima_submitted"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperimentMain[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,
       3,    1,   80,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   83,    2, 0x08 /* Private */,
       7,    1,   84,    2, 0x08 /* Private */,
       8,    0,   87,    2, 0x08 /* Private */,
       9,    0,   88,    2, 0x08 /* Private */,
      10,    0,   89,    2, 0x08 /* Private */,
      11,    0,   90,    2, 0x08 /* Private */,
      12,    0,   91,    2, 0x08 /* Private */,
      13,    0,   92,    2, 0x08 /* Private */,
      14,    0,   93,    2, 0x08 /* Private */,
      15,    0,   94,    2, 0x0a /* Public */,
      16,    1,   95,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,

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
    QMetaType::Void, 0x80000000 | 4,    5,

       0        // eod
};

void US_ExperimentMain::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperimentMain *_t = static_cast<US_ExperimentMain *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->us_exp_is_closed(); break;
        case 1: _t->to_live_update((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 2: _t->reset(); break;
        case 3: _t->newPanel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->statUpdate(); break;
        case 5: _t->panelUp(); break;
        case 6: _t->panelDown(); break;
        case 7: _t->help(); break;
        case 8: _t->disable_tabs_buttons(); break;
        case 9: _t->enable_tabs_buttons_readonly(); break;
        case 10: _t->enable_tabs_buttons(); break;
        case 11: _t->close_program(); break;
        case 12: _t->optima_submitted((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_ExperimentMain::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperimentMain::us_exp_is_closed)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (US_ExperimentMain::*_t)(QMap<QString,QString> & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperimentMain::to_live_update)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject US_ExperimentMain::staticMetaObject = {
    { &US_Widgets::staticMetaObject, qt_meta_stringdata_US_ExperimentMain.data,
      qt_meta_data_US_ExperimentMain,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperimentMain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperimentMain::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperimentMain.stringdata0))
        return static_cast<void*>(const_cast< US_ExperimentMain*>(this));
    return US_Widgets::qt_metacast(_clname);
}

int US_ExperimentMain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_Widgets::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void US_ExperimentMain::us_exp_is_closed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void US_ExperimentMain::to_live_update(QMap<QString,QString> & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
