/****************************************************************************
** US_Hydrodyn_Saxs_Hplc_Svd meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_svd.h'
**
** Created: Tue Feb 18 11:30:10 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_hplc_svd.h"
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

const char *US_Hydrodyn_Saxs_Hplc_Svd::className() const
{
    return "US_Hydrodyn_Saxs_Hplc_Svd";
}

QMetaObject *US_Hydrodyn_Saxs_Hplc_Svd::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Hplc_Svd( "US_Hydrodyn_Saxs_Hplc_Svd", &US_Hydrodyn_Saxs_Hplc_Svd::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Hplc_Svd::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Svd", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Hplc_Svd::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Svd", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Hplc_Svd::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"data_selection_changed", 0, 0 };
    static const QUMethod slot_1 = {"clear", 0, 0 };
    static const QUMethod slot_2 = {"to_hplc", 0, 0 };
    static const QUMethod slot_3 = {"color_rotate", 0, 0 };
    static const QUMethod slot_4 = {"replot", 0, 0 };
    static const QUMethod slot_5 = {"hide_data", 0, 0 };
    static const QUMethod slot_6 = {"clear_display", 0, 0 };
    static const QUMethod slot_7 = {"update_font", 0, 0 };
    static const QUMethod slot_8 = {"save", 0, 0 };
    static const QUMethod slot_9 = {"hide_editor", 0, 0 };
    static const QUMethod slot_10 = {"set_plot_errors", 0, 0 };
    static const QUMethod slot_11 = {"set_plot_errors_rev", 0, 0 };
    static const QUMethod slot_12 = {"set_plot_errors_sd", 0, 0 };
    static const QUMethod slot_13 = {"set_plot_errors_pct", 0, 0 };
    static const QUMethod slot_14 = {"set_plot_errors_ref", 0, 0 };
    static const QUMethod slot_15 = {"set_plot_errors_group", 0, 0 };
    static const QUMethod slot_16 = {"axis_x", 0, 0 };
    static const QUMethod slot_17 = {"axis_y", 0, 0 };
    static const QUMethod slot_18 = {"iq_it", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"q_start_text", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"q_end_text", 1, param_slot_20 };
    static const QUMethod slot_21 = {"sv_selection_changed", 0, 0 };
    static const QUMethod slot_22 = {"svd", 0, 0 };
    static const QUMethod slot_23 = {"stop", 0, 0 };
    static const QUMethod slot_24 = {"svd_plot", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ "axis_change", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"svd_plot", 1, param_slot_25 };
    static const QUMethod slot_26 = {"svd_save", 0, 0 };
    static const QUMethod slot_27 = {"recon", 0, 0 };
    static const QUMethod slot_28 = {"inc_rmsd_plot", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ "axis_change", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"inc_rmsd_plot", 1, param_slot_29 };
    static const QUMethod slot_30 = {"rmsd_save", 0, 0 };
    static const QUMethod slot_31 = {"inc_chi_plot", 0, 0 };
    static const QUParameter param_slot_32[] = {
	{ "axis_change", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"inc_chi_plot", 1, param_slot_32 };
    static const QUMethod slot_33 = {"inc_recon", 0, 0 };
    static const QUMethod slot_34 = {"indiv_recon", 0, 0 };
    static const QUMethod slot_35 = {"hide_process", 0, 0 };
    static const QUMethod slot_36 = {"help", 0, 0 };
    static const QUMethod slot_37 = {"cancel", 0, 0 };
    static const QUMethod slot_38 = {"setupGUI", 0, 0 };
    static const QUMethod slot_39 = {"update_enables", 0, 0 };
    static const QUParameter param_slot_40[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_40 = {"closeEvent", 1, param_slot_40 };
    static const QMetaData slot_tbl[] = {
	{ "data_selection_changed()", &slot_0, QMetaData::Private },
	{ "clear()", &slot_1, QMetaData::Private },
	{ "to_hplc()", &slot_2, QMetaData::Private },
	{ "color_rotate()", &slot_3, QMetaData::Private },
	{ "replot()", &slot_4, QMetaData::Private },
	{ "hide_data()", &slot_5, QMetaData::Private },
	{ "clear_display()", &slot_6, QMetaData::Private },
	{ "update_font()", &slot_7, QMetaData::Private },
	{ "save()", &slot_8, QMetaData::Private },
	{ "hide_editor()", &slot_9, QMetaData::Private },
	{ "set_plot_errors()", &slot_10, QMetaData::Private },
	{ "set_plot_errors_rev()", &slot_11, QMetaData::Private },
	{ "set_plot_errors_sd()", &slot_12, QMetaData::Private },
	{ "set_plot_errors_pct()", &slot_13, QMetaData::Private },
	{ "set_plot_errors_ref()", &slot_14, QMetaData::Private },
	{ "set_plot_errors_group()", &slot_15, QMetaData::Private },
	{ "axis_x()", &slot_16, QMetaData::Private },
	{ "axis_y()", &slot_17, QMetaData::Private },
	{ "iq_it()", &slot_18, QMetaData::Private },
	{ "q_start_text(const QString&)", &slot_19, QMetaData::Private },
	{ "q_end_text(const QString&)", &slot_20, QMetaData::Private },
	{ "sv_selection_changed()", &slot_21, QMetaData::Private },
	{ "svd()", &slot_22, QMetaData::Private },
	{ "stop()", &slot_23, QMetaData::Private },
	{ "svd_plot()", &slot_24, QMetaData::Private },
	{ "svd_plot(bool)", &slot_25, QMetaData::Private },
	{ "svd_save()", &slot_26, QMetaData::Private },
	{ "recon()", &slot_27, QMetaData::Private },
	{ "inc_rmsd_plot()", &slot_28, QMetaData::Private },
	{ "inc_rmsd_plot(bool)", &slot_29, QMetaData::Private },
	{ "rmsd_save()", &slot_30, QMetaData::Private },
	{ "inc_chi_plot()", &slot_31, QMetaData::Private },
	{ "inc_chi_plot(bool)", &slot_32, QMetaData::Private },
	{ "inc_recon()", &slot_33, QMetaData::Private },
	{ "indiv_recon()", &slot_34, QMetaData::Private },
	{ "hide_process()", &slot_35, QMetaData::Private },
	{ "help()", &slot_36, QMetaData::Private },
	{ "cancel()", &slot_37, QMetaData::Private },
	{ "setupGUI()", &slot_38, QMetaData::Private },
	{ "update_enables()", &slot_39, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_40, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Hplc_Svd", parentObject,
	slot_tbl, 41,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Hplc_Svd.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Hplc_Svd::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Hplc_Svd" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Hplc_Svd::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: data_selection_changed(); break;
    case 1: clear(); break;
    case 2: to_hplc(); break;
    case 3: color_rotate(); break;
    case 4: replot(); break;
    case 5: hide_data(); break;
    case 6: clear_display(); break;
    case 7: update_font(); break;
    case 8: save(); break;
    case 9: hide_editor(); break;
    case 10: set_plot_errors(); break;
    case 11: set_plot_errors_rev(); break;
    case 12: set_plot_errors_sd(); break;
    case 13: set_plot_errors_pct(); break;
    case 14: set_plot_errors_ref(); break;
    case 15: set_plot_errors_group(); break;
    case 16: axis_x(); break;
    case 17: axis_y(); break;
    case 18: iq_it(); break;
    case 19: q_start_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: q_end_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: sv_selection_changed(); break;
    case 22: svd(); break;
    case 23: stop(); break;
    case 24: svd_plot(); break;
    case 25: svd_plot((bool)static_QUType_bool.get(_o+1)); break;
    case 26: svd_save(); break;
    case 27: recon(); break;
    case 28: inc_rmsd_plot(); break;
    case 29: inc_rmsd_plot((bool)static_QUType_bool.get(_o+1)); break;
    case 30: rmsd_save(); break;
    case 31: inc_chi_plot(); break;
    case 32: inc_chi_plot((bool)static_QUType_bool.get(_o+1)); break;
    case 33: inc_recon(); break;
    case 34: indiv_recon(); break;
    case 35: hide_process(); break;
    case 36: help(); break;
    case 37: cancel(); break;
    case 38: setupGUI(); break;
    case 39: update_enables(); break;
    case 40: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Hplc_Svd::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Hplc_Svd::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Hplc_Svd::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
