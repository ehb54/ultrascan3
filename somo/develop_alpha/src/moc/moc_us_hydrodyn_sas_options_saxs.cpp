/****************************************************************************
** US_Hydrodyn_SasOptionsSaxs meta object code from reading C++ file 'us_hydrodyn_sas_options_saxs.h'
**
** Created: Wed Jan 15 14:28:59 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_sas_options_saxs.h"
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

const char *US_Hydrodyn_SasOptionsSaxs::className() const
{
    return "US_Hydrodyn_SasOptionsSaxs";
}

QMetaObject *US_Hydrodyn_SasOptionsSaxs::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SasOptionsSaxs( "US_Hydrodyn_SasOptionsSaxs", &US_Hydrodyn_SasOptionsSaxs::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SasOptionsSaxs::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsSaxs", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SasOptionsSaxs::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsSaxs", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SasOptionsSaxs::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_water_e_density", 1, param_slot_1 };
    static const QUMethod slot_2 = {"set_saxs_iq_native_debye", 0, 0 };
    static const QUMethod slot_3 = {"set_saxs_iq_native_sh", 0, 0 };
    static const QUMethod slot_4 = {"set_saxs_iq_native_hybrid", 0, 0 };
    static const QUMethod slot_5 = {"set_saxs_iq_native_hybrid2", 0, 0 };
    static const QUMethod slot_6 = {"set_saxs_iq_native_hybrid3", 0, 0 };
    static const QUMethod slot_7 = {"set_saxs_iq_hybrid_adaptive", 0, 0 };
    static const QUMethod slot_8 = {"set_saxs_iq_native_fast", 0, 0 };
    static const QUMethod slot_9 = {"set_saxs_iq_native_fast_compute_pr", 0, 0 };
    static const QUMethod slot_10 = {"set_saxs_iq_crysol", 0, 0 };
    static const QUMethod slot_11 = {"set_saxs_iq_foxs", 0, 0 };
    static const QUMethod slot_12 = {"set_saxs_iq_sastbx", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_fast_bin_size", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_fast_modulation", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_hybrid2_q_points", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_sh_max_harmonics", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_sh_fibonacci_grid_order", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_crysol_hydration_shell_contrast", 1, param_slot_18 };
    static const QUMethod slot_19 = {"set_crysol_default_load_difference_intensity", 0, 0 };
    static const QUMethod slot_20 = {"set_crysol_version_26", 0, 0 };
    static const QUMethod slot_21 = {"set_crysol_explicit_hydrogens", 0, 0 };
    static const QUMethod slot_22 = {"crysol_target", 0, 0 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"set_sastbx_method", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"set_ra", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"set_vol", 1, param_slot_25 };
    static const QUMethod slot_26 = {"cancel", 0, 0 };
    static const QUMethod slot_27 = {"help", 0, 0 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_28 = {"closeEvent", 1, param_slot_28 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_water_e_density(double)", &slot_1, QMetaData::Private },
	{ "set_saxs_iq_native_debye()", &slot_2, QMetaData::Private },
	{ "set_saxs_iq_native_sh()", &slot_3, QMetaData::Private },
	{ "set_saxs_iq_native_hybrid()", &slot_4, QMetaData::Private },
	{ "set_saxs_iq_native_hybrid2()", &slot_5, QMetaData::Private },
	{ "set_saxs_iq_native_hybrid3()", &slot_6, QMetaData::Private },
	{ "set_saxs_iq_hybrid_adaptive()", &slot_7, QMetaData::Private },
	{ "set_saxs_iq_native_fast()", &slot_8, QMetaData::Private },
	{ "set_saxs_iq_native_fast_compute_pr()", &slot_9, QMetaData::Private },
	{ "set_saxs_iq_crysol()", &slot_10, QMetaData::Private },
	{ "set_saxs_iq_foxs()", &slot_11, QMetaData::Private },
	{ "set_saxs_iq_sastbx()", &slot_12, QMetaData::Private },
	{ "update_fast_bin_size(double)", &slot_13, QMetaData::Private },
	{ "update_fast_modulation(double)", &slot_14, QMetaData::Private },
	{ "update_hybrid2_q_points(double)", &slot_15, QMetaData::Private },
	{ "update_sh_max_harmonics(double)", &slot_16, QMetaData::Private },
	{ "update_sh_fibonacci_grid_order(double)", &slot_17, QMetaData::Private },
	{ "update_crysol_hydration_shell_contrast(double)", &slot_18, QMetaData::Private },
	{ "set_crysol_default_load_difference_intensity()", &slot_19, QMetaData::Private },
	{ "set_crysol_version_26()", &slot_20, QMetaData::Private },
	{ "set_crysol_explicit_hydrogens()", &slot_21, QMetaData::Private },
	{ "crysol_target()", &slot_22, QMetaData::Private },
	{ "set_sastbx_method(int)", &slot_23, QMetaData::Private },
	{ "set_ra(const QString&)", &slot_24, QMetaData::Private },
	{ "set_vol(const QString&)", &slot_25, QMetaData::Private },
	{ "cancel()", &slot_26, QMetaData::Private },
	{ "help()", &slot_27, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_28, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SasOptionsSaxs", parentObject,
	slot_tbl, 29,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SasOptionsSaxs.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SasOptionsSaxs::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SasOptionsSaxs" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SasOptionsSaxs::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_water_e_density((double)static_QUType_double.get(_o+1)); break;
    case 2: set_saxs_iq_native_debye(); break;
    case 3: set_saxs_iq_native_sh(); break;
    case 4: set_saxs_iq_native_hybrid(); break;
    case 5: set_saxs_iq_native_hybrid2(); break;
    case 6: set_saxs_iq_native_hybrid3(); break;
    case 7: set_saxs_iq_hybrid_adaptive(); break;
    case 8: set_saxs_iq_native_fast(); break;
    case 9: set_saxs_iq_native_fast_compute_pr(); break;
    case 10: set_saxs_iq_crysol(); break;
    case 11: set_saxs_iq_foxs(); break;
    case 12: set_saxs_iq_sastbx(); break;
    case 13: update_fast_bin_size((double)static_QUType_double.get(_o+1)); break;
    case 14: update_fast_modulation((double)static_QUType_double.get(_o+1)); break;
    case 15: update_hybrid2_q_points((double)static_QUType_double.get(_o+1)); break;
    case 16: update_sh_max_harmonics((double)static_QUType_double.get(_o+1)); break;
    case 17: update_sh_fibonacci_grid_order((double)static_QUType_double.get(_o+1)); break;
    case 18: update_crysol_hydration_shell_contrast((double)static_QUType_double.get(_o+1)); break;
    case 19: set_crysol_default_load_difference_intensity(); break;
    case 20: set_crysol_version_26(); break;
    case 21: set_crysol_explicit_hydrogens(); break;
    case 22: crysol_target(); break;
    case 23: set_sastbx_method((int)static_QUType_int.get(_o+1)); break;
    case 24: set_ra((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: set_vol((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: cancel(); break;
    case 27: help(); break;
    case 28: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SasOptionsSaxs::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SasOptionsSaxs::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SasOptionsSaxs::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
