/****************************************************************************
** US_Sa2d_Control meta object code from reading C++ file 'us_sa2d_control.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_sa2d_control.h"
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

const char *US_Sa2d_Control::className() const
{
    return "US_Sa2d_Control";
}

QMetaObject *US_Sa2d_Control::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Sa2d_Control( "US_Sa2d_Control", &US_Sa2d_Control::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Sa2d_Control::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Sa2d_Control", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Sa2d_Control::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Sa2d_Control", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Sa2d_Control::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"calc_memory", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_max_s", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_min_s", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_max_ff0", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_min_ff0", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_resolution_s", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_resolution_ff0", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_uniform_steps", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_clip_steps", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_local_uniform_steps", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_random_steps", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_scaling_factor", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_scaling_factor2", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_random_distance", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_regfactor", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_metric", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_thread_count", 1, param_slot_18 };
    static const QUMethod slot_19 = {"set_posBaseline", 0, 0 };
    static const QUMethod slot_20 = {"set_negBaseline", 0, 0 };
    static const QUMethod slot_21 = {"set_mw1", 0, 0 };
    static const QUMethod slot_22 = {"set_mw2", 0, 0 };
    static const QUMethod slot_23 = {"set_s1", 0, 0 };
    static const QUMethod slot_24 = {"set_s2", 0, 0 };
    static const QUMethod slot_25 = {"set_D1", 0, 0 };
    static const QUMethod slot_26 = {"set_D2", 0, 0 };
    static const QUMethod slot_27 = {"set_f1", 0, 0 };
    static const QUMethod slot_28 = {"set_f2", 0, 0 };
    static const QUMethod slot_29 = {"set_ff01", 0, 0 };
    static const QUMethod slot_30 = {"set_ff02", 0, 0 };
    static const QUMethod slot_31 = {"set_random", 0, 0 };
    static const QUMethod slot_32 = {"set_uniform", 0, 0 };
    static const QUMethod slot_33 = {"set_local_uniform", 0, 0 };
    static const QUMethod slot_34 = {"set_regularize", 0, 0 };
    static const QUMethod slot_35 = {"set_coalesce", 0, 0 };
    static const QUMethod slot_36 = {"set_clip", 0, 0 };
    static const QUMethod slot_37 = {"set_autoupdate", 0, 0 };
    static const QUMethod slot_38 = {"set_fit_ti", 0, 0 };
    static const QUMethod slot_39 = {"set_fit_ri", 0, 0 };
    static const QUMethod slot_40 = {"plot", 0, 0 };
    static const QUMethod slot_41 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_42 = {"closeEvent", 1, param_slot_42 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Public },
	{ "help()", &slot_1, QMetaData::Public },
	{ "calc_memory()", &slot_2, QMetaData::Public },
	{ "update_max_s(double)", &slot_3, QMetaData::Private },
	{ "update_min_s(double)", &slot_4, QMetaData::Private },
	{ "update_max_ff0(double)", &slot_5, QMetaData::Private },
	{ "update_min_ff0(double)", &slot_6, QMetaData::Private },
	{ "update_resolution_s(double)", &slot_7, QMetaData::Private },
	{ "update_resolution_ff0(double)", &slot_8, QMetaData::Private },
	{ "update_uniform_steps(double)", &slot_9, QMetaData::Private },
	{ "update_clip_steps(double)", &slot_10, QMetaData::Private },
	{ "update_local_uniform_steps(double)", &slot_11, QMetaData::Private },
	{ "update_random_steps(double)", &slot_12, QMetaData::Private },
	{ "update_scaling_factor(double)", &slot_13, QMetaData::Private },
	{ "update_scaling_factor2(double)", &slot_14, QMetaData::Private },
	{ "update_random_distance(double)", &slot_15, QMetaData::Private },
	{ "update_regfactor(double)", &slot_16, QMetaData::Private },
	{ "update_metric(double)", &slot_17, QMetaData::Private },
	{ "update_thread_count(double)", &slot_18, QMetaData::Private },
	{ "set_posBaseline()", &slot_19, QMetaData::Private },
	{ "set_negBaseline()", &slot_20, QMetaData::Private },
	{ "set_mw1()", &slot_21, QMetaData::Private },
	{ "set_mw2()", &slot_22, QMetaData::Private },
	{ "set_s1()", &slot_23, QMetaData::Private },
	{ "set_s2()", &slot_24, QMetaData::Private },
	{ "set_D1()", &slot_25, QMetaData::Private },
	{ "set_D2()", &slot_26, QMetaData::Private },
	{ "set_f1()", &slot_27, QMetaData::Private },
	{ "set_f2()", &slot_28, QMetaData::Private },
	{ "set_ff01()", &slot_29, QMetaData::Private },
	{ "set_ff02()", &slot_30, QMetaData::Private },
	{ "set_random()", &slot_31, QMetaData::Private },
	{ "set_uniform()", &slot_32, QMetaData::Private },
	{ "set_local_uniform()", &slot_33, QMetaData::Private },
	{ "set_regularize()", &slot_34, QMetaData::Private },
	{ "set_coalesce()", &slot_35, QMetaData::Private },
	{ "set_clip()", &slot_36, QMetaData::Private },
	{ "set_autoupdate()", &slot_37, QMetaData::Private },
	{ "set_fit_ti()", &slot_38, QMetaData::Private },
	{ "set_fit_ri()", &slot_39, QMetaData::Private },
	{ "plot()", &slot_40, QMetaData::Public },
	{ "setup_GUI()", &slot_41, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_42, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"mem_changed", 0, 0 };
    static const QUMethod signal_1 = {"distribution_changed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "mem_changed()", &signal_0, QMetaData::Protected },
	{ "distribution_changed()", &signal_1, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Sa2d_Control", parentObject,
	slot_tbl, 43,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Sa2d_Control.setMetaObject( metaObj );
    return metaObj;
}

void* US_Sa2d_Control::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Sa2d_Control" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL mem_changed
void US_Sa2d_Control::mem_changed()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL distribution_changed
void US_Sa2d_Control::distribution_changed()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

bool US_Sa2d_Control::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: help(); break;
    case 2: calc_memory(); break;
    case 3: update_max_s((double)static_QUType_double.get(_o+1)); break;
    case 4: update_min_s((double)static_QUType_double.get(_o+1)); break;
    case 5: update_max_ff0((double)static_QUType_double.get(_o+1)); break;
    case 6: update_min_ff0((double)static_QUType_double.get(_o+1)); break;
    case 7: update_resolution_s((double)static_QUType_double.get(_o+1)); break;
    case 8: update_resolution_ff0((double)static_QUType_double.get(_o+1)); break;
    case 9: update_uniform_steps((double)static_QUType_double.get(_o+1)); break;
    case 10: update_clip_steps((double)static_QUType_double.get(_o+1)); break;
    case 11: update_local_uniform_steps((double)static_QUType_double.get(_o+1)); break;
    case 12: update_random_steps((double)static_QUType_double.get(_o+1)); break;
    case 13: update_scaling_factor((double)static_QUType_double.get(_o+1)); break;
    case 14: update_scaling_factor2((double)static_QUType_double.get(_o+1)); break;
    case 15: update_random_distance((double)static_QUType_double.get(_o+1)); break;
    case 16: update_regfactor((double)static_QUType_double.get(_o+1)); break;
    case 17: update_metric((double)static_QUType_double.get(_o+1)); break;
    case 18: update_thread_count((double)static_QUType_double.get(_o+1)); break;
    case 19: set_posBaseline(); break;
    case 20: set_negBaseline(); break;
    case 21: set_mw1(); break;
    case 22: set_mw2(); break;
    case 23: set_s1(); break;
    case 24: set_s2(); break;
    case 25: set_D1(); break;
    case 26: set_D2(); break;
    case 27: set_f1(); break;
    case 28: set_f2(); break;
    case 29: set_ff01(); break;
    case 30: set_ff02(); break;
    case 31: set_random(); break;
    case 32: set_uniform(); break;
    case 33: set_local_uniform(); break;
    case 34: set_regularize(); break;
    case 35: set_coalesce(); break;
    case 36: set_clip(); break;
    case 37: set_autoupdate(); break;
    case 38: set_fit_ti(); break;
    case 39: set_fit_ri(); break;
    case 40: plot(); break;
    case 41: setup_GUI(); break;
    case 42: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Sa2d_Control::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: mem_changed(); break;
    case 1: distribution_changed(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Sa2d_Control::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Sa2d_Control::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
