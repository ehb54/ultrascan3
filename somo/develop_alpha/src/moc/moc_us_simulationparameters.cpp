/****************************************************************************
** US_SimulationParameters meta object code from reading C++ file 'us_simulationparameters.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_simulationparameters.h"
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

const char *US_SimulationParameters::className() const
{
    return "US_SimulationParameters";
}

QMetaObject *US_SimulationParameters::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_SimulationParameters( "US_SimulationParameters", &US_SimulationParameters::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_SimulationParameters::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SimulationParameters", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_SimulationParameters::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SimulationParameters", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_SimulationParameters::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"closeEvent", 1, param_slot_0 };
    static const QUMethod slot_1 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_duration_hours", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_duration_minutes", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_delay_hours", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_delay_minutes", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_rotorspeed", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_simpoints", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_radial_resolution", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_meniscus", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_acceleration", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_number_of_speeds", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_bottom", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_scans", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_rnoise", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_inoise", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_rinoise", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"select_speed_profile", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"select_lamella", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"select_speed_profile", 1, param_slot_19 };
    static const QUMethod slot_20 = {"load", 0, 0 };
    static const QUMethod slot_21 = {"acceleration_flag", 0, 0 };
    static const QUMethod slot_22 = {"save", 0, 0 };
    static const QUMethod slot_23 = {"revert", 0, 0 };
    static const QUMethod slot_24 = {"check_delay", 0, 0 };
    static const QUMethod slot_25 = {"help", 0, 0 };
    static const QUMethod slot_26 = {"update_combobox", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_mesh", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_moving", 1, param_slot_28 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"select_centerpiece", 1, param_slot_29 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_30 = {"save", 1, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_31 = {"printError", 1, param_slot_31 };
    static const QUMethod slot_32 = {"check_params", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "closeEvent(QCloseEvent*)", &slot_0, QMetaData::Protected },
	{ "setupGUI()", &slot_1, QMetaData::Private },
	{ "update_duration_hours(double)", &slot_2, QMetaData::Private },
	{ "update_duration_minutes(double)", &slot_3, QMetaData::Private },
	{ "update_delay_hours(double)", &slot_4, QMetaData::Private },
	{ "update_delay_minutes(double)", &slot_5, QMetaData::Private },
	{ "update_rotorspeed(double)", &slot_6, QMetaData::Private },
	{ "update_simpoints(double)", &slot_7, QMetaData::Private },
	{ "update_radial_resolution(double)", &slot_8, QMetaData::Private },
	{ "update_meniscus(double)", &slot_9, QMetaData::Private },
	{ "update_acceleration(double)", &slot_10, QMetaData::Private },
	{ "update_number_of_speeds(double)", &slot_11, QMetaData::Private },
	{ "update_bottom(double)", &slot_12, QMetaData::Private },
	{ "update_scans(double)", &slot_13, QMetaData::Private },
	{ "update_rnoise(double)", &slot_14, QMetaData::Private },
	{ "update_inoise(double)", &slot_15, QMetaData::Private },
	{ "update_rinoise(double)", &slot_16, QMetaData::Private },
	{ "select_speed_profile(double)", &slot_17, QMetaData::Private },
	{ "select_lamella(double)", &slot_18, QMetaData::Private },
	{ "select_speed_profile(int)", &slot_19, QMetaData::Private },
	{ "load()", &slot_20, QMetaData::Private },
	{ "acceleration_flag()", &slot_21, QMetaData::Private },
	{ "save()", &slot_22, QMetaData::Private },
	{ "revert()", &slot_23, QMetaData::Private },
	{ "check_delay()", &slot_24, QMetaData::Private },
	{ "help()", &slot_25, QMetaData::Private },
	{ "update_combobox()", &slot_26, QMetaData::Private },
	{ "update_mesh(int)", &slot_27, QMetaData::Private },
	{ "update_moving(int)", &slot_28, QMetaData::Private },
	{ "select_centerpiece(int)", &slot_29, QMetaData::Private },
	{ "save(const QString&)", &slot_30, QMetaData::Private },
	{ "printError(const int&)", &slot_31, QMetaData::Private },
	{ "check_params()", &slot_32, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"simparams_name", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "simparams_name(QString)", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_SimulationParameters", parentObject,
	slot_tbl, 33,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_SimulationParameters.setMetaObject( metaObj );
    return metaObj;
}

void* US_SimulationParameters::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_SimulationParameters" ) )
	return this;
    return QDialog::qt_cast( clname );
}

// SIGNAL simparams_name
void US_SimulationParameters::simparams_name( QString t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool US_SimulationParameters::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 1: setupGUI(); break;
    case 2: update_duration_hours((double)static_QUType_double.get(_o+1)); break;
    case 3: update_duration_minutes((double)static_QUType_double.get(_o+1)); break;
    case 4: update_delay_hours((double)static_QUType_double.get(_o+1)); break;
    case 5: update_delay_minutes((double)static_QUType_double.get(_o+1)); break;
    case 6: update_rotorspeed((double)static_QUType_double.get(_o+1)); break;
    case 7: update_simpoints((double)static_QUType_double.get(_o+1)); break;
    case 8: update_radial_resolution((double)static_QUType_double.get(_o+1)); break;
    case 9: update_meniscus((double)static_QUType_double.get(_o+1)); break;
    case 10: update_acceleration((double)static_QUType_double.get(_o+1)); break;
    case 11: update_number_of_speeds((double)static_QUType_double.get(_o+1)); break;
    case 12: update_bottom((double)static_QUType_double.get(_o+1)); break;
    case 13: update_scans((double)static_QUType_double.get(_o+1)); break;
    case 14: update_rnoise((double)static_QUType_double.get(_o+1)); break;
    case 15: update_inoise((double)static_QUType_double.get(_o+1)); break;
    case 16: update_rinoise((double)static_QUType_double.get(_o+1)); break;
    case 17: select_speed_profile((double)static_QUType_double.get(_o+1)); break;
    case 18: select_lamella((double)static_QUType_double.get(_o+1)); break;
    case 19: select_speed_profile((int)static_QUType_int.get(_o+1)); break;
    case 20: load(); break;
    case 21: acceleration_flag(); break;
    case 22: save(); break;
    case 23: revert(); break;
    case 24: check_delay(); break;
    case 25: help(); break;
    case 26: update_combobox(); break;
    case 27: update_mesh((int)static_QUType_int.get(_o+1)); break;
    case 28: update_moving((int)static_QUType_int.get(_o+1)); break;
    case 29: select_centerpiece((int)static_QUType_int.get(_o+1)); break;
    case 30: save((const QString&)static_QUType_QString.get(_o+1)); break;
    case 31: printError((const int&)static_QUType_int.get(_o+1)); break;
    case 32: check_params(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_SimulationParameters::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: simparams_name((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return QDialog::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_SimulationParameters::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_SimulationParameters::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
