/****************************************************************************
** US_Hydrodyn_SasOptionsBeadModel meta object code from reading C++ file 'us_hydrodyn_sas_options_bead_model.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_sas_options_bead_model.h"
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

const char *US_Hydrodyn_SasOptionsBeadModel::className() const
{
    return "US_Hydrodyn_SasOptionsBeadModel";
}

QMetaObject *US_Hydrodyn_SasOptionsBeadModel::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SasOptionsBeadModel( "US_Hydrodyn_SasOptionsBeadModel", &US_Hydrodyn_SasOptionsBeadModel::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SasOptionsBeadModel::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsBeadModel", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SasOptionsBeadModel::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsBeadModel", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SasOptionsBeadModel::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_compute_saxs_coeff_for_bead_models", 0, 0 };
    static const QUMethod slot_2 = {"set_compute_sans_coeff_for_bead_models", 0, 0 };
    static const QUMethod slot_3 = {"set_bead_model_rayleigh", 0, 0 };
    static const QUMethod slot_4 = {"set_bead_models_use_bead_radius_ev", 0, 0 };
    static const QUMethod slot_5 = {"set_bead_models_rho0_in_scat_factors", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_dummy_saxs_name", 1, param_slot_6 };
    static const QUMethod slot_7 = {"set_dummy_atom_pdbs_in_nm", 0, 0 };
    static const QUMethod slot_8 = {"set_iq_global_avg_for_bead_models", 0, 0 };
    static const QUMethod slot_9 = {"set_bead_models_use_quick_fitting", 0, 0 };
    static const QUMethod slot_10 = {"set_bead_models_use_gsm_fitting", 0, 0 };
    static const QUMethod slot_11 = {"set_apply_loaded_sf_repeatedly_to_pdb", 0, 0 };
    static const QUMethod slot_12 = {"set_bead_models_use_var_len_sf", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_bead_models_var_len_sf_max", 1, param_slot_13 };
    static const QUMethod slot_14 = {"cancel", 0, 0 };
    static const QUMethod slot_15 = {"help", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_16 = {"closeEvent", 1, param_slot_16 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_compute_saxs_coeff_for_bead_models()", &slot_1, QMetaData::Private },
	{ "set_compute_sans_coeff_for_bead_models()", &slot_2, QMetaData::Private },
	{ "set_bead_model_rayleigh()", &slot_3, QMetaData::Private },
	{ "set_bead_models_use_bead_radius_ev()", &slot_4, QMetaData::Private },
	{ "set_bead_models_rho0_in_scat_factors()", &slot_5, QMetaData::Private },
	{ "update_dummy_saxs_name(const QString&)", &slot_6, QMetaData::Private },
	{ "set_dummy_atom_pdbs_in_nm()", &slot_7, QMetaData::Private },
	{ "set_iq_global_avg_for_bead_models()", &slot_8, QMetaData::Private },
	{ "set_bead_models_use_quick_fitting()", &slot_9, QMetaData::Private },
	{ "set_bead_models_use_gsm_fitting()", &slot_10, QMetaData::Private },
	{ "set_apply_loaded_sf_repeatedly_to_pdb()", &slot_11, QMetaData::Private },
	{ "set_bead_models_use_var_len_sf()", &slot_12, QMetaData::Private },
	{ "update_bead_models_var_len_sf_max(const QString&)", &slot_13, QMetaData::Private },
	{ "cancel()", &slot_14, QMetaData::Private },
	{ "help()", &slot_15, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_16, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SasOptionsBeadModel", parentObject,
	slot_tbl, 17,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SasOptionsBeadModel.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SasOptionsBeadModel::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SasOptionsBeadModel" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SasOptionsBeadModel::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_compute_saxs_coeff_for_bead_models(); break;
    case 2: set_compute_sans_coeff_for_bead_models(); break;
    case 3: set_bead_model_rayleigh(); break;
    case 4: set_bead_models_use_bead_radius_ev(); break;
    case 5: set_bead_models_rho0_in_scat_factors(); break;
    case 6: update_dummy_saxs_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: set_dummy_atom_pdbs_in_nm(); break;
    case 8: set_iq_global_avg_for_bead_models(); break;
    case 9: set_bead_models_use_quick_fitting(); break;
    case 10: set_bead_models_use_gsm_fitting(); break;
    case 11: set_apply_loaded_sf_repeatedly_to_pdb(); break;
    case 12: set_bead_models_use_var_len_sf(); break;
    case 13: update_bead_models_var_len_sf_max((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: cancel(); break;
    case 15: help(); break;
    case 16: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SasOptionsBeadModel::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SasOptionsBeadModel::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SasOptionsBeadModel::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
