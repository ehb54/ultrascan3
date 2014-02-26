/****************************************************************************
** US_EquilSim meta object code from reading C++ file 'us_equilsim.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_equilsim.h"
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

const char *US_EquilSim::className() const
{
    return "US_EquilSim";
}

QMetaObject *US_EquilSim::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_EquilSim( "US_EquilSim", &US_EquilSim::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_EquilSim::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EquilSim", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_EquilSim::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EquilSim", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_EquilSim::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"select_species", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"select_conc_steps", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"select_speed_start", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"select_speed_stop", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"select_speed_steps", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"select_column", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"select_bottom", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"select_datapoints", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"select_noise", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"select_nonlin_noise", 1, param_slot_9 };
    static const QUMethod slot_10 = {"select_model", 0, 0 };
    static const QUMethod slot_11 = {"change_equil1", 0, 0 };
    static const QUMethod slot_12 = {"change_equil2", 0, 0 };
    static const QUMethod slot_13 = {"change_equil3", 0, 0 };
    static const QUMethod slot_14 = {"change_vbar", 0, 0 };
    static const QUMethod slot_15 = {"change_extinction", 0, 0 };
    static const QUMethod slot_16 = {"change_density", 0, 0 };
    static const QUMethod slot_17 = {"help", 0, 0 };
    static const QUMethod slot_18 = {"plot", 0, 0 };
    static const QUMethod slot_19 = {"histogram", 0, 0 };
    static const QUMethod slot_20 = {"export_data", 0, 0 };
    static const QUMethod slot_21 = {"quit", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_equil1", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_equil2", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_equil3", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_mw", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_concentration", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_vbar", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_baseline", 1, param_slot_28 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"update_extinction", 1, param_slot_29 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_30 = {"update_conc_increment", 1, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_31 = {"update_pathlength", 1, param_slot_31 };
    static const QUParameter param_slot_32[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"update_temperature", 1, param_slot_32 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_33 = {"update_density", 1, param_slot_33 };
    static const QUParameter param_slot_34[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_34 = {"update_max_od", 1, param_slot_34 };
    static const QUParameter param_slot_35[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_35 = {"update_lambda", 1, param_slot_35 };
    static const QUMethod slot_36 = {"select_ultrascan", 0, 0 };
    static const QUMethod slot_37 = {"select_xla", 0, 0 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_38 = {"update_vbar", 2, param_slot_38 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_39 = {"update_e280", 1, param_slot_39 };
    static const QUParameter param_slot_40[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_40 = {"update_density_label", 2, param_slot_40 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_41 = {"calc_density_tb", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_ptr, "float**", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_42 = {"calc_conc", 2, param_slot_42 };
    static const QUParameter param_slot_43[] = {
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "float**", QUParameter::InOut }
    };
    static const QUMethod slot_43 = {"calc_function", 6, param_slot_43 };
    static const QUMethod slot_44 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_45[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_45 = {"closeEvent", 1, param_slot_45 };
    static const QUMethod slot_46 = {"plot_closeEvent", 0, 0 };
    static const QUMethod slot_47 = {"histogram_closeEvent", 0, 0 };
    static const QUParameter param_slot_48[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_48 = {"integral_root", 3, param_slot_48 };
    static const QUParameter param_slot_49[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_49 = {"find_root", 3, param_slot_49 };
    static const QUParameter param_slot_50[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_50 = {"calc_taylor", 3, param_slot_50 };
    static const QMetaData slot_tbl[] = {
	{ "select_species(double)", &slot_0, QMetaData::Private },
	{ "select_conc_steps(double)", &slot_1, QMetaData::Private },
	{ "select_speed_start(double)", &slot_2, QMetaData::Private },
	{ "select_speed_stop(double)", &slot_3, QMetaData::Private },
	{ "select_speed_steps(double)", &slot_4, QMetaData::Private },
	{ "select_column(double)", &slot_5, QMetaData::Private },
	{ "select_bottom(double)", &slot_6, QMetaData::Private },
	{ "select_datapoints(double)", &slot_7, QMetaData::Private },
	{ "select_noise(double)", &slot_8, QMetaData::Private },
	{ "select_nonlin_noise(double)", &slot_9, QMetaData::Private },
	{ "select_model()", &slot_10, QMetaData::Private },
	{ "change_equil1()", &slot_11, QMetaData::Private },
	{ "change_equil2()", &slot_12, QMetaData::Private },
	{ "change_equil3()", &slot_13, QMetaData::Private },
	{ "change_vbar()", &slot_14, QMetaData::Private },
	{ "change_extinction()", &slot_15, QMetaData::Private },
	{ "change_density()", &slot_16, QMetaData::Private },
	{ "help()", &slot_17, QMetaData::Private },
	{ "plot()", &slot_18, QMetaData::Private },
	{ "histogram()", &slot_19, QMetaData::Private },
	{ "export_data()", &slot_20, QMetaData::Private },
	{ "quit()", &slot_21, QMetaData::Private },
	{ "update_equil1(const QString&)", &slot_22, QMetaData::Private },
	{ "update_equil2(const QString&)", &slot_23, QMetaData::Private },
	{ "update_equil3(const QString&)", &slot_24, QMetaData::Private },
	{ "update_mw(const QString&)", &slot_25, QMetaData::Private },
	{ "update_concentration(const QString&)", &slot_26, QMetaData::Private },
	{ "update_vbar(const QString&)", &slot_27, QMetaData::Private },
	{ "update_baseline(const QString&)", &slot_28, QMetaData::Private },
	{ "update_extinction(const QString&)", &slot_29, QMetaData::Private },
	{ "update_conc_increment(const QString&)", &slot_30, QMetaData::Private },
	{ "update_pathlength(const QString&)", &slot_31, QMetaData::Private },
	{ "update_temperature(const QString&)", &slot_32, QMetaData::Private },
	{ "update_density(const QString&)", &slot_33, QMetaData::Private },
	{ "update_max_od(const QString&)", &slot_34, QMetaData::Private },
	{ "update_lambda(const QString&)", &slot_35, QMetaData::Private },
	{ "select_ultrascan()", &slot_36, QMetaData::Private },
	{ "select_xla()", &slot_37, QMetaData::Private },
	{ "update_vbar(float,float)", &slot_38, QMetaData::Private },
	{ "update_e280(float)", &slot_39, QMetaData::Private },
	{ "update_density_label(float,float)", &slot_40, QMetaData::Private },
	{ "calc_density_tb(float)", &slot_41, QMetaData::Private },
	{ "calc_conc(float***,float*)", &slot_42, QMetaData::Private },
	{ "calc_function(double**,double**,unsigned int,unsigned int,unsigned int,float***)", &slot_43, QMetaData::Private },
	{ "setup_GUI()", &slot_44, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_45, QMetaData::Protected },
	{ "plot_closeEvent()", &slot_46, QMetaData::Protected },
	{ "histogram_closeEvent()", &slot_47, QMetaData::Protected },
	{ "integral_root(float,float,float)", &slot_48, QMetaData::Protected },
	{ "find_root(float,float,float)", &slot_49, QMetaData::Protected },
	{ "calc_taylor(double,unsigned int)", &slot_50, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_EquilSim", parentObject,
	slot_tbl, 51,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_EquilSim.setMetaObject( metaObj );
    return metaObj;
}

void* US_EquilSim::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_EquilSim" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_EquilSim::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: select_species((double)static_QUType_double.get(_o+1)); break;
    case 1: select_conc_steps((double)static_QUType_double.get(_o+1)); break;
    case 2: select_speed_start((double)static_QUType_double.get(_o+1)); break;
    case 3: select_speed_stop((double)static_QUType_double.get(_o+1)); break;
    case 4: select_speed_steps((double)static_QUType_double.get(_o+1)); break;
    case 5: select_column((double)static_QUType_double.get(_o+1)); break;
    case 6: select_bottom((double)static_QUType_double.get(_o+1)); break;
    case 7: select_datapoints((double)static_QUType_double.get(_o+1)); break;
    case 8: select_noise((double)static_QUType_double.get(_o+1)); break;
    case 9: select_nonlin_noise((double)static_QUType_double.get(_o+1)); break;
    case 10: select_model(); break;
    case 11: change_equil1(); break;
    case 12: change_equil2(); break;
    case 13: change_equil3(); break;
    case 14: change_vbar(); break;
    case 15: change_extinction(); break;
    case 16: change_density(); break;
    case 17: help(); break;
    case 18: plot(); break;
    case 19: histogram(); break;
    case 20: export_data(); break;
    case 21: quit(); break;
    case 22: update_equil1((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: update_equil2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: update_equil3((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: update_mw((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: update_concentration((const QString&)static_QUType_QString.get(_o+1)); break;
    case 27: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 28: update_baseline((const QString&)static_QUType_QString.get(_o+1)); break;
    case 29: update_extinction((const QString&)static_QUType_QString.get(_o+1)); break;
    case 30: update_conc_increment((const QString&)static_QUType_QString.get(_o+1)); break;
    case 31: update_pathlength((const QString&)static_QUType_QString.get(_o+1)); break;
    case 32: update_temperature((const QString&)static_QUType_QString.get(_o+1)); break;
    case 33: update_density((const QString&)static_QUType_QString.get(_o+1)); break;
    case 34: update_max_od((const QString&)static_QUType_QString.get(_o+1)); break;
    case 35: update_lambda((const QString&)static_QUType_QString.get(_o+1)); break;
    case 36: select_ultrascan(); break;
    case 37: select_xla(); break;
    case 38: update_vbar((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 39: update_e280((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 40: update_density_label((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 41: calc_density_tb((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 42: calc_conc((float***)static_QUType_ptr.get(_o+1),(float*)static_QUType_ptr.get(_o+2)); break;
    case 43: calc_function((double**)static_QUType_ptr.get(_o+1),(double**)static_QUType_ptr.get(_o+2),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+3))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+4))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+5))),(float***)static_QUType_ptr.get(_o+6)); break;
    case 44: setup_GUI(); break;
    case 45: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 46: plot_closeEvent(); break;
    case 47: histogram_closeEvent(); break;
    case 48: integral_root((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3)))); break;
    case 49: find_root((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3)))); break;
    case 50: static_QUType_double.set(_o,calc_taylor((double)static_QUType_double.get(_o+1),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+2))))); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_EquilSim::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_EquilSim::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_EquilSim::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
