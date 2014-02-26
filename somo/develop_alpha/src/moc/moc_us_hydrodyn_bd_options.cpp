/****************************************************************************
** US_Hydrodyn_BD_Options meta object code from reading C++ file 'us_hydrodyn_bd_options.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_bd_options.h"
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

const char *US_Hydrodyn_BD_Options::className() const
{
    return "US_Hydrodyn_BD_Options";
}

QMetaObject *US_Hydrodyn_BD_Options::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_BD_Options( "US_Hydrodyn_BD_Options", &US_Hydrodyn_BD_Options::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_BD_Options::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_BD_Options", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_BD_Options::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_BD_Options", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_BD_Options::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"update_enables", 0, 0 };
    static const QUMethod slot_2 = {"update_labels", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_bd_threshold_pb_pb", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_bd_threshold_pb_sc", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_bd_threshold_sc_sc", 1, param_slot_5 };
    static const QUMethod slot_6 = {"set_do_rr", 0, 0 };
    static const QUMethod slot_7 = {"set_force_chem", 0, 0 };
    static const QUMethod slot_8 = {"set_show_pdb", 0, 0 };
    static const QUMethod slot_9 = {"set_run_browflex", 0, 0 };
    static const QUMethod slot_10 = {"set_icdm", 0, 0 };
    static const QUMethod slot_11 = {"set_compute_chem_pb_pb_force_constant", 0, 0 };
    static const QUMethod slot_12 = {"set_compute_chem_pb_pb_equilibrium_dist", 0, 0 };
    static const QUMethod slot_13 = {"set_compute_chem_pb_pb_max_elong", 0, 0 };
    static const QUMethod slot_14 = {"set_compute_chem_pb_sc_force_constant", 0, 0 };
    static const QUMethod slot_15 = {"set_compute_chem_pb_sc_equilibrium_dist", 0, 0 };
    static const QUMethod slot_16 = {"set_compute_chem_pb_sc_max_elong", 0, 0 };
    static const QUMethod slot_17 = {"set_compute_chem_sc_sc_force_constant", 0, 0 };
    static const QUMethod slot_18 = {"set_compute_chem_sc_sc_equilibrium_dist", 0, 0 };
    static const QUMethod slot_19 = {"set_compute_chem_sc_sc_max_elong", 0, 0 };
    static const QUMethod slot_20 = {"set_compute_pb_pb_force_constant", 0, 0 };
    static const QUMethod slot_21 = {"set_compute_pb_pb_equilibrium_dist", 0, 0 };
    static const QUMethod slot_22 = {"set_compute_pb_pb_max_elong", 0, 0 };
    static const QUMethod slot_23 = {"set_compute_pb_sc_force_constant", 0, 0 };
    static const QUMethod slot_24 = {"set_compute_pb_sc_equilibrium_dist", 0, 0 };
    static const QUMethod slot_25 = {"set_compute_pb_sc_max_elong", 0, 0 };
    static const QUMethod slot_26 = {"set_compute_sc_sc_force_constant", 0, 0 };
    static const QUMethod slot_27 = {"set_compute_sc_sc_equilibrium_dist", 0, 0 };
    static const QUMethod slot_28 = {"set_compute_sc_sc_max_elong", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"update_tprev", 1, param_slot_29 };
    static const QUParameter param_slot_30[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_30 = {"update_ttraj", 1, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_31 = {"update_deltat", 1, param_slot_31 };
    static const QUParameter param_slot_32[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"update_nmol", 1, param_slot_32 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_33 = {"update_npadif", 1, param_slot_33 };
    static const QUParameter param_slot_34[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_34 = {"update_nconf", 1, param_slot_34 };
    static const QUParameter param_slot_35[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_35 = {"update_iseed", 1, param_slot_35 };
    static const QUParameter param_slot_36[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_36 = {"set_bead_size_type", 1, param_slot_36 };
    static const QUParameter param_slot_37[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_37 = {"set_iorder", 1, param_slot_37 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"set_inter", 1, param_slot_38 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_39 = {"set_chem_pb_pb_bond_types", 1, param_slot_39 };
    static const QUParameter param_slot_40[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_40 = {"set_chem_pb_sc_bond_types", 1, param_slot_40 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_41 = {"set_chem_sc_sc_bond_types", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_42 = {"set_pb_pb_bond_types", 1, param_slot_42 };
    static const QUParameter param_slot_43[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_43 = {"set_pb_sc_bond_types", 1, param_slot_43 };
    static const QUParameter param_slot_44[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_44 = {"set_sc_sc_bond_types", 1, param_slot_44 };
    static const QUParameter param_slot_45[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_45 = {"update_chem_pb_pb_force_constant", 1, param_slot_45 };
    static const QUParameter param_slot_46[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_46 = {"update_chem_pb_pb_equilibrium_dist", 1, param_slot_46 };
    static const QUParameter param_slot_47[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_47 = {"update_chem_pb_pb_max_elong", 1, param_slot_47 };
    static const QUParameter param_slot_48[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_48 = {"update_chem_pb_sc_force_constant", 1, param_slot_48 };
    static const QUParameter param_slot_49[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_49 = {"update_chem_pb_sc_equilibrium_dist", 1, param_slot_49 };
    static const QUParameter param_slot_50[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_50 = {"update_chem_pb_sc_max_elong", 1, param_slot_50 };
    static const QUParameter param_slot_51[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_51 = {"update_chem_sc_sc_force_constant", 1, param_slot_51 };
    static const QUParameter param_slot_52[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_52 = {"update_chem_sc_sc_equilibrium_dist", 1, param_slot_52 };
    static const QUParameter param_slot_53[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_53 = {"update_chem_sc_sc_max_elong", 1, param_slot_53 };
    static const QUParameter param_slot_54[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_54 = {"update_pb_pb_force_constant", 1, param_slot_54 };
    static const QUParameter param_slot_55[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_55 = {"update_pb_pb_equilibrium_dist", 1, param_slot_55 };
    static const QUParameter param_slot_56[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_56 = {"update_pb_pb_max_elong", 1, param_slot_56 };
    static const QUParameter param_slot_57[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_57 = {"update_pb_sc_force_constant", 1, param_slot_57 };
    static const QUParameter param_slot_58[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_58 = {"update_pb_sc_equilibrium_dist", 1, param_slot_58 };
    static const QUParameter param_slot_59[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_59 = {"update_pb_sc_max_elong", 1, param_slot_59 };
    static const QUParameter param_slot_60[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_60 = {"update_sc_sc_force_constant", 1, param_slot_60 };
    static const QUParameter param_slot_61[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_61 = {"update_sc_sc_equilibrium_dist", 1, param_slot_61 };
    static const QUParameter param_slot_62[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_62 = {"update_sc_sc_max_elong", 1, param_slot_62 };
    static const QUMethod slot_63 = {"dup_fraenkel", 0, 0 };
    static const QUMethod slot_64 = {"cancel", 0, 0 };
    static const QUMethod slot_65 = {"help", 0, 0 };
    static const QUParameter param_slot_66[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_66 = {"closeEvent", 1, param_slot_66 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_enables()", &slot_1, QMetaData::Private },
	{ "update_labels()", &slot_2, QMetaData::Private },
	{ "update_bd_threshold_pb_pb(double)", &slot_3, QMetaData::Private },
	{ "update_bd_threshold_pb_sc(double)", &slot_4, QMetaData::Private },
	{ "update_bd_threshold_sc_sc(double)", &slot_5, QMetaData::Private },
	{ "set_do_rr()", &slot_6, QMetaData::Private },
	{ "set_force_chem()", &slot_7, QMetaData::Private },
	{ "set_show_pdb()", &slot_8, QMetaData::Private },
	{ "set_run_browflex()", &slot_9, QMetaData::Private },
	{ "set_icdm()", &slot_10, QMetaData::Private },
	{ "set_compute_chem_pb_pb_force_constant()", &slot_11, QMetaData::Private },
	{ "set_compute_chem_pb_pb_equilibrium_dist()", &slot_12, QMetaData::Private },
	{ "set_compute_chem_pb_pb_max_elong()", &slot_13, QMetaData::Private },
	{ "set_compute_chem_pb_sc_force_constant()", &slot_14, QMetaData::Private },
	{ "set_compute_chem_pb_sc_equilibrium_dist()", &slot_15, QMetaData::Private },
	{ "set_compute_chem_pb_sc_max_elong()", &slot_16, QMetaData::Private },
	{ "set_compute_chem_sc_sc_force_constant()", &slot_17, QMetaData::Private },
	{ "set_compute_chem_sc_sc_equilibrium_dist()", &slot_18, QMetaData::Private },
	{ "set_compute_chem_sc_sc_max_elong()", &slot_19, QMetaData::Private },
	{ "set_compute_pb_pb_force_constant()", &slot_20, QMetaData::Private },
	{ "set_compute_pb_pb_equilibrium_dist()", &slot_21, QMetaData::Private },
	{ "set_compute_pb_pb_max_elong()", &slot_22, QMetaData::Private },
	{ "set_compute_pb_sc_force_constant()", &slot_23, QMetaData::Private },
	{ "set_compute_pb_sc_equilibrium_dist()", &slot_24, QMetaData::Private },
	{ "set_compute_pb_sc_max_elong()", &slot_25, QMetaData::Private },
	{ "set_compute_sc_sc_force_constant()", &slot_26, QMetaData::Private },
	{ "set_compute_sc_sc_equilibrium_dist()", &slot_27, QMetaData::Private },
	{ "set_compute_sc_sc_max_elong()", &slot_28, QMetaData::Private },
	{ "update_tprev(const QString&)", &slot_29, QMetaData::Private },
	{ "update_ttraj(const QString&)", &slot_30, QMetaData::Private },
	{ "update_deltat(const QString&)", &slot_31, QMetaData::Private },
	{ "update_nmol(const QString&)", &slot_32, QMetaData::Private },
	{ "update_npadif(double)", &slot_33, QMetaData::Private },
	{ "update_nconf(const QString&)", &slot_34, QMetaData::Private },
	{ "update_iseed(const QString&)", &slot_35, QMetaData::Private },
	{ "set_bead_size_type(int)", &slot_36, QMetaData::Private },
	{ "set_iorder(int)", &slot_37, QMetaData::Private },
	{ "set_inter(int)", &slot_38, QMetaData::Private },
	{ "set_chem_pb_pb_bond_types(int)", &slot_39, QMetaData::Private },
	{ "set_chem_pb_sc_bond_types(int)", &slot_40, QMetaData::Private },
	{ "set_chem_sc_sc_bond_types(int)", &slot_41, QMetaData::Private },
	{ "set_pb_pb_bond_types(int)", &slot_42, QMetaData::Private },
	{ "set_pb_sc_bond_types(int)", &slot_43, QMetaData::Private },
	{ "set_sc_sc_bond_types(int)", &slot_44, QMetaData::Private },
	{ "update_chem_pb_pb_force_constant(const QString&)", &slot_45, QMetaData::Private },
	{ "update_chem_pb_pb_equilibrium_dist(const QString&)", &slot_46, QMetaData::Private },
	{ "update_chem_pb_pb_max_elong(const QString&)", &slot_47, QMetaData::Private },
	{ "update_chem_pb_sc_force_constant(const QString&)", &slot_48, QMetaData::Private },
	{ "update_chem_pb_sc_equilibrium_dist(const QString&)", &slot_49, QMetaData::Private },
	{ "update_chem_pb_sc_max_elong(const QString&)", &slot_50, QMetaData::Private },
	{ "update_chem_sc_sc_force_constant(const QString&)", &slot_51, QMetaData::Private },
	{ "update_chem_sc_sc_equilibrium_dist(const QString&)", &slot_52, QMetaData::Private },
	{ "update_chem_sc_sc_max_elong(const QString&)", &slot_53, QMetaData::Private },
	{ "update_pb_pb_force_constant(const QString&)", &slot_54, QMetaData::Private },
	{ "update_pb_pb_equilibrium_dist(const QString&)", &slot_55, QMetaData::Private },
	{ "update_pb_pb_max_elong(const QString&)", &slot_56, QMetaData::Private },
	{ "update_pb_sc_force_constant(const QString&)", &slot_57, QMetaData::Private },
	{ "update_pb_sc_equilibrium_dist(const QString&)", &slot_58, QMetaData::Private },
	{ "update_pb_sc_max_elong(const QString&)", &slot_59, QMetaData::Private },
	{ "update_sc_sc_force_constant(const QString&)", &slot_60, QMetaData::Private },
	{ "update_sc_sc_equilibrium_dist(const QString&)", &slot_61, QMetaData::Private },
	{ "update_sc_sc_max_elong(const QString&)", &slot_62, QMetaData::Private },
	{ "dup_fraenkel()", &slot_63, QMetaData::Private },
	{ "cancel()", &slot_64, QMetaData::Private },
	{ "help()", &slot_65, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_66, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_BD_Options", parentObject,
	slot_tbl, 67,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_BD_Options.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_BD_Options::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_BD_Options" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_BD_Options::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_enables(); break;
    case 2: update_labels(); break;
    case 3: update_bd_threshold_pb_pb((double)static_QUType_double.get(_o+1)); break;
    case 4: update_bd_threshold_pb_sc((double)static_QUType_double.get(_o+1)); break;
    case 5: update_bd_threshold_sc_sc((double)static_QUType_double.get(_o+1)); break;
    case 6: set_do_rr(); break;
    case 7: set_force_chem(); break;
    case 8: set_show_pdb(); break;
    case 9: set_run_browflex(); break;
    case 10: set_icdm(); break;
    case 11: set_compute_chem_pb_pb_force_constant(); break;
    case 12: set_compute_chem_pb_pb_equilibrium_dist(); break;
    case 13: set_compute_chem_pb_pb_max_elong(); break;
    case 14: set_compute_chem_pb_sc_force_constant(); break;
    case 15: set_compute_chem_pb_sc_equilibrium_dist(); break;
    case 16: set_compute_chem_pb_sc_max_elong(); break;
    case 17: set_compute_chem_sc_sc_force_constant(); break;
    case 18: set_compute_chem_sc_sc_equilibrium_dist(); break;
    case 19: set_compute_chem_sc_sc_max_elong(); break;
    case 20: set_compute_pb_pb_force_constant(); break;
    case 21: set_compute_pb_pb_equilibrium_dist(); break;
    case 22: set_compute_pb_pb_max_elong(); break;
    case 23: set_compute_pb_sc_force_constant(); break;
    case 24: set_compute_pb_sc_equilibrium_dist(); break;
    case 25: set_compute_pb_sc_max_elong(); break;
    case 26: set_compute_sc_sc_force_constant(); break;
    case 27: set_compute_sc_sc_equilibrium_dist(); break;
    case 28: set_compute_sc_sc_max_elong(); break;
    case 29: update_tprev((const QString&)static_QUType_QString.get(_o+1)); break;
    case 30: update_ttraj((const QString&)static_QUType_QString.get(_o+1)); break;
    case 31: update_deltat((const QString&)static_QUType_QString.get(_o+1)); break;
    case 32: update_nmol((const QString&)static_QUType_QString.get(_o+1)); break;
    case 33: update_npadif((double)static_QUType_double.get(_o+1)); break;
    case 34: update_nconf((const QString&)static_QUType_QString.get(_o+1)); break;
    case 35: update_iseed((const QString&)static_QUType_QString.get(_o+1)); break;
    case 36: set_bead_size_type((int)static_QUType_int.get(_o+1)); break;
    case 37: set_iorder((int)static_QUType_int.get(_o+1)); break;
    case 38: set_inter((int)static_QUType_int.get(_o+1)); break;
    case 39: set_chem_pb_pb_bond_types((int)static_QUType_int.get(_o+1)); break;
    case 40: set_chem_pb_sc_bond_types((int)static_QUType_int.get(_o+1)); break;
    case 41: set_chem_sc_sc_bond_types((int)static_QUType_int.get(_o+1)); break;
    case 42: set_pb_pb_bond_types((int)static_QUType_int.get(_o+1)); break;
    case 43: set_pb_sc_bond_types((int)static_QUType_int.get(_o+1)); break;
    case 44: set_sc_sc_bond_types((int)static_QUType_int.get(_o+1)); break;
    case 45: update_chem_pb_pb_force_constant((const QString&)static_QUType_QString.get(_o+1)); break;
    case 46: update_chem_pb_pb_equilibrium_dist((const QString&)static_QUType_QString.get(_o+1)); break;
    case 47: update_chem_pb_pb_max_elong((const QString&)static_QUType_QString.get(_o+1)); break;
    case 48: update_chem_pb_sc_force_constant((const QString&)static_QUType_QString.get(_o+1)); break;
    case 49: update_chem_pb_sc_equilibrium_dist((const QString&)static_QUType_QString.get(_o+1)); break;
    case 50: update_chem_pb_sc_max_elong((const QString&)static_QUType_QString.get(_o+1)); break;
    case 51: update_chem_sc_sc_force_constant((const QString&)static_QUType_QString.get(_o+1)); break;
    case 52: update_chem_sc_sc_equilibrium_dist((const QString&)static_QUType_QString.get(_o+1)); break;
    case 53: update_chem_sc_sc_max_elong((const QString&)static_QUType_QString.get(_o+1)); break;
    case 54: update_pb_pb_force_constant((const QString&)static_QUType_QString.get(_o+1)); break;
    case 55: update_pb_pb_equilibrium_dist((const QString&)static_QUType_QString.get(_o+1)); break;
    case 56: update_pb_pb_max_elong((const QString&)static_QUType_QString.get(_o+1)); break;
    case 57: update_pb_sc_force_constant((const QString&)static_QUType_QString.get(_o+1)); break;
    case 58: update_pb_sc_equilibrium_dist((const QString&)static_QUType_QString.get(_o+1)); break;
    case 59: update_pb_sc_max_elong((const QString&)static_QUType_QString.get(_o+1)); break;
    case 60: update_sc_sc_force_constant((const QString&)static_QUType_QString.get(_o+1)); break;
    case 61: update_sc_sc_equilibrium_dist((const QString&)static_QUType_QString.get(_o+1)); break;
    case 62: update_sc_sc_max_elong((const QString&)static_QUType_QString.get(_o+1)); break;
    case 63: dup_fraenkel(); break;
    case 64: cancel(); break;
    case 65: help(); break;
    case 66: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_BD_Options::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_BD_Options::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_BD_Options::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
