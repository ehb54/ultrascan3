/****************************************************************************
** US_Enter_Vbar_DB meta object code from reading C++ file 'us_db_tbl_vbar.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_tbl_vbar.h"
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

const char *US_Enter_Vbar_DB::className() const
{
    return "US_Enter_Vbar_DB";
}

QMetaObject *US_Enter_Vbar_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Enter_Vbar_DB( "US_Enter_Vbar_DB", &US_Enter_Vbar_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Enter_Vbar_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Enter_Vbar_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Enter_Vbar_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Enter_Vbar_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Enter_Vbar_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_description", 1, param_slot_0 };
    static const QUMethod slot_1 = {"update_sequence", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_vbar", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_e280", 1, param_slot_3 };
    static const QUMethod slot_4 = {"save_HD", 0, 0 };
    static const QUMethod slot_5 = {"save_DB", 0, 0 };
    static const QUMethod slot_6 = {"help", 0, 0 };
    static const QUMethod slot_7 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_description(const QString&)", &slot_0, QMetaData::Protected },
	{ "update_sequence()", &slot_1, QMetaData::Protected },
	{ "update_vbar(const QString&)", &slot_2, QMetaData::Protected },
	{ "update_e280(const QString&)", &slot_3, QMetaData::Protected },
	{ "save_HD()", &slot_4, QMetaData::Protected },
	{ "save_DB()", &slot_5, QMetaData::Protected },
	{ "help()", &slot_6, QMetaData::Protected },
	{ "quit()", &slot_7, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Enter_Vbar_DB", parentObject,
	slot_tbl, 8,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Enter_Vbar_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_Enter_Vbar_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Enter_Vbar_DB" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_Enter_Vbar_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_description((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_sequence(); break;
    case 2: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_e280((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: save_HD(); break;
    case 5: save_DB(); break;
    case 6: help(); break;
    case 7: quit(); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Enter_Vbar_DB::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Enter_Vbar_DB::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_Enter_Vbar_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_Vbar_DB::className() const
{
    return "US_Vbar_DB";
}

QMetaObject *US_Vbar_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Vbar_DB( "US_Vbar_DB", &US_Vbar_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Vbar_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Vbar_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Vbar_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Vbar_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Vbar_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"retrieve_vbar", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"result_output", 1, param_slot_1 };
    static const QUMethod slot_2 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_3 = {"setup_GUI2", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_temp", 1, param_slot_4 };
    static const QUMethod slot_5 = {"update_vbar", 0, 0 };
    static const QUMethod slot_6 = {"read_file", 0, 0 };
    static const QUMethod slot_7 = {"enter_pep", 0, 0 };
    static const QUMethod slot_8 = {"read_db", 0, 0 };
    static const QUMethod slot_9 = {"save_db", 0, 0 };
    static const QUMethod slot_10 = {"check_permission", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"delete_db", 1, param_slot_11 };
    static const QUMethod slot_12 = {"clear", 0, 0 };
    static const QUMethod slot_13 = {"reset", 0, 0 };
    static const QUMethod slot_14 = {"help", 0, 0 };
    static const QUMethod slot_15 = {"download", 0, 0 };
    static const QUMethod slot_16 = {"ok", 0, 0 };
    static const QUMethod slot_17 = {"info", 0, 0 };
    static const QUMethod slot_18 = {"sel_investigator", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_investigator_lbl", 2, param_slot_19 };
    static const QUMethod slot_20 = {"show_sequence", 0, 0 };
    static const QUParameter param_slot_21[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"select_vbar", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_22 = {"closeEvent", 1, param_slot_22 };
    static const QMetaData slot_tbl[] = {
	{ "retrieve_vbar(int)", &slot_0, QMetaData::Public },
	{ "result_output(QString)", &slot_1, QMetaData::Public },
	{ "setup_GUI()", &slot_2, QMetaData::Protected },
	{ "setup_GUI2()", &slot_3, QMetaData::Protected },
	{ "update_temp(const QString&)", &slot_4, QMetaData::Protected },
	{ "update_vbar()", &slot_5, QMetaData::Protected },
	{ "read_file()", &slot_6, QMetaData::Protected },
	{ "enter_pep()", &slot_7, QMetaData::Protected },
	{ "read_db()", &slot_8, QMetaData::Protected },
	{ "save_db()", &slot_9, QMetaData::Protected },
	{ "check_permission()", &slot_10, QMetaData::Protected },
	{ "delete_db(bool)", &slot_11, QMetaData::Protected },
	{ "clear()", &slot_12, QMetaData::Protected },
	{ "reset()", &slot_13, QMetaData::Protected },
	{ "help()", &slot_14, QMetaData::Protected },
	{ "download()", &slot_15, QMetaData::Protected },
	{ "ok()", &slot_16, QMetaData::Protected },
	{ "info()", &slot_17, QMetaData::Protected },
	{ "sel_investigator()", &slot_18, QMetaData::Protected },
	{ "update_investigator_lbl(QString,int)", &slot_19, QMetaData::Protected },
	{ "show_sequence()", &slot_20, QMetaData::Protected },
	{ "select_vbar(int)", &slot_21, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_22, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ "PepID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"idChanged", 1, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ "vbar", &static_QUType_ptr, "float", QUParameter::In },
	{ "vbar20", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_1 = {"valueChanged", 2, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ "e280", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_2 = {"e280Changed", 1, param_signal_2 };
    static const QUParameter param_signal_3[] = {
	{ "mw", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_3 = {"mwChanged", 1, param_signal_3 };
    static const QMetaData signal_tbl[] = {
	{ "idChanged(int)", &signal_0, QMetaData::Protected },
	{ "valueChanged(float,float)", &signal_1, QMetaData::Protected },
	{ "e280Changed(float)", &signal_2, QMetaData::Protected },
	{ "mwChanged(float)", &signal_3, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Vbar_DB", parentObject,
	slot_tbl, 23,
	signal_tbl, 4,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Vbar_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_Vbar_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Vbar_DB" ) )
	return this;
    return US_DB::qt_cast( clname );
}

// SIGNAL idChanged
void US_Vbar_DB::idChanged( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL valueChanged
void US_Vbar_DB::valueChanged( float t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL e280Changed
void US_Vbar_DB::e280Changed( float t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 2 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

// SIGNAL mwChanged
void US_Vbar_DB::mwChanged( float t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 3 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

bool US_Vbar_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: retrieve_vbar((int)static_QUType_int.get(_o+1)); break;
    case 1: result_output((QString)static_QUType_QString.get(_o+1)); break;
    case 2: setup_GUI(); break;
    case 3: setup_GUI2(); break;
    case 4: update_temp((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_vbar(); break;
    case 6: read_file(); break;
    case 7: enter_pep(); break;
    case 8: read_db(); break;
    case 9: save_db(); break;
    case 10: check_permission(); break;
    case 11: delete_db((bool)static_QUType_bool.get(_o+1)); break;
    case 12: clear(); break;
    case 13: reset(); break;
    case 14: help(); break;
    case 15: download(); break;
    case 16: ok(); break;
    case 17: info(); break;
    case 18: sel_investigator(); break;
    case 19: update_investigator_lbl((QString)static_QUType_QString.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 20: show_sequence(); break;
    case 21: select_vbar((int)static_QUType_int.get(_o+1)); break;
    case 22: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Vbar_DB::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: idChanged((int)static_QUType_int.get(_o+1)); break;
    case 1: valueChanged((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 2: e280Changed((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 3: mwChanged((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    default:
	return US_DB::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Vbar_DB::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_Vbar_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
