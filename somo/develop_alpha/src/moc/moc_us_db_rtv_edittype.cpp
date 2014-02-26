/****************************************************************************
** US_DB_RtvEditType meta object code from reading C++ file 'us_db_rtv_edittype.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_rtv_edittype.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_RtvEditType::className() const
{
    return "US_DB_RtvEditType";
}

QMetaObject *US_DB_RtvEditType::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RtvEditType( "US_DB_RtvEditType", &US_DB_RtvEditType::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RtvEditType::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvEditType", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RtvEditType::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvEditType", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RtvEditType::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"checkEditType", 0, 0 };
    static const QUMethod slot_2 = {"reset", 0, 0 };
    static const QUMethod slot_3 = {"retrieve", 0, 0 };
    static const QUMethod slot_4 = {"help", 0, 0 };
    static const QUMethod slot_5 = {"quit", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"select_experiment", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"select_optical", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"select_data", 1, param_slot_8 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "checkEditType()", &slot_1, QMetaData::Private },
	{ "reset()", &slot_2, QMetaData::Private },
	{ "retrieve()", &slot_3, QMetaData::Private },
	{ "help()", &slot_4, QMetaData::Private },
	{ "quit()", &slot_5, QMetaData::Private },
	{ "select_experiment(int)", &slot_6, QMetaData::Private },
	{ "select_optical(int)", &slot_7, QMetaData::Private },
	{ "select_data(int)", &slot_8, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RtvEditType", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RtvEditType.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RtvEditType::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RtvEditType" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_DB_RtvEditType::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: checkEditType(); break;
    case 2: reset(); break;
    case 3: retrieve(); break;
    case 4: help(); break;
    case 5: quit(); break;
    case 6: select_experiment((int)static_QUType_int.get(_o+1)); break;
    case 7: select_optical((int)static_QUType_int.get(_o+1)); break;
    case 8: select_data((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RtvEditType::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RtvEditType::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_DB_RtvEditType::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
