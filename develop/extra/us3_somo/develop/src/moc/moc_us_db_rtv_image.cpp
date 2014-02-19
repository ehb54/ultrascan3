/****************************************************************************
** US_DB_RtvImage meta object code from reading C++ file 'us_db_rtv_image.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_rtv_image.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_RtvImage::className() const
{
    return "US_DB_RtvImage";
}

QMetaObject *US_DB_RtvImage::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RtvImage( "US_DB_RtvImage", &US_DB_RtvImage::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RtvImage::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvImage", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RtvImage::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvImage", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RtvImage::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB_RtvInvestigator::staticMetaObject();
    static const QUMethod slot_0 = {"show_image", 0, 0 };
    static const QUMethod slot_1 = {"checkImage", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"check_image", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"select_image", 1, param_slot_3 };
    static const QMetaData slot_tbl[] = {
	{ "show_image()", &slot_0, QMetaData::Public },
	{ "checkImage()", &slot_1, QMetaData::Protected },
	{ "check_image(int)", &slot_2, QMetaData::Protected },
	{ "select_image(int)", &slot_3, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RtvImage", parentObject,
	slot_tbl, 4,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RtvImage.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RtvImage::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RtvImage" ) )
	return this;
    return US_DB_RtvInvestigator::qt_cast( clname );
}

bool US_DB_RtvImage::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: show_image(); break;
    case 1: checkImage(); break;
    case 2: check_image((int)static_QUType_int.get(_o+1)); break;
    case 3: select_image((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB_RtvInvestigator::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RtvImage::qt_emit( int _id, QUObject* _o )
{
    return US_DB_RtvInvestigator::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RtvImage::qt_property( int id, int f, QVariant* v)
{
    return US_DB_RtvInvestigator::qt_property( id, f, v);
}

bool US_DB_RtvImage::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
