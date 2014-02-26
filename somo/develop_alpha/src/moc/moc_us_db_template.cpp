/****************************************************************************
** US_DB_Template meta object code from reading C++ file 'us_db_template.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_template.h"
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

const char *US_DB_Template::className() const
{
    return "US_DB_Template";
}

QMetaObject *US_DB_Template::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_Template( "US_DB_Template", &US_DB_Template::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_Template::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_Template", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_Template::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_Template", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_Template::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"readFromStderr", 0, 0 };
    static const QUMethod slot_1 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_2 = {"check_permission", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"initialize", 1, param_slot_3 };
    static const QUMethod slot_4 = {"help", 0, 0 };
    static const QUMethod slot_5 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "readFromStderr()", &slot_0, QMetaData::Public },
	{ "setup_GUI()", &slot_1, QMetaData::Private },
	{ "check_permission()", &slot_2, QMetaData::Private },
	{ "initialize(bool)", &slot_3, QMetaData::Private },
	{ "help()", &slot_4, QMetaData::Private },
	{ "quit()", &slot_5, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_Template", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_Template.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_Template::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_Template" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_DB_Template::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: readFromStderr(); break;
    case 1: setup_GUI(); break;
    case 2: check_permission(); break;
    case 3: initialize((bool)static_QUType_bool.get(_o+1)); break;
    case 4: help(); break;
    case 5: quit(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_Template::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_Template::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_DB_Template::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
