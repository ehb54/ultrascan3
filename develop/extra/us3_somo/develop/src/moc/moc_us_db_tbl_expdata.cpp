/****************************************************************************
** US_ExpData_DB meta object code from reading C++ file 'us_db_tbl_expdata.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_tbl_expdata.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_ExpData_DB::className() const
{
    return "US_ExpData_DB";
}

QMetaObject *US_ExpData_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ExpData_DB( "US_ExpData_DB", &US_ExpData_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ExpData_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ExpData_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ExpData_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ExpData_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ExpData_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB_Widgets::staticMetaObject();
    static const QUMethod slot_0 = {"update_variables", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"sel_cell", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_description", 1, param_slot_2 };
    static const QUMethod slot_3 = {"load_run", 0, 0 };
    static const QUMethod slot_4 = {"sel_investigator", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_investigator_lbl", 2, param_slot_5 };
    static const QUMethod slot_6 = {"sel_date", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_date_lbl", 1, param_slot_7 };
    static const QUMethod slot_8 = {"add_db", 0, 0 };
    static const QUMethod slot_9 = {"query_db", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"sel_query", 1, param_slot_10 };
    static const QUMethod slot_11 = {"check_permission", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"delete_db", 1, param_slot_12 };
    static const QUMethod slot_13 = {"reset", 0, 0 };
    static const QUMethod slot_14 = {"help", 0, 0 };
    static const QUMethod slot_15 = {"quit", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_type", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_rotor", 1, param_slot_17 };
    static const QUMethod slot_18 = {"update_cell", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_flo_channel", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"retrieve_all", 3, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_21 = {"closeEvent", 1, param_slot_21 };
    static const QMetaData slot_tbl[] = {
	{ "update_variables()", &slot_0, QMetaData::Private },
	{ "sel_cell(int)", &slot_1, QMetaData::Private },
	{ "update_description(const QString&)", &slot_2, QMetaData::Private },
	{ "load_run()", &slot_3, QMetaData::Private },
	{ "sel_investigator()", &slot_4, QMetaData::Private },
	{ "update_investigator_lbl(QString,int)", &slot_5, QMetaData::Private },
	{ "sel_date()", &slot_6, QMetaData::Private },
	{ "update_date_lbl(QString)", &slot_7, QMetaData::Private },
	{ "add_db()", &slot_8, QMetaData::Private },
	{ "query_db()", &slot_9, QMetaData::Private },
	{ "sel_query(int)", &slot_10, QMetaData::Private },
	{ "check_permission()", &slot_11, QMetaData::Private },
	{ "delete_db(bool)", &slot_12, QMetaData::Private },
	{ "reset()", &slot_13, QMetaData::Private },
	{ "help()", &slot_14, QMetaData::Private },
	{ "quit()", &slot_15, QMetaData::Private },
	{ "update_type(int)", &slot_16, QMetaData::Public },
	{ "update_rotor(int)", &slot_17, QMetaData::Public },
	{ "update_cell()", &slot_18, QMetaData::Public },
	{ "update_flo_channel(const QString&)", &slot_19, QMetaData::Public },
	{ "retrieve_all(int,QString)", &slot_20, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_21, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ExpData_DB", parentObject,
	slot_tbl, 22,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ExpData_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_ExpData_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ExpData_DB" ) )
	return this;
    return US_DB_Widgets::qt_cast( clname );
}

bool US_ExpData_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_variables(); break;
    case 1: sel_cell((int)static_QUType_int.get(_o+1)); break;
    case 2: update_description((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: load_run(); break;
    case 4: sel_investigator(); break;
    case 5: update_investigator_lbl((QString)static_QUType_QString.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 6: sel_date(); break;
    case 7: update_date_lbl((QString)static_QUType_QString.get(_o+1)); break;
    case 8: add_db(); break;
    case 9: query_db(); break;
    case 10: sel_query((int)static_QUType_int.get(_o+1)); break;
    case 11: check_permission(); break;
    case 12: delete_db((bool)static_QUType_bool.get(_o+1)); break;
    case 13: reset(); break;
    case 14: help(); break;
    case 15: quit(); break;
    case 16: update_type((int)static_QUType_int.get(_o+1)); break;
    case 17: update_rotor((int)static_QUType_int.get(_o+1)); break;
    case 18: update_cell(); break;
    case 19: update_flo_channel((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: static_QUType_bool.set(_o,retrieve_all((int)static_QUType_int.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 21: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return US_DB_Widgets::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ExpData_DB::qt_emit( int _id, QUObject* _o )
{
    return US_DB_Widgets::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_ExpData_DB::qt_property( int id, int f, QVariant* v)
{
    return US_DB_Widgets::qt_property( id, f, v);
}

bool US_ExpData_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
