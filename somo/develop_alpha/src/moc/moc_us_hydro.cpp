/****************************************************************************
** US_Hydro1 meta object code from reading C++ file 'us_hydro.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydro.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3Frame>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydro1::className() const
{
    return "US_Hydro1";
}

QMetaObject *US_Hydro1::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydro1( "US_Hydro1", &US_Hydro1::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydro1::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydro1", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydro1::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydro1", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydro1::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_buffer_lbl", 2, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_density_lbl", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_viscosity_lbl", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_vbar_lbl", 2, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_vbar_lbl", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_temperature_lbl", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_mw_lbl", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_mw_lbl", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_ratio_lbl", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_ratio_lbl", 1, param_slot_9 };
    static const QUMethod slot_10 = {"update", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_ptr, "struct hydrosim", QUParameter::In }
    };
    static const QUMethod slot_11 = {"update", 1, param_slot_11 };
    static const QUMethod slot_12 = {"help", 0, 0 };
    static const QUMethod slot_13 = {"quit", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"calc_invtangent", 2, param_slot_14 };
    static const QUMethod slot_15 = {"calc_models", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_buffer_lbl(float,float)", &slot_0, QMetaData::Public },
	{ "update_density_lbl(const QString&)", &slot_1, QMetaData::Public },
	{ "update_viscosity_lbl(const QString&)", &slot_2, QMetaData::Public },
	{ "update_vbar_lbl(float,float)", &slot_3, QMetaData::Public },
	{ "update_vbar_lbl(const QString&)", &slot_4, QMetaData::Public },
	{ "update_temperature_lbl(const QString&)", &slot_5, QMetaData::Public },
	{ "update_mw_lbl(const QString&)", &slot_6, QMetaData::Public },
	{ "update_mw_lbl(float)", &slot_7, QMetaData::Public },
	{ "update_ratio_lbl(const QString&)", &slot_8, QMetaData::Public },
	{ "update_ratio_lbl(double)", &slot_9, QMetaData::Public },
	{ "update()", &slot_10, QMetaData::Public },
	{ "update(struct hydrosim*)", &slot_11, QMetaData::Public },
	{ "help()", &slot_12, QMetaData::Public },
	{ "quit()", &slot_13, QMetaData::Public },
	{ "calc_invtangent(double)", &slot_14, QMetaData::Public },
	{ "calc_models()", &slot_15, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QUMethod signal_1 = {"updated", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Public },
	{ "updated()", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydro1", parentObject,
	slot_tbl, 16,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydro1.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydro1::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydro1" ) )
	return this;
    return QDialog::qt_cast( clname );
}

// SIGNAL clicked
void US_Hydro1::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL updated
void US_Hydro1::updated()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

bool US_Hydro1::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_buffer_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 1: update_density_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_viscosity_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_vbar_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 4: update_vbar_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_temperature_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_mw_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_mw_lbl((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 8: update_ratio_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_ratio_lbl((double)static_QUType_double.get(_o+1)); break;
    case 10: update(); break;
    case 11: update((struct hydrosim*)static_QUType_ptr.get(_o+1)); break;
    case 12: help(); break;
    case 13: quit(); break;
    case 14: static_QUType_double.set(_o,calc_invtangent((double)static_QUType_double.get(_o+1))); break;
    case 15: calc_models(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydro1::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    case 1: updated(); break;
    default:
	return QDialog::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Hydro1::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydro1::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_Hydro2::className() const
{
    return "US_Hydro2";
}

QMetaObject *US_Hydro2::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydro2( "US_Hydro2", &US_Hydro2::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydro2::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydro2", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydro2::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydro2", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydro2::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_buffer_lbl", 2, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_density_lbl", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_viscosity_lbl", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_vbar_lbl", 2, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_vbar_lbl", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_temperature_lbl", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_param1_lbl", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_param2_lbl", 1, param_slot_7 };
    static const QUMethod slot_8 = {"update", 0, 0 };
    static const QUMethod slot_9 = {"do_mw_s", 0, 0 };
    static const QUMethod slot_10 = {"do_mw_d", 0, 0 };
    static const QUMethod slot_11 = {"do_s_d", 0, 0 };
    static const QUMethod slot_12 = {"help", 0, 0 };
    static const QUMethod slot_13 = {"quit", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"calc_invtangent", 2, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_15 = {"check_valid", 3, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_16 = {"root", 3, param_slot_16 };
    static const QUMethod slot_17 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_18 = {"closeEvent", 1, param_slot_18 };
    static const QMetaData slot_tbl[] = {
	{ "update_buffer_lbl(float,float)", &slot_0, QMetaData::Public },
	{ "update_density_lbl(const QString&)", &slot_1, QMetaData::Public },
	{ "update_viscosity_lbl(const QString&)", &slot_2, QMetaData::Public },
	{ "update_vbar_lbl(float,float)", &slot_3, QMetaData::Public },
	{ "update_vbar_lbl(const QString&)", &slot_4, QMetaData::Public },
	{ "update_temperature_lbl(const QString&)", &slot_5, QMetaData::Public },
	{ "update_param1_lbl(const QString&)", &slot_6, QMetaData::Public },
	{ "update_param2_lbl(const QString&)", &slot_7, QMetaData::Public },
	{ "update()", &slot_8, QMetaData::Public },
	{ "do_mw_s()", &slot_9, QMetaData::Public },
	{ "do_mw_d()", &slot_10, QMetaData::Public },
	{ "do_s_d()", &slot_11, QMetaData::Public },
	{ "help()", &slot_12, QMetaData::Public },
	{ "quit()", &slot_13, QMetaData::Public },
	{ "calc_invtangent(double)", &slot_14, QMetaData::Public },
	{ "check_valid(float,float)", &slot_15, QMetaData::Public },
	{ "root(int,float)", &slot_16, QMetaData::Public },
	{ "setup_GUI()", &slot_17, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_18, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydro2", parentObject,
	slot_tbl, 19,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydro2.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydro2::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydro2" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL clicked
void US_Hydro2::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_Hydro2::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_buffer_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 1: update_density_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_viscosity_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_vbar_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 4: update_vbar_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_temperature_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_param1_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_param2_lbl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update(); break;
    case 9: do_mw_s(); break;
    case 10: do_mw_d(); break;
    case 11: do_s_d(); break;
    case 12: help(); break;
    case 13: quit(); break;
    case 14: static_QUType_double.set(_o,calc_invtangent((double)static_QUType_double.get(_o+1))); break;
    case 15: static_QUType_bool.set(_o,check_valid((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))))); break;
    case 16: static_QUType_double.set(_o,root((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2))))); break;
    case 17: setup_GUI(); break;
    case 18: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydro2::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Hydro2::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydro2::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_Hydro3::className() const
{
    return "US_Hydro3";
}

QMetaObject *US_Hydro3::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydro3( "US_Hydro3", &US_Hydro3::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydro3::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydro3", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydro3::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydro3", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydro3::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"calc", 0, 0 };
    static const QUMethod slot_1 = {"update", 0, 0 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QUMethod slot_3 = {"quit", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_4 = {"closeEvent", 1, param_slot_4 };
    static const QMetaData slot_tbl[] = {
	{ "calc()", &slot_0, QMetaData::Public },
	{ "update()", &slot_1, QMetaData::Public },
	{ "help()", &slot_2, QMetaData::Public },
	{ "quit()", &slot_3, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_4, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydro3", parentObject,
	slot_tbl, 5,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydro3.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydro3::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydro3" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydro3::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: calc(); break;
    case 1: update(); break;
    case 2: help(); break;
    case 3: quit(); break;
    case 4: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydro3::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydro3::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydro3::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
