/****************************************************************************
** US_DB_Admin meta object code from reading C++ file 'us_db_admin.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_admin.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_Admin::className() const
{
    return "US_DB_Admin";
}

QMetaObject *US_DB_Admin::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_Admin( "US_DB_Admin", &US_DB_Admin::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_Admin::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_Admin", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_Admin::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_Admin", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_Admin::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_psswd", 1, param_slot_0 };
    static const QUMethod slot_1 = {"submit", 0, 0 };
    static const QUMethod slot_2 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_psswd(const QString&)", &slot_0, QMetaData::Public },
	{ "submit()", &slot_1, QMetaData::Public },
	{ "quit()", &slot_2, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"issue_pass", 1, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"issue_permission", 1, param_signal_1 };
    static const QMetaData signal_tbl[] = {
	{ "issue_pass(QString)", &signal_0, QMetaData::Public },
	{ "issue_permission(bool)", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_Admin", parentObject,
	slot_tbl, 3,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_Admin.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_Admin::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_Admin" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL issue_pass
void US_DB_Admin::issue_pass( QString t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

// SIGNAL issue_permission
void US_DB_Admin::issue_permission( bool t0 )
{
    activate_signal_bool( staticMetaObject()->signalOffset() + 1, t0 );
}

bool US_DB_Admin::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_psswd((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: submit(); break;
    case 2: quit(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_Admin::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: issue_pass((QString)static_QUType_QString.get(_o+1)); break;
    case 1: issue_permission((bool)static_QUType_bool.get(_o+1)); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_DB_Admin::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_DB_Admin::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
