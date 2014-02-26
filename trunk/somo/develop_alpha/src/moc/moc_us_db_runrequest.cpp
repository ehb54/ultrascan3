/****************************************************************************
** US_DB_RunRequest meta object code from reading C++ file 'us_db_runrequest.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_runrequest.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_RunRequest::className() const
{
    return "US_DB_RunRequest";
}

QMetaObject *US_DB_RunRequest::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RunRequest( "US_DB_RunRequest", &US_DB_RunRequest::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RunRequest::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RunRequest", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RunRequest::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RunRequest", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RunRequest::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUMethod slot_0 = {"check", 0, 0 };
    static const QUMethod slot_1 = {"reset", 0, 0 };
    static const QUMethod slot_2 = {"accept", 0, 0 };
    static const QUMethod slot_3 = {"help", 0, 0 };
    static const QUMethod slot_4 = {"quit", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"select_data", 1, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "check()", &slot_0, QMetaData::Private },
	{ "reset()", &slot_1, QMetaData::Private },
	{ "accept()", &slot_2, QMetaData::Private },
	{ "help()", &slot_3, QMetaData::Private },
	{ "quit()", &slot_4, QMetaData::Private },
	{ "select_data(int)", &slot_5, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ "RunRequestID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"issue_RRID", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "issue_RRID(int)", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RunRequest", parentObject,
	slot_tbl, 6,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RunRequest.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RunRequest::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RunRequest" ) )
	return this;
    return US_DB::qt_cast( clname );
}

// SIGNAL issue_RRID
void US_DB_RunRequest::issue_RRID( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool US_DB_RunRequest::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: check(); break;
    case 1: reset(); break;
    case 2: accept(); break;
    case 3: help(); break;
    case 4: quit(); break;
    case 5: select_data((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RunRequest::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: issue_RRID((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RunRequest::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_DB_RunRequest::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
