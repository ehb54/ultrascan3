/****************************************************************************
** US_Hydrodyn_Cluster_Config meta object code from reading C++ file 'us_hydrodyn_cluster_config.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_config.h"
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

const char *US_Hydrodyn_Cluster_Config::className() const
{
    return "US_Hydrodyn_Cluster_Config";
}

QMetaObject *US_Hydrodyn_Cluster_Config::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Config( "US_Hydrodyn_Cluster_Config", &US_Hydrodyn_Cluster_Config::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Config::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Config", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Config::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Config", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Config::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_cluster_id", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_cluster_pw", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_cluster_pw2", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_cluster_email", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_manage_url", 1, param_slot_5 };
    static const QUMethod slot_6 = {"check_user", 0, 0 };
    static const QUMethod slot_7 = {"add_user", 0, 0 };
    static const QUMethod slot_8 = {"systems", 0, 0 };
    static const QUMethod slot_9 = {"edit", 0, 0 };
    static const QUMethod slot_10 = {"add_new", 0, 0 };
    static const QUMethod slot_11 = {"delete_system", 0, 0 };
    static const QUMethod slot_12 = {"reset", 0, 0 };
    static const QUMethod slot_13 = {"save_config", 0, 0 };
    static const QUMethod slot_14 = {"cancel", 0, 0 };
    static const QUMethod slot_15 = {"help", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ "state", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"http_stateChanged", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ "resp", &static_QUType_ptr, "QHttpResponseHeader", QUParameter::In }
    };
    static const QUMethod slot_17 = {"http_responseHeaderReceived", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ "resp", &static_QUType_ptr, "QHttpResponseHeader", QUParameter::In }
    };
    static const QUMethod slot_18 = {"http_readyRead", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ "done", &static_QUType_int, 0, QUParameter::In },
	{ "total", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"http_dataSendProgress", 2, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ "done", &static_QUType_int, 0, QUParameter::In },
	{ "total", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"http_dataReadProgress", 2, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"http_requestStarted", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "error", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"http_requestFinished", 2, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ "error", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"http_done", 1, param_slot_23 };
    static const QUMethod slot_24 = {"socket_hostFound", 0, 0 };
    static const QUMethod slot_25 = {"socket_connected", 0, 0 };
    static const QUMethod slot_26 = {"socket_connectionClosed", 0, 0 };
    static const QUMethod slot_27 = {"socket_delayedCloseFinished", 0, 0 };
    static const QUMethod slot_28 = {"socket_readyRead", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ "nbytes", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"socket_bytesWritten", 1, param_slot_29 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_30 = {"socket_error", 1, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_31 = {"closeEvent", 1, param_slot_31 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_cluster_id(const QString&)", &slot_1, QMetaData::Private },
	{ "update_cluster_pw(const QString&)", &slot_2, QMetaData::Private },
	{ "update_cluster_pw2(const QString&)", &slot_3, QMetaData::Private },
	{ "update_cluster_email(const QString&)", &slot_4, QMetaData::Private },
	{ "update_manage_url(const QString&)", &slot_5, QMetaData::Private },
	{ "check_user()", &slot_6, QMetaData::Private },
	{ "add_user()", &slot_7, QMetaData::Private },
	{ "systems()", &slot_8, QMetaData::Private },
	{ "edit()", &slot_9, QMetaData::Private },
	{ "add_new()", &slot_10, QMetaData::Private },
	{ "delete_system()", &slot_11, QMetaData::Private },
	{ "reset()", &slot_12, QMetaData::Private },
	{ "save_config()", &slot_13, QMetaData::Private },
	{ "cancel()", &slot_14, QMetaData::Private },
	{ "help()", &slot_15, QMetaData::Private },
	{ "http_stateChanged(int)", &slot_16, QMetaData::Private },
	{ "http_responseHeaderReceived(const QHttpResponseHeader&)", &slot_17, QMetaData::Private },
	{ "http_readyRead(const QHttpResponseHeader&)", &slot_18, QMetaData::Private },
	{ "http_dataSendProgress(int,int)", &slot_19, QMetaData::Private },
	{ "http_dataReadProgress(int,int)", &slot_20, QMetaData::Private },
	{ "http_requestStarted(int)", &slot_21, QMetaData::Private },
	{ "http_requestFinished(int,bool)", &slot_22, QMetaData::Private },
	{ "http_done(bool)", &slot_23, QMetaData::Private },
	{ "socket_hostFound()", &slot_24, QMetaData::Private },
	{ "socket_connected()", &slot_25, QMetaData::Private },
	{ "socket_connectionClosed()", &slot_26, QMetaData::Private },
	{ "socket_delayedCloseFinished()", &slot_27, QMetaData::Private },
	{ "socket_readyRead()", &slot_28, QMetaData::Private },
	{ "socket_bytesWritten(int)", &slot_29, QMetaData::Private },
	{ "socket_error(int)", &slot_30, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_31, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Config", parentObject,
	slot_tbl, 32,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Config.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Config::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Config" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Config::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_cluster_id((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_cluster_pw((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_cluster_pw2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_cluster_email((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_manage_url((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: check_user(); break;
    case 7: add_user(); break;
    case 8: systems(); break;
    case 9: edit(); break;
    case 10: add_new(); break;
    case 11: delete_system(); break;
    case 12: reset(); break;
    case 13: save_config(); break;
    case 14: cancel(); break;
    case 15: help(); break;
    case 16: http_stateChanged((int)static_QUType_int.get(_o+1)); break;
    case 17: http_responseHeaderReceived((const Q3HttpResponseHeader&)*((const Q3HttpResponseHeader*)static_QUType_ptr.get(_o+1))); break;
    case 18: http_readyRead((const Q3HttpResponseHeader&)*((const Q3HttpResponseHeader*)static_QUType_ptr.get(_o+1))); break;
    case 19: http_dataSendProgress((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 20: http_dataReadProgress((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 21: http_requestStarted((int)static_QUType_int.get(_o+1)); break;
    case 22: http_requestFinished((int)static_QUType_int.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 23: http_done((bool)static_QUType_bool.get(_o+1)); break;
    case 24: socket_hostFound(); break;
    case 25: socket_connected(); break;
    case 26: socket_connectionClosed(); break;
    case 27: socket_delayedCloseFinished(); break;
    case 28: socket_readyRead(); break;
    case 29: socket_bytesWritten((int)static_QUType_int.get(_o+1)); break;
    case 30: socket_error((int)static_QUType_int.get(_o+1)); break;
    case 31: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Config::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Config::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Config::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
