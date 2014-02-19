/****************************************************************************
** US_DB_Sample meta object code from reading C++ file 'us_db_rtv_sample.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_rtv_sample.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_Sample::className() const
{
    return "US_DB_Sample";
}

QMetaObject *US_DB_Sample::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_Sample( "US_DB_Sample", &US_DB_Sample::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_Sample::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_Sample", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_Sample::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_Sample", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_Sample::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"load", 1, param_slot_0 };
    static const QUMethod slot_1 = {"view_buff", 0, 0 };
    static const QUMethod slot_2 = {"view_pep", 0, 0 };
    static const QUMethod slot_3 = {"view_DNA", 0, 0 };
    static const QUMethod slot_4 = {"view_image", 0, 0 };
    static const QUMethod slot_5 = {"help", 0, 0 };
    static const QUMethod slot_6 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "load(int)", &slot_0, QMetaData::Protected },
	{ "view_buff()", &slot_1, QMetaData::Protected },
	{ "view_pep()", &slot_2, QMetaData::Protected },
	{ "view_DNA()", &slot_3, QMetaData::Protected },
	{ "view_image()", &slot_4, QMetaData::Protected },
	{ "help()", &slot_5, QMetaData::Protected },
	{ "quit()", &slot_6, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_Sample", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_Sample.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_Sample::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_Sample" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_DB_Sample::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load((int)static_QUType_int.get(_o+1)); break;
    case 1: view_buff(); break;
    case 2: view_pep(); break;
    case 3: view_DNA(); break;
    case 4: view_image(); break;
    case 5: help(); break;
    case 6: quit(); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_Sample::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_Sample::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_DB_Sample::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_DB_RtvSample::className() const
{
    return "US_DB_RtvSample";
}

QMetaObject *US_DB_RtvSample::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RtvSample( "US_DB_RtvSample", &US_DB_RtvSample::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RtvSample::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvSample", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RtvSample::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvSample", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RtvSample::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB_RtvInvestigator::staticMetaObject();
    static const QUMethod slot_0 = {"checkSample", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"check_sample", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"select_sample", 1, param_slot_2 };
    static const QUMethod slot_3 = {"show_info", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "checkSample()", &slot_0, QMetaData::Protected },
	{ "check_sample(int)", &slot_1, QMetaData::Protected },
	{ "select_sample(int)", &slot_2, QMetaData::Protected },
	{ "show_info()", &slot_3, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RtvSample", parentObject,
	slot_tbl, 4,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RtvSample.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RtvSample::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RtvSample" ) )
	return this;
    return US_DB_RtvInvestigator::qt_cast( clname );
}

bool US_DB_RtvSample::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: checkSample(); break;
    case 1: check_sample((int)static_QUType_int.get(_o+1)); break;
    case 2: select_sample((int)static_QUType_int.get(_o+1)); break;
    case 3: show_info(); break;
    default:
	return US_DB_RtvInvestigator::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RtvSample::qt_emit( int _id, QUObject* _o )
{
    return US_DB_RtvInvestigator::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RtvSample::qt_property( int id, int f, QVariant* v)
{
    return US_DB_RtvInvestigator::qt_property( id, f, v);
}

bool US_DB_RtvSample::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
