/****************************************************************************
** US_Hydrodyn_SasOptionsExperimental meta object code from reading C++ file 'us_hydrodyn_sas_options_experimental.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_sas_options_experimental.h"
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

const char *US_Hydrodyn_SasOptionsExperimental::className() const
{
    return "US_Hydrodyn_SasOptionsExperimental";
}

QMetaObject *US_Hydrodyn_SasOptionsExperimental::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SasOptionsExperimental( "US_Hydrodyn_SasOptionsExperimental", &US_Hydrodyn_SasOptionsExperimental::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SasOptionsExperimental::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsExperimental", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SasOptionsExperimental::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsExperimental", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SasOptionsExperimental::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_iqq_scale_linear_offset", 0, 0 };
    static const QUMethod slot_2 = {"set_autocorrelate", 0, 0 };
    static const QUMethod slot_3 = {"set_hybrid_radius_excl_vol", 0, 0 };
    static const QUMethod slot_4 = {"set_subtract_radius", 0, 0 };
    static const QUMethod slot_5 = {"set_iqq_use_atomic_ff", 0, 0 };
    static const QUMethod slot_6 = {"set_iqq_use_saxs_excl_vol", 0, 0 };
    static const QUMethod slot_7 = {"set_use_somo_ff", 0, 0 };
    static const QUMethod slot_8 = {"set_iqq_scale_nnls", 0, 0 };
    static const QUMethod slot_9 = {"set_iqq_log_fitting", 0, 0 };
    static const QUMethod slot_10 = {"set_iqq_scale_play", 0, 0 };
    static const QUMethod slot_11 = {"set_alt_ff", 0, 0 };
    static const QUMethod slot_12 = {"set_five_term_gaussians", 0, 0 };
    static const QUMethod slot_13 = {"set_iq_exact_q", 0, 0 };
    static const QUMethod slot_14 = {"set_compute_exponentials", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_compute_exponential_terms", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_ev_exp_mult", 1, param_slot_16 };
    static const QUMethod slot_17 = {"set_compute_chi2shannon", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_chi2shannon_dmax", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_chi2shannon_k", 1, param_slot_19 };
    static const QUMethod slot_20 = {"set_alt_sh1", 0, 0 };
    static const QUMethod slot_21 = {"set_alt_sh2", 0, 0 };
    static const QUMethod slot_22 = {"set_create_shd", 0, 0 };
    static const QUMethod slot_23 = {"set_multiply_iq_by_atomic_volume", 0, 0 };
    static const QUMethod slot_24 = {"create_somo_ff", 0, 0 };
    static const QUMethod slot_25 = {"cancel", 0, 0 };
    static const QUMethod slot_26 = {"help", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_27 = {"closeEvent", 1, param_slot_27 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_iqq_scale_linear_offset()", &slot_1, QMetaData::Private },
	{ "set_autocorrelate()", &slot_2, QMetaData::Private },
	{ "set_hybrid_radius_excl_vol()", &slot_3, QMetaData::Private },
	{ "set_subtract_radius()", &slot_4, QMetaData::Private },
	{ "set_iqq_use_atomic_ff()", &slot_5, QMetaData::Private },
	{ "set_iqq_use_saxs_excl_vol()", &slot_6, QMetaData::Private },
	{ "set_use_somo_ff()", &slot_7, QMetaData::Private },
	{ "set_iqq_scale_nnls()", &slot_8, QMetaData::Private },
	{ "set_iqq_log_fitting()", &slot_9, QMetaData::Private },
	{ "set_iqq_scale_play()", &slot_10, QMetaData::Private },
	{ "set_alt_ff()", &slot_11, QMetaData::Private },
	{ "set_five_term_gaussians()", &slot_12, QMetaData::Private },
	{ "set_iq_exact_q()", &slot_13, QMetaData::Private },
	{ "set_compute_exponentials()", &slot_14, QMetaData::Private },
	{ "update_compute_exponential_terms(double)", &slot_15, QMetaData::Private },
	{ "update_ev_exp_mult(const QString&)", &slot_16, QMetaData::Private },
	{ "set_compute_chi2shannon()", &slot_17, QMetaData::Private },
	{ "update_chi2shannon_dmax(const QString&)", &slot_18, QMetaData::Private },
	{ "update_chi2shannon_k(const QString&)", &slot_19, QMetaData::Private },
	{ "set_alt_sh1()", &slot_20, QMetaData::Private },
	{ "set_alt_sh2()", &slot_21, QMetaData::Private },
	{ "set_create_shd()", &slot_22, QMetaData::Private },
	{ "set_multiply_iq_by_atomic_volume()", &slot_23, QMetaData::Private },
	{ "create_somo_ff()", &slot_24, QMetaData::Private },
	{ "cancel()", &slot_25, QMetaData::Private },
	{ "help()", &slot_26, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_27, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SasOptionsExperimental", parentObject,
	slot_tbl, 28,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SasOptionsExperimental.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SasOptionsExperimental::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SasOptionsExperimental" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SasOptionsExperimental::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_iqq_scale_linear_offset(); break;
    case 2: set_autocorrelate(); break;
    case 3: set_hybrid_radius_excl_vol(); break;
    case 4: set_subtract_radius(); break;
    case 5: set_iqq_use_atomic_ff(); break;
    case 6: set_iqq_use_saxs_excl_vol(); break;
    case 7: set_use_somo_ff(); break;
    case 8: set_iqq_scale_nnls(); break;
    case 9: set_iqq_log_fitting(); break;
    case 10: set_iqq_scale_play(); break;
    case 11: set_alt_ff(); break;
    case 12: set_five_term_gaussians(); break;
    case 13: set_iq_exact_q(); break;
    case 14: set_compute_exponentials(); break;
    case 15: update_compute_exponential_terms((double)static_QUType_double.get(_o+1)); break;
    case 16: update_ev_exp_mult((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: set_compute_chi2shannon(); break;
    case 18: update_chi2shannon_dmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: update_chi2shannon_k((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: set_alt_sh1(); break;
    case 21: set_alt_sh2(); break;
    case 22: set_create_shd(); break;
    case 23: set_multiply_iq_by_atomic_volume(); break;
    case 24: create_somo_ff(); break;
    case 25: cancel(); break;
    case 26: help(); break;
    case 27: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SasOptionsExperimental::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SasOptionsExperimental::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SasOptionsExperimental::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
