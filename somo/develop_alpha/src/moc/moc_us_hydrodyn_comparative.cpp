/****************************************************************************
** US_Hydrodyn_Comparative meta object code from reading C++ file 'us_hydrodyn_comparative.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_comparative.h"
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

const char *US_Hydrodyn_Comparative::className() const
{
    return "US_Hydrodyn_Comparative";
}

QMetaObject *US_Hydrodyn_Comparative::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Comparative( "US_Hydrodyn_Comparative", &US_Hydrodyn_Comparative::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Comparative::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Comparative", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Comparative::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Comparative", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Comparative::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_by_pct", 0, 0 };
    static const QUMethod slot_2 = {"set_rank", 0, 0 };
    static const QUMethod slot_3 = {"set_weight_controls", 0, 0 };
    static const QUMethod slot_4 = {"set_by_ec", 0, 0 };
    static const QUMethod slot_5 = {"set_active_s", 0, 0 };
    static const QUMethod slot_6 = {"set_active_D", 0, 0 };
    static const QUMethod slot_7 = {"set_active_sr", 0, 0 };
    static const QUMethod slot_8 = {"set_active_fr", 0, 0 };
    static const QUMethod slot_9 = {"set_active_rg", 0, 0 };
    static const QUMethod slot_10 = {"set_active_tau", 0, 0 };
    static const QUMethod slot_11 = {"set_active_eta", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_target_s", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_rank_s", 1, param_slot_13 };
    static const QUMethod slot_14 = {"set_include_in_weight_s", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_weight_s", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_buckets_s", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_min_s", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_max_s", 1, param_slot_18 };
    static const QUMethod slot_19 = {"set_store_reference_s", 0, 0 };
    static const QUMethod slot_20 = {"set_store_diff_s", 0, 0 };
    static const QUMethod slot_21 = {"set_store_abs_diff_s", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_target_D", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_rank_D", 1, param_slot_23 };
    static const QUMethod slot_24 = {"set_include_in_weight_D", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_weight_D", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_buckets_D", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_min_D", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_max_D", 1, param_slot_28 };
    static const QUMethod slot_29 = {"set_store_reference_D", 0, 0 };
    static const QUMethod slot_30 = {"set_store_diff_D", 0, 0 };
    static const QUMethod slot_31 = {"set_store_abs_diff_D", 0, 0 };
    static const QUParameter param_slot_32[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"update_target_sr", 1, param_slot_32 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_33 = {"update_rank_sr", 1, param_slot_33 };
    static const QUMethod slot_34 = {"set_include_in_weight_sr", 0, 0 };
    static const QUParameter param_slot_35[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_35 = {"update_weight_sr", 1, param_slot_35 };
    static const QUParameter param_slot_36[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_36 = {"update_buckets_sr", 1, param_slot_36 };
    static const QUParameter param_slot_37[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_37 = {"update_min_sr", 1, param_slot_37 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"update_max_sr", 1, param_slot_38 };
    static const QUMethod slot_39 = {"set_store_reference_sr", 0, 0 };
    static const QUMethod slot_40 = {"set_store_diff_sr", 0, 0 };
    static const QUMethod slot_41 = {"set_store_abs_diff_sr", 0, 0 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_42 = {"update_target_fr", 1, param_slot_42 };
    static const QUParameter param_slot_43[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_43 = {"update_rank_fr", 1, param_slot_43 };
    static const QUMethod slot_44 = {"set_include_in_weight_fr", 0, 0 };
    static const QUParameter param_slot_45[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_45 = {"update_weight_fr", 1, param_slot_45 };
    static const QUParameter param_slot_46[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_46 = {"update_buckets_fr", 1, param_slot_46 };
    static const QUParameter param_slot_47[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_47 = {"update_min_fr", 1, param_slot_47 };
    static const QUParameter param_slot_48[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_48 = {"update_max_fr", 1, param_slot_48 };
    static const QUMethod slot_49 = {"set_store_reference_fr", 0, 0 };
    static const QUMethod slot_50 = {"set_store_diff_fr", 0, 0 };
    static const QUMethod slot_51 = {"set_store_abs_diff_fr", 0, 0 };
    static const QUParameter param_slot_52[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_52 = {"update_target_rg", 1, param_slot_52 };
    static const QUParameter param_slot_53[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_53 = {"update_rank_rg", 1, param_slot_53 };
    static const QUMethod slot_54 = {"set_include_in_weight_rg", 0, 0 };
    static const QUParameter param_slot_55[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_55 = {"update_weight_rg", 1, param_slot_55 };
    static const QUParameter param_slot_56[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_56 = {"update_buckets_rg", 1, param_slot_56 };
    static const QUParameter param_slot_57[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_57 = {"update_min_rg", 1, param_slot_57 };
    static const QUParameter param_slot_58[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_58 = {"update_max_rg", 1, param_slot_58 };
    static const QUMethod slot_59 = {"set_store_reference_rg", 0, 0 };
    static const QUMethod slot_60 = {"set_store_diff_rg", 0, 0 };
    static const QUMethod slot_61 = {"set_store_abs_diff_rg", 0, 0 };
    static const QUParameter param_slot_62[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_62 = {"update_target_tau", 1, param_slot_62 };
    static const QUParameter param_slot_63[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_63 = {"update_rank_tau", 1, param_slot_63 };
    static const QUMethod slot_64 = {"set_include_in_weight_tau", 0, 0 };
    static const QUParameter param_slot_65[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_65 = {"update_weight_tau", 1, param_slot_65 };
    static const QUParameter param_slot_66[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_66 = {"update_buckets_tau", 1, param_slot_66 };
    static const QUParameter param_slot_67[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_67 = {"update_min_tau", 1, param_slot_67 };
    static const QUParameter param_slot_68[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_68 = {"update_max_tau", 1, param_slot_68 };
    static const QUMethod slot_69 = {"set_store_reference_tau", 0, 0 };
    static const QUMethod slot_70 = {"set_store_diff_tau", 0, 0 };
    static const QUMethod slot_71 = {"set_store_abs_diff_tau", 0, 0 };
    static const QUParameter param_slot_72[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_72 = {"update_target_eta", 1, param_slot_72 };
    static const QUParameter param_slot_73[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_73 = {"update_rank_eta", 1, param_slot_73 };
    static const QUMethod slot_74 = {"set_include_in_weight_eta", 0, 0 };
    static const QUParameter param_slot_75[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_75 = {"update_weight_eta", 1, param_slot_75 };
    static const QUParameter param_slot_76[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_76 = {"update_buckets_eta", 1, param_slot_76 };
    static const QUParameter param_slot_77[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_77 = {"update_min_eta", 1, param_slot_77 };
    static const QUParameter param_slot_78[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_78 = {"update_max_eta", 1, param_slot_78 };
    static const QUMethod slot_79 = {"set_store_reference_eta", 0, 0 };
    static const QUMethod slot_80 = {"set_store_diff_eta", 0, 0 };
    static const QUMethod slot_81 = {"set_store_abs_diff_eta", 0, 0 };
    static const QUMethod slot_82 = {"load_param", 0, 0 };
    static const QUMethod slot_83 = {"reset_param", 0, 0 };
    static const QUMethod slot_84 = {"save_param", 0, 0 };
    static const QUMethod slot_85 = {"load_csv", 0, 0 };
    static const QUMethod slot_86 = {"process_csv", 0, 0 };
    static const QUMethod slot_87 = {"save_csv", 0, 0 };
    static const QUMethod slot_88 = {"update_loaded", 0, 0 };
    static const QUMethod slot_89 = {"loaded_select_all", 0, 0 };
    static const QUMethod slot_90 = {"loaded_view", 0, 0 };
    static const QUMethod slot_91 = {"loaded_merge", 0, 0 };
    static const QUMethod slot_92 = {"loaded_set_ranges", 0, 0 };
    static const QUMethod slot_93 = {"loaded_remove", 0, 0 };
    static const QUMethod slot_94 = {"update_selected", 0, 0 };
    static const QUMethod slot_95 = {"selected_select_all", 0, 0 };
    static const QUMethod slot_96 = {"selected_merge", 0, 0 };
    static const QUMethod slot_97 = {"selected_set_ranges", 0, 0 };
    static const QUMethod slot_98 = {"selected_remove", 0, 0 };
    static const QUMethod slot_99 = {"clear_display", 0, 0 };
    static const QUMethod slot_100 = {"print", 0, 0 };
    static const QUMethod slot_101 = {"update_font", 0, 0 };
    static const QUMethod slot_102 = {"save", 0, 0 };
    static const QUMethod slot_103 = {"cancel", 0, 0 };
    static const QUMethod slot_104 = {"help", 0, 0 };
    static const QUParameter param_slot_105[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_105 = {"closeEvent", 1, param_slot_105 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_by_pct()", &slot_1, QMetaData::Private },
	{ "set_rank()", &slot_2, QMetaData::Private },
	{ "set_weight_controls()", &slot_3, QMetaData::Private },
	{ "set_by_ec()", &slot_4, QMetaData::Private },
	{ "set_active_s()", &slot_5, QMetaData::Private },
	{ "set_active_D()", &slot_6, QMetaData::Private },
	{ "set_active_sr()", &slot_7, QMetaData::Private },
	{ "set_active_fr()", &slot_8, QMetaData::Private },
	{ "set_active_rg()", &slot_9, QMetaData::Private },
	{ "set_active_tau()", &slot_10, QMetaData::Private },
	{ "set_active_eta()", &slot_11, QMetaData::Private },
	{ "update_target_s(const QString&)", &slot_12, QMetaData::Private },
	{ "update_rank_s(const QString&)", &slot_13, QMetaData::Private },
	{ "set_include_in_weight_s()", &slot_14, QMetaData::Private },
	{ "update_weight_s(const QString&)", &slot_15, QMetaData::Private },
	{ "update_buckets_s(const QString&)", &slot_16, QMetaData::Private },
	{ "update_min_s(const QString&)", &slot_17, QMetaData::Private },
	{ "update_max_s(const QString&)", &slot_18, QMetaData::Private },
	{ "set_store_reference_s()", &slot_19, QMetaData::Private },
	{ "set_store_diff_s()", &slot_20, QMetaData::Private },
	{ "set_store_abs_diff_s()", &slot_21, QMetaData::Private },
	{ "update_target_D(const QString&)", &slot_22, QMetaData::Private },
	{ "update_rank_D(const QString&)", &slot_23, QMetaData::Private },
	{ "set_include_in_weight_D()", &slot_24, QMetaData::Private },
	{ "update_weight_D(const QString&)", &slot_25, QMetaData::Private },
	{ "update_buckets_D(const QString&)", &slot_26, QMetaData::Private },
	{ "update_min_D(const QString&)", &slot_27, QMetaData::Private },
	{ "update_max_D(const QString&)", &slot_28, QMetaData::Private },
	{ "set_store_reference_D()", &slot_29, QMetaData::Private },
	{ "set_store_diff_D()", &slot_30, QMetaData::Private },
	{ "set_store_abs_diff_D()", &slot_31, QMetaData::Private },
	{ "update_target_sr(const QString&)", &slot_32, QMetaData::Private },
	{ "update_rank_sr(const QString&)", &slot_33, QMetaData::Private },
	{ "set_include_in_weight_sr()", &slot_34, QMetaData::Private },
	{ "update_weight_sr(const QString&)", &slot_35, QMetaData::Private },
	{ "update_buckets_sr(const QString&)", &slot_36, QMetaData::Private },
	{ "update_min_sr(const QString&)", &slot_37, QMetaData::Private },
	{ "update_max_sr(const QString&)", &slot_38, QMetaData::Private },
	{ "set_store_reference_sr()", &slot_39, QMetaData::Private },
	{ "set_store_diff_sr()", &slot_40, QMetaData::Private },
	{ "set_store_abs_diff_sr()", &slot_41, QMetaData::Private },
	{ "update_target_fr(const QString&)", &slot_42, QMetaData::Private },
	{ "update_rank_fr(const QString&)", &slot_43, QMetaData::Private },
	{ "set_include_in_weight_fr()", &slot_44, QMetaData::Private },
	{ "update_weight_fr(const QString&)", &slot_45, QMetaData::Private },
	{ "update_buckets_fr(const QString&)", &slot_46, QMetaData::Private },
	{ "update_min_fr(const QString&)", &slot_47, QMetaData::Private },
	{ "update_max_fr(const QString&)", &slot_48, QMetaData::Private },
	{ "set_store_reference_fr()", &slot_49, QMetaData::Private },
	{ "set_store_diff_fr()", &slot_50, QMetaData::Private },
	{ "set_store_abs_diff_fr()", &slot_51, QMetaData::Private },
	{ "update_target_rg(const QString&)", &slot_52, QMetaData::Private },
	{ "update_rank_rg(const QString&)", &slot_53, QMetaData::Private },
	{ "set_include_in_weight_rg()", &slot_54, QMetaData::Private },
	{ "update_weight_rg(const QString&)", &slot_55, QMetaData::Private },
	{ "update_buckets_rg(const QString&)", &slot_56, QMetaData::Private },
	{ "update_min_rg(const QString&)", &slot_57, QMetaData::Private },
	{ "update_max_rg(const QString&)", &slot_58, QMetaData::Private },
	{ "set_store_reference_rg()", &slot_59, QMetaData::Private },
	{ "set_store_diff_rg()", &slot_60, QMetaData::Private },
	{ "set_store_abs_diff_rg()", &slot_61, QMetaData::Private },
	{ "update_target_tau(const QString&)", &slot_62, QMetaData::Private },
	{ "update_rank_tau(const QString&)", &slot_63, QMetaData::Private },
	{ "set_include_in_weight_tau()", &slot_64, QMetaData::Private },
	{ "update_weight_tau(const QString&)", &slot_65, QMetaData::Private },
	{ "update_buckets_tau(const QString&)", &slot_66, QMetaData::Private },
	{ "update_min_tau(const QString&)", &slot_67, QMetaData::Private },
	{ "update_max_tau(const QString&)", &slot_68, QMetaData::Private },
	{ "set_store_reference_tau()", &slot_69, QMetaData::Private },
	{ "set_store_diff_tau()", &slot_70, QMetaData::Private },
	{ "set_store_abs_diff_tau()", &slot_71, QMetaData::Private },
	{ "update_target_eta(const QString&)", &slot_72, QMetaData::Private },
	{ "update_rank_eta(const QString&)", &slot_73, QMetaData::Private },
	{ "set_include_in_weight_eta()", &slot_74, QMetaData::Private },
	{ "update_weight_eta(const QString&)", &slot_75, QMetaData::Private },
	{ "update_buckets_eta(const QString&)", &slot_76, QMetaData::Private },
	{ "update_min_eta(const QString&)", &slot_77, QMetaData::Private },
	{ "update_max_eta(const QString&)", &slot_78, QMetaData::Private },
	{ "set_store_reference_eta()", &slot_79, QMetaData::Private },
	{ "set_store_diff_eta()", &slot_80, QMetaData::Private },
	{ "set_store_abs_diff_eta()", &slot_81, QMetaData::Private },
	{ "load_param()", &slot_82, QMetaData::Private },
	{ "reset_param()", &slot_83, QMetaData::Private },
	{ "save_param()", &slot_84, QMetaData::Private },
	{ "load_csv()", &slot_85, QMetaData::Private },
	{ "process_csv()", &slot_86, QMetaData::Private },
	{ "save_csv()", &slot_87, QMetaData::Private },
	{ "update_loaded()", &slot_88, QMetaData::Private },
	{ "loaded_select_all()", &slot_89, QMetaData::Private },
	{ "loaded_view()", &slot_90, QMetaData::Private },
	{ "loaded_merge()", &slot_91, QMetaData::Private },
	{ "loaded_set_ranges()", &slot_92, QMetaData::Private },
	{ "loaded_remove()", &slot_93, QMetaData::Private },
	{ "update_selected()", &slot_94, QMetaData::Private },
	{ "selected_select_all()", &slot_95, QMetaData::Private },
	{ "selected_merge()", &slot_96, QMetaData::Private },
	{ "selected_set_ranges()", &slot_97, QMetaData::Private },
	{ "selected_remove()", &slot_98, QMetaData::Private },
	{ "clear_display()", &slot_99, QMetaData::Private },
	{ "print()", &slot_100, QMetaData::Private },
	{ "update_font()", &slot_101, QMetaData::Private },
	{ "save()", &slot_102, QMetaData::Private },
	{ "cancel()", &slot_103, QMetaData::Private },
	{ "help()", &slot_104, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_105, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Comparative", parentObject,
	slot_tbl, 106,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Comparative.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Comparative::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Comparative" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Comparative::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_by_pct(); break;
    case 2: set_rank(); break;
    case 3: set_weight_controls(); break;
    case 4: set_by_ec(); break;
    case 5: set_active_s(); break;
    case 6: set_active_D(); break;
    case 7: set_active_sr(); break;
    case 8: set_active_fr(); break;
    case 9: set_active_rg(); break;
    case 10: set_active_tau(); break;
    case 11: set_active_eta(); break;
    case 12: update_target_s((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_rank_s((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: set_include_in_weight_s(); break;
    case 15: update_weight_s((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_buckets_s((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: update_min_s((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_max_s((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: set_store_reference_s(); break;
    case 20: set_store_diff_s(); break;
    case 21: set_store_abs_diff_s(); break;
    case 22: update_target_D((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: update_rank_D((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: set_include_in_weight_D(); break;
    case 25: update_weight_D((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: update_buckets_D((const QString&)static_QUType_QString.get(_o+1)); break;
    case 27: update_min_D((const QString&)static_QUType_QString.get(_o+1)); break;
    case 28: update_max_D((const QString&)static_QUType_QString.get(_o+1)); break;
    case 29: set_store_reference_D(); break;
    case 30: set_store_diff_D(); break;
    case 31: set_store_abs_diff_D(); break;
    case 32: update_target_sr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 33: update_rank_sr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 34: set_include_in_weight_sr(); break;
    case 35: update_weight_sr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 36: update_buckets_sr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 37: update_min_sr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 38: update_max_sr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 39: set_store_reference_sr(); break;
    case 40: set_store_diff_sr(); break;
    case 41: set_store_abs_diff_sr(); break;
    case 42: update_target_fr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 43: update_rank_fr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 44: set_include_in_weight_fr(); break;
    case 45: update_weight_fr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 46: update_buckets_fr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 47: update_min_fr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 48: update_max_fr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 49: set_store_reference_fr(); break;
    case 50: set_store_diff_fr(); break;
    case 51: set_store_abs_diff_fr(); break;
    case 52: update_target_rg((const QString&)static_QUType_QString.get(_o+1)); break;
    case 53: update_rank_rg((const QString&)static_QUType_QString.get(_o+1)); break;
    case 54: set_include_in_weight_rg(); break;
    case 55: update_weight_rg((const QString&)static_QUType_QString.get(_o+1)); break;
    case 56: update_buckets_rg((const QString&)static_QUType_QString.get(_o+1)); break;
    case 57: update_min_rg((const QString&)static_QUType_QString.get(_o+1)); break;
    case 58: update_max_rg((const QString&)static_QUType_QString.get(_o+1)); break;
    case 59: set_store_reference_rg(); break;
    case 60: set_store_diff_rg(); break;
    case 61: set_store_abs_diff_rg(); break;
    case 62: update_target_tau((const QString&)static_QUType_QString.get(_o+1)); break;
    case 63: update_rank_tau((const QString&)static_QUType_QString.get(_o+1)); break;
    case 64: set_include_in_weight_tau(); break;
    case 65: update_weight_tau((const QString&)static_QUType_QString.get(_o+1)); break;
    case 66: update_buckets_tau((const QString&)static_QUType_QString.get(_o+1)); break;
    case 67: update_min_tau((const QString&)static_QUType_QString.get(_o+1)); break;
    case 68: update_max_tau((const QString&)static_QUType_QString.get(_o+1)); break;
    case 69: set_store_reference_tau(); break;
    case 70: set_store_diff_tau(); break;
    case 71: set_store_abs_diff_tau(); break;
    case 72: update_target_eta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 73: update_rank_eta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 74: set_include_in_weight_eta(); break;
    case 75: update_weight_eta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 76: update_buckets_eta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 77: update_min_eta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 78: update_max_eta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 79: set_store_reference_eta(); break;
    case 80: set_store_diff_eta(); break;
    case 81: set_store_abs_diff_eta(); break;
    case 82: load_param(); break;
    case 83: reset_param(); break;
    case 84: save_param(); break;
    case 85: load_csv(); break;
    case 86: process_csv(); break;
    case 87: save_csv(); break;
    case 88: update_loaded(); break;
    case 89: loaded_select_all(); break;
    case 90: loaded_view(); break;
    case 91: loaded_merge(); break;
    case 92: loaded_set_ranges(); break;
    case 93: loaded_remove(); break;
    case 94: update_selected(); break;
    case 95: selected_select_all(); break;
    case 96: selected_merge(); break;
    case 97: selected_set_ranges(); break;
    case 98: selected_remove(); break;
    case 99: clear_display(); break;
    case 100: print(); break;
    case 101: update_font(); break;
    case 102: save(); break;
    case 103: cancel(); break;
    case 104: help(); break;
    case 105: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Comparative::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Comparative::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Comparative::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
