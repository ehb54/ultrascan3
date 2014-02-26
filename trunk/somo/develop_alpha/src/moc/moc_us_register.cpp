/****************************************************************************
** US_Register meta object code from reading C++ file 'us_register.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_register.h"
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

const char *US_Register::className() const
{
    return "US_Register";
}

QMetaObject *US_Register::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Register( "US_Register", &US_Register::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Register::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Register", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Register::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Register", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Register::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"license_info", 1, param_slot_0 };
    static const QUMethod slot_1 = {"us_license", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_2 = {"read", 1, param_slot_2 };
    static const QMetaData slot_tbl[] = {
	{ "license_info(const QString&)", &slot_0, QMetaData::Private },
	{ "us_license()", &slot_1, QMetaData::Private },
	{ "read()", &slot_2, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Register", parentObject,
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Register.setMetaObject( metaObj );
    return metaObj;
}

void* US_Register::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Register" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Register::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: license_info((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: us_license(); break;
    case 2: static_QUType_bool.set(_o,read()); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Register::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Register::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Register::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
