/****************************************************************************
** US_Cmdline_App meta object code from reading C++ file 'us_cmdline_app.h'
**
** Created: Thu Jan 30 05:25:07 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "us_cmdline_app.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Cmdline_App::className() const
{
    return "US_Cmdline_App";
}

QMetaObject *US_Cmdline_App::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Cmdline_App( "US_Cmdline_App", &US_Cmdline_App::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Cmdline_App::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Cmdline_App", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Cmdline_App::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Cmdline_App", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Cmdline_App::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUMethod slot_0 = {"readFromStdout", 0, 0 };
    static const QUMethod slot_1 = {"readFromStderr", 0, 0 };
    static const QUMethod slot_2 = {"processExited", 0, 0 };
    static const QUMethod slot_3 = {"launchFinished", 0, 0 };
    static const QUMethod slot_4 = {"timeout", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "readFromStdout()", &slot_0, QMetaData::Private },
	{ "readFromStderr()", &slot_1, QMetaData::Private },
	{ "processExited()", &slot_2, QMetaData::Private },
	{ "launchFinished()", &slot_3, QMetaData::Private },
	{ "timeout()", &slot_4, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Cmdline_App", parentObject,
	slot_tbl, 5,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Cmdline_App.setMetaObject( metaObj );
    return metaObj;
}

void* US_Cmdline_App::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Cmdline_App" ) )
	return this;
    return QObject::qt_cast( clname );
}

bool US_Cmdline_App::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: readFromStdout(); break;
    case 1: readFromStderr(); break;
    case 2: processExited(); break;
    case 3: launchFinished(); break;
    case 4: timeout(); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Cmdline_App::qt_emit( int _id, QUObject* _o )
{
    return QObject::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Cmdline_App::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool US_Cmdline_App::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
