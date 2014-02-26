/****************************************************************************
** US_Buffer_DB meta object code from reading C++ file 'us_db_tbl_buffer.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_tbl_buffer.h"
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

const char *US_Buffer_DB::className() const
{
    return "US_Buffer_DB";
}

QMetaObject *US_Buffer_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Buffer_DB( "US_Buffer_DB", &US_Buffer_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Buffer_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Buffer_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Buffer_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Buffer_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Buffer_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"get_buffer", 2, param_slot_0 };
    static const QUMethod slot_1 = {"buf_init", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_2 = {"read_template_file", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"setInvestigator", 1, param_slot_3 };
    static const QUMethod slot_4 = {"recalc_density", 0, 0 };
    static const QUMethod slot_5 = {"recalc_viscosity", 0, 0 };
    static const QUMethod slot_6 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_7 = {"read_buffer", 0, 0 };
    static const QUMethod slot_8 = {"save_buffer", 0, 0 };
    static const QUMethod slot_9 = {"read_db", 0, 0 };
    static const QUMethod slot_10 = {"save_db", 0, 0 };
    static const QUMethod slot_11 = {"check_permission", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"del_db", 1, param_slot_12 };
    static const QUMethod slot_13 = {"help", 0, 0 };
    static const QUMethod slot_14 = {"reset", 0, 0 };
    static const QUMethod slot_15 = {"accept", 0, 0 };
    static const QUMethod slot_16 = {"update_db", 0, 0 };
    static const QUMethod slot_17 = {"sel_investigator", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_investigator_lbl", 2, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"select_buff", 1, param_slot_19 };
    static const QUMethod slot_20 = {"add_component", 0, 0 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"remove_component", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"list_component", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_concentration", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_description", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_density", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_viscosity", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_refractive_index", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_28 = {"closeEvent", 1, param_slot_28 };
    static const QMetaData slot_tbl[] = {
	{ "get_buffer(int)", &slot_0, QMetaData::Public },
	{ "buf_init()", &slot_1, QMetaData::Protected },
	{ "read_template_file()", &slot_2, QMetaData::Protected },
	{ "setInvestigator(const int)", &slot_3, QMetaData::Protected },
	{ "recalc_density()", &slot_4, QMetaData::Protected },
	{ "recalc_viscosity()", &slot_5, QMetaData::Protected },
	{ "setup_GUI()", &slot_6, QMetaData::Protected },
	{ "read_buffer()", &slot_7, QMetaData::Protected },
	{ "save_buffer()", &slot_8, QMetaData::Protected },
	{ "read_db()", &slot_9, QMetaData::Protected },
	{ "save_db()", &slot_10, QMetaData::Protected },
	{ "check_permission()", &slot_11, QMetaData::Protected },
	{ "del_db(bool)", &slot_12, QMetaData::Protected },
	{ "help()", &slot_13, QMetaData::Protected },
	{ "reset()", &slot_14, QMetaData::Protected },
	{ "accept()", &slot_15, QMetaData::Protected },
	{ "update_db()", &slot_16, QMetaData::Protected },
	{ "sel_investigator()", &slot_17, QMetaData::Protected },
	{ "update_investigator_lbl(QString,int)", &slot_18, QMetaData::Protected },
	{ "select_buff(int)", &slot_19, QMetaData::Protected },
	{ "add_component()", &slot_20, QMetaData::Protected },
	{ "remove_component(int)", &slot_21, QMetaData::Protected },
	{ "list_component(int)", &slot_22, QMetaData::Protected },
	{ "update_concentration(const QString&)", &slot_23, QMetaData::Protected },
	{ "update_description(const QString&)", &slot_24, QMetaData::Protected },
	{ "update_density(const QString&)", &slot_25, QMetaData::Protected },
	{ "update_viscosity(const QString&)", &slot_26, QMetaData::Protected },
	{ "update_refractive_index(const QString&)", &slot_27, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_28, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ "BuffID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"IDChanged", 1, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ "density", &static_QUType_ptr, "float", QUParameter::In },
	{ "viscosity", &static_QUType_ptr, "float", QUParameter::In },
	{ "refractive_index", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_1 = {"valueChanged", 3, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ "density", &static_QUType_ptr, "float", QUParameter::In },
	{ "viscosity", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_2 = {"valueChanged", 2, param_signal_2 };
    static const QMetaData signal_tbl[] = {
	{ "IDChanged(int)", &signal_0, QMetaData::Protected },
	{ "valueChanged(float,float,float)", &signal_1, QMetaData::Protected },
	{ "valueChanged(float,float)", &signal_2, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Buffer_DB", parentObject,
	slot_tbl, 29,
	signal_tbl, 3,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Buffer_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_Buffer_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Buffer_DB" ) )
	return this;
    return US_DB::qt_cast( clname );
}

// SIGNAL IDChanged
void US_Buffer_DB::IDChanged( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL valueChanged
void US_Buffer_DB::valueChanged( float t0, float t1, float t2 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[4];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    static_QUType_ptr.set(o+3,&t2);
    activate_signal( clist, o );
}

// SIGNAL valueChanged
void US_Buffer_DB::valueChanged( float t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 2 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

bool US_Buffer_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: static_QUType_bool.set(_o,get_buffer((int)static_QUType_int.get(_o+1))); break;
    case 1: buf_init(); break;
    case 2: static_QUType_bool.set(_o,read_template_file()); break;
    case 3: setInvestigator((const int)static_QUType_int.get(_o+1)); break;
    case 4: recalc_density(); break;
    case 5: recalc_viscosity(); break;
    case 6: setup_GUI(); break;
    case 7: read_buffer(); break;
    case 8: save_buffer(); break;
    case 9: read_db(); break;
    case 10: save_db(); break;
    case 11: check_permission(); break;
    case 12: del_db((bool)static_QUType_bool.get(_o+1)); break;
    case 13: help(); break;
    case 14: reset(); break;
    case 15: accept(); break;
    case 16: update_db(); break;
    case 17: sel_investigator(); break;
    case 18: update_investigator_lbl((QString)static_QUType_QString.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 19: select_buff((int)static_QUType_int.get(_o+1)); break;
    case 20: add_component(); break;
    case 21: remove_component((int)static_QUType_int.get(_o+1)); break;
    case 22: list_component((int)static_QUType_int.get(_o+1)); break;
    case 23: update_concentration((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: update_description((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: update_density((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: update_viscosity((const QString&)static_QUType_QString.get(_o+1)); break;
    case 27: update_refractive_index((const QString&)static_QUType_QString.get(_o+1)); break;
    case 28: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Buffer_DB::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: IDChanged((int)static_QUType_int.get(_o+1)); break;
    case 1: valueChanged((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3)))); break;
    case 2: valueChanged((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    default:
	return US_DB::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Buffer_DB::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_Buffer_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
