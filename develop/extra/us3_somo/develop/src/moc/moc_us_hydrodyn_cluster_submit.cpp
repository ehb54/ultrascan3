/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_submit.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_submit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_submit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Submit[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      34,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      38,   27,   27,   27, 0x08,
      53,   27,   27,   27, 0x08,
      77,   27,   27,   27, 0x08,
      93,   27,   27,   27, 0x08,
     110,   27,   27,   27, 0x08,
     125,   27,   27,   27, 0x08,
     136,   27,   27,   27, 0x08,
     153,   27,   27,   27, 0x08,
     166,   27,   27,   27, 0x08,
     175,   27,   27,   27, 0x08,
     184,   27,   27,   27, 0x08,
     200,   27,   27,   27, 0x08,
     214,   27,   27,   27, 0x08,
     221,   27,   27,   27, 0x08,
     228,   27,   27,   27, 0x08,
     237,   27,   27,   27, 0x08,
     250,  244,   27,   27, 0x08,
     278,  273,   27,   27, 0x08,
     328,  273,   27,   27, 0x08,
     376,  365,   27,   27, 0x08,
     407,  365,   27,   27, 0x08,
     441,  438,   27,   27, 0x08,
     475,  466,   27,   27, 0x08,
     512,  506,   27,   27, 0x08,
     528,  244,   27,   27, 0x08,
     550,  438,   27,   27, 0x08,
     574,  466,   27,   27, 0x08,
     604,  506,   27,   27, 0x08,
     619,   27,   27,   27, 0x08,
     648,   27,   27,   27, 0x08,
     677,   27,   27,   27, 0x08,
     705,   27,   27,   27, 0x08,
     734,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Submit[] = {
    "US_Hydrodyn_Cluster_Submit\0\0systems()\0"
    "process_next()\0process_prepare_stage()\0"
    "process_stage()\0process_submit()\0"
    "process_move()\0setupGUI()\0update_enables()\0"
    "select_all()\0remove()\0submit()\0"
    "clear_display()\0update_font()\0save()\0"
    "stop()\0cancel()\0help()\0state\0"
    "http_stateChanged(int)\0resp\0"
    "http_responseHeaderReceived(Q3HttpResponseHeader)\0"
    "http_readyRead(Q3HttpResponseHeader)\0"
    "done,total\0http_dataSendProgress(int,int)\0"
    "http_dataReadProgress(int,int)\0id\0"
    "http_requestStarted(int)\0id,error\0"
    "http_requestFinished(int,bool)\0error\0"
    "http_done(bool)\0ftp_stateChanged(int)\0"
    "ftp_commandStarted(int)\0"
    "ftp_commandFinished(int,bool)\0"
    "ftp_done(bool)\0system_proc_readFromStdout()\0"
    "system_proc_readFromStderr()\0"
    "system_proc_processExited()\0"
    "system_proc_launchFinished()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster_Submit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Submit *_t = static_cast<US_Hydrodyn_Cluster_Submit *>(_o);
        switch (_id) {
        case 0: _t->systems(); break;
        case 1: _t->process_next(); break;
        case 2: _t->process_prepare_stage(); break;
        case 3: _t->process_stage(); break;
        case 4: _t->process_submit(); break;
        case 5: _t->process_move(); break;
        case 6: _t->setupGUI(); break;
        case 7: _t->update_enables(); break;
        case 8: _t->select_all(); break;
        case 9: _t->remove(); break;
        case 10: _t->submit(); break;
        case 11: _t->clear_display(); break;
        case 12: _t->update_font(); break;
        case 13: _t->save(); break;
        case 14: _t->stop(); break;
        case 15: _t->cancel(); break;
        case 16: _t->help(); break;
        case 17: _t->http_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->http_responseHeaderReceived((*reinterpret_cast< const Q3HttpResponseHeader(*)>(_a[1]))); break;
        case 19: _t->http_readyRead((*reinterpret_cast< const Q3HttpResponseHeader(*)>(_a[1]))); break;
        case 20: _t->http_dataSendProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 21: _t->http_dataReadProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 22: _t->http_requestStarted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->http_requestFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 24: _t->http_done((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 25: _t->ftp_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 26: _t->ftp_commandStarted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 27: _t->ftp_commandFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 28: _t->ftp_done((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 29: _t->system_proc_readFromStdout(); break;
        case 30: _t->system_proc_readFromStderr(); break;
        case 31: _t->system_proc_processExited(); break;
        case 32: _t->system_proc_launchFinished(); break;
        case 33: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Submit::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Submit::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Submit,
      qt_meta_data_US_Hydrodyn_Cluster_Submit, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Submit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Submit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Submit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Submit))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Submit*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Submit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 34)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 34;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
