/****************************************************************************
** US_Hydrodyn_Saxs_Hplc meta object code from reading C++ file 'us_hydrodyn_saxs_hplc.h'
**
** Created: Tue Feb 18 13:30:33 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_hplc.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QMouseEvent>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Saxs_Hplc::className() const
{
    return "US_Hydrodyn_Saxs_Hplc";
}

QMetaObject *US_Hydrodyn_Saxs_Hplc::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Hplc( "US_Hydrodyn_Saxs_Hplc", &US_Hydrodyn_Saxs_Hplc::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Hplc::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Hplc::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Hplc::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"color_rotate", 0, 0 };
    static const QUMethod slot_2 = {"line_width", 0, 0 };
    static const QUMethod slot_3 = {"dir_pressed", 0, 0 };
    static const QUMethod slot_4 = {"created_dir_pressed", 0, 0 };
    static const QUMethod slot_5 = {"hide_files", 0, 0 };
    static const QUMethod slot_6 = {"hide_created_files", 0, 0 };
    static const QUMethod slot_7 = {"update_enables", 0, 0 };
    static const QUMethod slot_8 = {"update_files", 0, 0 };
    static const QUMethod slot_9 = {"update_created_files", 0, 0 };
    static const QUMethod slot_10 = {"add_files", 0, 0 };
    static const QUMethod slot_11 = {"similar_files", 0, 0 };
    static const QUMethod slot_12 = {"conc", 0, 0 };
    static const QUMethod slot_13 = {"clear_files", 0, 0 };
    static const QUMethod slot_14 = {"regex_load", 0, 0 };
    static const QUMethod slot_15 = {"select_all", 0, 0 };
    static const QUMethod slot_16 = {"select_nth", 0, 0 };
    static const QUMethod slot_17 = {"invert", 0, 0 };
    static const QUMethod slot_18 = {"join", 0, 0 };
    static const QUMethod slot_19 = {"adjacent", 0, 0 };
    static const QUMethod slot_20 = {"to_saxs", 0, 0 };
    static const QUMethod slot_21 = {"view", 0, 0 };
    static const QUMethod slot_22 = {"movie", 0, 0 };
    static const QUMethod slot_23 = {"rescale", 0, 0 };
    static const QUMethod slot_24 = {"conc_avg", 0, 0 };
    static const QUMethod slot_25 = {"normalize", 0, 0 };
    static const QUMethod slot_26 = {"add", 0, 0 };
    static const QUMethod slot_27 = {"avg", 0, 0 };
    static const QUMethod slot_28 = {"smooth", 0, 0 };
    static const QUMethod slot_29 = {"svd", 0, 0 };
    static const QUMethod slot_30 = {"repeak", 0, 0 };
    static const QUMethod slot_31 = {"create_i_of_t", 0, 0 };
    static const QUMethod slot_32 = {"create_i_of_q", 0, 0 };
    static const QUMethod slot_33 = {"set_conc_file", 0, 0 };
    static const QUMethod slot_34 = {"set_detector", 0, 0 };
    static const QUMethod slot_35 = {"set_hplc", 0, 0 };
    static const QUMethod slot_36 = {"set_empty", 0, 0 };
    static const QUMethod slot_37 = {"set_signal", 0, 0 };
    static const QUMethod slot_38 = {"select_all_created", 0, 0 };
    static const QUMethod slot_39 = {"invert_all_created", 0, 0 };
    static const QUMethod slot_40 = {"adjacent_created", 0, 0 };
    static const QUMethod slot_41 = {"remove_created", 0, 0 };
    static const QUMethod slot_42 = {"save_created_csv", 0, 0 };
    static const QUMethod slot_43 = {"save_created", 0, 0 };
    static const QUMethod slot_44 = {"show_created", 0, 0 };
    static const QUMethod slot_45 = {"show_only_created", 0, 0 };
    static const QUMethod slot_46 = {"stack_push_all", 0, 0 };
    static const QUMethod slot_47 = {"stack_push_sel", 0, 0 };
    static const QUMethod slot_48 = {"stack_copy", 0, 0 };
    static const QUMethod slot_49 = {"stack_pcopy", 0, 0 };
    static const QUMethod slot_50 = {"stack_paste", 0, 0 };
    static const QUMethod slot_51 = {"stack_clear", 0, 0 };
    static const QUMethod slot_52 = {"stack_drop", 0, 0 };
    static const QUMethod slot_53 = {"stack_join", 0, 0 };
    static const QUMethod slot_54 = {"stack_rot_up", 0, 0 };
    static const QUMethod slot_55 = {"stack_rot_down", 0, 0 };
    static const QUMethod slot_56 = {"stack_swap", 0, 0 };
    static const QUMethod slot_57 = {"wheel_start", 0, 0 };
    static const QUMethod slot_58 = {"p3d", 0, 0 };
    static const QUMethod slot_59 = {"ref", 0, 0 };
    static const QUMethod slot_60 = {"errors", 0, 0 };
    static const QUMethod slot_61 = {"wheel_cancel", 0, 0 };
    static const QUMethod slot_62 = {"wheel_save", 0, 0 };
    static const QUMethod slot_63 = {"clear_display", 0, 0 };
    static const QUMethod slot_64 = {"update_font", 0, 0 };
    static const QUMethod slot_65 = {"save", 0, 0 };
    static const QUMethod slot_66 = {"hide_editor", 0, 0 };
    static const QUMethod slot_67 = {"help", 0, 0 };
    static const QUMethod slot_68 = {"options", 0, 0 };
    static const QUMethod slot_69 = {"cancel", 0, 0 };
    static const QUParameter param_slot_70[] = {
	{ "rect", &static_QUType_ptr, "QwtDoubleRect", QUParameter::In }
    };
    static const QUMethod slot_70 = {"plot_zoomed", 1, param_slot_70 };
    static const QUParameter param_slot_71[] = {
	{ "me", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_71 = {"plot_mouse", 1, param_slot_71 };
    static const QUParameter param_slot_72[] = {
	{ "rect", &static_QUType_ptr, "QwtDoubleRect", QUParameter::In }
    };
    static const QUMethod slot_72 = {"plot_errors_zoomed", 1, param_slot_72 };
    static const QUParameter param_slot_73[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_73 = {"adjust_wheel", 1, param_slot_73 };
    static const QUMethod slot_74 = {"gauss_start", 0, 0 };
    static const QUMethod slot_75 = {"gauss_clear", 0, 0 };
    static const QUMethod slot_76 = {"gauss_new", 0, 0 };
    static const QUMethod slot_77 = {"gauss_delete", 0, 0 };
    static const QUMethod slot_78 = {"gauss_prev", 0, 0 };
    static const QUMethod slot_79 = {"gauss_next", 0, 0 };
    static const QUMethod slot_80 = {"gauss_fit", 0, 0 };
    static const QUMethod slot_81 = {"gauss_save", 0, 0 };
    static const QUParameter param_slot_82[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_82 = {"gauss_pos_text", 1, param_slot_82 };
    static const QUParameter param_slot_83[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_83 = {"gauss_pos_width_text", 1, param_slot_83 };
    static const QUParameter param_slot_84[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_84 = {"gauss_pos_height_text", 1, param_slot_84 };
    static const QUParameter param_slot_85[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_85 = {"gauss_pos_dist1_text", 1, param_slot_85 };
    static const QUParameter param_slot_86[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_86 = {"gauss_pos_dist2_text", 1, param_slot_86 };
    static const QUParameter param_slot_87[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_87 = {"gauss_fit_start_text", 1, param_slot_87 };
    static const QUParameter param_slot_88[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_88 = {"gauss_fit_end_text", 1, param_slot_88 };
    static const QUParameter param_slot_89[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_89 = {"gauss_pos_focus", 1, param_slot_89 };
    static const QUParameter param_slot_90[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_90 = {"gauss_pos_width_focus", 1, param_slot_90 };
    static const QUParameter param_slot_91[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_91 = {"gauss_pos_height_focus", 1, param_slot_91 };
    static const QUParameter param_slot_92[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_92 = {"gauss_pos_dist1_focus", 1, param_slot_92 };
    static const QUParameter param_slot_93[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_93 = {"gauss_pos_dist2_focus", 1, param_slot_93 };
    static const QUParameter param_slot_94[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_94 = {"gauss_fit_start_focus", 1, param_slot_94 };
    static const QUParameter param_slot_95[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_95 = {"gauss_fit_end_focus", 1, param_slot_95 };
    static const QUMethod slot_96 = {"ggauss_start", 0, 0 };
    static const QUMethod slot_97 = {"ggauss_rmsd", 0, 0 };
    static const QUMethod slot_98 = {"ggauss_results", 0, 0 };
    static const QUMethod slot_99 = {"set_sd_weight", 0, 0 };
    static const QUMethod slot_100 = {"set_fix_width", 0, 0 };
    static const QUMethod slot_101 = {"set_fix_dist1", 0, 0 };
    static const QUMethod slot_102 = {"set_fix_dist2", 0, 0 };
    static const QUMethod slot_103 = {"gauss_as_curves", 0, 0 };
    static const QUMethod slot_104 = {"baseline_start", 0, 0 };
    static const QUMethod slot_105 = {"baseline_apply", 0, 0 };
    static const QUParameter param_slot_106[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_106 = {"baseline_start_s_text", 1, param_slot_106 };
    static const QUParameter param_slot_107[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_107 = {"baseline_start_text", 1, param_slot_107 };
    static const QUParameter param_slot_108[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_108 = {"baseline_start_e_text", 1, param_slot_108 };
    static const QUParameter param_slot_109[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_109 = {"baseline_end_s_text", 1, param_slot_109 };
    static const QUParameter param_slot_110[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_110 = {"baseline_end_text", 1, param_slot_110 };
    static const QUParameter param_slot_111[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_111 = {"baseline_end_e_text", 1, param_slot_111 };
    static const QUParameter param_slot_112[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_112 = {"baseline_start_s_focus", 1, param_slot_112 };
    static const QUParameter param_slot_113[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_113 = {"baseline_start_focus", 1, param_slot_113 };
    static const QUParameter param_slot_114[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_114 = {"baseline_start_e_focus", 1, param_slot_114 };
    static const QUParameter param_slot_115[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_115 = {"baseline_end_s_focus", 1, param_slot_115 };
    static const QUParameter param_slot_116[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_116 = {"baseline_end_focus", 1, param_slot_116 };
    static const QUParameter param_slot_117[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_117 = {"baseline_end_e_focus", 1, param_slot_117 };
    static const QUMethod slot_118 = {"select_vis", 0, 0 };
    static const QUMethod slot_119 = {"remove_vis", 0, 0 };
    static const QUMethod slot_120 = {"crop_left", 0, 0 };
    static const QUMethod slot_121 = {"crop_common", 0, 0 };
    static const QUMethod slot_122 = {"crop_vis", 0, 0 };
    static const QUMethod slot_123 = {"crop_zero", 0, 0 };
    static const QUMethod slot_124 = {"crop_undo", 0, 0 };
    static const QUMethod slot_125 = {"crop_right", 0, 0 };
    static const QUMethod slot_126 = {"legend", 0, 0 };
    static const QUMethod slot_127 = {"axis_x", 0, 0 };
    static const QUMethod slot_128 = {"axis_y", 0, 0 };
    static const QUMethod slot_129 = {"legend_set", 0, 0 };
    static const QUParameter param_slot_130[] = {
	{ 0, &static_QUType_ptr, "QListBoxItem", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_130 = {"rename_created", 2, param_slot_130 };
    static const QUMethod slot_131 = {"set_plot_errors_rev", 0, 0 };
    static const QUMethod slot_132 = {"set_plot_errors_sd", 0, 0 };
    static const QUMethod slot_133 = {"set_plot_errors_pct", 0, 0 };
    static const QUMethod slot_134 = {"set_plot_errors_group", 0, 0 };
    static const QUMethod slot_135 = {"save_state", 0, 0 };
    static const QUParameter param_slot_136[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_136 = {"closeEvent", 1, param_slot_136 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "color_rotate()", &slot_1, QMetaData::Private },
	{ "line_width()", &slot_2, QMetaData::Private },
	{ "dir_pressed()", &slot_3, QMetaData::Private },
	{ "created_dir_pressed()", &slot_4, QMetaData::Private },
	{ "hide_files()", &slot_5, QMetaData::Private },
	{ "hide_created_files()", &slot_6, QMetaData::Private },
	{ "update_enables()", &slot_7, QMetaData::Private },
	{ "update_files()", &slot_8, QMetaData::Private },
	{ "update_created_files()", &slot_9, QMetaData::Private },
	{ "add_files()", &slot_10, QMetaData::Private },
	{ "similar_files()", &slot_11, QMetaData::Private },
	{ "conc()", &slot_12, QMetaData::Private },
	{ "clear_files()", &slot_13, QMetaData::Private },
	{ "regex_load()", &slot_14, QMetaData::Private },
	{ "select_all()", &slot_15, QMetaData::Private },
	{ "select_nth()", &slot_16, QMetaData::Private },
	{ "invert()", &slot_17, QMetaData::Private },
	{ "join()", &slot_18, QMetaData::Private },
	{ "adjacent()", &slot_19, QMetaData::Private },
	{ "to_saxs()", &slot_20, QMetaData::Private },
	{ "view()", &slot_21, QMetaData::Private },
	{ "movie()", &slot_22, QMetaData::Private },
	{ "rescale()", &slot_23, QMetaData::Private },
	{ "conc_avg()", &slot_24, QMetaData::Private },
	{ "normalize()", &slot_25, QMetaData::Private },
	{ "add()", &slot_26, QMetaData::Private },
	{ "avg()", &slot_27, QMetaData::Private },
	{ "smooth()", &slot_28, QMetaData::Private },
	{ "svd()", &slot_29, QMetaData::Private },
	{ "repeak()", &slot_30, QMetaData::Private },
	{ "create_i_of_t()", &slot_31, QMetaData::Private },
	{ "create_i_of_q()", &slot_32, QMetaData::Private },
	{ "set_conc_file()", &slot_33, QMetaData::Private },
	{ "set_detector()", &slot_34, QMetaData::Private },
	{ "set_hplc()", &slot_35, QMetaData::Private },
	{ "set_empty()", &slot_36, QMetaData::Private },
	{ "set_signal()", &slot_37, QMetaData::Private },
	{ "select_all_created()", &slot_38, QMetaData::Private },
	{ "invert_all_created()", &slot_39, QMetaData::Private },
	{ "adjacent_created()", &slot_40, QMetaData::Private },
	{ "remove_created()", &slot_41, QMetaData::Private },
	{ "save_created_csv()", &slot_42, QMetaData::Private },
	{ "save_created()", &slot_43, QMetaData::Private },
	{ "show_created()", &slot_44, QMetaData::Private },
	{ "show_only_created()", &slot_45, QMetaData::Private },
	{ "stack_push_all()", &slot_46, QMetaData::Private },
	{ "stack_push_sel()", &slot_47, QMetaData::Private },
	{ "stack_copy()", &slot_48, QMetaData::Private },
	{ "stack_pcopy()", &slot_49, QMetaData::Private },
	{ "stack_paste()", &slot_50, QMetaData::Private },
	{ "stack_clear()", &slot_51, QMetaData::Private },
	{ "stack_drop()", &slot_52, QMetaData::Private },
	{ "stack_join()", &slot_53, QMetaData::Private },
	{ "stack_rot_up()", &slot_54, QMetaData::Private },
	{ "stack_rot_down()", &slot_55, QMetaData::Private },
	{ "stack_swap()", &slot_56, QMetaData::Private },
	{ "wheel_start()", &slot_57, QMetaData::Private },
	{ "p3d()", &slot_58, QMetaData::Private },
	{ "ref()", &slot_59, QMetaData::Private },
	{ "errors()", &slot_60, QMetaData::Private },
	{ "wheel_cancel()", &slot_61, QMetaData::Private },
	{ "wheel_save()", &slot_62, QMetaData::Private },
	{ "clear_display()", &slot_63, QMetaData::Private },
	{ "update_font()", &slot_64, QMetaData::Private },
	{ "save()", &slot_65, QMetaData::Private },
	{ "hide_editor()", &slot_66, QMetaData::Private },
	{ "help()", &slot_67, QMetaData::Private },
	{ "options()", &slot_68, QMetaData::Private },
	{ "cancel()", &slot_69, QMetaData::Private },
	{ "plot_zoomed(const QwtDoubleRect&)", &slot_70, QMetaData::Private },
	{ "plot_mouse(const QMouseEvent&)", &slot_71, QMetaData::Private },
	{ "plot_errors_zoomed(const QwtDoubleRect&)", &slot_72, QMetaData::Private },
	{ "adjust_wheel(double)", &slot_73, QMetaData::Private },
	{ "gauss_start()", &slot_74, QMetaData::Private },
	{ "gauss_clear()", &slot_75, QMetaData::Private },
	{ "gauss_new()", &slot_76, QMetaData::Private },
	{ "gauss_delete()", &slot_77, QMetaData::Private },
	{ "gauss_prev()", &slot_78, QMetaData::Private },
	{ "gauss_next()", &slot_79, QMetaData::Private },
	{ "gauss_fit()", &slot_80, QMetaData::Private },
	{ "gauss_save()", &slot_81, QMetaData::Private },
	{ "gauss_pos_text(const QString&)", &slot_82, QMetaData::Private },
	{ "gauss_pos_width_text(const QString&)", &slot_83, QMetaData::Private },
	{ "gauss_pos_height_text(const QString&)", &slot_84, QMetaData::Private },
	{ "gauss_pos_dist1_text(const QString&)", &slot_85, QMetaData::Private },
	{ "gauss_pos_dist2_text(const QString&)", &slot_86, QMetaData::Private },
	{ "gauss_fit_start_text(const QString&)", &slot_87, QMetaData::Private },
	{ "gauss_fit_end_text(const QString&)", &slot_88, QMetaData::Private },
	{ "gauss_pos_focus(bool)", &slot_89, QMetaData::Private },
	{ "gauss_pos_width_focus(bool)", &slot_90, QMetaData::Private },
	{ "gauss_pos_height_focus(bool)", &slot_91, QMetaData::Private },
	{ "gauss_pos_dist1_focus(bool)", &slot_92, QMetaData::Private },
	{ "gauss_pos_dist2_focus(bool)", &slot_93, QMetaData::Private },
	{ "gauss_fit_start_focus(bool)", &slot_94, QMetaData::Private },
	{ "gauss_fit_end_focus(bool)", &slot_95, QMetaData::Private },
	{ "ggauss_start()", &slot_96, QMetaData::Private },
	{ "ggauss_rmsd()", &slot_97, QMetaData::Private },
	{ "ggauss_results()", &slot_98, QMetaData::Private },
	{ "set_sd_weight()", &slot_99, QMetaData::Private },
	{ "set_fix_width()", &slot_100, QMetaData::Private },
	{ "set_fix_dist1()", &slot_101, QMetaData::Private },
	{ "set_fix_dist2()", &slot_102, QMetaData::Private },
	{ "gauss_as_curves()", &slot_103, QMetaData::Private },
	{ "baseline_start()", &slot_104, QMetaData::Private },
	{ "baseline_apply()", &slot_105, QMetaData::Private },
	{ "baseline_start_s_text(const QString&)", &slot_106, QMetaData::Private },
	{ "baseline_start_text(const QString&)", &slot_107, QMetaData::Private },
	{ "baseline_start_e_text(const QString&)", &slot_108, QMetaData::Private },
	{ "baseline_end_s_text(const QString&)", &slot_109, QMetaData::Private },
	{ "baseline_end_text(const QString&)", &slot_110, QMetaData::Private },
	{ "baseline_end_e_text(const QString&)", &slot_111, QMetaData::Private },
	{ "baseline_start_s_focus(bool)", &slot_112, QMetaData::Private },
	{ "baseline_start_focus(bool)", &slot_113, QMetaData::Private },
	{ "baseline_start_e_focus(bool)", &slot_114, QMetaData::Private },
	{ "baseline_end_s_focus(bool)", &slot_115, QMetaData::Private },
	{ "baseline_end_focus(bool)", &slot_116, QMetaData::Private },
	{ "baseline_end_e_focus(bool)", &slot_117, QMetaData::Private },
	{ "select_vis()", &slot_118, QMetaData::Private },
	{ "remove_vis()", &slot_119, QMetaData::Private },
	{ "crop_left()", &slot_120, QMetaData::Private },
	{ "crop_common()", &slot_121, QMetaData::Private },
	{ "crop_vis()", &slot_122, QMetaData::Private },
	{ "crop_zero()", &slot_123, QMetaData::Private },
	{ "crop_undo()", &slot_124, QMetaData::Private },
	{ "crop_right()", &slot_125, QMetaData::Private },
	{ "legend()", &slot_126, QMetaData::Private },
	{ "axis_x()", &slot_127, QMetaData::Private },
	{ "axis_y()", &slot_128, QMetaData::Private },
	{ "legend_set()", &slot_129, QMetaData::Private },
	{ "rename_created(QListBoxItem*,const QPoint&)", &slot_130, QMetaData::Private },
	{ "set_plot_errors_rev()", &slot_131, QMetaData::Private },
	{ "set_plot_errors_sd()", &slot_132, QMetaData::Private },
	{ "set_plot_errors_pct()", &slot_133, QMetaData::Private },
	{ "set_plot_errors_group()", &slot_134, QMetaData::Private },
	{ "save_state()", &slot_135, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_136, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Hplc", parentObject,
	slot_tbl, 137,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Hplc.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Hplc::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Hplc" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Hplc::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: color_rotate(); break;
    case 2: line_width(); break;
    case 3: dir_pressed(); break;
    case 4: created_dir_pressed(); break;
    case 5: hide_files(); break;
    case 6: hide_created_files(); break;
    case 7: update_enables(); break;
    case 8: update_files(); break;
    case 9: update_created_files(); break;
    case 10: add_files(); break;
    case 11: similar_files(); break;
    case 12: conc(); break;
    case 13: clear_files(); break;
    case 14: regex_load(); break;
    case 15: select_all(); break;
    case 16: select_nth(); break;
    case 17: invert(); break;
    case 18: join(); break;
    case 19: adjacent(); break;
    case 20: to_saxs(); break;
    case 21: view(); break;
    case 22: movie(); break;
    case 23: rescale(); break;
    case 24: conc_avg(); break;
    case 25: normalize(); break;
    case 26: add(); break;
    case 27: avg(); break;
    case 28: smooth(); break;
    case 29: svd(); break;
    case 30: repeak(); break;
    case 31: create_i_of_t(); break;
    case 32: create_i_of_q(); break;
    case 33: set_conc_file(); break;
    case 34: set_detector(); break;
    case 35: set_hplc(); break;
    case 36: set_empty(); break;
    case 37: set_signal(); break;
    case 38: select_all_created(); break;
    case 39: invert_all_created(); break;
    case 40: adjacent_created(); break;
    case 41: remove_created(); break;
    case 42: save_created_csv(); break;
    case 43: save_created(); break;
    case 44: show_created(); break;
    case 45: show_only_created(); break;
    case 46: stack_push_all(); break;
    case 47: stack_push_sel(); break;
    case 48: stack_copy(); break;
    case 49: stack_pcopy(); break;
    case 50: stack_paste(); break;
    case 51: stack_clear(); break;
    case 52: stack_drop(); break;
    case 53: stack_join(); break;
    case 54: stack_rot_up(); break;
    case 55: stack_rot_down(); break;
    case 56: stack_swap(); break;
    case 57: wheel_start(); break;
    case 58: p3d(); break;
    case 59: ref(); break;
    case 60: errors(); break;
    case 61: wheel_cancel(); break;
    case 62: wheel_save(); break;
    case 63: clear_display(); break;
    case 64: update_font(); break;
    case 65: save(); break;
    case 66: hide_editor(); break;
    case 67: help(); break;
    case 68: options(); break;
    case 69: cancel(); break;
    case 70: plot_zoomed((const QwtDoubleRect&)*((const QwtDoubleRect*)static_QUType_ptr.get(_o+1))); break;
    case 71: plot_mouse((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 72: plot_errors_zoomed((const QwtDoubleRect&)*((const QwtDoubleRect*)static_QUType_ptr.get(_o+1))); break;
    case 73: adjust_wheel((double)static_QUType_double.get(_o+1)); break;
    case 74: gauss_start(); break;
    case 75: gauss_clear(); break;
    case 76: gauss_new(); break;
    case 77: gauss_delete(); break;
    case 78: gauss_prev(); break;
    case 79: gauss_next(); break;
    case 80: gauss_fit(); break;
    case 81: gauss_save(); break;
    case 82: gauss_pos_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 83: gauss_pos_width_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 84: gauss_pos_height_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 85: gauss_pos_dist1_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 86: gauss_pos_dist2_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 87: gauss_fit_start_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 88: gauss_fit_end_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 89: gauss_pos_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 90: gauss_pos_width_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 91: gauss_pos_height_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 92: gauss_pos_dist1_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 93: gauss_pos_dist2_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 94: gauss_fit_start_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 95: gauss_fit_end_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 96: ggauss_start(); break;
    case 97: ggauss_rmsd(); break;
    case 98: ggauss_results(); break;
    case 99: set_sd_weight(); break;
    case 100: set_fix_width(); break;
    case 101: set_fix_dist1(); break;
    case 102: set_fix_dist2(); break;
    case 103: gauss_as_curves(); break;
    case 104: baseline_start(); break;
    case 105: baseline_apply(); break;
    case 106: baseline_start_s_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 107: baseline_start_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 108: baseline_start_e_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 109: baseline_end_s_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 110: baseline_end_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 111: baseline_end_e_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 112: baseline_start_s_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 113: baseline_start_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 114: baseline_start_e_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 115: baseline_end_s_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 116: baseline_end_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 117: baseline_end_e_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 118: select_vis(); break;
    case 119: remove_vis(); break;
    case 120: crop_left(); break;
    case 121: crop_common(); break;
    case 122: crop_vis(); break;
    case 123: crop_zero(); break;
    case 124: crop_undo(); break;
    case 125: crop_right(); break;
    case 126: legend(); break;
    case 127: axis_x(); break;
    case 128: axis_y(); break;
    case 129: legend_set(); break;
    case 130: rename_created((Q3ListBoxItem*)static_QUType_ptr.get(_o+1),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+2))); break;
    case 131: set_plot_errors_rev(); break;
    case 132: set_plot_errors_sd(); break;
    case 133: set_plot_errors_pct(); break;
    case 134: set_plot_errors_group(); break;
    case 135: save_state(); break;
    case 136: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Hplc::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Hplc::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Hplc::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
