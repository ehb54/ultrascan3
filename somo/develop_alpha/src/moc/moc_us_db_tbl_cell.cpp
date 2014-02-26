/****************************************************************************
** US_Cell_DB meta object code from reading C++ file 'us_db_tbl_cell.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_tbl_cell.h"
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

const char *US_Cell_DB::className() const
{
    return "US_Cell_DB";
}

QMetaObject *US_Cell_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Cell_DB( "US_Cell_DB", &US_Cell_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Cell_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Cell_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Cell_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Cell_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Cell_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"sel_DNA1", 0, 0 };
    static const QUMethod slot_2 = {"sel_DNA2", 0, 0 };
    static const QUMethod slot_3 = {"sel_DNA3", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ "DNAID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_DNA1_lbl", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "DNAID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_DNA2_lbl", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "DNAID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_DNA3_lbl", 1, param_slot_6 };
    static const QUMethod slot_7 = {"sel_buffer", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ "BuffID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_buffer_lbl", 1, param_slot_8 };
    static const QUMethod slot_9 = {"sel_vbar1", 0, 0 };
    static const QUMethod slot_10 = {"sel_vbar2", 0, 0 };
    static const QUMethod slot_11 = {"sel_vbar3", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ "PepID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_vbar1_lbl", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ "PepID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_vbar2_lbl", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ "PepID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_vbar3_lbl", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"sel_channel", 1, param_slot_15 };
    static const QUMethod slot_16 = {"add_db", 0, 0 };
    static const QUMethod slot_17 = {"query_db", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"sel_query", 1, param_slot_18 };
    static const QUMethod slot_19 = {"check_permission", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"delete_db", 1, param_slot_20 };
    static const QUMethod slot_21 = {"reset", 0, 0 };
    static const QUMethod slot_22 = {"help", 0, 0 };
    static const QUMethod slot_23 = {"quit", 0, 0 };
    static const QUParameter param_slot_24[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_24 = {"closeEvent", 1, param_slot_24 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "sel_DNA1()", &slot_1, QMetaData::Private },
	{ "sel_DNA2()", &slot_2, QMetaData::Private },
	{ "sel_DNA3()", &slot_3, QMetaData::Private },
	{ "update_DNA1_lbl(int)", &slot_4, QMetaData::Private },
	{ "update_DNA2_lbl(int)", &slot_5, QMetaData::Private },
	{ "update_DNA3_lbl(int)", &slot_6, QMetaData::Private },
	{ "sel_buffer()", &slot_7, QMetaData::Private },
	{ "update_buffer_lbl(int)", &slot_8, QMetaData::Private },
	{ "sel_vbar1()", &slot_9, QMetaData::Private },
	{ "sel_vbar2()", &slot_10, QMetaData::Private },
	{ "sel_vbar3()", &slot_11, QMetaData::Private },
	{ "update_vbar1_lbl(int)", &slot_12, QMetaData::Private },
	{ "update_vbar2_lbl(int)", &slot_13, QMetaData::Private },
	{ "update_vbar3_lbl(int)", &slot_14, QMetaData::Private },
	{ "sel_channel(int)", &slot_15, QMetaData::Private },
	{ "add_db()", &slot_16, QMetaData::Private },
	{ "query_db()", &slot_17, QMetaData::Private },
	{ "sel_query(int)", &slot_18, QMetaData::Private },
	{ "check_permission()", &slot_19, QMetaData::Private },
	{ "delete_db(bool)", &slot_20, QMetaData::Private },
	{ "reset()", &slot_21, QMetaData::Private },
	{ "help()", &slot_22, QMetaData::Private },
	{ "quit()", &slot_23, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_24, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Cell_DB", parentObject,
	slot_tbl, 25,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Cell_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_Cell_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Cell_DB" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_Cell_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: sel_DNA1(); break;
    case 2: sel_DNA2(); break;
    case 3: sel_DNA3(); break;
    case 4: update_DNA1_lbl((int)static_QUType_int.get(_o+1)); break;
    case 5: update_DNA2_lbl((int)static_QUType_int.get(_o+1)); break;
    case 6: update_DNA3_lbl((int)static_QUType_int.get(_o+1)); break;
    case 7: sel_buffer(); break;
    case 8: update_buffer_lbl((int)static_QUType_int.get(_o+1)); break;
    case 9: sel_vbar1(); break;
    case 10: sel_vbar2(); break;
    case 11: sel_vbar3(); break;
    case 12: update_vbar1_lbl((int)static_QUType_int.get(_o+1)); break;
    case 13: update_vbar2_lbl((int)static_QUType_int.get(_o+1)); break;
    case 14: update_vbar3_lbl((int)static_QUType_int.get(_o+1)); break;
    case 15: sel_channel((int)static_QUType_int.get(_o+1)); break;
    case 16: add_db(); break;
    case 17: query_db(); break;
    case 18: sel_query((int)static_QUType_int.get(_o+1)); break;
    case 19: check_permission(); break;
    case 20: delete_db((bool)static_QUType_bool.get(_o+1)); break;
    case 21: reset(); break;
    case 22: help(); break;
    case 23: quit(); break;
    case 24: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Cell_DB::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Cell_DB::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_Cell_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
