/****************************************************************************
** US_DB_Equil meta object code from reading C++ file 'us_db_equil.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_equil.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_Equil::className() const
{
    return "US_DB_Equil";
}

QMetaObject *US_DB_Equil::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_Equil( "US_DB_Equil", &US_DB_Equil::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_Equil::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_Equil", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_Equil::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_Equil", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_Equil::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB_Widgets::staticMetaObject();
    static const QUMethod slot_0 = {"load_HD", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"quit", 0, 0 };
    static const QUMethod slot_3 = {"query_db", 0, 0 };
    static const QUMethod slot_4 = {"save_db", 0, 0 };
    static const QUMethod slot_5 = {"retrieve_db", 0, 0 };
    static const QUMethod slot_6 = {"check_permission", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"select_result", 1, param_slot_7 };
    static const QUMethod slot_8 = {"reset", 0, 0 };
    static const QUMethod slot_9 = {"display", 0, 0 };
    static const QUMethod slot_10 = {"sel_runrequest", 0, 0 };
    static const QUMethod slot_11 = {"sel_investigator", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_runrequest_lbl", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_investigator_lbl", 2, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"delete_db", 1, param_slot_14 };
    static const QMetaData slot_tbl[] = {
	{ "load_HD()", &slot_0, QMetaData::Private },
	{ "help()", &slot_1, QMetaData::Private },
	{ "quit()", &slot_2, QMetaData::Private },
	{ "query_db()", &slot_3, QMetaData::Private },
	{ "save_db()", &slot_4, QMetaData::Private },
	{ "retrieve_db()", &slot_5, QMetaData::Private },
	{ "check_permission()", &slot_6, QMetaData::Private },
	{ "select_result(int)", &slot_7, QMetaData::Private },
	{ "reset()", &slot_8, QMetaData::Private },
	{ "display()", &slot_9, QMetaData::Private },
	{ "sel_runrequest()", &slot_10, QMetaData::Private },
	{ "sel_investigator()", &slot_11, QMetaData::Private },
	{ "update_runrequest_lbl(int)", &slot_12, QMetaData::Private },
	{ "update_investigator_lbl(QString,int)", &slot_13, QMetaData::Private },
	{ "delete_db(bool)", &slot_14, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_Equil", parentObject,
	slot_tbl, 15,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_Equil.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_Equil::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_Equil" ) )
	return this;
    return US_DB_Widgets::qt_cast( clname );
}

bool US_DB_Equil::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load_HD(); break;
    case 1: help(); break;
    case 2: quit(); break;
    case 3: query_db(); break;
    case 4: save_db(); break;
    case 5: retrieve_db(); break;
    case 6: check_permission(); break;
    case 7: select_result((int)static_QUType_int.get(_o+1)); break;
    case 8: reset(); break;
    case 9: display(); break;
    case 10: sel_runrequest(); break;
    case 11: sel_investigator(); break;
    case 12: update_runrequest_lbl((int)static_QUType_int.get(_o+1)); break;
    case 13: update_investigator_lbl((QString)static_QUType_QString.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 14: delete_db((bool)static_QUType_bool.get(_o+1)); break;
    default:
	return US_DB_Widgets::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_Equil::qt_emit( int _id, QUObject* _o )
{
    return US_DB_Widgets::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_Equil::qt_property( int id, int f, QVariant* v)
{
    return US_DB_Widgets::qt_property( id, f, v);
}

bool US_DB_Equil::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
