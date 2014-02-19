/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_cluster_status.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_cluster_status.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_cluster_status.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Cluster_Status[] = {

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
      56,   27,   27,   27, 0x08,
      66,   27,   27,   27, 0x08,
      75,   27,   27,   27, 0x08,
      86,   27,   27,   27, 0x08,
     106,   27,   27,   27, 0x08,
     122,   27,   27,   27, 0x08,
     136,   27,   27,   27, 0x08,
     143,   27,   27,   27, 0x08,
     150,   27,   27,   27, 0x08,
     159,   27,   27,   27, 0x08,
     166,   27,   27,   27, 0x08,
     180,   27,   27,   27, 0x08,
     198,   27,   27,   27, 0x08,
     224,  218,   27,   27, 0x08,
     252,  247,   27,   27, 0x08,
     302,  247,   27,   27, 0x08,
     350,  339,   27,   27, 0x08,
     381,  339,   27,   27, 0x08,
     415,  412,   27,   27, 0x08,
     449,  440,   27,   27, 0x08,
     486,  480,   27,   27, 0x08,
     502,  218,   27,   27, 0x08,
     524,  412,   27,   27, 0x08,
     548,  440,   27,   27, 0x08,
     578,  480,   27,   27, 0x08,
     593,   27,   27,   27, 0x08,
     622,   27,   27,   27, 0x08,
     651,   27,   27,   27, 0x08,
     679,   27,   27,   27, 0x08,
     708,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Cluster_Status[] = {
    "US_Hydrodyn_Cluster_Status\0\0setupGUI()\0"
    "update_enables()\0refresh()\0remove()\0"
    "retrieve()\0retrieve_selected()\0"
    "clear_display()\0update_font()\0save()\0"
    "stop()\0cancel()\0help()\0next_status()\0"
    "get_next_status()\0get_next_retrieve()\0"
    "state\0http_stateChanged(int)\0resp\0"
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

void US_Hydrodyn_Cluster_Status::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Cluster_Status *_t = static_cast<US_Hydrodyn_Cluster_Status *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->update_enables(); break;
        case 2: _t->refresh(); break;
        case 3: _t->remove(); break;
        case 4: _t->retrieve(); break;
        case 5: _t->retrieve_selected(); break;
        case 6: _t->clear_display(); break;
        case 7: _t->update_font(); break;
        case 8: _t->save(); break;
        case 9: _t->stop(); break;
        case 10: _t->cancel(); break;
        case 11: _t->help(); break;
        case 12: _t->next_status(); break;
        case 13: _t->get_next_status(); break;
        case 14: _t->get_next_retrieve(); break;
        case 15: _t->http_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->http_responseHeaderReceived((*reinterpret_cast< const Q3HttpResponseHeader(*)>(_a[1]))); break;
        case 17: _t->http_readyRead((*reinterpret_cast< const Q3HttpResponseHeader(*)>(_a[1]))); break;
        case 18: _t->http_dataSendProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 19: _t->http_dataReadProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 20: _t->http_requestStarted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->http_requestFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 22: _t->http_done((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 23: _t->ftp_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: _t->ftp_commandStarted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: _t->ftp_commandFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 26: _t->ftp_done((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 27: _t->system_proc_readFromStdout(); break;
        case 28: _t->system_proc_readFromStderr(); break;
        case 29: _t->system_proc_processExited(); break;
        case 30: _t->system_proc_launchFinished(); break;
        case 31: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Cluster_Status::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Cluster_Status::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Cluster_Status,
      qt_meta_data_US_Hydrodyn_Cluster_Status, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Cluster_Status::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Cluster_Status::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Cluster_Status::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Cluster_Status))
        return static_cast<void*>(const_cast< US_Hydrodyn_Cluster_Status*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Cluster_Status::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
