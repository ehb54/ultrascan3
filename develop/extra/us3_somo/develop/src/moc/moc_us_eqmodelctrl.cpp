/****************************************************************************
** US_EqModelControl meta object code from reading C++ file 'us_eqmodelctrl.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_eqmodelctrl.h"
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

const char *US_EqModelControl::className() const
{
    return "US_EqModelControl";
}

QMetaObject *US_EqModelControl::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_EqModelControl( "US_EqModelControl", &US_EqModelControl::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_EqModelControl::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EqModelControl", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_EqModelControl::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EqModelControl", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_EqModelControl::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_component1", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_component2", 1, param_slot_1 };
    static const QUMethod slot_2 = {"update_component_labels", 0, 0 };
    static const QUMethod slot_3 = {"mw_float", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"mw_float", 1, param_slot_4 };
    static const QUMethod slot_5 = {"mw_constrained", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"get_mw_value", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"get_mw_range", 1, param_slot_7 };
    static const QUMethod slot_8 = {"vbar_float", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"vbar_float", 1, param_slot_9 };
    static const QUMethod slot_10 = {"vbar_constrained", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"get_vbar_value", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"get_vbar_range", 1, param_slot_12 };
    static const QUMethod slot_13 = {"read_vbar", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_vbar_lbl", 2, param_slot_14 };
    static const QUMethod slot_15 = {"virial_float", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"virial_float", 1, param_slot_16 };
    static const QUMethod slot_17 = {"virial_constrained", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"get_virial_value", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"get_virial_range", 1, param_slot_19 };
    static const QUMethod slot_20 = {"eqconst1_float", 0, 0 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"eqconst1_float", 1, param_slot_21 };
    static const QUMethod slot_22 = {"eqconst1_constrained", 0, 0 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"get_eqconst1_value", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"get_eqconst1_range", 1, param_slot_24 };
    static const QUMethod slot_25 = {"eqconst2_float", 0, 0 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"eqconst2_float", 1, param_slot_26 };
    static const QUMethod slot_27 = {"eqconst2_constrained", 0, 0 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"get_eqconst2_value", 1, param_slot_28 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"get_eqconst2_range", 1, param_slot_29 };
    static const QUMethod slot_30 = {"eqconst3_float", 0, 0 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_31 = {"eqconst3_float", 1, param_slot_31 };
    static const QUMethod slot_32 = {"eqconst3_constrained", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_33 = {"get_eqconst3_value", 1, param_slot_33 };
    static const QUParameter param_slot_34[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_34 = {"get_eqconst3_range", 1, param_slot_34 };
    static const QUMethod slot_35 = {"eqconst4_float", 0, 0 };
    static const QUParameter param_slot_36[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_36 = {"eqconst4_float", 1, param_slot_36 };
    static const QUMethod slot_37 = {"eqconst4_constrained", 0, 0 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"get_eqconst4_value", 1, param_slot_38 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_39 = {"get_eqconst4_range", 1, param_slot_39 };
    static const QUMethod slot_40 = {"baseline_float", 0, 0 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_41 = {"baseline_float", 1, param_slot_41 };
    static const QUMethod slot_42 = {"baseline_constrained", 0, 0 };
    static const QUParameter param_slot_43[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_43 = {"get_baseline_value", 1, param_slot_43 };
    static const QUParameter param_slot_44[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_44 = {"get_baseline_range", 1, param_slot_44 };
    static const QUMethod slot_45 = {"amplitude_float", 0, 0 };
    static const QUParameter param_slot_46[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_46 = {"amplitude_float", 1, param_slot_46 };
    static const QUMethod slot_47 = {"amplitude_constrained", 0, 0 };
    static const QUParameter param_slot_48[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_48 = {"get_amplitude_value", 1, param_slot_48 };
    static const QUParameter param_slot_49[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_49 = {"get_amplitude_range", 1, param_slot_49 };
    static const QUMethod slot_50 = {"setup_model", 0, 0 };
    static const QUMethod slot_51 = {"apply_density_range", 0, 0 };
    static const QUMethod slot_52 = {"apply_extinction_range", 0, 0 };
    static const QUMethod slot_53 = {"apply_pathlength_range", 0, 0 };
    static const QUParameter param_slot_54[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_54 = {"update_projectName", 1, param_slot_54 };
    static const QUParameter param_slot_55[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_55 = {"update_sigma", 1, param_slot_55 };
    static const QUParameter param_slot_56[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_56 = {"update_scan_range_start", 1, param_slot_56 };
    static const QUParameter param_slot_57[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_57 = {"update_scan_range_stop", 1, param_slot_57 };
    static const QUParameter param_slot_58[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_58 = {"update_ext_range_start", 1, param_slot_58 };
    static const QUParameter param_slot_59[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_59 = {"update_ext_range_stop", 1, param_slot_59 };
    static const QUParameter param_slot_60[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_60 = {"update_pathlength_range_start", 1, param_slot_60 };
    static const QUParameter param_slot_61[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_61 = {"update_pathlength_range_stop", 1, param_slot_61 };
    static const QUParameter param_slot_62[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_62 = {"update_scan_number", 1, param_slot_62 };
    static const QUParameter param_slot_63[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_63 = {"update_scan_info", 1, param_slot_63 };
    static const QUMethod slot_64 = {"update_eqconst1", 0, 0 };
    static const QUMethod slot_65 = {"update_eqconst2", 0, 0 };
    static const QUMethod slot_66 = {"update_eqconst3", 0, 0 };
    static const QUMethod slot_67 = {"update_eqconst4", 0, 0 };
    static const QUMethod slot_68 = {"update_density1", 0, 0 };
    static const QUParameter param_slot_69[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_69 = {"update_density2", 1, param_slot_69 };
    static const QUParameter param_slot_70[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_70 = {"update_density_label", 2, param_slot_70 };
    static const QUMethod slot_71 = {"update_extinction1", 0, 0 };
    static const QUParameter param_slot_72[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_72 = {"update_extinction2", 1, param_slot_72 };
    static const QUParameter param_slot_73[] = {
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_73 = {"update_extinction3", 3, param_slot_73 };
    static const QUMethod slot_74 = {"update_extinction4", 0, 0 };
    static const QUParameter param_slot_75[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_75 = {"update_pathlength2", 1, param_slot_75 };
    static const QUMethod slot_76 = {"update_include", 0, 0 };
    static const QUMethod slot_77 = {"change_include", 0, 0 };
    static const QUParameter param_slot_78[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_78 = {"calc_density_tb", 2, param_slot_78 };
    static const QUMethod slot_79 = {"quit", 0, 0 };
    static const QUMethod slot_80 = {"help", 0, 0 };
    static const QUMethod slot_81 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_82[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_82 = {"closeEvent", 1, param_slot_82 };
    static const QMetaData slot_tbl[] = {
	{ "update_component1(double)", &slot_0, QMetaData::Public },
	{ "update_component2(double)", &slot_1, QMetaData::Public },
	{ "update_component_labels()", &slot_2, QMetaData::Public },
	{ "mw_float()", &slot_3, QMetaData::Public },
	{ "mw_float(bool)", &slot_4, QMetaData::Public },
	{ "mw_constrained()", &slot_5, QMetaData::Public },
	{ "get_mw_value(const QString&)", &slot_6, QMetaData::Public },
	{ "get_mw_range(const QString&)", &slot_7, QMetaData::Public },
	{ "vbar_float()", &slot_8, QMetaData::Public },
	{ "vbar_float(bool)", &slot_9, QMetaData::Public },
	{ "vbar_constrained()", &slot_10, QMetaData::Public },
	{ "get_vbar_value(const QString&)", &slot_11, QMetaData::Public },
	{ "get_vbar_range(const QString&)", &slot_12, QMetaData::Public },
	{ "read_vbar()", &slot_13, QMetaData::Public },
	{ "update_vbar_lbl(float,float)", &slot_14, QMetaData::Public },
	{ "virial_float()", &slot_15, QMetaData::Public },
	{ "virial_float(bool)", &slot_16, QMetaData::Public },
	{ "virial_constrained()", &slot_17, QMetaData::Public },
	{ "get_virial_value(const QString&)", &slot_18, QMetaData::Public },
	{ "get_virial_range(const QString&)", &slot_19, QMetaData::Public },
	{ "eqconst1_float()", &slot_20, QMetaData::Public },
	{ "eqconst1_float(bool)", &slot_21, QMetaData::Public },
	{ "eqconst1_constrained()", &slot_22, QMetaData::Public },
	{ "get_eqconst1_value(const QString&)", &slot_23, QMetaData::Public },
	{ "get_eqconst1_range(const QString&)", &slot_24, QMetaData::Public },
	{ "eqconst2_float()", &slot_25, QMetaData::Public },
	{ "eqconst2_float(bool)", &slot_26, QMetaData::Public },
	{ "eqconst2_constrained()", &slot_27, QMetaData::Public },
	{ "get_eqconst2_value(const QString&)", &slot_28, QMetaData::Public },
	{ "get_eqconst2_range(const QString&)", &slot_29, QMetaData::Public },
	{ "eqconst3_float()", &slot_30, QMetaData::Public },
	{ "eqconst3_float(bool)", &slot_31, QMetaData::Public },
	{ "eqconst3_constrained()", &slot_32, QMetaData::Public },
	{ "get_eqconst3_value(const QString&)", &slot_33, QMetaData::Public },
	{ "get_eqconst3_range(const QString&)", &slot_34, QMetaData::Public },
	{ "eqconst4_float()", &slot_35, QMetaData::Public },
	{ "eqconst4_float(bool)", &slot_36, QMetaData::Public },
	{ "eqconst4_constrained()", &slot_37, QMetaData::Public },
	{ "get_eqconst4_value(const QString&)", &slot_38, QMetaData::Public },
	{ "get_eqconst4_range(const QString&)", &slot_39, QMetaData::Public },
	{ "baseline_float()", &slot_40, QMetaData::Public },
	{ "baseline_float(bool)", &slot_41, QMetaData::Public },
	{ "baseline_constrained()", &slot_42, QMetaData::Public },
	{ "get_baseline_value(const QString&)", &slot_43, QMetaData::Public },
	{ "get_baseline_range(const QString&)", &slot_44, QMetaData::Public },
	{ "amplitude_float()", &slot_45, QMetaData::Public },
	{ "amplitude_float(bool)", &slot_46, QMetaData::Public },
	{ "amplitude_constrained()", &slot_47, QMetaData::Public },
	{ "get_amplitude_value(const QString&)", &slot_48, QMetaData::Public },
	{ "get_amplitude_range(const QString&)", &slot_49, QMetaData::Public },
	{ "setup_model()", &slot_50, QMetaData::Public },
	{ "apply_density_range()", &slot_51, QMetaData::Public },
	{ "apply_extinction_range()", &slot_52, QMetaData::Public },
	{ "apply_pathlength_range()", &slot_53, QMetaData::Public },
	{ "update_projectName(const QString&)", &slot_54, QMetaData::Public },
	{ "update_sigma(const QString&)", &slot_55, QMetaData::Public },
	{ "update_scan_range_start(const QString&)", &slot_56, QMetaData::Public },
	{ "update_scan_range_stop(const QString&)", &slot_57, QMetaData::Public },
	{ "update_ext_range_start(const QString&)", &slot_58, QMetaData::Public },
	{ "update_ext_range_stop(const QString&)", &slot_59, QMetaData::Public },
	{ "update_pathlength_range_start(const QString&)", &slot_60, QMetaData::Public },
	{ "update_pathlength_range_stop(const QString&)", &slot_61, QMetaData::Public },
	{ "update_scan_number(int)", &slot_62, QMetaData::Public },
	{ "update_scan_info(int)", &slot_63, QMetaData::Public },
	{ "update_eqconst1()", &slot_64, QMetaData::Public },
	{ "update_eqconst2()", &slot_65, QMetaData::Public },
	{ "update_eqconst3()", &slot_66, QMetaData::Public },
	{ "update_eqconst4()", &slot_67, QMetaData::Public },
	{ "update_density1()", &slot_68, QMetaData::Public },
	{ "update_density2(const QString&)", &slot_69, QMetaData::Public },
	{ "update_density_label(float,float)", &slot_70, QMetaData::Public },
	{ "update_extinction1()", &slot_71, QMetaData::Public },
	{ "update_extinction2(const QString&)", &slot_72, QMetaData::Public },
	{ "update_extinction3(double*,double*,unsigned int)", &slot_73, QMetaData::Public },
	{ "update_extinction4()", &slot_74, QMetaData::Public },
	{ "update_pathlength2(const QString&)", &slot_75, QMetaData::Public },
	{ "update_include()", &slot_76, QMetaData::Public },
	{ "change_include()", &slot_77, QMetaData::Public },
	{ "calc_density_tb(float,float)", &slot_78, QMetaData::Public },
	{ "quit()", &slot_79, QMetaData::Public },
	{ "help()", &slot_80, QMetaData::Public },
	{ "setup_GUI()", &slot_81, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_82, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"ControlWindowClosed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "ControlWindowClosed()", &signal_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_EqModelControl", parentObject,
	slot_tbl, 83,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_EqModelControl.setMetaObject( metaObj );
    return metaObj;
}

void* US_EqModelControl::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_EqModelControl" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL ControlWindowClosed
void US_EqModelControl::ControlWindowClosed()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_EqModelControl::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_component1((double)static_QUType_double.get(_o+1)); break;
    case 1: update_component2((double)static_QUType_double.get(_o+1)); break;
    case 2: update_component_labels(); break;
    case 3: mw_float(); break;
    case 4: mw_float((bool)static_QUType_bool.get(_o+1)); break;
    case 5: mw_constrained(); break;
    case 6: get_mw_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: get_mw_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: vbar_float(); break;
    case 9: vbar_float((bool)static_QUType_bool.get(_o+1)); break;
    case 10: vbar_constrained(); break;
    case 11: get_vbar_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: get_vbar_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: read_vbar(); break;
    case 14: update_vbar_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 15: virial_float(); break;
    case 16: virial_float((bool)static_QUType_bool.get(_o+1)); break;
    case 17: virial_constrained(); break;
    case 18: get_virial_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: get_virial_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: eqconst1_float(); break;
    case 21: eqconst1_float((bool)static_QUType_bool.get(_o+1)); break;
    case 22: eqconst1_constrained(); break;
    case 23: get_eqconst1_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: get_eqconst1_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: eqconst2_float(); break;
    case 26: eqconst2_float((bool)static_QUType_bool.get(_o+1)); break;
    case 27: eqconst2_constrained(); break;
    case 28: get_eqconst2_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 29: get_eqconst2_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 30: eqconst3_float(); break;
    case 31: eqconst3_float((bool)static_QUType_bool.get(_o+1)); break;
    case 32: eqconst3_constrained(); break;
    case 33: get_eqconst3_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 34: get_eqconst3_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 35: eqconst4_float(); break;
    case 36: eqconst4_float((bool)static_QUType_bool.get(_o+1)); break;
    case 37: eqconst4_constrained(); break;
    case 38: get_eqconst4_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 39: get_eqconst4_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 40: baseline_float(); break;
    case 41: baseline_float((bool)static_QUType_bool.get(_o+1)); break;
    case 42: baseline_constrained(); break;
    case 43: get_baseline_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 44: get_baseline_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 45: amplitude_float(); break;
    case 46: amplitude_float((bool)static_QUType_bool.get(_o+1)); break;
    case 47: amplitude_constrained(); break;
    case 48: get_amplitude_value((const QString&)static_QUType_QString.get(_o+1)); break;
    case 49: get_amplitude_range((const QString&)static_QUType_QString.get(_o+1)); break;
    case 50: setup_model(); break;
    case 51: apply_density_range(); break;
    case 52: apply_extinction_range(); break;
    case 53: apply_pathlength_range(); break;
    case 54: update_projectName((const QString&)static_QUType_QString.get(_o+1)); break;
    case 55: update_sigma((const QString&)static_QUType_QString.get(_o+1)); break;
    case 56: update_scan_range_start((const QString&)static_QUType_QString.get(_o+1)); break;
    case 57: update_scan_range_stop((const QString&)static_QUType_QString.get(_o+1)); break;
    case 58: update_ext_range_start((const QString&)static_QUType_QString.get(_o+1)); break;
    case 59: update_ext_range_stop((const QString&)static_QUType_QString.get(_o+1)); break;
    case 60: update_pathlength_range_start((const QString&)static_QUType_QString.get(_o+1)); break;
    case 61: update_pathlength_range_stop((const QString&)static_QUType_QString.get(_o+1)); break;
    case 62: update_scan_number((int)static_QUType_int.get(_o+1)); break;
    case 63: update_scan_info((int)static_QUType_int.get(_o+1)); break;
    case 64: update_eqconst1(); break;
    case 65: update_eqconst2(); break;
    case 66: update_eqconst3(); break;
    case 67: update_eqconst4(); break;
    case 68: update_density1(); break;
    case 69: update_density2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 70: update_density_label((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 71: update_extinction1(); break;
    case 72: update_extinction2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 73: update_extinction3((double*)static_QUType_varptr.get(_o+1),(double*)static_QUType_varptr.get(_o+2),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+3)))); break;
    case 74: update_extinction4(); break;
    case 75: update_pathlength2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 76: update_include(); break;
    case 77: change_include(); break;
    case 78: calc_density_tb((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 79: quit(); break;
    case 80: help(); break;
    case 81: setup_GUI(); break;
    case 82: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_EqModelControl::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: ControlWindowClosed(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_EqModelControl::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_EqModelControl::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
