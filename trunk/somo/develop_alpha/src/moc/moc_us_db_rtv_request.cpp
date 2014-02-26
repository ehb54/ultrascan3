/****************************************************************************
** US_RequestStatus meta object code from reading C++ file 'us_db_rtv_request.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_rtv_request.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_RequestStatus::className() const
{
    return "US_RequestStatus";
}

QMetaObject *US_RequestStatus::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_RequestStatus( "US_RequestStatus", &US_RequestStatus::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_RequestStatus::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_RequestStatus", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_RequestStatus::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_RequestStatus", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_RequestStatus::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUMethod slot_0 = {"select_cb1", 0, 0 };
    static const QUMethod slot_1 = {"select_cb2", 0, 0 };
    static const QUMethod slot_2 = {"select_cb3", 0, 0 };
    static const QUMethod slot_3 = {"select_cb4", 0, 0 };
    static const QUMethod slot_4 = {"select_cb5", 0, 0 };
    static const QUMethod slot_5 = {"select_cb6", 0, 0 };
    static const QUMethod slot_6 = {"select_cb7", 0, 0 };
    static const QUMethod slot_7 = {"update_status", 0, 0 };
    static const QUMethod slot_8 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "select_cb1()", &slot_0, QMetaData::Protected },
	{ "select_cb2()", &slot_1, QMetaData::Protected },
	{ "select_cb3()", &slot_2, QMetaData::Protected },
	{ "select_cb4()", &slot_3, QMetaData::Protected },
	{ "select_cb5()", &slot_4, QMetaData::Protected },
	{ "select_cb6()", &slot_5, QMetaData::Protected },
	{ "select_cb7()", &slot_6, QMetaData::Protected },
	{ "update_status()", &slot_7, QMetaData::Protected },
	{ "quit()", &slot_8, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ "Status", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"statusChanged", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "statusChanged(QString)", &signal_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_RequestStatus", parentObject,
	slot_tbl, 9,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_RequestStatus.setMetaObject( metaObj );
    return metaObj;
}

void* US_RequestStatus::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_RequestStatus" ) )
	return this;
    return US_DB::qt_cast( clname );
}

// SIGNAL statusChanged
void US_RequestStatus::statusChanged( QString t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool US_RequestStatus::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: select_cb1(); break;
    case 1: select_cb2(); break;
    case 2: select_cb3(); break;
    case 3: select_cb4(); break;
    case 4: select_cb5(); break;
    case 5: select_cb6(); break;
    case 6: select_cb7(); break;
    case 7: update_status(); break;
    case 8: quit(); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_RequestStatus::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: statusChanged((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return US_DB::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_RequestStatus::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_RequestStatus::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_DB_RtvRequest::className() const
{
    return "US_DB_RtvRequest";
}

QMetaObject *US_DB_RtvRequest::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RtvRequest( "US_DB_RtvRequest", &US_DB_RtvRequest::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RtvRequest::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvRequest", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RtvRequest::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvRequest", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RtvRequest::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB_RtvInvestigator::staticMetaObject();
    static const QUMethod slot_0 = {"checkRequest", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"check_request", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"select_request", 1, param_slot_2 };
    static const QUMethod slot_3 = {"check_status", 0, 0 };
    static const QUMethod slot_4 = {"edit_notes", 0, 0 };
    static const QUMethod slot_5 = {"show_info", 0, 0 };
    static const QUMethod slot_6 = {"get_GUI", 0, 0 };
    static const QUMethod slot_7 = {"setup_GUI", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "checkRequest()", &slot_0, QMetaData::Protected },
	{ "check_request(int)", &slot_1, QMetaData::Protected },
	{ "select_request(int)", &slot_2, QMetaData::Protected },
	{ "check_status()", &slot_3, QMetaData::Protected },
	{ "edit_notes()", &slot_4, QMetaData::Protected },
	{ "show_info()", &slot_5, QMetaData::Protected },
	{ "get_GUI()", &slot_6, QMetaData::Protected },
	{ "setup_GUI()", &slot_7, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RtvRequest", parentObject,
	slot_tbl, 8,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RtvRequest.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RtvRequest::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RtvRequest" ) )
	return this;
    return US_DB_RtvInvestigator::qt_cast( clname );
}

bool US_DB_RtvRequest::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: checkRequest(); break;
    case 1: check_request((int)static_QUType_int.get(_o+1)); break;
    case 2: select_request((int)static_QUType_int.get(_o+1)); break;
    case 3: check_status(); break;
    case 4: edit_notes(); break;
    case 5: show_info(); break;
    case 6: get_GUI(); break;
    case 7: setup_GUI(); break;
    default:
	return US_DB_RtvInvestigator::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RtvRequest::qt_emit( int _id, QUObject* _o )
{
    return US_DB_RtvInvestigator::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RtvRequest::qt_property( int id, int f, QVariant* v)
{
    return US_DB_RtvInvestigator::qt_property( id, f, v);
}

bool US_DB_RtvRequest::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
