/****************************************************************************
** US_Hydrodyn_Cluster_Submit meta object code from reading C++ file 'us_hydrodyn_cluster_submit.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_submit.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Cluster_Submit::className() const
{
    return "US_Hydrodyn_Cluster_Submit";
}

QMetaObject *US_Hydrodyn_Cluster_Submit::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Submit( "US_Hydrodyn_Cluster_Submit", &US_Hydrodyn_Cluster_Submit::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Submit::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Submit", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Submit::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Submit", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Submit::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"systems", 0, 0 };
    static const QUMethod slot_1 = {"process_next", 0, 0 };
    static const QUMethod slot_2 = {"process_prepare_stage", 0, 0 };
    static const QUMethod slot_3 = {"process_stage", 0, 0 };
    static const QUMethod slot_4 = {"process_submit", 0, 0 };
    static const QUMethod slot_5 = {"process_move", 0, 0 };
    static const QUMethod slot_6 = {"setupGUI", 0, 0 };
    static const QUMethod slot_7 = {"update_enables", 0, 0 };
    static const QUMethod slot_8 = {"select_all", 0, 0 };
    static const QUMethod slot_9 = {"remove", 0, 0 };
    static const QUMethod slot_10 = {"submit", 0, 0 };
    static const QUMethod slot_11 = {"clear_display", 0, 0 };
    static const QUMethod slot_12 = {"update_font", 0, 0 };
    static const QUMethod slot_13 = {"save", 0, 0 };
    static const QUMethod slot_14 = {"stop", 0, 0 };
    static const QUMethod slot_15 = {"cancel", 0, 0 };
    static const QUMethod slot_16 = {"help", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ "state", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"http_stateChanged", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ "resp", &static_QUType_ptr, "QHttpResponseHeader", QUParameter::In }
    };
    static const QUMethod slot_18 = {"http_responseHeaderReceived", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ "resp", &static_QUType_ptr, "QHttpResponseHeader", QUParameter::In }
    };
    static const QUMethod slot_19 = {"http_readyRead", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ "done", &static_QUType_int, 0, QUParameter::In },
	{ "total", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"http_dataSendProgress", 2, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ "done", &static_QUType_int, 0, QUParameter::In },
	{ "total", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"http_dataReadProgress", 2, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"http_requestStarted", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "error", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"http_requestFinished", 2, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ "error", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"http_done", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ "state", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"ftp_stateChanged", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"ftp_commandStarted", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "error", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"ftp_commandFinished", 2, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ "error", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"ftp_done", 1, param_slot_28 };
    static const QUMethod slot_29 = {"system_proc_readFromStdout", 0, 0 };
    static const QUMethod slot_30 = {"system_proc_readFromStderr", 0, 0 };
    static const QUMethod slot_31 = {"system_proc_processExited", 0, 0 };
    static const QUMethod slot_32 = {"system_proc_launchFinished", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_33 = {"closeEvent", 1, param_slot_33 };
    static const QMetaData slot_tbl[] = {
	{ "systems()", &slot_0, QMetaData::Private },
	{ "process_next()", &slot_1, QMetaData::Private },
	{ "process_prepare_stage()", &slot_2, QMetaData::Private },
	{ "process_stage()", &slot_3, QMetaData::Private },
	{ "process_submit()", &slot_4, QMetaData::Private },
	{ "process_move()", &slot_5, QMetaData::Private },
	{ "setupGUI()", &slot_6, QMetaData::Private },
	{ "update_enables()", &slot_7, QMetaData::Private },
	{ "select_all()", &slot_8, QMetaData::Private },
	{ "remove()", &slot_9, QMetaData::Private },
	{ "submit()", &slot_10, QMetaData::Private },
	{ "clear_display()", &slot_11, QMetaData::Private },
	{ "update_font()", &slot_12, QMetaData::Private },
	{ "save()", &slot_13, QMetaData::Private },
	{ "stop()", &slot_14, QMetaData::Private },
	{ "cancel()", &slot_15, QMetaData::Private },
	{ "help()", &slot_16, QMetaData::Private },
	{ "http_stateChanged(int)", &slot_17, QMetaData::Private },
	{ "http_responseHeaderReceived(const QHttpResponseHeader&)", &slot_18, QMetaData::Private },
	{ "http_readyRead(const QHttpResponseHeader&)", &slot_19, QMetaData::Private },
	{ "http_dataSendProgress(int,int)", &slot_20, QMetaData::Private },
	{ "http_dataReadProgress(int,int)", &slot_21, QMetaData::Private },
	{ "http_requestStarted(int)", &slot_22, QMetaData::Private },
	{ "http_requestFinished(int,bool)", &slot_23, QMetaData::Private },
	{ "http_done(bool)", &slot_24, QMetaData::Private },
	{ "ftp_stateChanged(int)", &slot_25, QMetaData::Private },
	{ "ftp_commandStarted(int)", &slot_26, QMetaData::Private },
	{ "ftp_commandFinished(int,bool)", &slot_27, QMetaData::Private },
	{ "ftp_done(bool)", &slot_28, QMetaData::Private },
	{ "system_proc_readFromStdout()", &slot_29, QMetaData::Private },
	{ "system_proc_readFromStderr()", &slot_30, QMetaData::Private },
	{ "system_proc_processExited()", &slot_31, QMetaData::Private },
	{ "system_proc_launchFinished()", &slot_32, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_33, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Submit", parentObject,
	slot_tbl, 34,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Submit.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Submit::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Submit" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Submit::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: systems(); break;
    case 1: process_next(); break;
    case 2: process_prepare_stage(); break;
    case 3: process_stage(); break;
    case 4: process_submit(); break;
    case 5: process_move(); break;
    case 6: setupGUI(); break;
    case 7: update_enables(); break;
    case 8: select_all(); break;
    case 9: remove(); break;
    case 10: submit(); break;
    case 11: clear_display(); break;
    case 12: update_font(); break;
    case 13: save(); break;
    case 14: stop(); break;
    case 15: cancel(); break;
    case 16: help(); break;
    case 17: http_stateChanged((int)static_QUType_int.get(_o+1)); break;
    case 18: http_responseHeaderReceived((const Q3HttpResponseHeader&)*((const Q3HttpResponseHeader*)static_QUType_ptr.get(_o+1))); break;
    case 19: http_readyRead((const Q3HttpResponseHeader&)*((const Q3HttpResponseHeader*)static_QUType_ptr.get(_o+1))); break;
    case 20: http_dataSendProgress((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 21: http_dataReadProgress((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 22: http_requestStarted((int)static_QUType_int.get(_o+1)); break;
    case 23: http_requestFinished((int)static_QUType_int.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 24: http_done((bool)static_QUType_bool.get(_o+1)); break;
    case 25: ftp_stateChanged((int)static_QUType_int.get(_o+1)); break;
    case 26: ftp_commandStarted((int)static_QUType_int.get(_o+1)); break;
    case 27: ftp_commandFinished((int)static_QUType_int.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 28: ftp_done((bool)static_QUType_bool.get(_o+1)); break;
    case 29: system_proc_readFromStdout(); break;
    case 30: system_proc_readFromStderr(); break;
    case 31: system_proc_processExited(); break;
    case 32: system_proc_launchFinished(); break;
    case 33: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Submit::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Submit::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Submit::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
