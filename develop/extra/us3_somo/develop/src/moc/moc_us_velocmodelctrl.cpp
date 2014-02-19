/****************************************************************************
** US_VelocModelControl meta object code from reading C++ file 'us_velocmodelctrl.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_velocmodelctrl.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_VelocModelControl::className() const
{
    return "US_VelocModelControl";
}

QMetaObject *US_VelocModelControl::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_VelocModelControl( "US_VelocModelControl", &US_VelocModelControl::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_VelocModelControl::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_VelocModelControl", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_VelocModelControl::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_VelocModelControl", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_VelocModelControl::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"update_labels", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_meniscus", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_meniscus_range", 1, param_slot_2 };
    static const QUMethod slot_3 = {"update_meniscus_fix", 0, 0 };
    static const QUMethod slot_4 = {"update_meniscus_float", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_baseline", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_baseline_range", 1, param_slot_6 };
    static const QUMethod slot_7 = {"update_baseline_fix", 0, 0 };
    static const QUMethod slot_8 = {"update_baseline_float", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_slope", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_slope_range", 1, param_slot_10 };
    static const QUMethod slot_11 = {"update_slope_fix", 0, 0 };
    static const QUMethod slot_12 = {"update_slope_float", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_stray", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_stray_range", 1, param_slot_14 };
    static const QUMethod slot_15 = {"update_stray_fix", 0, 0 };
    static const QUMethod slot_16 = {"update_stray_float", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"assign_component", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"assign_simpoints", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_sed", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_sed_range", 1, param_slot_20 };
    static const QUMethod slot_21 = {"update_sed_fix", 0, 0 };
    static const QUMethod slot_22 = {"update_sed_float", 0, 0 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_diff", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_diff_range", 1, param_slot_24 };
    static const QUMethod slot_25 = {"update_diff_fix", 0, 0 };
    static const QUMethod slot_26 = {"update_diff_float", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_conc", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_conc_range", 1, param_slot_28 };
    static const QUMethod slot_29 = {"update_conc_fix", 0, 0 };
    static const QUMethod slot_30 = {"update_conc_float", 0, 0 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_31 = {"update_sigma", 1, param_slot_31 };
    static const QUParameter param_slot_32[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"update_sigma_range", 1, param_slot_32 };
    static const QUMethod slot_33 = {"update_sigma_fix", 0, 0 };
    static const QUMethod slot_34 = {"update_sigma_float", 0, 0 };
    static const QUParameter param_slot_35[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_35 = {"update_delta", 1, param_slot_35 };
    static const QUParameter param_slot_36[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_36 = {"update_delta_range", 1, param_slot_36 };
    static const QUMethod slot_37 = {"update_delta_fix", 0, 0 };
    static const QUMethod slot_38 = {"update_delta_float", 0, 0 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_39 = {"update_mw", 1, param_slot_39 };
    static const QUMethod slot_40 = {"update_mw_fix", 0, 0 };
    static const QUMethod slot_41 = {"update_mw_float", 0, 0 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_42 = {"update_model_vbar", 1, param_slot_42 };
    static const QUParameter param_slot_43[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_43 = {"update_vbar_lbl", 2, param_slot_43 };
    static const QUMethod slot_44 = {"read_vbar", 0, 0 };
    static const QUMethod slot_45 = {"update_vbar_fix", 0, 0 };
    static const QUMethod slot_46 = {"update_vbar_float", 0, 0 };
    static const QUMethod slot_47 = {"save_model", 0, 0 };
    static const QUParameter param_slot_48[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_48 = {"save_model", 1, param_slot_48 };
    static const QUMethod slot_49 = {"load_model", 0, 0 };
    static const QUParameter param_slot_50[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_50 = {"load_model", 1, param_slot_50 };
    static const QUParameter param_slot_51[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_51 = {"calc_mw", 1, param_slot_51 };
    static const QUParameter param_slot_52[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_52 = {"calc_vbar", 1, param_slot_52 };
    static const QUParameter param_slot_53[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_53 = {"calc_D", 1, param_slot_53 };
    static const QUMethod slot_54 = {"quit", 0, 0 };
    static const QUMethod slot_55 = {"help", 0, 0 };
    static const QUMethod slot_56 = {"show_model", 0, 0 };
    static const QUMethod slot_57 = {"reset", 0, 0 };
    static const QUMethod slot_58 = {"reset_range", 0, 0 };
    static const QUMethod slot_59 = {"meniscus_red", 0, 0 };
    static const QUMethod slot_60 = {"meniscus_green", 0, 0 };
    static const QUMethod slot_61 = {"meniscus_dk_green", 0, 0 };
    static const QUMethod slot_62 = {"baseline_red", 0, 0 };
    static const QUMethod slot_63 = {"baseline_green", 0, 0 };
    static const QUMethod slot_64 = {"baseline_dk_green", 0, 0 };
    static const QUMethod slot_65 = {"slope_red", 0, 0 };
    static const QUMethod slot_66 = {"slope_green", 0, 0 };
    static const QUMethod slot_67 = {"slope_dk_green", 0, 0 };
    static const QUMethod slot_68 = {"stray_red", 0, 0 };
    static const QUMethod slot_69 = {"stray_green", 0, 0 };
    static const QUMethod slot_70 = {"stray_dk_green", 0, 0 };
    static const QUMethod slot_71 = {"sed_red", 0, 0 };
    static const QUMethod slot_72 = {"sed_green", 0, 0 };
    static const QUMethod slot_73 = {"sed_dk_green", 0, 0 };
    static const QUMethod slot_74 = {"diff_red", 0, 0 };
    static const QUMethod slot_75 = {"diff_green", 0, 0 };
    static const QUMethod slot_76 = {"diff_dk_green", 0, 0 };
    static const QUMethod slot_77 = {"conc_red", 0, 0 };
    static const QUMethod slot_78 = {"conc_green", 0, 0 };
    static const QUMethod slot_79 = {"conc_dk_green", 0, 0 };
    static const QUMethod slot_80 = {"sigma_red", 0, 0 };
    static const QUMethod slot_81 = {"sigma_green", 0, 0 };
    static const QUMethod slot_82 = {"sigma_dk_green", 0, 0 };
    static const QUMethod slot_83 = {"delta_red", 0, 0 };
    static const QUMethod slot_84 = {"delta_green", 0, 0 };
    static const QUMethod slot_85 = {"delta_dk_green", 0, 0 };
    static const QUParameter param_slot_86[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_86 = {"closeEvent", 1, param_slot_86 };
    static const QMetaData slot_tbl[] = {
	{ "update_labels()", &slot_0, QMetaData::Public },
	{ "update_meniscus(const QString&)", &slot_1, QMetaData::Public },
	{ "update_meniscus_range(const QString&)", &slot_2, QMetaData::Public },
	{ "update_meniscus_fix()", &slot_3, QMetaData::Public },
	{ "update_meniscus_float()", &slot_4, QMetaData::Public },
	{ "update_baseline(const QString&)", &slot_5, QMetaData::Public },
	{ "update_baseline_range(const QString&)", &slot_6, QMetaData::Public },
	{ "update_baseline_fix()", &slot_7, QMetaData::Public },
	{ "update_baseline_float()", &slot_8, QMetaData::Public },
	{ "update_slope(const QString&)", &slot_9, QMetaData::Public },
	{ "update_slope_range(const QString&)", &slot_10, QMetaData::Public },
	{ "update_slope_fix()", &slot_11, QMetaData::Public },
	{ "update_slope_float()", &slot_12, QMetaData::Public },
	{ "update_stray(const QString&)", &slot_13, QMetaData::Public },
	{ "update_stray_range(const QString&)", &slot_14, QMetaData::Public },
	{ "update_stray_fix()", &slot_15, QMetaData::Public },
	{ "update_stray_float()", &slot_16, QMetaData::Public },
	{ "assign_component(double)", &slot_17, QMetaData::Public },
	{ "assign_simpoints(double)", &slot_18, QMetaData::Public },
	{ "update_sed(const QString&)", &slot_19, QMetaData::Public },
	{ "update_sed_range(const QString&)", &slot_20, QMetaData::Public },
	{ "update_sed_fix()", &slot_21, QMetaData::Public },
	{ "update_sed_float()", &slot_22, QMetaData::Public },
	{ "update_diff(const QString&)", &slot_23, QMetaData::Public },
	{ "update_diff_range(const QString&)", &slot_24, QMetaData::Public },
	{ "update_diff_fix()", &slot_25, QMetaData::Public },
	{ "update_diff_float()", &slot_26, QMetaData::Public },
	{ "update_conc(const QString&)", &slot_27, QMetaData::Public },
	{ "update_conc_range(const QString&)", &slot_28, QMetaData::Public },
	{ "update_conc_fix()", &slot_29, QMetaData::Public },
	{ "update_conc_float()", &slot_30, QMetaData::Public },
	{ "update_sigma(const QString&)", &slot_31, QMetaData::Public },
	{ "update_sigma_range(const QString&)", &slot_32, QMetaData::Public },
	{ "update_sigma_fix()", &slot_33, QMetaData::Public },
	{ "update_sigma_float()", &slot_34, QMetaData::Public },
	{ "update_delta(const QString&)", &slot_35, QMetaData::Public },
	{ "update_delta_range(const QString&)", &slot_36, QMetaData::Public },
	{ "update_delta_fix()", &slot_37, QMetaData::Public },
	{ "update_delta_float()", &slot_38, QMetaData::Public },
	{ "update_mw(const QString&)", &slot_39, QMetaData::Public },
	{ "update_mw_fix()", &slot_40, QMetaData::Public },
	{ "update_mw_float()", &slot_41, QMetaData::Public },
	{ "update_model_vbar(const QString&)", &slot_42, QMetaData::Public },
	{ "update_vbar_lbl(float,float)", &slot_43, QMetaData::Public },
	{ "read_vbar()", &slot_44, QMetaData::Public },
	{ "update_vbar_fix()", &slot_45, QMetaData::Public },
	{ "update_vbar_float()", &slot_46, QMetaData::Public },
	{ "save_model()", &slot_47, QMetaData::Public },
	{ "save_model(const QString&)", &slot_48, QMetaData::Public },
	{ "load_model()", &slot_49, QMetaData::Public },
	{ "load_model(const QString&)", &slot_50, QMetaData::Public },
	{ "calc_mw(const int)", &slot_51, QMetaData::Public },
	{ "calc_vbar(const int)", &slot_52, QMetaData::Public },
	{ "calc_D(const int)", &slot_53, QMetaData::Public },
	{ "quit()", &slot_54, QMetaData::Public },
	{ "help()", &slot_55, QMetaData::Public },
	{ "show_model()", &slot_56, QMetaData::Public },
	{ "reset()", &slot_57, QMetaData::Public },
	{ "reset_range()", &slot_58, QMetaData::Public },
	{ "meniscus_red()", &slot_59, QMetaData::Public },
	{ "meniscus_green()", &slot_60, QMetaData::Public },
	{ "meniscus_dk_green()", &slot_61, QMetaData::Public },
	{ "baseline_red()", &slot_62, QMetaData::Public },
	{ "baseline_green()", &slot_63, QMetaData::Public },
	{ "baseline_dk_green()", &slot_64, QMetaData::Public },
	{ "slope_red()", &slot_65, QMetaData::Public },
	{ "slope_green()", &slot_66, QMetaData::Public },
	{ "slope_dk_green()", &slot_67, QMetaData::Public },
	{ "stray_red()", &slot_68, QMetaData::Public },
	{ "stray_green()", &slot_69, QMetaData::Public },
	{ "stray_dk_green()", &slot_70, QMetaData::Public },
	{ "sed_red()", &slot_71, QMetaData::Public },
	{ "sed_green()", &slot_72, QMetaData::Public },
	{ "sed_dk_green()", &slot_73, QMetaData::Public },
	{ "diff_red()", &slot_74, QMetaData::Public },
	{ "diff_green()", &slot_75, QMetaData::Public },
	{ "diff_dk_green()", &slot_76, QMetaData::Public },
	{ "conc_red()", &slot_77, QMetaData::Public },
	{ "conc_green()", &slot_78, QMetaData::Public },
	{ "conc_dk_green()", &slot_79, QMetaData::Public },
	{ "sigma_red()", &slot_80, QMetaData::Public },
	{ "sigma_green()", &slot_81, QMetaData::Public },
	{ "sigma_dk_green()", &slot_82, QMetaData::Public },
	{ "delta_red()", &slot_83, QMetaData::Public },
	{ "delta_green()", &slot_84, QMetaData::Public },
	{ "delta_dk_green()", &slot_85, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_86, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_VelocModelControl", parentObject,
	slot_tbl, 87,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_VelocModelControl.setMetaObject( metaObj );
    return metaObj;
}

void* US_VelocModelControl::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_VelocModelControl" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_VelocModelControl::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_labels(); break;
    case 1: update_meniscus((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_meniscus_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_meniscus_fix(); break;
    case 4: update_meniscus_float(); break;
    case 5: update_baseline((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_baseline_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_baseline_fix(); break;
    case 8: update_baseline_float(); break;
    case 9: update_slope((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_slope_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_slope_fix(); break;
    case 12: update_slope_float(); break;
    case 13: update_stray((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_stray_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: update_stray_fix(); break;
    case 16: update_stray_float(); break;
    case 17: assign_component((double)static_QUType_double.get(_o+1)); break;
    case 18: assign_simpoints((double)static_QUType_double.get(_o+1)); break;
    case 19: update_sed((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: update_sed_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: update_sed_fix(); break;
    case 22: update_sed_float(); break;
    case 23: update_diff((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: update_diff_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: update_diff_fix(); break;
    case 26: update_diff_float(); break;
    case 27: update_conc((const QString&)static_QUType_QString.get(_o+1)); break;
    case 28: update_conc_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 29: update_conc_fix(); break;
    case 30: update_conc_float(); break;
    case 31: update_sigma((const QString&)static_QUType_QString.get(_o+1)); break;
    case 32: update_sigma_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 33: update_sigma_fix(); break;
    case 34: update_sigma_float(); break;
    case 35: update_delta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 36: update_delta_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 37: update_delta_fix(); break;
    case 38: update_delta_float(); break;
    case 39: update_mw((const QString&)static_QUType_QString.get(_o+1)); break;
    case 40: update_mw_fix(); break;
    case 41: update_mw_float(); break;
    case 42: update_model_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 43: update_vbar_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 44: read_vbar(); break;
    case 45: update_vbar_fix(); break;
    case 46: update_vbar_float(); break;
    case 47: save_model(); break;
    case 48: save_model((const QString&)static_QUType_QString.get(_o+1)); break;
    case 49: load_model(); break;
    case 50: load_model((const QString&)static_QUType_QString.get(_o+1)); break;
    case 51: calc_mw((const int)static_QUType_int.get(_o+1)); break;
    case 52: calc_vbar((const int)static_QUType_int.get(_o+1)); break;
    case 53: calc_D((const int)static_QUType_int.get(_o+1)); break;
    case 54: quit(); break;
    case 55: help(); break;
    case 56: show_model(); break;
    case 57: reset(); break;
    case 58: reset_range(); break;
    case 59: meniscus_red(); break;
    case 60: meniscus_green(); break;
    case 61: meniscus_dk_green(); break;
    case 62: baseline_red(); break;
    case 63: baseline_green(); break;
    case 64: baseline_dk_green(); break;
    case 65: slope_red(); break;
    case 66: slope_green(); break;
    case 67: slope_dk_green(); break;
    case 68: stray_red(); break;
    case 69: stray_green(); break;
    case 70: stray_dk_green(); break;
    case 71: sed_red(); break;
    case 72: sed_green(); break;
    case 73: sed_dk_green(); break;
    case 74: diff_red(); break;
    case 75: diff_green(); break;
    case 76: diff_dk_green(); break;
    case 77: conc_red(); break;
    case 78: conc_green(); break;
    case 79: conc_dk_green(); break;
    case 80: sigma_red(); break;
    case 81: sigma_green(); break;
    case 82: sigma_dk_green(); break;
    case 83: delta_red(); break;
    case 84: delta_green(); break;
    case 85: delta_dk_green(); break;
    case 86: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_VelocModelControl::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_VelocModelControl::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_VelocModelControl::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
