/****************************************************************************
** US_Hydrodyn_SasOptionsGuinier meta object code from reading C++ file 'us_hydrodyn_sas_options_guinier.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_sas_options_guinier.h"
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

const char *US_Hydrodyn_SasOptionsGuinier::className() const
{
    return "US_Hydrodyn_SasOptionsGuinier";
}

QMetaObject *US_Hydrodyn_SasOptionsGuinier::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SasOptionsGuinier( "US_Hydrodyn_SasOptionsGuinier", &US_Hydrodyn_SasOptionsGuinier::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SasOptionsGuinier::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsGuinier", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SasOptionsGuinier::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsGuinier", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SasOptionsGuinier::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_guinier_csv", 0, 0 };
    static const QUMethod slot_2 = {"set_guinier_csv_save_data", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_qRgmax", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_qstart", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_qend", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_cs_qRgmax", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_cs_qstart", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_cs_qend", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_Rt_qRtmax", 1, param_slot_9 };
    static const QUMethod slot_10 = {"set_guinier_auto_fit", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_pointsmin", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_pointsmax", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_guinier_csv_filename", 1, param_slot_13 };
    static const QUMethod slot_14 = {"curve_conc", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_conc", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_psv", 1, param_slot_16 };
    static const QUMethod slot_17 = {"set_use_cs_psv", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_cs_psv", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_I0_exp", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_I0_theo", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_diffusion_len", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_electron_nucleon_ratio", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_nucleon_mass", 1, param_slot_23 };
    static const QUMethod slot_24 = {"set_guinier_outlier_reject", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_guinier_outlier_reject_dist", 1, param_slot_25 };
    static const QUMethod slot_26 = {"set_guinier_use_sd", 0, 0 };
    static const QUMethod slot_27 = {"set_guinier_use_qRlimit", 0, 0 };
    static const QUMethod slot_28 = {"set_guinier_use_standards", 0, 0 };
    static const QUMethod slot_29 = {"guinier", 0, 0 };
    static const QUMethod slot_30 = {"cs_guinier", 0, 0 };
    static const QUMethod slot_31 = {"Rt_guinier", 0, 0 };
    static const QUMethod slot_32 = {"cancel", 0, 0 };
    static const QUMethod slot_33 = {"help", 0, 0 };
    static const QUParameter param_slot_34[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_34 = {"closeEvent", 1, param_slot_34 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_guinier_csv()", &slot_1, QMetaData::Private },
	{ "set_guinier_csv_save_data()", &slot_2, QMetaData::Private },
	{ "update_qRgmax(const QString&)", &slot_3, QMetaData::Private },
	{ "update_qstart(const QString&)", &slot_4, QMetaData::Private },
	{ "update_qend(const QString&)", &slot_5, QMetaData::Private },
	{ "update_cs_qRgmax(const QString&)", &slot_6, QMetaData::Private },
	{ "update_cs_qstart(const QString&)", &slot_7, QMetaData::Private },
	{ "update_cs_qend(const QString&)", &slot_8, QMetaData::Private },
	{ "update_Rt_qRtmax(const QString&)", &slot_9, QMetaData::Private },
	{ "set_guinier_auto_fit()", &slot_10, QMetaData::Private },
	{ "update_pointsmin(const QString&)", &slot_11, QMetaData::Private },
	{ "update_pointsmax(const QString&)", &slot_12, QMetaData::Private },
	{ "update_guinier_csv_filename(const QString&)", &slot_13, QMetaData::Private },
	{ "curve_conc()", &slot_14, QMetaData::Private },
	{ "update_conc(const QString&)", &slot_15, QMetaData::Private },
	{ "update_psv(const QString&)", &slot_16, QMetaData::Private },
	{ "set_use_cs_psv()", &slot_17, QMetaData::Private },
	{ "update_cs_psv(const QString&)", &slot_18, QMetaData::Private },
	{ "update_I0_exp(const QString&)", &slot_19, QMetaData::Private },
	{ "update_I0_theo(const QString&)", &slot_20, QMetaData::Private },
	{ "update_diffusion_len(const QString&)", &slot_21, QMetaData::Private },
	{ "update_electron_nucleon_ratio(const QString&)", &slot_22, QMetaData::Private },
	{ "update_nucleon_mass(const QString&)", &slot_23, QMetaData::Private },
	{ "set_guinier_outlier_reject()", &slot_24, QMetaData::Private },
	{ "update_guinier_outlier_reject_dist(const QString&)", &slot_25, QMetaData::Private },
	{ "set_guinier_use_sd()", &slot_26, QMetaData::Private },
	{ "set_guinier_use_qRlimit()", &slot_27, QMetaData::Private },
	{ "set_guinier_use_standards()", &slot_28, QMetaData::Private },
	{ "guinier()", &slot_29, QMetaData::Private },
	{ "cs_guinier()", &slot_30, QMetaData::Private },
	{ "Rt_guinier()", &slot_31, QMetaData::Private },
	{ "cancel()", &slot_32, QMetaData::Private },
	{ "help()", &slot_33, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_34, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SasOptionsGuinier", parentObject,
	slot_tbl, 35,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SasOptionsGuinier.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SasOptionsGuinier::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SasOptionsGuinier" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SasOptionsGuinier::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_guinier_csv(); break;
    case 2: set_guinier_csv_save_data(); break;
    case 3: update_qRgmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_qstart((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_qend((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_cs_qRgmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_cs_qstart((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_cs_qend((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_Rt_qRtmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: set_guinier_auto_fit(); break;
    case 11: update_pointsmin((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_pointsmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_guinier_csv_filename((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: curve_conc(); break;
    case 15: update_conc((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_psv((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: set_use_cs_psv(); break;
    case 18: update_cs_psv((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: update_I0_exp((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: update_I0_theo((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: update_diffusion_len((const QString&)static_QUType_QString.get(_o+1)); break;
    case 22: update_electron_nucleon_ratio((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: update_nucleon_mass((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: set_guinier_outlier_reject(); break;
    case 25: update_guinier_outlier_reject_dist((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: set_guinier_use_sd(); break;
    case 27: set_guinier_use_qRlimit(); break;
    case 28: set_guinier_use_standards(); break;
    case 29: guinier(); break;
    case 30: cs_guinier(); break;
    case 31: Rt_guinier(); break;
    case 32: cancel(); break;
    case 33: help(); break;
    case 34: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SasOptionsGuinier::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SasOptionsGuinier::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SasOptionsGuinier::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
