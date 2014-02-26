/****************************************************************************
** US_Admin meta object code from reading C++ file 'us_admin.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_admin.h"
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

const char *US_Admin::className() const
{
    return "US_Admin";
}

QMetaObject *US_Admin::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Admin( "US_Admin", &US_Admin::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Admin::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Admin", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Admin::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Admin", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Admin::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_psswd1", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_psswd2", 1, param_slot_1 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QUMethod slot_3 = {"save", 0, 0 };
    static const QUMethod slot_4 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_psswd1(const QString&)", &slot_0, QMetaData::Public },
	{ "update_psswd2(const QString&)", &slot_1, QMetaData::Public },
	{ "help()", &slot_2, QMetaData::Public },
	{ "save()", &slot_3, QMetaData::Public },
	{ "quit()", &slot_4, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Admin", parentObject,
	slot_tbl, 5,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Admin.setMetaObject( metaObj );
    return metaObj;
}

void* US_Admin::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Admin" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Admin::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_psswd1((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_psswd2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: help(); break;
    case 3: save(); break;
    case 4: quit(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Admin::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Admin::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Admin::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
