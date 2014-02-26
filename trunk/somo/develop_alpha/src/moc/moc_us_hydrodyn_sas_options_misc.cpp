/****************************************************************************
** US_Hydrodyn_SasOptionsMisc meta object code from reading C++ file 'us_hydrodyn_sas_options_misc.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_sas_options_misc.h"
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

const char *US_Hydrodyn_SasOptionsMisc::className() const
{
    return "US_Hydrodyn_SasOptionsMisc";
}

QMetaObject *US_Hydrodyn_SasOptionsMisc::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SasOptionsMisc( "US_Hydrodyn_SasOptionsMisc", &US_Hydrodyn_SasOptionsMisc::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SasOptionsMisc::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsMisc", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SasOptionsMisc::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsMisc", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SasOptionsMisc::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"default_atom_filename", 0, 0 };
    static const QUMethod slot_2 = {"default_hybrid_filename", 0, 0 };
    static const QUMethod slot_3 = {"default_saxs_filename", 0, 0 };
    static const QUMethod slot_4 = {"default_ff_filename", 0, 0 };
    static const QUMethod slot_5 = {"set_iq_ask", 0, 0 };
    static const QUMethod slot_6 = {"set_iq_scale_ask", 0, 0 };
    static const QUMethod slot_7 = {"set_iqq_ask_target_grid", 0, 0 };
    static const QUMethod slot_8 = {"set_iq_scale_angstrom", 0, 0 };
    static const QUMethod slot_9 = {"set_iq_scale_nm", 0, 0 };
    static const QUMethod slot_10 = {"set_iqq_expt_data_contains_variances", 0, 0 };
    static const QUMethod slot_11 = {"set_iqq_kratky_fit", 0, 0 };
    static const QUMethod slot_12 = {"set_ignore_errors", 0, 0 };
    static const QUMethod slot_13 = {"set_disable_iq_scaling", 0, 0 };
    static const QUMethod slot_14 = {"set_iqq_scale_chi2_fitting", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_swh_excl_vol", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_scale_excl_vol", 1, param_slot_16 };
    static const QUMethod slot_17 = {"set_use_iq_target_ev", 0, 0 };
    static const QUMethod slot_18 = {"set_set_iq_target_ev_from_vbar", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_iq_target_ev", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_iqq_scale_minq", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_iqq_scale_maxq", 1, param_slot_21 };
    static const QUMethod slot_22 = {"clear_mw_cache", 0, 0 };
    static const QUMethod slot_23 = {"cancel", 0, 0 };
    static const QUMethod slot_24 = {"help", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_25 = {"closeEvent", 1, param_slot_25 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "default_atom_filename()", &slot_1, QMetaData::Private },
	{ "default_hybrid_filename()", &slot_2, QMetaData::Private },
	{ "default_saxs_filename()", &slot_3, QMetaData::Private },
	{ "default_ff_filename()", &slot_4, QMetaData::Private },
	{ "set_iq_ask()", &slot_5, QMetaData::Private },
	{ "set_iq_scale_ask()", &slot_6, QMetaData::Private },
	{ "set_iqq_ask_target_grid()", &slot_7, QMetaData::Private },
	{ "set_iq_scale_angstrom()", &slot_8, QMetaData::Private },
	{ "set_iq_scale_nm()", &slot_9, QMetaData::Private },
	{ "set_iqq_expt_data_contains_variances()", &slot_10, QMetaData::Private },
	{ "set_iqq_kratky_fit()", &slot_11, QMetaData::Private },
	{ "set_ignore_errors()", &slot_12, QMetaData::Private },
	{ "set_disable_iq_scaling()", &slot_13, QMetaData::Private },
	{ "set_iqq_scale_chi2_fitting()", &slot_14, QMetaData::Private },
	{ "update_swh_excl_vol(const QString&)", &slot_15, QMetaData::Private },
	{ "update_scale_excl_vol(double)", &slot_16, QMetaData::Private },
	{ "set_use_iq_target_ev()", &slot_17, QMetaData::Private },
	{ "set_set_iq_target_ev_from_vbar()", &slot_18, QMetaData::Private },
	{ "update_iq_target_ev(const QString&)", &slot_19, QMetaData::Private },
	{ "update_iqq_scale_minq(const QString&)", &slot_20, QMetaData::Private },
	{ "update_iqq_scale_maxq(const QString&)", &slot_21, QMetaData::Private },
	{ "clear_mw_cache()", &slot_22, QMetaData::Private },
	{ "cancel()", &slot_23, QMetaData::Private },
	{ "help()", &slot_24, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_25, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SasOptionsMisc", parentObject,
	slot_tbl, 26,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SasOptionsMisc.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SasOptionsMisc::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SasOptionsMisc" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SasOptionsMisc::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: default_atom_filename(); break;
    case 2: default_hybrid_filename(); break;
    case 3: default_saxs_filename(); break;
    case 4: default_ff_filename(); break;
    case 5: set_iq_ask(); break;
    case 6: set_iq_scale_ask(); break;
    case 7: set_iqq_ask_target_grid(); break;
    case 8: set_iq_scale_angstrom(); break;
    case 9: set_iq_scale_nm(); break;
    case 10: set_iqq_expt_data_contains_variances(); break;
    case 11: set_iqq_kratky_fit(); break;
    case 12: set_ignore_errors(); break;
    case 13: set_disable_iq_scaling(); break;
    case 14: set_iqq_scale_chi2_fitting(); break;
    case 15: update_swh_excl_vol((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_scale_excl_vol((double)static_QUType_double.get(_o+1)); break;
    case 17: set_use_iq_target_ev(); break;
    case 18: set_set_iq_target_ev_from_vbar(); break;
    case 19: update_iq_target_ev((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: update_iqq_scale_minq((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: update_iqq_scale_maxq((const QString&)static_QUType_QString.get(_o+1)); break;
    case 22: clear_mw_cache(); break;
    case 23: cancel(); break;
    case 24: help(); break;
    case 25: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SasOptionsMisc::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SasOptionsMisc::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SasOptionsMisc::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
