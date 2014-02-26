/****************************************************************************
** US_Hydrodyn_Saxs meta object code from reading C++ file 'us_hydrodyn_saxs.h'
**
** Created: Fri Feb 21 12:53:42 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs.h"
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

const char *US_Hydrodyn_Saxs::className() const
{
    return "US_Hydrodyn_Saxs";
}

QMetaObject *US_Hydrodyn_Saxs::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs( "US_Hydrodyn_Saxs", &US_Hydrodyn_Saxs::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"manual_guinier_process", 0, 0 };
    static const QUMethod slot_1 = {"set_manual_guinier", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"adjust_wheel", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"manual_guinier_fit_start_text", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"manual_guinier_fit_end_text", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"manual_guinier_fit_start_focus", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"manual_guinier_fit_end_focus", 1, param_slot_6 };
    static const QUMethod slot_7 = {"set_resid_pct", 0, 0 };
    static const QUMethod slot_8 = {"set_resid_sd", 0, 0 };
    static const QUMethod slot_9 = {"set_resid_show_errorbars", 0, 0 };
    static const QUMethod slot_10 = {"set_resid_show", 0, 0 };
    static const QUMethod slot_11 = {"hide_pr", 0, 0 };
    static const QUMethod slot_12 = {"hide_iq", 0, 0 };
    static const QUMethod slot_13 = {"hide_settings", 0, 0 };
    static const QUMethod slot_14 = {"foxs_readFromStdout", 0, 0 };
    static const QUMethod slot_15 = {"foxs_readFromStderr", 0, 0 };
    static const QUMethod slot_16 = {"foxs_launchFinished", 0, 0 };
    static const QUMethod slot_17 = {"foxs_processExited", 0, 0 };
    static const QUMethod slot_18 = {"crysol_readFromStdout", 0, 0 };
    static const QUMethod slot_19 = {"crysol_readFromStderr", 0, 0 };
    static const QUMethod slot_20 = {"crysol_launchFinished", 0, 0 };
    static const QUMethod slot_21 = {"crysol_processExited", 0, 0 };
    static const QUMethod slot_22 = {"crysol_wroteToStdin", 0, 0 };
    static const QUMethod slot_23 = {"crysol_timeout", 0, 0 };
    static const QUMethod slot_24 = {"cryson_readFromStdout", 0, 0 };
    static const QUMethod slot_25 = {"cryson_readFromStderr", 0, 0 };
    static const QUMethod slot_26 = {"cryson_launchFinished", 0, 0 };
    static const QUMethod slot_27 = {"cryson_processExited", 0, 0 };
    static const QUMethod slot_28 = {"sastbx_readFromStdout", 0, 0 };
    static const QUMethod slot_29 = {"sastbx_readFromStderr", 0, 0 };
    static const QUMethod slot_30 = {"sastbx_launchFinished", 0, 0 };
    static const QUMethod slot_31 = {"sastbx_processExited", 0, 0 };
    static const QUParameter param_slot_32[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ "sa", &static_QUType_ptr, "saxs", QUParameter::InOut },
	{ "sh", &static_QUType_ptr, "saxs", QUParameter::InOut },
	{ "nr", &static_QUType_QString, 0, QUParameter::InOut },
	{ "na", &static_QUType_QString, 0, QUParameter::InOut },
	{ "naf", &static_QUType_QString, 0, QUParameter::InOut },
	{ "h", &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ "q", &static_QUType_double, 0, QUParameter::In },
	{ "q_o_4pi2", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"compute_ff", 9, param_slot_32 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ "s", &static_QUType_ptr, "saxs", QUParameter::InOut },
	{ "q_o_4pi2", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_33 = {"compute_ff_bead_model", 3, param_slot_33 };
    static const QUMethod slot_34 = {"fix_sas_options", 0, 0 };
    static const QUMethod slot_35 = {"clear_guinier", 0, 0 };
    static const QUMethod slot_36 = {"clear_cs_guinier", 0, 0 };
    static const QUMethod slot_37 = {"clear_Rt_guinier", 0, 0 };
    static const QUMethod slot_38 = {"show_plot_saxs_sans", 0, 0 };
    static const QUMethod slot_39 = {"show_plot_pr", 0, 0 };
    static const QUMethod slot_40 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_41 = {"set_saxs_sans", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_42 = {"set_saxs_iq", 1, param_slot_42 };
    static const QUParameter param_slot_43[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_43 = {"set_sans_iq", 1, param_slot_43 };
    static const QUMethod slot_44 = {"load_saxs_sans", 0, 0 };
    static const QUMethod slot_45 = {"load_plot_saxs", 0, 0 };
    static const QUMethod slot_46 = {"set_grid", 0, 0 };
    static const QUMethod slot_47 = {"show_plot_saxs", 0, 0 };
    static const QUMethod slot_48 = {"load_saxs", 0, 0 };
    static const QUParameter param_slot_49[] = {
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_49 = {"load_saxs", 1, param_slot_49 };
    static const QUParameter param_slot_50[] = {
	{ "filename", &static_QUType_QString, 0, QUParameter::In },
	{ "just_plotted_curves", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_50 = {"load_saxs", 2, param_slot_50 };
    static const QUMethod slot_51 = {"clear_plot_saxs", 0, 0 };
    static const QUParameter param_slot_52[] = {
	{ "quiet", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_52 = {"clear_plot_saxs", 1, param_slot_52 };
    static const QUMethod slot_53 = {"show_plot_sans", 0, 0 };
    static const QUMethod slot_54 = {"load_sans", 0, 0 };
    static const QUParameter param_slot_55[] = {
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_55 = {"load_sans", 1, param_slot_55 };
    static const QUParameter param_slot_56[] = {
	{ "filename", &static_QUType_QString, 0, QUParameter::In },
	{ "just_plotted_curves", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_56 = {"load_sans", 2, param_slot_56 };
    static const QUParameter param_slot_57[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_57 = {"update_bin_size", 1, param_slot_57 };
    static const QUParameter param_slot_58[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_58 = {"update_smooth", 1, param_slot_58 };
    static const QUParameter param_slot_59[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_59 = {"update_guinier_cutoff", 1, param_slot_59 };
    static const QUMethod slot_60 = {"show_pr_contrib", 0, 0 };
    static const QUParameter param_slot_61[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_61 = {"update_pr_contrib_low", 1, param_slot_61 };
    static const QUParameter param_slot_62[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_62 = {"update_pr_contrib_high", 1, param_slot_62 };
    static const QUParameter param_slot_63[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_63 = {"set_curve", 1, param_slot_63 };
    static const QUMethod slot_64 = {"load_pr", 0, 0 };
    static const QUParameter param_slot_65[] = {
	{ "just_plotted_curves", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_65 = {"load_pr", 1, param_slot_65 };
    static const QUMethod slot_66 = {"load_plot_pr", 0, 0 };
    static const QUMethod slot_67 = {"clear_plot_pr", 0, 0 };
    static const QUMethod slot_68 = {"cancel", 0, 0 };
    static const QUMethod slot_69 = {"help", 0, 0 };
    static const QUMethod slot_70 = {"options", 0, 0 };
    static const QUMethod slot_71 = {"stop", 0, 0 };
    static const QUMethod slot_72 = {"clear_display", 0, 0 };
    static const QUMethod slot_73 = {"print", 0, 0 };
    static const QUMethod slot_74 = {"update_font", 0, 0 };
    static const QUMethod slot_75 = {"save", 0, 0 };
    static const QUMethod slot_76 = {"select_atom_file", 0, 0 };
    static const QUMethod slot_77 = {"select_hybrid_file", 0, 0 };
    static const QUMethod slot_78 = {"select_saxs_file", 0, 0 };
    static const QUParameter param_slot_79[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_79 = {"select_atom_file", 1, param_slot_79 };
    static const QUParameter param_slot_80[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_80 = {"select_hybrid_file", 1, param_slot_80 };
    static const QUParameter param_slot_81[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_81 = {"select_saxs_file", 1, param_slot_81 };
    static const QUParameter param_slot_82[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In }
    };
    static const QUMethod slot_82 = {"normalize_pr", 2, param_slot_82 };
    static const QUParameter param_slot_83[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ "mw", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_83 = {"normalize_pr", 3, param_slot_83 };
    static const QUMethod slot_84 = {"update_saxs_sans", 0, 0 };
    static const QUMethod slot_85 = {"guinier_window", 0, 0 };
    static const QUMethod slot_86 = {"run_guinier_analysis", 0, 0 };
    static const QUMethod slot_87 = {"run_guinier_cs", 0, 0 };
    static const QUMethod slot_88 = {"run_guinier_Rt", 0, 0 };
    static const QUParameter param_slot_89[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out }
    };
    static const QUMethod slot_89 = {"saxs_filestring", 1, param_slot_89 };
    static const QUParameter param_slot_90[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out }
    };
    static const QUMethod slot_90 = {"sprr_filestring", 1, param_slot_90 };
    static const QUMethod slot_91 = {"set_create_native_saxs", 0, 0 };
    static const QUMethod slot_92 = {"set_guinier", 0, 0 };
    static const QUMethod slot_93 = {"set_cs_guinier", 0, 0 };
    static const QUMethod slot_94 = {"set_Rt_guinier", 0, 0 };
    static const QUMethod slot_95 = {"set_pr_contrib", 0, 0 };
    static const QUMethod slot_96 = {"set_user_range", 0, 0 };
    static const QUMethod slot_97 = {"set_kratky", 0, 0 };
    static const QUParameter param_slot_98[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_98 = {"update_guinier_lowq2", 1, param_slot_98 };
    static const QUParameter param_slot_99[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_99 = {"update_guinier_highq2", 1, param_slot_99 };
    static const QUParameter param_slot_100[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_100 = {"update_user_lowq", 1, param_slot_100 };
    static const QUParameter param_slot_101[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_101 = {"update_user_highq", 1, param_slot_101 };
    static const QUParameter param_slot_102[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_102 = {"update_user_lowI", 1, param_slot_102 };
    static const QUParameter param_slot_103[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_103 = {"update_user_highI", 1, param_slot_103 };
    static const QUMethod slot_104 = {"load_gnom", 0, 0 };
    static const QUMethod slot_105 = {"ift", 0, 0 };
    static const QUMethod slot_106 = {"saxs_search", 0, 0 };
    static const QUMethod slot_107 = {"saxs_screen", 0, 0 };
    static const QUMethod slot_108 = {"saxs_buffer", 0, 0 };
    static const QUMethod slot_109 = {"saxs_hplc", 0, 0 };
    static const QUMethod slot_110 = {"saxs_xsr", 0, 0 };
    static const QUMethod slot_111 = {"saxs_1d", 0, 0 };
    static const QUMethod slot_112 = {"saxs_2d", 0, 0 };
    static const QUParameter param_slot_113[] = {
	{ 0, &static_QUType_ptr, "long", QUParameter::In }
    };
    static const QUMethod slot_113 = {"plot_saxs_clicked", 1, param_slot_113 };
    static const QUParameter param_slot_114[] = {
	{ 0, &static_QUType_ptr, "long", QUParameter::In }
    };
    static const QUMethod slot_114 = {"plot_pr_clicked", 1, param_slot_114 };
    static const QUParameter param_slot_115[] = {
	{ 0, &static_QUType_ptr, "QwtPlotItem", QUParameter::In }
    };
    static const QUMethod slot_115 = {"plot_saxs_item_clicked", 1, param_slot_115 };
    static const QUParameter param_slot_116[] = {
	{ 0, &static_QUType_ptr, "QwtPlotItem", QUParameter::In }
    };
    static const QUMethod slot_116 = {"plot_pr_item_clicked", 1, param_slot_116 };
    static const QUMethod slot_117 = {"saxs_legend", 0, 0 };
    static const QUMethod slot_118 = {"pr_legend", 0, 0 };
    static const QUParameter param_slot_119[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_119 = {"closeEvent", 1, param_slot_119 };
    static const QMetaData slot_tbl[] = {
	{ "manual_guinier_process()", &slot_0, QMetaData::Private },
	{ "set_manual_guinier()", &slot_1, QMetaData::Private },
	{ "adjust_wheel(double)", &slot_2, QMetaData::Private },
	{ "manual_guinier_fit_start_text(const QString&)", &slot_3, QMetaData::Private },
	{ "manual_guinier_fit_end_text(const QString&)", &slot_4, QMetaData::Private },
	{ "manual_guinier_fit_start_focus(bool)", &slot_5, QMetaData::Private },
	{ "manual_guinier_fit_end_focus(bool)", &slot_6, QMetaData::Private },
	{ "set_resid_pct()", &slot_7, QMetaData::Private },
	{ "set_resid_sd()", &slot_8, QMetaData::Private },
	{ "set_resid_show_errorbars()", &slot_9, QMetaData::Private },
	{ "set_resid_show()", &slot_10, QMetaData::Private },
	{ "hide_pr()", &slot_11, QMetaData::Private },
	{ "hide_iq()", &slot_12, QMetaData::Private },
	{ "hide_settings()", &slot_13, QMetaData::Private },
	{ "foxs_readFromStdout()", &slot_14, QMetaData::Private },
	{ "foxs_readFromStderr()", &slot_15, QMetaData::Private },
	{ "foxs_launchFinished()", &slot_16, QMetaData::Private },
	{ "foxs_processExited()", &slot_17, QMetaData::Private },
	{ "crysol_readFromStdout()", &slot_18, QMetaData::Private },
	{ "crysol_readFromStderr()", &slot_19, QMetaData::Private },
	{ "crysol_launchFinished()", &slot_20, QMetaData::Private },
	{ "crysol_processExited()", &slot_21, QMetaData::Private },
	{ "crysol_wroteToStdin()", &slot_22, QMetaData::Private },
	{ "crysol_timeout()", &slot_23, QMetaData::Private },
	{ "cryson_readFromStdout()", &slot_24, QMetaData::Private },
	{ "cryson_readFromStderr()", &slot_25, QMetaData::Private },
	{ "cryson_launchFinished()", &slot_26, QMetaData::Private },
	{ "cryson_processExited()", &slot_27, QMetaData::Private },
	{ "sastbx_readFromStdout()", &slot_28, QMetaData::Private },
	{ "sastbx_readFromStderr()", &slot_29, QMetaData::Private },
	{ "sastbx_launchFinished()", &slot_30, QMetaData::Private },
	{ "sastbx_processExited()", &slot_31, QMetaData::Private },
	{ "compute_ff(saxs&,saxs&,QString&,QString&,QString&,unsigned int,double,double)", &slot_32, QMetaData::Private },
	{ "compute_ff_bead_model(saxs&,double)", &slot_33, QMetaData::Private },
	{ "fix_sas_options()", &slot_34, QMetaData::Private },
	{ "clear_guinier()", &slot_35, QMetaData::Private },
	{ "clear_cs_guinier()", &slot_36, QMetaData::Private },
	{ "clear_Rt_guinier()", &slot_37, QMetaData::Private },
	{ "show_plot_saxs_sans()", &slot_38, QMetaData::Public },
	{ "show_plot_pr()", &slot_39, QMetaData::Public },
	{ "setupGUI()", &slot_40, QMetaData::Private },
	{ "set_saxs_sans(int)", &slot_41, QMetaData::Private },
	{ "set_saxs_iq(int)", &slot_42, QMetaData::Private },
	{ "set_sans_iq(int)", &slot_43, QMetaData::Private },
	{ "load_saxs_sans()", &slot_44, QMetaData::Private },
	{ "load_plot_saxs()", &slot_45, QMetaData::Private },
	{ "set_grid()", &slot_46, QMetaData::Private },
	{ "show_plot_saxs()", &slot_47, QMetaData::Private },
	{ "load_saxs()", &slot_48, QMetaData::Private },
	{ "load_saxs(QString)", &slot_49, QMetaData::Private },
	{ "load_saxs(QString,bool)", &slot_50, QMetaData::Private },
	{ "clear_plot_saxs()", &slot_51, QMetaData::Private },
	{ "clear_plot_saxs(bool)", &slot_52, QMetaData::Private },
	{ "show_plot_sans()", &slot_53, QMetaData::Private },
	{ "load_sans()", &slot_54, QMetaData::Private },
	{ "load_sans(QString)", &slot_55, QMetaData::Private },
	{ "load_sans(QString,bool)", &slot_56, QMetaData::Private },
	{ "update_bin_size(double)", &slot_57, QMetaData::Private },
	{ "update_smooth(double)", &slot_58, QMetaData::Private },
	{ "update_guinier_cutoff(double)", &slot_59, QMetaData::Private },
	{ "show_pr_contrib()", &slot_60, QMetaData::Private },
	{ "update_pr_contrib_low(const QString&)", &slot_61, QMetaData::Private },
	{ "update_pr_contrib_high(const QString&)", &slot_62, QMetaData::Private },
	{ "set_curve(int)", &slot_63, QMetaData::Private },
	{ "load_pr()", &slot_64, QMetaData::Private },
	{ "load_pr(bool)", &slot_65, QMetaData::Private },
	{ "load_plot_pr()", &slot_66, QMetaData::Private },
	{ "clear_plot_pr()", &slot_67, QMetaData::Private },
	{ "cancel()", &slot_68, QMetaData::Private },
	{ "help()", &slot_69, QMetaData::Private },
	{ "options()", &slot_70, QMetaData::Private },
	{ "stop()", &slot_71, QMetaData::Private },
	{ "clear_display()", &slot_72, QMetaData::Private },
	{ "print()", &slot_73, QMetaData::Private },
	{ "update_font()", &slot_74, QMetaData::Private },
	{ "save()", &slot_75, QMetaData::Private },
	{ "select_atom_file()", &slot_76, QMetaData::Private },
	{ "select_hybrid_file()", &slot_77, QMetaData::Private },
	{ "select_saxs_file()", &slot_78, QMetaData::Private },
	{ "select_atom_file(const QString&)", &slot_79, QMetaData::Private },
	{ "select_hybrid_file(const QString&)", &slot_80, QMetaData::Private },
	{ "select_saxs_file(const QString&)", &slot_81, QMetaData::Private },
	{ "normalize_pr(vector<double>,vector<double>*)", &slot_82, QMetaData::Private },
	{ "normalize_pr(vector<double>,vector<double>*,double)", &slot_83, QMetaData::Private },
	{ "update_saxs_sans()", &slot_84, QMetaData::Private },
	{ "guinier_window()", &slot_85, QMetaData::Private },
	{ "run_guinier_analysis()", &slot_86, QMetaData::Private },
	{ "run_guinier_cs()", &slot_87, QMetaData::Private },
	{ "run_guinier_Rt()", &slot_88, QMetaData::Private },
	{ "saxs_filestring()", &slot_89, QMetaData::Private },
	{ "sprr_filestring()", &slot_90, QMetaData::Private },
	{ "set_create_native_saxs()", &slot_91, QMetaData::Private },
	{ "set_guinier()", &slot_92, QMetaData::Private },
	{ "set_cs_guinier()", &slot_93, QMetaData::Private },
	{ "set_Rt_guinier()", &slot_94, QMetaData::Private },
	{ "set_pr_contrib()", &slot_95, QMetaData::Private },
	{ "set_user_range()", &slot_96, QMetaData::Private },
	{ "set_kratky()", &slot_97, QMetaData::Private },
	{ "update_guinier_lowq2(const QString&)", &slot_98, QMetaData::Private },
	{ "update_guinier_highq2(const QString&)", &slot_99, QMetaData::Private },
	{ "update_user_lowq(const QString&)", &slot_100, QMetaData::Private },
	{ "update_user_highq(const QString&)", &slot_101, QMetaData::Private },
	{ "update_user_lowI(const QString&)", &slot_102, QMetaData::Private },
	{ "update_user_highI(const QString&)", &slot_103, QMetaData::Private },
	{ "load_gnom()", &slot_104, QMetaData::Private },
	{ "ift()", &slot_105, QMetaData::Private },
	{ "saxs_search()", &slot_106, QMetaData::Private },
	{ "saxs_screen()", &slot_107, QMetaData::Private },
	{ "saxs_buffer()", &slot_108, QMetaData::Private },
	{ "saxs_hplc()", &slot_109, QMetaData::Private },
	{ "saxs_xsr()", &slot_110, QMetaData::Private },
	{ "saxs_1d()", &slot_111, QMetaData::Private },
	{ "saxs_2d()", &slot_112, QMetaData::Private },
	{ "plot_saxs_clicked(long)", &slot_113, QMetaData::Private },
	{ "plot_pr_clicked(long)", &slot_114, QMetaData::Private },
	{ "plot_saxs_item_clicked(QwtPlotItem*)", &slot_115, QMetaData::Private },
	{ "plot_pr_item_clicked(QwtPlotItem*)", &slot_116, QMetaData::Private },
	{ "saxs_legend()", &slot_117, QMetaData::Private },
	{ "pr_legend()", &slot_118, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_119, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs", parentObject,
	slot_tbl, 120,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: manual_guinier_process(); break;
    case 1: set_manual_guinier(); break;
    case 2: adjust_wheel((double)static_QUType_double.get(_o+1)); break;
    case 3: manual_guinier_fit_start_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: manual_guinier_fit_end_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: manual_guinier_fit_start_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 6: manual_guinier_fit_end_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 7: set_resid_pct(); break;
    case 8: set_resid_sd(); break;
    case 9: set_resid_show_errorbars(); break;
    case 10: set_resid_show(); break;
    case 11: hide_pr(); break;
    case 12: hide_iq(); break;
    case 13: hide_settings(); break;
    case 14: foxs_readFromStdout(); break;
    case 15: foxs_readFromStderr(); break;
    case 16: foxs_launchFinished(); break;
    case 17: foxs_processExited(); break;
    case 18: crysol_readFromStdout(); break;
    case 19: crysol_readFromStderr(); break;
    case 20: crysol_launchFinished(); break;
    case 21: crysol_processExited(); break;
    case 22: crysol_wroteToStdin(); break;
    case 23: crysol_timeout(); break;
    case 24: cryson_readFromStdout(); break;
    case 25: cryson_readFromStderr(); break;
    case 26: cryson_launchFinished(); break;
    case 27: cryson_processExited(); break;
    case 28: sastbx_readFromStdout(); break;
    case 29: sastbx_readFromStderr(); break;
    case 30: sastbx_launchFinished(); break;
    case 31: sastbx_processExited(); break;
    case 32: static_QUType_double.set(_o,compute_ff((saxs&)*((saxs*)static_QUType_ptr.get(_o+1)),(saxs&)*((saxs*)static_QUType_ptr.get(_o+2)),(QString&)static_QUType_QString.get(_o+3),(QString&)static_QUType_QString.get(_o+4),(QString&)static_QUType_QString.get(_o+5),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+6))),(double)static_QUType_double.get(_o+7),(double)static_QUType_double.get(_o+8))); break;
    case 33: static_QUType_double.set(_o,compute_ff_bead_model((saxs&)*((saxs*)static_QUType_ptr.get(_o+1)),(double)static_QUType_double.get(_o+2))); break;
    case 34: fix_sas_options(); break;
    case 35: clear_guinier(); break;
    case 36: clear_cs_guinier(); break;
    case 37: clear_Rt_guinier(); break;
    case 38: show_plot_saxs_sans(); break;
    case 39: show_plot_pr(); break;
    case 40: setupGUI(); break;
    case 41: set_saxs_sans((int)static_QUType_int.get(_o+1)); break;
    case 42: set_saxs_iq((int)static_QUType_int.get(_o+1)); break;
    case 43: set_sans_iq((int)static_QUType_int.get(_o+1)); break;
    case 44: load_saxs_sans(); break;
    case 45: load_plot_saxs(); break;
    case 46: set_grid(); break;
    case 47: show_plot_saxs(); break;
    case 48: load_saxs(); break;
    case 49: load_saxs((QString)static_QUType_QString.get(_o+1)); break;
    case 50: load_saxs((QString)static_QUType_QString.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 51: clear_plot_saxs(); break;
    case 52: clear_plot_saxs((bool)static_QUType_bool.get(_o+1)); break;
    case 53: show_plot_sans(); break;
    case 54: load_sans(); break;
    case 55: load_sans((QString)static_QUType_QString.get(_o+1)); break;
    case 56: load_sans((QString)static_QUType_QString.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 57: update_bin_size((double)static_QUType_double.get(_o+1)); break;
    case 58: update_smooth((double)static_QUType_double.get(_o+1)); break;
    case 59: update_guinier_cutoff((double)static_QUType_double.get(_o+1)); break;
    case 60: show_pr_contrib(); break;
    case 61: update_pr_contrib_low((const QString&)static_QUType_QString.get(_o+1)); break;
    case 62: update_pr_contrib_high((const QString&)static_QUType_QString.get(_o+1)); break;
    case 63: set_curve((int)static_QUType_int.get(_o+1)); break;
    case 64: load_pr(); break;
    case 65: load_pr((bool)static_QUType_bool.get(_o+1)); break;
    case 66: load_plot_pr(); break;
    case 67: clear_plot_pr(); break;
    case 68: cancel(); break;
    case 69: help(); break;
    case 70: options(); break;
    case 71: stop(); break;
    case 72: clear_display(); break;
    case 73: print(); break;
    case 74: update_font(); break;
    case 75: save(); break;
    case 76: select_atom_file(); break;
    case 77: select_hybrid_file(); break;
    case 78: select_saxs_file(); break;
    case 79: select_atom_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 80: select_hybrid_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 81: select_saxs_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 82: normalize_pr((vector<double>)(*((vector<double>*)static_QUType_ptr.get(_o+1))),(vector<double>*)static_QUType_ptr.get(_o+2)); break;
    case 83: normalize_pr((vector<double>)(*((vector<double>*)static_QUType_ptr.get(_o+1))),(vector<double>*)static_QUType_ptr.get(_o+2),(double)static_QUType_double.get(_o+3)); break;
    case 84: update_saxs_sans(); break;
    case 85: guinier_window(); break;
    case 86: run_guinier_analysis(); break;
    case 87: run_guinier_cs(); break;
    case 88: run_guinier_Rt(); break;
    case 89: static_QUType_QString.set(_o,saxs_filestring()); break;
    case 90: static_QUType_QString.set(_o,sprr_filestring()); break;
    case 91: set_create_native_saxs(); break;
    case 92: set_guinier(); break;
    case 93: set_cs_guinier(); break;
    case 94: set_Rt_guinier(); break;
    case 95: set_pr_contrib(); break;
    case 96: set_user_range(); break;
    case 97: set_kratky(); break;
    case 98: update_guinier_lowq2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 99: update_guinier_highq2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 100: update_user_lowq((const QString&)static_QUType_QString.get(_o+1)); break;
    case 101: update_user_highq((const QString&)static_QUType_QString.get(_o+1)); break;
    case 102: update_user_lowI((const QString&)static_QUType_QString.get(_o+1)); break;
    case 103: update_user_highI((const QString&)static_QUType_QString.get(_o+1)); break;
    case 104: load_gnom(); break;
    case 105: ift(); break;
    case 106: saxs_search(); break;
    case 107: saxs_screen(); break;
    case 108: saxs_buffer(); break;
    case 109: saxs_hplc(); break;
    case 110: saxs_xsr(); break;
    case 111: saxs_1d(); break;
    case 112: saxs_2d(); break;
    case 113: plot_saxs_clicked((long)(*((long*)static_QUType_ptr.get(_o+1)))); break;
    case 114: plot_pr_clicked((long)(*((long*)static_QUType_ptr.get(_o+1)))); break;
    case 115: plot_saxs_item_clicked((QwtPlotItem*)static_QUType_ptr.get(_o+1)); break;
    case 116: plot_pr_item_clicked((QwtPlotItem*)static_QUType_ptr.get(_o+1)); break;
    case 117: saxs_legend(); break;
    case 118: pr_legend(); break;
    case 119: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
