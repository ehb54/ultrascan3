/****************************************************************************
** US_EquilTime meta object code from reading C++ file 'us_equiltime.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_equiltime.h"
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

const char *US_EquilTime::className() const
{
    return "US_EquilTime";
}

QMetaObject *US_EquilTime::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_EquilTime( "US_EquilTime", &US_EquilTime::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_EquilTime::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EquilTime", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_EquilTime::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EquilTime", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_EquilTime::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"simulate_times", 0, 0 };
    static const QUMethod slot_1 = {"simulate_component", 0, 0 };
    static const QUMethod slot_2 = {"update_component", 0, 0 };
    static const QUMethod slot_3 = {"quit", 0, 0 };
    static const QUMethod slot_4 = {"help", 0, 0 };
    static const QUMethod slot_5 = {"GUI", 0, 0 };
    static const QUMethod slot_6 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_7 = {"calc_function", 0, 0 };
    static const QUMethod slot_8 = {"init_finite_element", 0, 0 };
    static const QUMethod slot_9 = {"select_inner", 0, 0 };
    static const QUMethod slot_10 = {"select_center", 0, 0 };
    static const QUMethod slot_11 = {"select_outer", 0, 0 };
    static const QUMethod slot_12 = {"select_custom", 0, 0 };
    static const QUMethod slot_13 = {"select_sphere", 0, 0 };
    static const QUMethod slot_14 = {"select_prolate", 0, 0 };
    static const QUMethod slot_15 = {"select_oblate", 0, 0 };
    static const QUMethod slot_16 = {"select_rod", 0, 0 };
    static const QUMethod slot_17 = {"select_rpm", 0, 0 };
    static const QUMethod slot_18 = {"select_monitor", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_topradius", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_bottomradius", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_speedstart", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_speedstop", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_speedsteps", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_tolerance", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_timesteps", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_delta_r", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_delta_t", 1, param_slot_27 };
    static const QUMethod slot_28 = {"calc_speeds", 0, 0 };
    static const QUMethod slot_29 = {"save", 0, 0 };
    static const QUParameter param_slot_30[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_30 = {"closeEvent", 1, param_slot_30 };
    static const QMetaData slot_tbl[] = {
	{ "simulate_times()", &slot_0, QMetaData::Private },
	{ "simulate_component()", &slot_1, QMetaData::Private },
	{ "update_component()", &slot_2, QMetaData::Private },
	{ "quit()", &slot_3, QMetaData::Private },
	{ "help()", &slot_4, QMetaData::Private },
	{ "GUI()", &slot_5, QMetaData::Private },
	{ "setup_GUI()", &slot_6, QMetaData::Private },
	{ "calc_function()", &slot_7, QMetaData::Private },
	{ "init_finite_element()", &slot_8, QMetaData::Private },
	{ "select_inner()", &slot_9, QMetaData::Private },
	{ "select_center()", &slot_10, QMetaData::Private },
	{ "select_outer()", &slot_11, QMetaData::Private },
	{ "select_custom()", &slot_12, QMetaData::Private },
	{ "select_sphere()", &slot_13, QMetaData::Private },
	{ "select_prolate()", &slot_14, QMetaData::Private },
	{ "select_oblate()", &slot_15, QMetaData::Private },
	{ "select_rod()", &slot_16, QMetaData::Private },
	{ "select_rpm()", &slot_17, QMetaData::Private },
	{ "select_monitor()", &slot_18, QMetaData::Private },
	{ "update_topradius(double)", &slot_19, QMetaData::Private },
	{ "update_bottomradius(double)", &slot_20, QMetaData::Private },
	{ "update_speedstart(double)", &slot_21, QMetaData::Private },
	{ "update_speedstop(double)", &slot_22, QMetaData::Private },
	{ "update_speedsteps(double)", &slot_23, QMetaData::Private },
	{ "update_tolerance(double)", &slot_24, QMetaData::Private },
	{ "update_timesteps(double)", &slot_25, QMetaData::Private },
	{ "update_delta_r(double)", &slot_26, QMetaData::Private },
	{ "update_delta_t(double)", &slot_27, QMetaData::Private },
	{ "calc_speeds()", &slot_28, QMetaData::Private },
	{ "save()", &slot_29, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_30, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_EquilTime", parentObject,
	slot_tbl, 31,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_EquilTime.setMetaObject( metaObj );
    return metaObj;
}

void* US_EquilTime::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_EquilTime" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_EquilTime::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: simulate_times(); break;
    case 1: simulate_component(); break;
    case 2: update_component(); break;
    case 3: quit(); break;
    case 4: help(); break;
    case 5: GUI(); break;
    case 6: setup_GUI(); break;
    case 7: calc_function(); break;
    case 8: init_finite_element(); break;
    case 9: select_inner(); break;
    case 10: select_center(); break;
    case 11: select_outer(); break;
    case 12: select_custom(); break;
    case 13: select_sphere(); break;
    case 14: select_prolate(); break;
    case 15: select_oblate(); break;
    case 16: select_rod(); break;
    case 17: select_rpm(); break;
    case 18: select_monitor(); break;
    case 19: update_topradius((double)static_QUType_double.get(_o+1)); break;
    case 20: update_bottomradius((double)static_QUType_double.get(_o+1)); break;
    case 21: update_speedstart((double)static_QUType_double.get(_o+1)); break;
    case 22: update_speedstop((double)static_QUType_double.get(_o+1)); break;
    case 23: update_speedsteps((double)static_QUType_double.get(_o+1)); break;
    case 24: update_tolerance((double)static_QUType_double.get(_o+1)); break;
    case 25: update_timesteps((double)static_QUType_double.get(_o+1)); break;
    case 26: update_delta_r((double)static_QUType_double.get(_o+1)); break;
    case 27: update_delta_t((double)static_QUType_double.get(_o+1)); break;
    case 28: calc_speeds(); break;
    case 29: save(); break;
    case 30: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_EquilTime::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_EquilTime::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_EquilTime::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
