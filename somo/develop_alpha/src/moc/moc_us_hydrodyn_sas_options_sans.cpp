/****************************************************************************
** US_Hydrodyn_SasOptionsSans meta object code from reading C++ file 'us_hydrodyn_sas_options_sans.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_sas_options_sans.h"
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

const char *US_Hydrodyn_SasOptionsSans::className() const
{
    return "US_Hydrodyn_SasOptionsSans";
}

QMetaObject *US_Hydrodyn_SasOptionsSans::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SasOptionsSans( "US_Hydrodyn_SasOptionsSans", &US_Hydrodyn_SasOptionsSans::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SasOptionsSans::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsSans", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SasOptionsSans::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsSans", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SasOptionsSans::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_h_scat_len", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_d_scat_len", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_h2o_scat_len_dens", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_d2o_scat_len_dens", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_d2o_conc", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_frac_of_exch_pep", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_perdeuteration", 1, param_slot_7 };
    static const QUMethod slot_8 = {"set_sans_iq_native_debye", 0, 0 };
    static const QUMethod slot_9 = {"set_sans_iq_native_hybrid", 0, 0 };
    static const QUMethod slot_10 = {"set_sans_iq_native_hybrid2", 0, 0 };
    static const QUMethod slot_11 = {"set_sans_iq_native_hybrid3", 0, 0 };
    static const QUMethod slot_12 = {"set_sans_iq_hybrid_adaptive", 0, 0 };
    static const QUMethod slot_13 = {"set_sans_iq_native_fast", 0, 0 };
    static const QUMethod slot_14 = {"set_sans_iq_native_fast_compute_pr", 0, 0 };
    static const QUMethod slot_15 = {"set_sans_iq_cryson", 0, 0 };
    static const QUMethod slot_16 = {"set_cryson_manual_hs", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_cryson_sh_max_harmonics", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_cryson_sh_fibonacci_grid_order", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_cryson_hydration_shell_contrast", 1, param_slot_19 };
    static const QUMethod slot_20 = {"cancel", 0, 0 };
    static const QUMethod slot_21 = {"help", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_22 = {"closeEvent", 1, param_slot_22 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_h_scat_len(double)", &slot_1, QMetaData::Private },
	{ "update_d_scat_len(double)", &slot_2, QMetaData::Private },
	{ "update_h2o_scat_len_dens(double)", &slot_3, QMetaData::Private },
	{ "update_d2o_scat_len_dens(double)", &slot_4, QMetaData::Private },
	{ "update_d2o_conc(double)", &slot_5, QMetaData::Private },
	{ "update_frac_of_exch_pep(double)", &slot_6, QMetaData::Private },
	{ "update_perdeuteration(double)", &slot_7, QMetaData::Private },
	{ "set_sans_iq_native_debye()", &slot_8, QMetaData::Private },
	{ "set_sans_iq_native_hybrid()", &slot_9, QMetaData::Private },
	{ "set_sans_iq_native_hybrid2()", &slot_10, QMetaData::Private },
	{ "set_sans_iq_native_hybrid3()", &slot_11, QMetaData::Private },
	{ "set_sans_iq_hybrid_adaptive()", &slot_12, QMetaData::Private },
	{ "set_sans_iq_native_fast()", &slot_13, QMetaData::Private },
	{ "set_sans_iq_native_fast_compute_pr()", &slot_14, QMetaData::Private },
	{ "set_sans_iq_cryson()", &slot_15, QMetaData::Private },
	{ "set_cryson_manual_hs()", &slot_16, QMetaData::Private },
	{ "update_cryson_sh_max_harmonics(double)", &slot_17, QMetaData::Private },
	{ "update_cryson_sh_fibonacci_grid_order(double)", &slot_18, QMetaData::Private },
	{ "update_cryson_hydration_shell_contrast(double)", &slot_19, QMetaData::Private },
	{ "cancel()", &slot_20, QMetaData::Private },
	{ "help()", &slot_21, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_22, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SasOptionsSans", parentObject,
	slot_tbl, 23,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SasOptionsSans.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SasOptionsSans::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SasOptionsSans" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SasOptionsSans::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_h_scat_len((double)static_QUType_double.get(_o+1)); break;
    case 2: update_d_scat_len((double)static_QUType_double.get(_o+1)); break;
    case 3: update_h2o_scat_len_dens((double)static_QUType_double.get(_o+1)); break;
    case 4: update_d2o_scat_len_dens((double)static_QUType_double.get(_o+1)); break;
    case 5: update_d2o_conc((double)static_QUType_double.get(_o+1)); break;
    case 6: update_frac_of_exch_pep((double)static_QUType_double.get(_o+1)); break;
    case 7: update_perdeuteration((double)static_QUType_double.get(_o+1)); break;
    case 8: set_sans_iq_native_debye(); break;
    case 9: set_sans_iq_native_hybrid(); break;
    case 10: set_sans_iq_native_hybrid2(); break;
    case 11: set_sans_iq_native_hybrid3(); break;
    case 12: set_sans_iq_hybrid_adaptive(); break;
    case 13: set_sans_iq_native_fast(); break;
    case 14: set_sans_iq_native_fast_compute_pr(); break;
    case 15: set_sans_iq_cryson(); break;
    case 16: set_cryson_manual_hs(); break;
    case 17: update_cryson_sh_max_harmonics((double)static_QUType_double.get(_o+1)); break;
    case 18: update_cryson_sh_fibonacci_grid_order((double)static_QUType_double.get(_o+1)); break;
    case 19: update_cryson_hydration_shell_contrast((double)static_QUType_double.get(_o+1)); break;
    case 20: cancel(); break;
    case 21: help(); break;
    case 22: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SasOptionsSans::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SasOptionsSans::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SasOptionsSans::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
