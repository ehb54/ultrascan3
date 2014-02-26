/****************************************************************************
** US_Hydrodyn_Saxs_1d meta object code from reading C++ file 'us_hydrodyn_saxs_1d.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_1d.h"
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

const char *US_Hydrodyn_Saxs_1d::className() const
{
    return "US_Hydrodyn_Saxs_1d";
}

QMetaObject *US_Hydrodyn_Saxs_1d::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_1d( "US_Hydrodyn_Saxs_1d", &US_Hydrodyn_Saxs_1d::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_1d::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_1d", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_1d::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_1d", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_1d::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_lambda", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_detector_distance", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_detector_width", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_detector_pixels_width", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_sample_rotations", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_axis_rotations", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_rho0", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_deltaR", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_probe_radius", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_threshold", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_target_ev", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_spec_multiplier", 1, param_slot_12 };
    static const QUMethod slot_13 = {"set_random_rotations", 0, 0 };
    static const QUMethod slot_14 = {"set_planar_method", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_15 = {"find_target_ev_thresh", 1, param_slot_15 };
    static const QUMethod slot_16 = {"set_target_ev", 0, 0 };
    static const QUMethod slot_17 = {"info", 0, 0 };
    static const QUMethod slot_18 = {"start", 0, 0 };
    static const QUMethod slot_19 = {"stop", 0, 0 };
    static const QUMethod slot_20 = {"clear_display", 0, 0 };
    static const QUMethod slot_21 = {"update_font", 0, 0 };
    static const QUMethod slot_22 = {"save", 0, 0 };
    static const QUMethod slot_23 = {"save_data", 0, 0 };
    static const QUMethod slot_24 = {"to_somo", 0, 0 };
    static const QUMethod slot_25 = {"cancel", 0, 0 };
    static const QUMethod slot_26 = {"help", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_27 = {"closeEvent", 1, param_slot_27 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_lambda(const QString&)", &slot_1, QMetaData::Private },
	{ "update_detector_distance(const QString&)", &slot_2, QMetaData::Private },
	{ "update_detector_width(const QString&)", &slot_3, QMetaData::Private },
	{ "update_detector_pixels_width(const QString&)", &slot_4, QMetaData::Private },
	{ "update_sample_rotations(const QString&)", &slot_5, QMetaData::Private },
	{ "update_axis_rotations(const QString&)", &slot_6, QMetaData::Private },
	{ "update_rho0(const QString&)", &slot_7, QMetaData::Private },
	{ "update_deltaR(const QString&)", &slot_8, QMetaData::Private },
	{ "update_probe_radius(const QString&)", &slot_9, QMetaData::Private },
	{ "update_threshold(const QString&)", &slot_10, QMetaData::Private },
	{ "update_target_ev(const QString&)", &slot_11, QMetaData::Private },
	{ "update_spec_multiplier(const QString&)", &slot_12, QMetaData::Private },
	{ "set_random_rotations()", &slot_13, QMetaData::Private },
	{ "set_planar_method()", &slot_14, QMetaData::Private },
	{ "find_target_ev_thresh()", &slot_15, QMetaData::Private },
	{ "set_target_ev()", &slot_16, QMetaData::Private },
	{ "info()", &slot_17, QMetaData::Private },
	{ "start()", &slot_18, QMetaData::Private },
	{ "stop()", &slot_19, QMetaData::Private },
	{ "clear_display()", &slot_20, QMetaData::Private },
	{ "update_font()", &slot_21, QMetaData::Private },
	{ "save()", &slot_22, QMetaData::Private },
	{ "save_data()", &slot_23, QMetaData::Private },
	{ "to_somo()", &slot_24, QMetaData::Private },
	{ "cancel()", &slot_25, QMetaData::Private },
	{ "help()", &slot_26, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_27, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_1d", parentObject,
	slot_tbl, 28,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_1d.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_1d::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_1d" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_1d::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_lambda((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_detector_distance((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_detector_width((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_detector_pixels_width((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_sample_rotations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_axis_rotations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_rho0((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_deltaR((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_probe_radius((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_threshold((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_target_ev((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_spec_multiplier((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: set_random_rotations(); break;
    case 14: set_planar_method(); break;
    case 15: static_QUType_bool.set(_o,find_target_ev_thresh()); break;
    case 16: set_target_ev(); break;
    case 17: info(); break;
    case 18: start(); break;
    case 19: stop(); break;
    case 20: clear_display(); break;
    case 21: update_font(); break;
    case 22: save(); break;
    case 23: save_data(); break;
    case 24: to_somo(); break;
    case 25: cancel(); break;
    case 26: help(); break;
    case 27: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_1d::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_1d::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_1d::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
