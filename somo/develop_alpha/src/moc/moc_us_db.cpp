/****************************************************************************
** US_DB meta object code from reading C++ file 'us_db.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB::className() const
{
    return "US_DB";
}

QMetaObject *US_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB( "US_DB", &US_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"newMessage", 2, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "newMessage(QString,int)", &slot_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: newMessage((QString)static_QUType_QString.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
