/****************************************************************************
** US_DB_RtvDate meta object code from reading C++ file 'us_db_rtv_date.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_rtv_date.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_RtvDate::className() const
{
    return "US_DB_RtvDate";
}

QMetaObject *US_DB_RtvDate::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RtvDate( "US_DB_RtvDate", &US_DB_RtvDate::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RtvDate::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvDate", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RtvDate::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvDate", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RtvDate::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"select_date", 0, 0 };
    static const QUMethod slot_2 = {"checkDate", 0, 0 };
    static const QUMethod slot_3 = {"retrieve", 0, 0 };
    static const QUMethod slot_4 = {"reset", 0, 0 };
    static const QUMethod slot_5 = {"help", 0, 0 };
    static const QUMethod slot_6 = {"quit", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_date_lbl", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"select_data", 1, param_slot_8 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "select_date()", &slot_1, QMetaData::Private },
	{ "checkDate()", &slot_2, QMetaData::Private },
	{ "retrieve()", &slot_3, QMetaData::Private },
	{ "reset()", &slot_4, QMetaData::Private },
	{ "help()", &slot_5, QMetaData::Private },
	{ "quit()", &slot_6, QMetaData::Private },
	{ "update_date_lbl(QString)", &slot_7, QMetaData::Private },
	{ "select_data(int)", &slot_8, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RtvDate", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RtvDate.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RtvDate::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RtvDate" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_DB_RtvDate::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: select_date(); break;
    case 2: checkDate(); break;
    case 3: retrieve(); break;
    case 4: reset(); break;
    case 5: help(); break;
    case 6: quit(); break;
    case 7: update_date_lbl((QString)static_QUType_QString.get(_o+1)); break;
    case 8: select_data((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RtvDate::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RtvDate::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_DB_RtvDate::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
