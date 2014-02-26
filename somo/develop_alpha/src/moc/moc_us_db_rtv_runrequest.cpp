/****************************************************************************
** US_RunRequest meta object code from reading C++ file 'us_db_rtv_runrequest.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_rtv_runrequest.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_RunRequest::className() const
{
    return "US_RunRequest";
}

QMetaObject *US_RunRequest::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_RunRequest( "US_RunRequest", &US_RunRequest::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_RunRequest::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_RunRequest", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_RunRequest::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_RunRequest", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_RunRequest::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"load", 1, param_slot_0 };
    static const QUMethod slot_1 = {"view_sample1", 0, 0 };
    static const QUMethod slot_2 = {"view_sample2", 0, 0 };
    static const QUMethod slot_3 = {"view_sample3", 0, 0 };
    static const QUMethod slot_4 = {"view_sample4", 0, 0 };
    static const QUMethod slot_5 = {"view_sample5", 0, 0 };
    static const QUMethod slot_6 = {"view_sample6", 0, 0 };
    static const QUMethod slot_7 = {"view_sample7", 0, 0 };
    static const QUMethod slot_8 = {"view_sample8", 0, 0 };
    static const QUMethod slot_9 = {"help", 0, 0 };
    static const QUMethod slot_10 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "load(int)", &slot_0, QMetaData::Protected },
	{ "view_sample1()", &slot_1, QMetaData::Protected },
	{ "view_sample2()", &slot_2, QMetaData::Protected },
	{ "view_sample3()", &slot_3, QMetaData::Protected },
	{ "view_sample4()", &slot_4, QMetaData::Protected },
	{ "view_sample5()", &slot_5, QMetaData::Protected },
	{ "view_sample6()", &slot_6, QMetaData::Protected },
	{ "view_sample7()", &slot_7, QMetaData::Protected },
	{ "view_sample8()", &slot_8, QMetaData::Protected },
	{ "help()", &slot_9, QMetaData::Protected },
	{ "quit()", &slot_10, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_RunRequest", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_RunRequest.setMetaObject( metaObj );
    return metaObj;
}

void* US_RunRequest::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_RunRequest" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_RunRequest::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load((int)static_QUType_int.get(_o+1)); break;
    case 1: view_sample1(); break;
    case 2: view_sample2(); break;
    case 3: view_sample3(); break;
    case 4: view_sample4(); break;
    case 5: view_sample5(); break;
    case 6: view_sample6(); break;
    case 7: view_sample7(); break;
    case 8: view_sample8(); break;
    case 9: help(); break;
    case 10: quit(); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_RunRequest::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_RunRequest::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_RunRequest::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_DB_RtvRunRequest::className() const
{
    return "US_DB_RtvRunRequest";
}

QMetaObject *US_DB_RtvRunRequest::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RtvRunRequest( "US_DB_RtvRunRequest", &US_DB_RtvRunRequest::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RtvRunRequest::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvRunRequest", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RtvRunRequest::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvRunRequest", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RtvRunRequest::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB_RtvInvestigator::staticMetaObject();
    static const QUMethod slot_0 = {"show_runrequest", 0, 0 };
    static const QUMethod slot_1 = {"checkRunRequest", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"check_runrequest", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"select_runrequest", 1, param_slot_3 };
    static const QMetaData slot_tbl[] = {
	{ "show_runrequest()", &slot_0, QMetaData::Public },
	{ "checkRunRequest()", &slot_1, QMetaData::Protected },
	{ "check_runrequest(int)", &slot_2, QMetaData::Protected },
	{ "select_runrequest(int)", &slot_3, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RtvRunRequest", parentObject,
	slot_tbl, 4,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RtvRunRequest.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RtvRunRequest::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RtvRunRequest" ) )
	return this;
    return US_DB_RtvInvestigator::qt_cast( clname );
}

bool US_DB_RtvRunRequest::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: show_runrequest(); break;
    case 1: checkRunRequest(); break;
    case 2: check_runrequest((int)static_QUType_int.get(_o+1)); break;
    case 3: select_runrequest((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB_RtvInvestigator::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RtvRunRequest::qt_emit( int _id, QUObject* _o )
{
    return US_DB_RtvInvestigator::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RtvRunRequest::qt_property( int id, int f, QVariant* v)
{
    return US_DB_RtvInvestigator::qt_property( id, f, v);
}

bool US_DB_RtvRunRequest::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
