/****************************************************************************
** US_Database meta object code from reading C++ file 'us_database.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_database.h"
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

const char *US_Database::className() const
{
    return "US_Database";
}

QMetaObject *US_Database::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Database( "US_Database", &US_Database::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Database::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Database", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Database::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Database", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Database::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"select_db", 1, param_slot_1 };
    static const QUMethod slot_2 = {"check_add", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"add", 1, param_slot_3 };
    static const QUMethod slot_4 = {"check_del", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"del", 1, param_slot_5 };
    static const QUMethod slot_6 = {"check_save", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"save", 1, param_slot_7 };
    static const QUMethod slot_8 = {"clear", 0, 0 };
    static const QUMethod slot_9 = {"reset", 0, 0 };
    static const QUMethod slot_10 = {"help", 0, 0 };
    static const QUMethod slot_11 = {"quit", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_description", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_username", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_password", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_dbname", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_host", 1, param_slot_16 };
    static const QUMethod slot_17 = {"select_mysql", 0, 0 };
    static const QUMethod slot_18 = {"select_oracle", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "select_db(int)", &slot_1, QMetaData::Private },
	{ "check_add()", &slot_2, QMetaData::Private },
	{ "add(bool)", &slot_3, QMetaData::Private },
	{ "check_del()", &slot_4, QMetaData::Private },
	{ "del(bool)", &slot_5, QMetaData::Private },
	{ "check_save()", &slot_6, QMetaData::Private },
	{ "save(QString)", &slot_7, QMetaData::Private },
	{ "clear()", &slot_8, QMetaData::Private },
	{ "reset()", &slot_9, QMetaData::Private },
	{ "help()", &slot_10, QMetaData::Private },
	{ "quit()", &slot_11, QMetaData::Private },
	{ "update_description(const QString&)", &slot_12, QMetaData::Private },
	{ "update_username(const QString&)", &slot_13, QMetaData::Private },
	{ "update_password(const QString&)", &slot_14, QMetaData::Private },
	{ "update_dbname(const QString&)", &slot_15, QMetaData::Private },
	{ "update_host(const QString&)", &slot_16, QMetaData::Private },
	{ "select_mysql()", &slot_17, QMetaData::Private },
	{ "select_oracle()", &slot_18, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Database", parentObject,
	slot_tbl, 19,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Database.setMetaObject( metaObj );
    return metaObj;
}

void* US_Database::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Database" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Database::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: select_db((int)static_QUType_int.get(_o+1)); break;
    case 2: check_add(); break;
    case 3: add((bool)static_QUType_bool.get(_o+1)); break;
    case 4: check_del(); break;
    case 5: del((bool)static_QUType_bool.get(_o+1)); break;
    case 6: check_save(); break;
    case 7: save((QString)static_QUType_QString.get(_o+1)); break;
    case 8: clear(); break;
    case 9: reset(); break;
    case 10: help(); break;
    case 11: quit(); break;
    case 12: update_description((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_username((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_password((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: update_dbname((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_host((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: select_mysql(); break;
    case 18: select_oracle(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Database::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Database::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Database::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
