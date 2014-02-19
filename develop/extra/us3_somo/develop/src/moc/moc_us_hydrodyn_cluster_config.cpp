/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_config.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_config.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_config.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Config[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      39,   27,   27,   27, 0x08,
      66,   27,   27,   27, 0x08,
      93,   27,   27,   27, 0x08,
     121,   27,   27,   27, 0x08,
     151,   27,   27,   27, 0x08,
     178,   27,   27,   27, 0x08,
     191,   27,   27,   27, 0x08,
     202,   27,   27,   27, 0x08,
     212,   27,   27,   27, 0x08,
     219,   27,   27,   27, 0x08,
     229,   27,   27,   27, 0x08,
     245,   27,   27,   27, 0x08,
     253,   27,   27,   27, 0x08,
     267,   27,   27,   27, 0x08,
     276,   27,   27,   27, 0x08,
     289,  283,   27,   27, 0x08,
     317,  312,   27,   27, 0x08,
     367,  312,   27,   27, 0x08,
     415,  404,   27,   27, 0x08,
     446,  404,   27,   27, 0x08,
     480,  477,   27,   27, 0x08,
     514,  505,   27,   27, 0x08,
     551,  545,   27,   27, 0x08,
     567,   27,   27,   27, 0x08,
     586,   27,   27,   27, 0x08,
     605,   27,   27,   27, 0x08,
     631,   27,   27,   27, 0x08,
     661,   27,   27,   27, 0x08,
     687,  680,   27,   27, 0x08,
     712,   27,   27,   27, 0x08,
     730,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Config[] = {
    "US_Hydrodyn_Cluster_Config\0\0setupGUI()\0"
    "update_cluster_id(QString)\0"
    "update_cluster_pw(QString)\0"
    "update_cluster_pw2(QString)\0"
    "update_cluster_email(QString)\0"
    "update_manage_url(QString)\0check_user()\0"
    "add_user()\0systems()\0edit()\0add_new()\0"
    "delete_system()\0reset()\0save_config()\0"
    "cancel()\0help()\0state\0http_stateChanged(int)\0"
    "resp\0http_responseHeaderReceived(Q3HttpResponseHeader)\0"
    "http_readyRead(Q3HttpResponseHeader)\0"
    "done,total\0http_dataSendProgress(int,int)\0"
    "http_dataReadProgress(int,int)\0id\0"
    "http_requestStarted(int)\0id,error\0"
    "http_requestFinished(int,bool)\0error\0"
    "http_done(bool)\0socket_hostFound()\0"
    "socket_connected()\0socket_connectionClosed()\0"
    "socket_delayedCloseFinished()\0"
    "socket_readyRead()\0nbytes\0"
    "socket_bytesWritten(int)\0socket_error(int)\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Cluster_Config::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Config *_t = static_cast<US_Hydrodyn_Cluster_Config *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_cluster_id((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_cluster_pw((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_cluster_pw2((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_cluster_email((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_manage_url((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->check_user(); break;
        case 7: _t->add_user(); break;
        case 8: _t->systems(); break;
        case 9: _t->edit(); break;
        case 10: _t->add_new(); break;
        case 11: _t->delete_system(); break;
        case 12: _t->reset(); break;
        case 13: _t->save_config(); break;
        case 14: _t->cancel(); break;
        case 15: _t->help(); break;
        case 16: _t->http_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->http_responseHeaderReceived((*reinterpret_cast< const Q3HttpResponseHeader(*)>(_a[1]))); break;
        case 18: _t->http_readyRead((*reinterpret_cast< const Q3HttpResponseHeader(*)>(_a[1]))); break;
        case 19: _t->http_dataSendProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 20: _t->http_dataReadProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 21: _t->http_requestStarted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 22: _t->http_requestFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 23: _t->http_done((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 24: _t->socket_hostFound(); break;
        case 25: _t->socket_connected(); break;
        case 26: _t->socket_connectionClosed(); break;
        case 27: _t->socket_delayedCloseFinished(); break;
        case 28: _t->socket_readyRead(); break;
        case 29: _t->socket_bytesWritten((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: _t->socket_error((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 31: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Config::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Config::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Config,
      qt_meta_data_US_Hydrodyn_Cluster_Config, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Config::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Config::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Config::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Config))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Config*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Config::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 32)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 32;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
