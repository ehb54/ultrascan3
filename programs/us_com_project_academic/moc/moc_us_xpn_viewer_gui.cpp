/****************************************************************************
** Meta object code from reading C++ file 'us_xpn_viewer_gui.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../us_xpn_viewer/us_xpn_viewer_gui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_xpn_viewer_gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_SpeedoMeter_t {
    QByteArrayData data[1];
    char stringdata0[12];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SpeedoMeter_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SpeedoMeter_t qt_meta_stringdata_SpeedoMeter = {
    {
QT_MOC_LITERAL(0, 0, 11) // "SpeedoMeter"

    },
    "SpeedoMeter"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SpeedoMeter[] = {

 // content:
       7,       // revision
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

void SpeedoMeter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject SpeedoMeter::staticMetaObject = {
    { &QwtDial::staticMetaObject, qt_meta_stringdata_SpeedoMeter.data,
      qt_meta_data_SpeedoMeter,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *SpeedoMeter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SpeedoMeter::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_SpeedoMeter.stringdata0))
        return static_cast<void*>(const_cast< SpeedoMeter*>(this));
    return QwtDial::qt_metacast(_clname);
}

int SpeedoMeter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtDial::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
struct qt_meta_stringdata_DialBox_t {
    QByteArrayData data[4];
    char stringdata0[18];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DialBox_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DialBox_t qt_meta_stringdata_DialBox = {
    {
QT_MOC_LITERAL(0, 0, 7), // "DialBox"
QT_MOC_LITERAL(1, 8, 6), // "setNum"
QT_MOC_LITERAL(2, 15, 0), // ""
QT_MOC_LITERAL(3, 16, 1) // "v"

    },
    "DialBox\0setNum\0\0v"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DialBox[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,

       0        // eod
};

void DialBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DialBox *_t = static_cast<DialBox *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setNum((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject DialBox::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DialBox.data,
      qt_meta_data_DialBox,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *DialBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DialBox::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_DialBox.stringdata0))
        return static_cast<void*>(const_cast< DialBox*>(this));
    return QWidget::qt_metacast(_clname);
}

int DialBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_WheelBox_t {
    QByteArrayData data[4];
    char stringdata0[19];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WheelBox_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WheelBox_t qt_meta_stringdata_WheelBox = {
    {
QT_MOC_LITERAL(0, 0, 8), // "WheelBox"
QT_MOC_LITERAL(1, 9, 6), // "setNum"
QT_MOC_LITERAL(2, 16, 0), // ""
QT_MOC_LITERAL(3, 17, 1) // "v"

    },
    "WheelBox\0setNum\0\0v"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WheelBox[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,

       0        // eod
};

void WheelBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        WheelBox *_t = static_cast<WheelBox *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setNum((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject WheelBox::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_WheelBox.data,
      qt_meta_data_WheelBox,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *WheelBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WheelBox::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_WheelBox.stringdata0))
        return static_cast<void*>(const_cast< WheelBox*>(this));
    return QWidget::qt_metacast(_clname);
}

int WheelBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_US_XpnDataViewer_t {
    QByteArrayData data[53];
    char stringdata0[723];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_XpnDataViewer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_XpnDataViewer_t qt_meta_stringdata_US_XpnDataViewer = {
    {
QT_MOC_LITERAL(0, 0, 16), // "US_XpnDataViewer"
QT_MOC_LITERAL(1, 17, 24), // "experiment_complete_auto"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 8), // "QString&"
QT_MOC_LITERAL(4, 52, 5), // "reset"
QT_MOC_LITERAL(5, 58, 12), // "load_xpn_raw"
QT_MOC_LITERAL(6, 71, 17), // "load_xpn_raw_auto"
QT_MOC_LITERAL(7, 89, 14), // "check_for_data"
QT_MOC_LITERAL(8, 104, 22), // "QMap<QString,QString>&"
QT_MOC_LITERAL(9, 127, 17), // "check_for_sysdata"
QT_MOC_LITERAL(10, 145, 21), // "retrieve_xpn_raw_auto"
QT_MOC_LITERAL(11, 167, 15), // "reloadData_auto"
QT_MOC_LITERAL(12, 183, 21), // "CheckExpComplete_auto"
QT_MOC_LITERAL(13, 205, 12), // "load_auc_xpn"
QT_MOC_LITERAL(14, 218, 12), // "plot_current"
QT_MOC_LITERAL(15, 231, 11), // "plot_titles"
QT_MOC_LITERAL(16, 243, 8), // "plot_all"
QT_MOC_LITERAL(17, 252, 14), // "enableControls"
QT_MOC_LITERAL(18, 267, 10), // "runDetails"
QT_MOC_LITERAL(19, 278, 10), // "export_auc"
QT_MOC_LITERAL(20, 289, 15), // "export_auc_auto"
QT_MOC_LITERAL(21, 305, 12), // "changeRadius"
QT_MOC_LITERAL(22, 318, 12), // "changeCellCh"
QT_MOC_LITERAL(23, 331, 12), // "changeRecord"
QT_MOC_LITERAL(24, 344, 12), // "changeOptics"
QT_MOC_LITERAL(25, 357, 12), // "changeReload"
QT_MOC_LITERAL(26, 370, 14), // "changeInterval"
QT_MOC_LITERAL(27, 385, 8), // "resetAll"
QT_MOC_LITERAL(28, 394, 8), // "prevPlot"
QT_MOC_LITERAL(29, 403, 8), // "nextPlot"
QT_MOC_LITERAL(30, 412, 14), // "compute_ranges"
QT_MOC_LITERAL(31, 427, 14), // "connect_ranges"
QT_MOC_LITERAL(32, 442, 12), // "exclude_from"
QT_MOC_LITERAL(33, 455, 10), // "exclude_to"
QT_MOC_LITERAL(34, 466, 13), // "exclude_scans"
QT_MOC_LITERAL(35, 480, 13), // "include_scans"
QT_MOC_LITERAL(36, 494, 10), // "dvec_index"
QT_MOC_LITERAL(37, 505, 16), // "QVector<double>&"
QT_MOC_LITERAL(38, 522, 13), // "showTimeState"
QT_MOC_LITERAL(39, 536, 13), // "status_report"
QT_MOC_LITERAL(40, 550, 10), // "reloadData"
QT_MOC_LITERAL(41, 561, 10), // "timerEvent"
QT_MOC_LITERAL(42, 572, 12), // "QTimerEvent*"
QT_MOC_LITERAL(43, 585, 1), // "e"
QT_MOC_LITERAL(44, 587, 14), // "selectColorMap"
QT_MOC_LITERAL(45, 602, 13), // "correct_radii"
QT_MOC_LITERAL(46, 616, 12), // "currentRectf"
QT_MOC_LITERAL(47, 629, 4), // "help"
QT_MOC_LITERAL(48, 634, 20), // "read_optima_machines"
QT_MOC_LITERAL(49, 655, 7), // "US_DB2*"
QT_MOC_LITERAL(50, 663, 12), // "changeOptima"
QT_MOC_LITERAL(51, 676, 23), // "selectOptimaByName_auto"
QT_MOC_LITERAL(52, 700, 22) // "test_optima_connection"

    },
    "US_XpnDataViewer\0experiment_complete_auto\0"
    "\0QString&\0reset\0load_xpn_raw\0"
    "load_xpn_raw_auto\0check_for_data\0"
    "QMap<QString,QString>&\0check_for_sysdata\0"
    "retrieve_xpn_raw_auto\0reloadData_auto\0"
    "CheckExpComplete_auto\0load_auc_xpn\0"
    "plot_current\0plot_titles\0plot_all\0"
    "enableControls\0runDetails\0export_auc\0"
    "export_auc_auto\0changeRadius\0changeCellCh\0"
    "changeRecord\0changeOptics\0changeReload\0"
    "changeInterval\0resetAll\0prevPlot\0"
    "nextPlot\0compute_ranges\0connect_ranges\0"
    "exclude_from\0exclude_to\0exclude_scans\0"
    "include_scans\0dvec_index\0QVector<double>&\0"
    "showTimeState\0status_report\0reloadData\0"
    "timerEvent\0QTimerEvent*\0e\0selectColorMap\0"
    "correct_radii\0currentRectf\0help\0"
    "read_optima_machines\0US_DB2*\0changeOptima\0"
    "selectOptimaByName_auto\0test_optima_connection"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_XpnDataViewer[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      46,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,  244,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,  249,    2, 0x08 /* Private */,
       5,    0,  250,    2, 0x08 /* Private */,
       6,    0,  251,    2, 0x08 /* Private */,
       7,    1,  252,    2, 0x08 /* Private */,
       9,    0,  255,    2, 0x08 /* Private */,
      10,    0,  256,    2, 0x08 /* Private */,
      11,    0,  257,    2, 0x08 /* Private */,
      12,    1,  258,    2, 0x08 /* Private */,
      13,    0,  261,    2, 0x08 /* Private */,
      14,    0,  262,    2, 0x08 /* Private */,
      15,    0,  263,    2, 0x08 /* Private */,
      16,    0,  264,    2, 0x08 /* Private */,
      17,    0,  265,    2, 0x08 /* Private */,
      18,    0,  266,    2, 0x08 /* Private */,
      19,    0,  267,    2, 0x08 /* Private */,
      20,    0,  268,    2, 0x08 /* Private */,
      21,    0,  269,    2, 0x08 /* Private */,
      22,    0,  270,    2, 0x08 /* Private */,
      23,    0,  271,    2, 0x08 /* Private */,
      24,    0,  272,    2, 0x08 /* Private */,
      25,    0,  273,    2, 0x08 /* Private */,
      26,    0,  274,    2, 0x08 /* Private */,
      27,    0,  275,    2, 0x08 /* Private */,
      28,    0,  276,    2, 0x08 /* Private */,
      29,    0,  277,    2, 0x08 /* Private */,
      30,    0,  278,    2, 0x08 /* Private */,
      31,    1,  279,    2, 0x08 /* Private */,
      32,    1,  282,    2, 0x08 /* Private */,
      33,    1,  285,    2, 0x08 /* Private */,
      34,    0,  288,    2, 0x08 /* Private */,
      35,    0,  289,    2, 0x08 /* Private */,
      36,    2,  290,    2, 0x08 /* Private */,
      38,    0,  295,    2, 0x08 /* Private */,
      39,    1,  296,    2, 0x08 /* Private */,
      40,    0,  299,    2, 0x08 /* Private */,
      41,    1,  300,    2, 0x08 /* Private */,
      44,    0,  303,    2, 0x08 /* Private */,
      45,    0,  304,    2, 0x08 /* Private */,
      46,    1,  305,    2, 0x08 /* Private */,
      47,    0,  308,    2, 0x08 /* Private */,
      48,    1,  309,    2, 0x08 /* Private */,
      48,    0,  312,    2, 0x28 /* Private | MethodCloned */,
      50,    1,  313,    2, 0x08 /* Private */,
      51,    1,  316,    2, 0x08 /* Private */,
      52,    0,  319,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    2,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Void, 0x80000000 | 8,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool, 0x80000000 | 3,    2,
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
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Int, 0x80000000 | 37, QMetaType::Double,    2,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 42,   43,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRectF,    2,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 49,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void US_XpnDataViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_XpnDataViewer *_t = static_cast<US_XpnDataViewer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->experiment_complete_auto((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->reset(); break;
        case 2: _t->load_xpn_raw(); break;
        case 3: { bool _r = _t->load_xpn_raw_auto();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 4: _t->check_for_data((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 5: _t->check_for_sysdata(); break;
        case 6: _t->retrieve_xpn_raw_auto(); break;
        case 7: _t->reloadData_auto(); break;
        case 8: { bool _r = _t->CheckExpComplete_auto((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 9: _t->load_auc_xpn(); break;
        case 10: _t->plot_current(); break;
        case 11: _t->plot_titles(); break;
        case 12: _t->plot_all(); break;
        case 13: _t->enableControls(); break;
        case 14: _t->runDetails(); break;
        case 15: _t->export_auc(); break;
        case 16: _t->export_auc_auto(); break;
        case 17: _t->changeRadius(); break;
        case 18: _t->changeCellCh(); break;
        case 19: _t->changeRecord(); break;
        case 20: _t->changeOptics(); break;
        case 21: _t->changeReload(); break;
        case 22: _t->changeInterval(); break;
        case 23: _t->resetAll(); break;
        case 24: _t->prevPlot(); break;
        case 25: _t->nextPlot(); break;
        case 26: _t->compute_ranges(); break;
        case 27: _t->connect_ranges((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 28: _t->exclude_from((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 29: _t->exclude_to((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 30: _t->exclude_scans(); break;
        case 31: _t->include_scans(); break;
        case 32: { int _r = _t->dvec_index((*reinterpret_cast< QVector<double>(*)>(_a[1])),(*reinterpret_cast< const double(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 33: _t->showTimeState(); break;
        case 34: _t->status_report((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 35: _t->reloadData(); break;
        case 36: _t->timerEvent((*reinterpret_cast< QTimerEvent*(*)>(_a[1]))); break;
        case 37: _t->selectColorMap(); break;
        case 38: _t->correct_radii(); break;
        case 39: _t->currentRectf((*reinterpret_cast< QRectF(*)>(_a[1]))); break;
        case 40: _t->help(); break;
        case 41: _t->read_optima_machines((*reinterpret_cast< US_DB2*(*)>(_a[1]))); break;
        case 42: _t->read_optima_machines(); break;
        case 43: _t->changeOptima((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 44: _t->selectOptimaByName_auto((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 45: _t->test_optima_connection(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_XpnDataViewer::*_t)(QString & , QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_XpnDataViewer::experiment_complete_auto)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject US_XpnDataViewer::staticMetaObject = {
    { &US_Widgets::staticMetaObject, qt_meta_stringdata_US_XpnDataViewer.data,
      qt_meta_data_US_XpnDataViewer,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_XpnDataViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_XpnDataViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_XpnDataViewer.stringdata0))
        return static_cast<void*>(const_cast< US_XpnDataViewer*>(this));
    return US_Widgets::qt_metacast(_clname);
}

int US_XpnDataViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_Widgets::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 46)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 46;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 46)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 46;
    }
    return _id;
}

// SIGNAL 0
void US_XpnDataViewer::experiment_complete_auto(QString & _t1, QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
