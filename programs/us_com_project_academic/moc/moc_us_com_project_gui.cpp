/****************************************************************************
** Meta object code from reading C++ file 'us_com_project_gui.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../us_com_project/us_com_project_gui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_com_project_gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_US_ExperGui_t {
    QByteArrayData data[9];
    char stringdata0[149];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ExperGui_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ExperGui_t qt_meta_stringdata_US_ExperGui = {
    {
QT_MOC_LITERAL(0, 0, 11), // "US_ExperGui"
QT_MOC_LITERAL(1, 12, 21), // "switch_to_live_update"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 22), // "QMap<QString,QString>&"
QT_MOC_LITERAL(4, 58, 16), // "protocol_details"
QT_MOC_LITERAL(5, 75, 13), // "set_auto_mode"
QT_MOC_LITERAL(6, 89, 16), // "manageExperiment"
QT_MOC_LITERAL(7, 106, 27), // "us_exp_is_closed_set_button"
QT_MOC_LITERAL(8, 134, 14) // "to_live_update"

    },
    "US_ExperGui\0switch_to_live_update\0\0"
    "QMap<QString,QString>&\0protocol_details\0"
    "set_auto_mode\0manageExperiment\0"
    "us_exp_is_closed_set_button\0to_live_update"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ExperGui[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       5,    0,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   43,    2, 0x08 /* Private */,
       7,    0,   44,    2, 0x08 /* Private */,
       8,    1,   45,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void US_ExperGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ExperGui *_t = static_cast<US_ExperGui *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->switch_to_live_update((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 1: _t->set_auto_mode(); break;
        case 2: _t->manageExperiment(); break;
        case 3: _t->us_exp_is_closed_set_button(); break;
        case 4: _t->to_live_update((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_ExperGui::*_t)(QMap<QString,QString> & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperGui::switch_to_live_update)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (US_ExperGui::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ExperGui::set_auto_mode)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject US_ExperGui::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ExperGui.data,
      qt_meta_data_US_ExperGui,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ExperGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ExperGui::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ExperGui.stringdata0))
        return static_cast<void*>(const_cast< US_ExperGui*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ExperGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void US_ExperGui::switch_to_live_update(QMap<QString,QString> & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void US_ExperGui::set_auto_mode()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}
struct qt_meta_stringdata_US_ObservGui_t {
    QByteArrayData data[11];
    char stringdata0[168];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ObservGui_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ObservGui_t qt_meta_stringdata_US_ObservGui = {
    {
QT_MOC_LITERAL(0, 0, 12), // "US_ObservGui"
QT_MOC_LITERAL(1, 13, 13), // "to_xpn_viewer"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 22), // "QMap<QString,QString>&"
QT_MOC_LITERAL(4, 51, 16), // "protocol_details"
QT_MOC_LITERAL(5, 68, 25), // "switch_to_post_processing"
QT_MOC_LITERAL(6, 94, 8), // "QString&"
QT_MOC_LITERAL(7, 103, 7), // "currDir"
QT_MOC_LITERAL(8, 111, 12), // "protocolName"
QT_MOC_LITERAL(9, 124, 24), // "process_protocol_details"
QT_MOC_LITERAL(10, 149, 18) // "to_post_processing"

    },
    "US_ObservGui\0to_xpn_viewer\0\0"
    "QMap<QString,QString>&\0protocol_details\0"
    "switch_to_post_processing\0QString&\0"
    "currDir\0protocolName\0process_protocol_details\0"
    "to_post_processing"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ObservGui[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,
       5,    2,   37,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    1,   42,    2, 0x08 /* Private */,
      10,    2,   45,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,

       0        // eod
};

void US_ObservGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ObservGui *_t = static_cast<US_ObservGui *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->to_xpn_viewer((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 1: _t->switch_to_post_processing((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->process_protocol_details((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 3: _t->to_post_processing((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_ObservGui::*_t)(QMap<QString,QString> & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ObservGui::to_xpn_viewer)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (US_ObservGui::*_t)(QString & , QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ObservGui::switch_to_post_processing)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject US_ObservGui::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ObservGui.data,
      qt_meta_data_US_ObservGui,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ObservGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ObservGui::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ObservGui.stringdata0))
        return static_cast<void*>(const_cast< US_ObservGui*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ObservGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void US_ObservGui::to_xpn_viewer(QMap<QString,QString> & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void US_ObservGui::switch_to_post_processing(QString & _t1, QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_US_PostProdGui_t {
    QByteArrayData data[9];
    char stringdata0[113];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_PostProdGui_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_PostProdGui_t qt_meta_stringdata_US_PostProdGui = {
    {
QT_MOC_LITERAL(0, 0, 14), // "US_PostProdGui"
QT_MOC_LITERAL(1, 15, 12), // "to_post_prod"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 8), // "QString&"
QT_MOC_LITERAL(4, 38, 7), // "currDir"
QT_MOC_LITERAL(5, 46, 12), // "protocolName"
QT_MOC_LITERAL(6, 59, 18), // "switch_to_analysis"
QT_MOC_LITERAL(7, 78, 22), // "import_data_us_convert"
QT_MOC_LITERAL(8, 101, 11) // "to_analysis"

    },
    "US_PostProdGui\0to_post_prod\0\0QString&\0"
    "currDir\0protocolName\0switch_to_analysis\0"
    "import_data_us_convert\0to_analysis"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_PostProdGui[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,
       6,    2,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    2,   44,    2, 0x08 /* Private */,
       8,    2,   49,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,

       0        // eod
};

void US_PostProdGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_PostProdGui *_t = static_cast<US_PostProdGui *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->to_post_prod((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->switch_to_analysis((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->import_data_us_convert((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: _t->to_analysis((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_PostProdGui::*_t)(QString & , QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_PostProdGui::to_post_prod)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (US_PostProdGui::*_t)(QString & , QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_PostProdGui::switch_to_analysis)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject US_PostProdGui::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_PostProdGui.data,
      qt_meta_data_US_PostProdGui,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_PostProdGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_PostProdGui::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_PostProdGui.stringdata0))
        return static_cast<void*>(const_cast< US_PostProdGui*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_PostProdGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void US_PostProdGui::to_post_prod(QString & _t1, QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void US_PostProdGui::switch_to_analysis(QString & _t1, QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_US_AnalysisGui_t {
    QByteArrayData data[7];
    char stringdata0[73];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_AnalysisGui_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_AnalysisGui_t qt_meta_stringdata_US_AnalysisGui = {
    {
QT_MOC_LITERAL(0, 0, 14), // "US_AnalysisGui"
QT_MOC_LITERAL(1, 15, 14), // "start_analysis"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 8), // "QString&"
QT_MOC_LITERAL(4, 40, 7), // "currDir"
QT_MOC_LITERAL(5, 48, 12), // "protocolName"
QT_MOC_LITERAL(6, 61, 11) // "do_analysis"

    },
    "US_AnalysisGui\0start_analysis\0\0QString&\0"
    "currDir\0protocolName\0do_analysis"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_AnalysisGui[] = {

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
       1,    2,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   29,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,

       0        // eod
};

void US_AnalysisGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_AnalysisGui *_t = static_cast<US_AnalysisGui *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->start_analysis((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->do_analysis((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_AnalysisGui::*_t)(QString & , QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_AnalysisGui::start_analysis)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject US_AnalysisGui::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_AnalysisGui.data,
      qt_meta_data_US_AnalysisGui,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_AnalysisGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_AnalysisGui::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_AnalysisGui.stringdata0))
        return static_cast<void*>(const_cast< US_AnalysisGui*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_AnalysisGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void US_AnalysisGui::start_analysis(QString & _t1, QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_US_ReportGui_t {
    QByteArrayData data[1];
    char stringdata0[13];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ReportGui_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ReportGui_t qt_meta_stringdata_US_ReportGui = {
    {
QT_MOC_LITERAL(0, 0, 12) // "US_ReportGui"

    },
    "US_ReportGui"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ReportGui[] = {

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

void US_ReportGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject US_ReportGui::staticMetaObject = {
    { &US_WidgetsDialog::staticMetaObject, qt_meta_stringdata_US_ReportGui.data,
      qt_meta_data_US_ReportGui,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ReportGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ReportGui::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ReportGui.stringdata0))
        return static_cast<void*>(const_cast< US_ReportGui*>(this));
    return US_WidgetsDialog::qt_metacast(_clname);
}

int US_ReportGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_WidgetsDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
struct qt_meta_stringdata_US_ComProjectMain_t {
    QByteArrayData data[13];
    char stringdata0[216];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_US_ComProjectMain_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_US_ComProjectMain_t qt_meta_stringdata_US_ComProjectMain = {
    {
QT_MOC_LITERAL(0, 0, 17), // "US_ComProjectMain"
QT_MOC_LITERAL(1, 18, 19), // "pass_to_live_update"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 22), // "QMap<QString,QString>&"
QT_MOC_LITERAL(4, 62, 16), // "protocol_details"
QT_MOC_LITERAL(5, 79, 22), // "import_data_us_convert"
QT_MOC_LITERAL(6, 102, 8), // "QString&"
QT_MOC_LITERAL(7, 111, 7), // "currDir"
QT_MOC_LITERAL(8, 119, 12), // "protocolName"
QT_MOC_LITERAL(9, 132, 16), // "pass_to_analysis"
QT_MOC_LITERAL(10, 149, 21), // "switch_to_live_update"
QT_MOC_LITERAL(11, 171, 25), // "switch_to_post_processing"
QT_MOC_LITERAL(12, 197, 18) // "switch_to_analysis"

    },
    "US_ComProjectMain\0pass_to_live_update\0"
    "\0QMap<QString,QString>&\0protocol_details\0"
    "import_data_us_convert\0QString&\0currDir\0"
    "protocolName\0pass_to_analysis\0"
    "switch_to_live_update\0switch_to_post_processing\0"
    "switch_to_analysis"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_US_ComProjectMain[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       5,    2,   47,    2, 0x06 /* Public */,
       9,    2,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    1,   57,    2, 0x08 /* Private */,
      11,    2,   60,    2, 0x08 /* Private */,
      12,    2,   65,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,

       0        // eod
};

void US_ComProjectMain::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        US_ComProjectMain *_t = static_cast<US_ComProjectMain *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->pass_to_live_update((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 1: _t->import_data_us_convert((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->pass_to_analysis((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: _t->switch_to_live_update((*reinterpret_cast< QMap<QString,QString>(*)>(_a[1]))); break;
        case 4: _t->switch_to_post_processing((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->switch_to_analysis((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (US_ComProjectMain::*_t)(QMap<QString,QString> & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ComProjectMain::pass_to_live_update)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (US_ComProjectMain::*_t)(QString & , QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ComProjectMain::import_data_us_convert)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (US_ComProjectMain::*_t)(QString & , QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&US_ComProjectMain::pass_to_analysis)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject US_ComProjectMain::staticMetaObject = {
    { &US_Widgets::staticMetaObject, qt_meta_stringdata_US_ComProjectMain.data,
      qt_meta_data_US_ComProjectMain,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *US_ComProjectMain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *US_ComProjectMain::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_US_ComProjectMain.stringdata0))
        return static_cast<void*>(const_cast< US_ComProjectMain*>(this));
    return US_Widgets::qt_metacast(_clname);
}

int US_ComProjectMain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = US_Widgets::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void US_ComProjectMain::pass_to_live_update(QMap<QString,QString> & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void US_ComProjectMain::import_data_us_convert(QString & _t1, QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void US_ComProjectMain::pass_to_analysis(QString & _t1, QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
