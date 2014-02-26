/****************************************************************************
** US_Hydrodyn meta object code from reading C++ file 'us_hydrodyn.h'
**
** Created: Mon Jan 20 09:32:49 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn.h"
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

const char *US_Hydrodyn::className() const
{
    return "US_Hydrodyn";
}

QMetaObject *US_Hydrodyn::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn( "US_Hydrodyn", &US_Hydrodyn::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"show_zeno_options", 0, 0 };
    static const QUMethod slot_1 = {"display_default_differences", 0, 0 };
    static const QUMethod slot_2 = {"clear_display", 0, 0 };
    static const QUMethod slot_3 = {"reload_pdb", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_4 = {"calc_somo", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_5 = {"calc_grid_pdb", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_6 = {"calc_grid", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_7 = {"calc_hydro", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "bead_model", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"calc_iqq", 2, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "bead_model", &static_QUType_bool, 0, QUParameter::In },
	{ "create_native_saxs", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"calc_iqq", 3, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "bead_model", &static_QUType_bool, 0, QUParameter::In },
	{ "create_native_saxs", &static_QUType_bool, 0, QUParameter::In },
	{ "do_raise", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"calc_iqq", 4, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "bead_model", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"calc_prr", 2, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "bead_model", &static_QUType_bool, 0, QUParameter::In },
	{ "create_native_saxs", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"calc_prr", 3, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "bead_model", &static_QUType_bool, 0, QUParameter::In },
	{ "create_native_saxs", &static_QUType_bool, 0, QUParameter::In },
	{ "do_raise", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"calc_prr", 4, param_slot_13 };
    static const QUMethod slot_14 = {"select_save_params", 0, 0 };
    static const QUMethod slot_15 = {"show_saxs_options", 0, 0 };
    static const QUMethod slot_16 = {"show_bd_options", 0, 0 };
    static const QUMethod slot_17 = {"show_anaflex_options", 0, 0 };
    static const QUMethod slot_18 = {"show_dmd_options", 0, 0 };
    static const QUMethod slot_19 = {"read_residue_file", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"read_config", 2, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "QFile", QUParameter::InOut }
    };
    static const QUMethod slot_21 = {"read_config", 2, param_slot_21 };
    static const QUMethod slot_22 = {"show_misc", 0, 0 };
    static const QUMethod slot_23 = {"bd_prepare", 0, 0 };
    static const QUMethod slot_24 = {"bd_load", 0, 0 };
    static const QUMethod slot_25 = {"bd_edit", 0, 0 };
    static const QUMethod slot_26 = {"bd_run", 0, 0 };
    static const QUMethod slot_27 = {"bd_load_results", 0, 0 };
    static const QUMethod slot_28 = {"anaflex_prepare", 0, 0 };
    static const QUMethod slot_29 = {"anaflex_load", 0, 0 };
    static const QUMethod slot_30 = {"anaflex_edit", 0, 0 };
    static const QUMethod slot_31 = {"anaflex_run", 0, 0 };
    static const QUMethod slot_32 = {"anaflex_load_results", 0, 0 };
    static const QUMethod slot_33 = {"stop_calc", 0, 0 };
    static const QUMethod slot_34 = {"visualize", 0, 0 };
    static const QUParameter param_slot_35[] = {
	{ "movie_frame", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_35 = {"visualize", 1, param_slot_35 };
    static const QUParameter param_slot_36[] = {
	{ "movie_frame", &static_QUType_bool, 0, QUParameter::In },
	{ "dir", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_36 = {"visualize", 2, param_slot_36 };
    static const QUParameter param_slot_37[] = {
	{ "movie_frame", &static_QUType_bool, 0, QUParameter::In },
	{ "dir", &static_QUType_QString, 0, QUParameter::In },
	{ "scale", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_37 = {"visualize", 3, param_slot_37 };
    static const QUParameter param_slot_38[] = {
	{ "movie_frame", &static_QUType_bool, 0, QUParameter::In },
	{ "dir", &static_QUType_QString, 0, QUParameter::In },
	{ "scale", &static_QUType_ptr, "float", QUParameter::In },
	{ "black_background", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"visualize", 4, param_slot_38 };
    static const QUMethod slot_39 = {"dmd_static_pairs", 0, 0 };
    static const QUMethod slot_40 = {"pdb_saxs", 0, 0 };
    static const QUParameter param_slot_41[] = {
	{ "create_native_saxs", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_41 = {"pdb_saxs", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ "create_native_saxs", &static_QUType_bool, 0, QUParameter::In },
	{ "do_raise", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_42 = {"pdb_saxs", 2, param_slot_42 };
    static const QUMethod slot_43 = {"bead_saxs", 0, 0 };
    static const QUParameter param_slot_44[] = {
	{ "create_native_saxs", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_44 = {"bead_saxs", 1, param_slot_44 };
    static const QUParameter param_slot_45[] = {
	{ "create_native_saxs", &static_QUType_bool, 0, QUParameter::In },
	{ "do_raise", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_45 = {"bead_saxs", 2, param_slot_45 };
    static const QUParameter param_slot_46[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_46 = {"pdb_hydrate_for_saxs", 1, param_slot_46 };
    static const QUParameter param_slot_47[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "quiet", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_47 = {"pdb_hydrate_for_saxs", 2, param_slot_47 };
    static const QUMethod slot_48 = {"make_test_set", 0, 0 };
    static const QUMethod slot_49 = {"rescale_bead_model", 0, 0 };
    static const QUParameter param_slot_50[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_50 = {"equi_grid_bead_model", 1, param_slot_50 };
    static const QUParameter param_slot_51[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ "dR", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_51 = {"equi_grid_bead_model", 2, param_slot_51 };
    static const QUMethod slot_52 = {"browflex_readFromStdout", 0, 0 };
    static const QUMethod slot_53 = {"browflex_readFromStderr", 0, 0 };
    static const QUMethod slot_54 = {"browflex_launchFinished", 0, 0 };
    static const QUMethod slot_55 = {"browflex_processExited", 0, 0 };
    static const QUMethod slot_56 = {"anaflex_readFromStdout", 0, 0 };
    static const QUMethod slot_57 = {"anaflex_readFromStderr", 0, 0 };
    static const QUMethod slot_58 = {"anaflex_launchFinished", 0, 0 };
    static const QUMethod slot_59 = {"anaflex_processExited", 0, 0 };
    static const QUMethod slot_60 = {"load_pdb", 0, 0 };
    static const QUMethod slot_61 = {"show_batch", 0, 0 };
    static const QUParameter param_slot_62[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_62 = {"read_pdb", 2, param_slot_62 };
    static const QUParameter param_slot_63[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "filename", &static_QUType_QString, 0, QUParameter::In },
	{ "only_overlap", &static_QUType_bool, 0, QUParameter::InOut }
    };
    static const QUMethod slot_63 = {"read_bead_model", 3, param_slot_63 };
    static const QUMethod slot_64 = {"load_bead_model", 0, 0 };
    static const QUMethod slot_65 = {"setupGUI", 0, 0 };
    static const QUMethod slot_66 = {"select_residue_file", 0, 0 };
    static const QUParameter param_slot_67[] = {
	{ 0, &static_QUType_ptr, "struct PDB_chain", QUParameter::In }
    };
    static const QUMethod slot_67 = {"clear_temp_chain", 1, param_slot_67 };
    static const QUParameter param_slot_68[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "struct PDB_chain", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x03", QUParameter::In }
    };
    static const QUMethod slot_68 = {"assign_atom", 4, param_slot_68 };
    static const QUMethod slot_69 = {"cancel", 0, 0 };
    static const QUMethod slot_70 = {"help", 0, 0 };
    static const QUMethod slot_71 = {"config", 0, 0 };
    static const QUMethod slot_72 = {"edit_atom", 0, 0 };
    static const QUMethod slot_73 = {"hybrid", 0, 0 };
    static const QUMethod slot_74 = {"residue", 0, 0 };
    static const QUMethod slot_75 = {"do_saxs", 0, 0 };
    static const QUParameter param_slot_76[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_76 = {"select_model", 1, param_slot_76 };
    static const QUParameter param_slot_77[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "error_string", &static_QUType_varptr, "\x03", QUParameter::In }
    };
    static const QUMethod slot_77 = {"create_beads", 2, param_slot_77 };
    static const QUParameter param_slot_78[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "error_string", &static_QUType_varptr, "\x03", QUParameter::In },
	{ "quiet", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_78 = {"create_beads", 3, param_slot_78 };
    static const QUParameter param_slot_79[] = {
	{ 0, &static_QUType_ptr, "PDB_model", QUParameter::In }
    };
    static const QUMethod slot_79 = {"get_atom_map", 1, param_slot_79 };
    static const QUParameter param_slot_80[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "error_string", &static_QUType_varptr, "\x03", QUParameter::In },
	{ 0, &static_QUType_ptr, "PDB_model", QUParameter::In }
    };
    static const QUMethod slot_80 = {"check_for_missing_atoms", 3, param_slot_80 };
    static const QUParameter param_slot_81[] = {
	{ "model", &static_QUType_ptr, "PDB_model", QUParameter::In }
    };
    static const QUMethod slot_81 = {"build_molecule_maps", 1, param_slot_81 };
    static const QUParameter param_slot_82[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "sc", &static_QUType_bool, 0, QUParameter::In },
	{ "mc", &static_QUType_bool, 0, QUParameter::In },
	{ "buried", &static_QUType_bool, 0, QUParameter::In },
	{ "tolerance", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_82 = {"overlap_check", 5, param_slot_82 };
    static const QUParameter param_slot_83[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_83 = {"compute_asa", 1, param_slot_83 };
    static const QUParameter param_slot_84[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "bd_mode", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_84 = {"compute_asa", 2, param_slot_84 };
    static const QUMethod slot_85 = {"show_asa", 0, 0 };
    static const QUMethod slot_86 = {"show_bd", 0, 0 };
    static const QUMethod slot_87 = {"show_overlap", 0, 0 };
    static const QUMethod slot_88 = {"show_grid_overlap", 0, 0 };
    static const QUMethod slot_89 = {"show_bead_output", 0, 0 };
    static const QUMethod slot_90 = {"show_hydro", 0, 0 };
    static const QUMethod slot_91 = {"show_grid", 0, 0 };
    static const QUMethod slot_92 = {"show_advanced_config", 0, 0 };
    static const QUMethod slot_93 = {"view_pdb", 0, 0 };
    static const QUMethod slot_94 = {"pdb_tool", 0, 0 };
    static const QUMethod slot_95 = {"pdb_parsing", 0, 0 };
    static const QUMethod slot_96 = {"pdb_visualization", 0, 0 };
    static const QUMethod slot_97 = {"view_asa", 0, 0 };
    static const QUMethod slot_98 = {"view_bead_model", 0, 0 };
    static const QUParameter param_slot_99[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_99 = {"view_file", 1, param_slot_99 };
    static const QUParameter param_slot_100[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ "title", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_100 = {"view_file", 2, param_slot_100 };
    static const QUMethod slot_101 = {"bead_check", 0, 0 };
    static const QUParameter param_slot_102[] = {
	{ "use_threshold", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_102 = {"bead_check", 1, param_slot_102 };
    static const QUParameter param_slot_103[] = {
	{ "use_threshold", &static_QUType_bool, 0, QUParameter::In },
	{ "message_type", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_103 = {"bead_check", 2, param_slot_103 };
    static const QUMethod slot_104 = {"load_config", 0, 0 };
    static const QUMethod slot_105 = {"write_config", 0, 0 };
    static const QUParameter param_slot_106[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_106 = {"write_config", 1, param_slot_106 };
    static const QUMethod slot_107 = {"reset", 0, 0 };
    static const QUMethod slot_108 = {"set_default", 0, 0 };
    static const QUParameter param_slot_109[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_109 = {"update_bead_model_file", 1, param_slot_109 };
    static const QUParameter param_slot_110[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_110 = {"update_bead_model_prefix", 1, param_slot_110 };
    static const QUMethod slot_111 = {"radial_reduction", 0, 0 };
    static const QUParameter param_slot_112[] = {
	{ "from_grid", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_112 = {"radial_reduction", 1, param_slot_112 };
    static const QUMethod slot_113 = {"show_hydro_results", 0, 0 };
    static const QUMethod slot_114 = {"open_hydro_results", 0, 0 };
    static const QUParameter param_slot_115[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In }
    };
    static const QUMethod slot_115 = {"write_bead_asa", 2, param_slot_115 };
    static const QUParameter param_slot_116[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In }
    };
    static const QUMethod slot_116 = {"write_bead_tsv", 2, param_slot_116 };
    static const QUParameter param_slot_117[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In }
    };
    static const QUMethod slot_117 = {"write_bead_ebf", 2, param_slot_117 };
    static const QUParameter param_slot_118[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In }
    };
    static const QUMethod slot_118 = {"write_bead_spt", 2, param_slot_118 };
    static const QUParameter param_slot_119[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In },
	{ "movie_frame", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_119 = {"write_bead_spt", 3, param_slot_119 };
    static const QUParameter param_slot_120[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In },
	{ "movie_frame", &static_QUType_bool, 0, QUParameter::In },
	{ "scale", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_120 = {"write_bead_spt", 4, param_slot_120 };
    static const QUParameter param_slot_121[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In },
	{ "movie_frame", &static_QUType_bool, 0, QUParameter::In },
	{ "scale", &static_QUType_ptr, "float", QUParameter::In },
	{ "black_background", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_121 = {"write_bead_spt", 5, param_slot_121 };
    static const QUParameter param_slot_122[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In }
    };
    static const QUMethod slot_122 = {"write_bead_model", 2, param_slot_122 };
    static const QUParameter param_slot_123[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In },
	{ "extra_text", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_123 = {"write_bead_model", 3, param_slot_123 };
    static const QUParameter param_slot_124[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In }
    };
    static const QUMethod slot_124 = {"write_corr", 2, param_slot_124 };
    static const QUParameter param_slot_125[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In }
    };
    static const QUMethod slot_125 = {"read_corr", 3, param_slot_125 };
    static const QUParameter param_slot_126[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_126 = {"printError", 1, param_slot_126 };
    static const QUParameter param_slot_127[] = {
	{ 0, &static_QUType_ptr, "QProcess", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_127 = {"closeAttnt", 2, param_slot_127 };
    static const QUParameter param_slot_128[] = {
	{ 0, &static_QUType_ptr, "struct PDB_model", QUParameter::In }
    };
    static const QUMethod slot_128 = {"calc_vbar", 1, param_slot_128 };
    static const QUMethod slot_129 = {"update_vbar", 0, 0 };
    static const QUMethod slot_130 = {"append_options_log_somo", 0, 0 };
    static const QUMethod slot_131 = {"append_options_log_atob", 0, 0 };
    static const QUParameter param_slot_132[] = {
	{ 0, &static_QUType_ptr, "vector<PDB_model>", QUParameter::In }
    };
    static const QUMethod slot_132 = {"list_model_vector", 1, param_slot_132 };
    static const QUParameter param_slot_133[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out }
    };
    static const QUMethod slot_133 = {"default_differences_load_pdb", 1, param_slot_133 };
    static const QUParameter param_slot_134[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out }
    };
    static const QUMethod slot_134 = {"default_differences_somo", 1, param_slot_134 };
    static const QUParameter param_slot_135[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out }
    };
    static const QUMethod slot_135 = {"default_differences_grid", 1, param_slot_135 };
    static const QUParameter param_slot_136[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out }
    };
    static const QUMethod slot_136 = {"default_differences_hydro", 1, param_slot_136 };
    static const QUParameter param_slot_137[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out }
    };
    static const QUMethod slot_137 = {"default_differences_misc", 1, param_slot_137 };
    static const QUParameter param_slot_138[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out }
    };
    static const QUMethod slot_138 = {"default_differences_saxs_options", 1, param_slot_138 };
    static const QUMethod slot_139 = {"save", 0, 0 };
    static const QUMethod slot_140 = {"print", 0, 0 };
    static const QUMethod slot_141 = {"update_font", 0, 0 };
    static const QUMethod slot_142 = {"set_calcAutoHydro", 0, 0 };
    static const QUMethod slot_143 = {"set_setSuffix", 0, 0 };
    static const QUMethod slot_144 = {"set_overwrite", 0, 0 };
    static const QUMethod slot_145 = {"set_saveParams", 0, 0 };
    static const QUMethod slot_146 = {"select_comparative", 0, 0 };
    static const QUMethod slot_147 = {"best_analysis", 0, 0 };
    static const QUMethod slot_148 = {"dmd_run", 0, 0 };
    static const QUParameter param_slot_149[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_149 = {"bd_valid_browflex_main", 2, param_slot_149 };
    static const QUParameter param_slot_150[] = {
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_150 = {"bd_load_error", 1, param_slot_150 };
    static const QUParameter param_slot_151[] = {
	{ "dir", &static_QUType_QString, 0, QUParameter::In },
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_151 = {"bd_edit_util", 2, param_slot_151 };
    static const QUMethod slot_152 = {"bd_load_results_after_anaflex", 0, 0 };
    static const QUParameter param_slot_153[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_153 = {"anaflex_valid_anaflex_main", 2, param_slot_153 };
    static const QUParameter param_slot_154[] = {
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_154 = {"anaflex_load_error", 1, param_slot_154 };
    static const QUParameter param_slot_155[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_155 = {"create_browflex_files", 1, param_slot_155 };
    static const QUParameter param_slot_156[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_156 = {"run_browflex", 1, param_slot_156 };
    static const QUParameter param_slot_157[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_157 = {"browflex_get_no_of_beads", 2, param_slot_157 };
    static const QUParameter param_slot_158[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_158 = {"compute_pb_normals", 1, param_slot_158 };
    static const QUParameter param_slot_159[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "PDB_model", QUParameter::In }
    };
    static const QUMethod slot_159 = {"build_pb_structures", 2, param_slot_159 };
    static const QUParameter param_slot_160[] = {
	{ "p1", &static_QUType_ptr, "point", QUParameter::In },
	{ "p2", &static_QUType_ptr, "point", QUParameter::In }
    };
    static const QUMethod slot_160 = {"minus", 2, param_slot_160 };
    static const QUParameter param_slot_161[] = {
	{ "p1", &static_QUType_ptr, "point", QUParameter::In },
	{ "p2", &static_QUType_ptr, "point", QUParameter::In }
    };
    static const QUMethod slot_161 = {"cross", 2, param_slot_161 };
    static const QUParameter param_slot_162[] = {
	{ "p1", &static_QUType_ptr, "point", QUParameter::In },
	{ "p2", &static_QUType_ptr, "point", QUParameter::In }
    };
    static const QUMethod slot_162 = {"dot", 2, param_slot_162 };
    static const QUParameter param_slot_163[] = {
	{ "p1", &static_QUType_ptr, "point", QUParameter::In }
    };
    static const QUMethod slot_163 = {"normal", 1, param_slot_163 };
    static const QUParameter param_slot_164[] = {
	{ "a1", &static_QUType_ptr, "PDB_atom", QUParameter::In },
	{ "a2", &static_QUType_ptr, "PDB_atom", QUParameter::In },
	{ "a3", &static_QUType_ptr, "PDB_atom", QUParameter::In }
    };
    static const QUMethod slot_164 = {"plane", 3, param_slot_164 };
    static const QUParameter param_slot_165[] = {
	{ "p1", &static_QUType_ptr, "point", QUParameter::In },
	{ "p2", &static_QUType_ptr, "point", QUParameter::In },
	{ "p3", &static_QUType_ptr, "point", QUParameter::In }
    };
    static const QUMethod slot_165 = {"plane", 3, param_slot_165 };
    static const QUParameter param_slot_166[] = {
	{ "v", &static_QUType_ptr, "vector<point>", QUParameter::In }
    };
    static const QUMethod slot_166 = {"average", 1, param_slot_166 };
    static const QUParameter param_slot_167[] = {
	{ "p1", &static_QUType_ptr, "point", QUParameter::In },
	{ "p2", &static_QUType_ptr, "point", QUParameter::In }
    };
    static const QUMethod slot_167 = {"dist", 2, param_slot_167 };
    static const QUParameter param_slot_168[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_168 = {"compute_bd_connections", 1, param_slot_168 };
    static const QUParameter param_slot_169[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "fname", &static_QUType_QString, 0, QUParameter::In },
	{ "model", &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In }
    };
    static const QUMethod slot_169 = {"write_pdb", 3, param_slot_169 };
    static const QUParameter param_slot_170[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "fname", &static_QUType_QString, 0, QUParameter::In },
	{ "model", &static_QUType_ptr, "PDB_model", QUParameter::In },
	{ "thresh", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_170 = {"write_contact_plot", 4, param_slot_170 };
    static const QUParameter param_slot_171[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "fname", &static_QUType_QString, 0, QUParameter::In },
	{ "model", &static_QUType_ptr, "vector<PDB_atom>", QUParameter::In },
	{ "thresh", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_171 = {"write_contact_plot", 4, param_slot_171 };
    static const QUParameter param_slot_172[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_172 = {"create_anaflex_files", 1, param_slot_172 };
    static const QUParameter param_slot_173[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "use_mode", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_173 = {"create_anaflex_files", 2, param_slot_173 };
    static const QUParameter param_slot_174[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "use_mode", &static_QUType_int, 0, QUParameter::In },
	{ "sub_mode", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_174 = {"create_anaflex_files", 3, param_slot_174 };
    static const QUParameter param_slot_175[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_175 = {"run_anaflex", 1, param_slot_175 };
    static const QUParameter param_slot_176[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "use_mode", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_176 = {"run_anaflex", 2, param_slot_176 };
    static const QUParameter param_slot_177[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "use_mode", &static_QUType_int, 0, QUParameter::In },
	{ "sub_mode", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_177 = {"run_anaflex", 3, param_slot_177 };
    static const QUMethod slot_178 = {"run_us_config", 0, 0 };
    static const QUMethod slot_179 = {"run_us_admin", 0, 0 };
    static const QUParameter param_slot_180[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_180 = {"closeEvent", 1, param_slot_180 };
    static const QMetaData slot_tbl[] = {
	{ "show_zeno_options()", &slot_0, QMetaData::Public },
	{ "display_default_differences()", &slot_1, QMetaData::Public },
	{ "clear_display()", &slot_2, QMetaData::Public },
	{ "reload_pdb()", &slot_3, QMetaData::Public },
	{ "calc_somo()", &slot_4, QMetaData::Public },
	{ "calc_grid_pdb()", &slot_5, QMetaData::Public },
	{ "calc_grid()", &slot_6, QMetaData::Public },
	{ "calc_hydro()", &slot_7, QMetaData::Public },
	{ "calc_iqq(bool)", &slot_8, QMetaData::Public },
	{ "calc_iqq(bool,bool)", &slot_9, QMetaData::Public },
	{ "calc_iqq(bool,bool,bool)", &slot_10, QMetaData::Public },
	{ "calc_prr(bool)", &slot_11, QMetaData::Public },
	{ "calc_prr(bool,bool)", &slot_12, QMetaData::Public },
	{ "calc_prr(bool,bool,bool)", &slot_13, QMetaData::Public },
	{ "select_save_params()", &slot_14, QMetaData::Public },
	{ "show_saxs_options()", &slot_15, QMetaData::Public },
	{ "show_bd_options()", &slot_16, QMetaData::Public },
	{ "show_anaflex_options()", &slot_17, QMetaData::Public },
	{ "show_dmd_options()", &slot_18, QMetaData::Public },
	{ "read_residue_file()", &slot_19, QMetaData::Public },
	{ "read_config(const QString&)", &slot_20, QMetaData::Public },
	{ "read_config(QFile&)", &slot_21, QMetaData::Public },
	{ "show_misc()", &slot_22, QMetaData::Public },
	{ "bd_prepare()", &slot_23, QMetaData::Public },
	{ "bd_load()", &slot_24, QMetaData::Public },
	{ "bd_edit()", &slot_25, QMetaData::Public },
	{ "bd_run()", &slot_26, QMetaData::Public },
	{ "bd_load_results()", &slot_27, QMetaData::Public },
	{ "anaflex_prepare()", &slot_28, QMetaData::Public },
	{ "anaflex_load()", &slot_29, QMetaData::Public },
	{ "anaflex_edit()", &slot_30, QMetaData::Public },
	{ "anaflex_run()", &slot_31, QMetaData::Public },
	{ "anaflex_load_results()", &slot_32, QMetaData::Public },
	{ "stop_calc()", &slot_33, QMetaData::Public },
	{ "visualize()", &slot_34, QMetaData::Public },
	{ "visualize(bool)", &slot_35, QMetaData::Public },
	{ "visualize(bool,QString)", &slot_36, QMetaData::Public },
	{ "visualize(bool,QString,float)", &slot_37, QMetaData::Public },
	{ "visualize(bool,QString,float,bool)", &slot_38, QMetaData::Public },
	{ "dmd_static_pairs()", &slot_39, QMetaData::Public },
	{ "pdb_saxs()", &slot_40, QMetaData::Public },
	{ "pdb_saxs(bool)", &slot_41, QMetaData::Public },
	{ "pdb_saxs(bool,bool)", &slot_42, QMetaData::Public },
	{ "bead_saxs()", &slot_43, QMetaData::Public },
	{ "bead_saxs(bool)", &slot_44, QMetaData::Public },
	{ "bead_saxs(bool,bool)", &slot_45, QMetaData::Public },
	{ "pdb_hydrate_for_saxs()", &slot_46, QMetaData::Public },
	{ "pdb_hydrate_for_saxs(bool)", &slot_47, QMetaData::Public },
	{ "make_test_set()", &slot_48, QMetaData::Private },
	{ "rescale_bead_model()", &slot_49, QMetaData::Private },
	{ "equi_grid_bead_model()", &slot_50, QMetaData::Private },
	{ "equi_grid_bead_model(double)", &slot_51, QMetaData::Private },
	{ "browflex_readFromStdout()", &slot_52, QMetaData::Private },
	{ "browflex_readFromStderr()", &slot_53, QMetaData::Private },
	{ "browflex_launchFinished()", &slot_54, QMetaData::Private },
	{ "browflex_processExited()", &slot_55, QMetaData::Private },
	{ "anaflex_readFromStdout()", &slot_56, QMetaData::Private },
	{ "anaflex_readFromStderr()", &slot_57, QMetaData::Private },
	{ "anaflex_launchFinished()", &slot_58, QMetaData::Private },
	{ "anaflex_processExited()", &slot_59, QMetaData::Private },
	{ "load_pdb()", &slot_60, QMetaData::Private },
	{ "show_batch()", &slot_61, QMetaData::Private },
	{ "read_pdb(const QString&)", &slot_62, QMetaData::Private },
	{ "read_bead_model(QString,bool&)", &slot_63, QMetaData::Private },
	{ "load_bead_model()", &slot_64, QMetaData::Private },
	{ "setupGUI()", &slot_65, QMetaData::Private },
	{ "select_residue_file()", &slot_66, QMetaData::Private },
	{ "clear_temp_chain(struct PDB_chain*)", &slot_67, QMetaData::Private },
	{ "assign_atom(const QString&,struct PDB_chain*,QString*)", &slot_68, QMetaData::Private },
	{ "cancel()", &slot_69, QMetaData::Private },
	{ "help()", &slot_70, QMetaData::Private },
	{ "config()", &slot_71, QMetaData::Private },
	{ "edit_atom()", &slot_72, QMetaData::Private },
	{ "hybrid()", &slot_73, QMetaData::Private },
	{ "residue()", &slot_74, QMetaData::Private },
	{ "do_saxs()", &slot_75, QMetaData::Private },
	{ "select_model(int)", &slot_76, QMetaData::Private },
	{ "create_beads(QString*)", &slot_77, QMetaData::Private },
	{ "create_beads(QString*,bool)", &slot_78, QMetaData::Private },
	{ "get_atom_map(PDB_model*)", &slot_79, QMetaData::Private },
	{ "check_for_missing_atoms(QString*,PDB_model*)", &slot_80, QMetaData::Private },
	{ "build_molecule_maps(PDB_model*)", &slot_81, QMetaData::Private },
	{ "overlap_check(bool,bool,bool,double)", &slot_82, QMetaData::Private },
	{ "compute_asa()", &slot_83, QMetaData::Private },
	{ "compute_asa(bool)", &slot_84, QMetaData::Private },
	{ "show_asa()", &slot_85, QMetaData::Private },
	{ "show_bd()", &slot_86, QMetaData::Private },
	{ "show_overlap()", &slot_87, QMetaData::Private },
	{ "show_grid_overlap()", &slot_88, QMetaData::Private },
	{ "show_bead_output()", &slot_89, QMetaData::Private },
	{ "show_hydro()", &slot_90, QMetaData::Private },
	{ "show_grid()", &slot_91, QMetaData::Private },
	{ "show_advanced_config()", &slot_92, QMetaData::Private },
	{ "view_pdb()", &slot_93, QMetaData::Private },
	{ "pdb_tool()", &slot_94, QMetaData::Private },
	{ "pdb_parsing()", &slot_95, QMetaData::Private },
	{ "pdb_visualization()", &slot_96, QMetaData::Private },
	{ "view_asa()", &slot_97, QMetaData::Private },
	{ "view_bead_model()", &slot_98, QMetaData::Private },
	{ "view_file(const QString&)", &slot_99, QMetaData::Private },
	{ "view_file(const QString&,QString)", &slot_100, QMetaData::Private },
	{ "bead_check()", &slot_101, QMetaData::Private },
	{ "bead_check(bool)", &slot_102, QMetaData::Private },
	{ "bead_check(bool,bool)", &slot_103, QMetaData::Private },
	{ "load_config()", &slot_104, QMetaData::Private },
	{ "write_config()", &slot_105, QMetaData::Private },
	{ "write_config(const QString&)", &slot_106, QMetaData::Private },
	{ "reset()", &slot_107, QMetaData::Private },
	{ "set_default()", &slot_108, QMetaData::Private },
	{ "update_bead_model_file(const QString&)", &slot_109, QMetaData::Private },
	{ "update_bead_model_prefix(const QString&)", &slot_110, QMetaData::Private },
	{ "radial_reduction()", &slot_111, QMetaData::Private },
	{ "radial_reduction(bool)", &slot_112, QMetaData::Private },
	{ "show_hydro_results()", &slot_113, QMetaData::Private },
	{ "open_hydro_results()", &slot_114, QMetaData::Private },
	{ "write_bead_asa(QString,vector<PDB_atom>*)", &slot_115, QMetaData::Private },
	{ "write_bead_tsv(QString,vector<PDB_atom>*)", &slot_116, QMetaData::Private },
	{ "write_bead_ebf(QString,vector<PDB_atom>*)", &slot_117, QMetaData::Private },
	{ "write_bead_spt(QString,vector<PDB_atom>*)", &slot_118, QMetaData::Private },
	{ "write_bead_spt(QString,vector<PDB_atom>*,bool)", &slot_119, QMetaData::Private },
	{ "write_bead_spt(QString,vector<PDB_atom>*,bool,float)", &slot_120, QMetaData::Private },
	{ "write_bead_spt(QString,vector<PDB_atom>*,bool,float,bool)", &slot_121, QMetaData::Private },
	{ "write_bead_model(QString,vector<PDB_atom>*)", &slot_122, QMetaData::Private },
	{ "write_bead_model(QString,vector<PDB_atom>*,QString)", &slot_123, QMetaData::Private },
	{ "write_corr(QString,vector<PDB_atom>*)", &slot_124, QMetaData::Private },
	{ "read_corr(QString,vector<PDB_atom>*)", &slot_125, QMetaData::Private },
	{ "printError(const QString&)", &slot_126, QMetaData::Private },
	{ "closeAttnt(QProcess*,QString)", &slot_127, QMetaData::Private },
	{ "calc_vbar(struct PDB_model*)", &slot_128, QMetaData::Private },
	{ "update_vbar()", &slot_129, QMetaData::Private },
	{ "append_options_log_somo()", &slot_130, QMetaData::Private },
	{ "append_options_log_atob()", &slot_131, QMetaData::Private },
	{ "list_model_vector(vector<PDB_model>*)", &slot_132, QMetaData::Private },
	{ "default_differences_load_pdb()", &slot_133, QMetaData::Private },
	{ "default_differences_somo()", &slot_134, QMetaData::Private },
	{ "default_differences_grid()", &slot_135, QMetaData::Private },
	{ "default_differences_hydro()", &slot_136, QMetaData::Private },
	{ "default_differences_misc()", &slot_137, QMetaData::Private },
	{ "default_differences_saxs_options()", &slot_138, QMetaData::Private },
	{ "save()", &slot_139, QMetaData::Private },
	{ "print()", &slot_140, QMetaData::Private },
	{ "update_font()", &slot_141, QMetaData::Private },
	{ "set_calcAutoHydro()", &slot_142, QMetaData::Private },
	{ "set_setSuffix()", &slot_143, QMetaData::Private },
	{ "set_overwrite()", &slot_144, QMetaData::Private },
	{ "set_saveParams()", &slot_145, QMetaData::Private },
	{ "select_comparative()", &slot_146, QMetaData::Private },
	{ "best_analysis()", &slot_147, QMetaData::Private },
	{ "dmd_run()", &slot_148, QMetaData::Private },
	{ "bd_valid_browflex_main(QString)", &slot_149, QMetaData::Private },
	{ "bd_load_error(QString)", &slot_150, QMetaData::Private },
	{ "bd_edit_util(QString,QString)", &slot_151, QMetaData::Private },
	{ "bd_load_results_after_anaflex()", &slot_152, QMetaData::Private },
	{ "anaflex_valid_anaflex_main(QString)", &slot_153, QMetaData::Private },
	{ "anaflex_load_error(QString)", &slot_154, QMetaData::Private },
	{ "create_browflex_files()", &slot_155, QMetaData::Private },
	{ "run_browflex()", &slot_156, QMetaData::Private },
	{ "browflex_get_no_of_beads(QString)", &slot_157, QMetaData::Private },
	{ "compute_pb_normals()", &slot_158, QMetaData::Private },
	{ "build_pb_structures(PDB_model*)", &slot_159, QMetaData::Private },
	{ "minus(point,point)", &slot_160, QMetaData::Private },
	{ "cross(point,point)", &slot_161, QMetaData::Private },
	{ "dot(point,point)", &slot_162, QMetaData::Private },
	{ "normal(point)", &slot_163, QMetaData::Private },
	{ "plane(PDB_atom*,PDB_atom*,PDB_atom*)", &slot_164, QMetaData::Private },
	{ "plane(point,point,point)", &slot_165, QMetaData::Private },
	{ "average(vector<point>*)", &slot_166, QMetaData::Private },
	{ "dist(point,point)", &slot_167, QMetaData::Private },
	{ "compute_bd_connections()", &slot_168, QMetaData::Private },
	{ "write_pdb(QString,vector<PDB_atom>*)", &slot_169, QMetaData::Private },
	{ "write_contact_plot(QString,PDB_model*,float)", &slot_170, QMetaData::Private },
	{ "write_contact_plot(QString,vector<PDB_atom>*,float)", &slot_171, QMetaData::Private },
	{ "create_anaflex_files()", &slot_172, QMetaData::Private },
	{ "create_anaflex_files(int)", &slot_173, QMetaData::Private },
	{ "create_anaflex_files(int,int)", &slot_174, QMetaData::Private },
	{ "run_anaflex()", &slot_175, QMetaData::Private },
	{ "run_anaflex(int)", &slot_176, QMetaData::Private },
	{ "run_anaflex(int,int)", &slot_177, QMetaData::Private },
	{ "run_us_config()", &slot_178, QMetaData::Private },
	{ "run_us_admin()", &slot_179, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_180, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn", parentObject,
	slot_tbl, 181,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: show_zeno_options(); break;
    case 1: display_default_differences(); break;
    case 2: clear_display(); break;
    case 3: reload_pdb(); break;
    case 4: static_QUType_int.set(_o,calc_somo()); break;
    case 5: static_QUType_int.set(_o,calc_grid_pdb()); break;
    case 6: static_QUType_int.set(_o,calc_grid()); break;
    case 7: static_QUType_int.set(_o,calc_hydro()); break;
    case 8: static_QUType_int.set(_o,calc_iqq((bool)static_QUType_bool.get(_o+1))); break;
    case 9: static_QUType_int.set(_o,calc_iqq((bool)static_QUType_bool.get(_o+1),(bool)static_QUType_bool.get(_o+2))); break;
    case 10: static_QUType_int.set(_o,calc_iqq((bool)static_QUType_bool.get(_o+1),(bool)static_QUType_bool.get(_o+2),(bool)static_QUType_bool.get(_o+3))); break;
    case 11: static_QUType_int.set(_o,calc_prr((bool)static_QUType_bool.get(_o+1))); break;
    case 12: static_QUType_int.set(_o,calc_prr((bool)static_QUType_bool.get(_o+1),(bool)static_QUType_bool.get(_o+2))); break;
    case 13: static_QUType_int.set(_o,calc_prr((bool)static_QUType_bool.get(_o+1),(bool)static_QUType_bool.get(_o+2),(bool)static_QUType_bool.get(_o+3))); break;
    case 14: select_save_params(); break;
    case 15: show_saxs_options(); break;
    case 16: show_bd_options(); break;
    case 17: show_anaflex_options(); break;
    case 18: show_dmd_options(); break;
    case 19: read_residue_file(); break;
    case 20: static_QUType_int.set(_o,read_config((const QString&)static_QUType_QString.get(_o+1))); break;
    case 21: static_QUType_int.set(_o,read_config((QFile&)*((QFile*)static_QUType_ptr.get(_o+1)))); break;
    case 22: show_misc(); break;
    case 23: bd_prepare(); break;
    case 24: bd_load(); break;
    case 25: bd_edit(); break;
    case 26: bd_run(); break;
    case 27: bd_load_results(); break;
    case 28: anaflex_prepare(); break;
    case 29: anaflex_load(); break;
    case 30: anaflex_edit(); break;
    case 31: anaflex_run(); break;
    case 32: anaflex_load_results(); break;
    case 33: stop_calc(); break;
    case 34: visualize(); break;
    case 35: visualize((bool)static_QUType_bool.get(_o+1)); break;
    case 36: visualize((bool)static_QUType_bool.get(_o+1),(QString)static_QUType_QString.get(_o+2)); break;
    case 37: visualize((bool)static_QUType_bool.get(_o+1),(QString)static_QUType_QString.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3)))); break;
    case 38: visualize((bool)static_QUType_bool.get(_o+1),(QString)static_QUType_QString.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))),(bool)static_QUType_bool.get(_o+4)); break;
    case 39: dmd_static_pairs(); break;
    case 40: pdb_saxs(); break;
    case 41: pdb_saxs((bool)static_QUType_bool.get(_o+1)); break;
    case 42: pdb_saxs((bool)static_QUType_bool.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 43: bead_saxs(); break;
    case 44: bead_saxs((bool)static_QUType_bool.get(_o+1)); break;
    case 45: bead_saxs((bool)static_QUType_bool.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 46: static_QUType_int.set(_o,pdb_hydrate_for_saxs()); break;
    case 47: static_QUType_int.set(_o,pdb_hydrate_for_saxs((bool)static_QUType_bool.get(_o+1))); break;
    case 48: make_test_set(); break;
    case 49: rescale_bead_model(); break;
    case 50: static_QUType_bool.set(_o,equi_grid_bead_model()); break;
    case 51: static_QUType_bool.set(_o,equi_grid_bead_model((double)static_QUType_double.get(_o+1))); break;
    case 52: browflex_readFromStdout(); break;
    case 53: browflex_readFromStderr(); break;
    case 54: browflex_launchFinished(); break;
    case 55: browflex_processExited(); break;
    case 56: anaflex_readFromStdout(); break;
    case 57: anaflex_readFromStderr(); break;
    case 58: anaflex_launchFinished(); break;
    case 59: anaflex_processExited(); break;
    case 60: load_pdb(); break;
    case 61: show_batch(); break;
    case 62: static_QUType_int.set(_o,read_pdb((const QString&)static_QUType_QString.get(_o+1))); break;
    case 63: static_QUType_int.set(_o,read_bead_model((QString)static_QUType_QString.get(_o+1),(bool&)static_QUType_bool.get(_o+2))); break;
    case 64: load_bead_model(); break;
    case 65: setupGUI(); break;
    case 66: select_residue_file(); break;
    case 67: clear_temp_chain((struct PDB_chain*)static_QUType_ptr.get(_o+1)); break;
    case 68: static_QUType_bool.set(_o,assign_atom((const QString&)static_QUType_QString.get(_o+1),(struct PDB_chain*)static_QUType_ptr.get(_o+2),(QString*)static_QUType_varptr.get(_o+3))); break;
    case 69: cancel(); break;
    case 70: help(); break;
    case 71: config(); break;
    case 72: edit_atom(); break;
    case 73: hybrid(); break;
    case 74: residue(); break;
    case 75: do_saxs(); break;
    case 76: select_model((int)static_QUType_int.get(_o+1)); break;
    case 77: static_QUType_int.set(_o,create_beads((QString*)static_QUType_varptr.get(_o+1))); break;
    case 78: static_QUType_int.set(_o,create_beads((QString*)static_QUType_varptr.get(_o+1),(bool)static_QUType_bool.get(_o+2))); break;
    case 79: get_atom_map((PDB_model*)static_QUType_ptr.get(_o+1)); break;
    case 80: static_QUType_int.set(_o,check_for_missing_atoms((QString*)static_QUType_varptr.get(_o+1),(PDB_model*)static_QUType_ptr.get(_o+2))); break;
    case 81: build_molecule_maps((PDB_model*)static_QUType_ptr.get(_o+1)); break;
    case 82: static_QUType_int.set(_o,overlap_check((bool)static_QUType_bool.get(_o+1),(bool)static_QUType_bool.get(_o+2),(bool)static_QUType_bool.get(_o+3),(double)static_QUType_double.get(_o+4))); break;
    case 83: static_QUType_int.set(_o,compute_asa()); break;
    case 84: static_QUType_int.set(_o,compute_asa((bool)static_QUType_bool.get(_o+1))); break;
    case 85: show_asa(); break;
    case 86: show_bd(); break;
    case 87: show_overlap(); break;
    case 88: show_grid_overlap(); break;
    case 89: show_bead_output(); break;
    case 90: show_hydro(); break;
    case 91: show_grid(); break;
    case 92: show_advanced_config(); break;
    case 93: view_pdb(); break;
    case 94: pdb_tool(); break;
    case 95: pdb_parsing(); break;
    case 96: pdb_visualization(); break;
    case 97: view_asa(); break;
    case 98: view_bead_model(); break;
    case 99: view_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 100: view_file((const QString&)static_QUType_QString.get(_o+1),(QString)static_QUType_QString.get(_o+2)); break;
    case 101: bead_check(); break;
    case 102: bead_check((bool)static_QUType_bool.get(_o+1)); break;
    case 103: bead_check((bool)static_QUType_bool.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 104: load_config(); break;
    case 105: write_config(); break;
    case 106: write_config((const QString&)static_QUType_QString.get(_o+1)); break;
    case 107: reset(); break;
    case 108: set_default(); break;
    case 109: update_bead_model_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 110: update_bead_model_prefix((const QString&)static_QUType_QString.get(_o+1)); break;
    case 111: radial_reduction(); break;
    case 112: radial_reduction((bool)static_QUType_bool.get(_o+1)); break;
    case 113: show_hydro_results(); break;
    case 114: open_hydro_results(); break;
    case 115: write_bead_asa((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2)); break;
    case 116: write_bead_tsv((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2)); break;
    case 117: write_bead_ebf((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2)); break;
    case 118: write_bead_spt((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2)); break;
    case 119: write_bead_spt((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2),(bool)static_QUType_bool.get(_o+3)); break;
    case 120: write_bead_spt((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2),(bool)static_QUType_bool.get(_o+3),(float)(*((float*)static_QUType_ptr.get(_o+4)))); break;
    case 121: write_bead_spt((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2),(bool)static_QUType_bool.get(_o+3),(float)(*((float*)static_QUType_ptr.get(_o+4))),(bool)static_QUType_bool.get(_o+5)); break;
    case 122: write_bead_model((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2)); break;
    case 123: write_bead_model((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2),(QString)static_QUType_QString.get(_o+3)); break;
    case 124: write_corr((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2)); break;
    case 125: static_QUType_bool.set(_o,read_corr((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2))); break;
    case 126: printError((const QString&)static_QUType_QString.get(_o+1)); break;
    case 127: closeAttnt((Q3Process*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2)); break;
    case 128: calc_vbar((struct PDB_model*)static_QUType_ptr.get(_o+1)); break;
    case 129: update_vbar(); break;
    case 130: append_options_log_somo(); break;
    case 131: append_options_log_atob(); break;
    case 132: list_model_vector((vector<PDB_model>*)static_QUType_ptr.get(_o+1)); break;
    case 133: static_QUType_QString.set(_o,default_differences_load_pdb()); break;
    case 134: static_QUType_QString.set(_o,default_differences_somo()); break;
    case 135: static_QUType_QString.set(_o,default_differences_grid()); break;
    case 136: static_QUType_QString.set(_o,default_differences_hydro()); break;
    case 137: static_QUType_QString.set(_o,default_differences_misc()); break;
    case 138: static_QUType_QString.set(_o,default_differences_saxs_options()); break;
    case 139: save(); break;
    case 140: print(); break;
    case 141: update_font(); break;
    case 142: set_calcAutoHydro(); break;
    case 143: set_setSuffix(); break;
    case 144: set_overwrite(); break;
    case 145: set_saveParams(); break;
    case 146: select_comparative(); break;
    case 147: best_analysis(); break;
    case 148: dmd_run(); break;
    case 149: static_QUType_bool.set(_o,bd_valid_browflex_main((QString)static_QUType_QString.get(_o+1))); break;
    case 150: bd_load_error((QString)static_QUType_QString.get(_o+1)); break;
    case 151: bd_edit_util((QString)static_QUType_QString.get(_o+1),(QString)static_QUType_QString.get(_o+2)); break;
    case 152: bd_load_results_after_anaflex(); break;
    case 153: static_QUType_bool.set(_o,anaflex_valid_anaflex_main((QString)static_QUType_QString.get(_o+1))); break;
    case 154: anaflex_load_error((QString)static_QUType_QString.get(_o+1)); break;
    case 155: static_QUType_int.set(_o,create_browflex_files()); break;
    case 156: static_QUType_int.set(_o,run_browflex()); break;
    case 157: static_QUType_int.set(_o,browflex_get_no_of_beads((QString)static_QUType_QString.get(_o+1))); break;
    case 158: static_QUType_int.set(_o,compute_pb_normals()); break;
    case 159: static_QUType_int.set(_o,build_pb_structures((PDB_model*)static_QUType_ptr.get(_o+1))); break;
    case 160: minus((point)(*((point*)static_QUType_ptr.get(_o+1))),(point)(*((point*)static_QUType_ptr.get(_o+2)))); break;
    case 161: cross((point)(*((point*)static_QUType_ptr.get(_o+1))),(point)(*((point*)static_QUType_ptr.get(_o+2)))); break;
    case 162: dot((point)(*((point*)static_QUType_ptr.get(_o+1))),(point)(*((point*)static_QUType_ptr.get(_o+2)))); break;
    case 163: normal((point)(*((point*)static_QUType_ptr.get(_o+1)))); break;
    case 164: plane((PDB_atom*)static_QUType_ptr.get(_o+1),(PDB_atom*)static_QUType_ptr.get(_o+2),(PDB_atom*)static_QUType_ptr.get(_o+3)); break;
    case 165: plane((point)(*((point*)static_QUType_ptr.get(_o+1))),(point)(*((point*)static_QUType_ptr.get(_o+2))),(point)(*((point*)static_QUType_ptr.get(_o+3)))); break;
    case 166: average((vector<point>*)static_QUType_ptr.get(_o+1)); break;
    case 167: dist((point)(*((point*)static_QUType_ptr.get(_o+1))),(point)(*((point*)static_QUType_ptr.get(_o+2)))); break;
    case 168: static_QUType_int.set(_o,compute_bd_connections()); break;
    case 169: static_QUType_int.set(_o,write_pdb((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2))); break;
    case 170: static_QUType_int.set(_o,write_contact_plot((QString)static_QUType_QString.get(_o+1),(PDB_model*)static_QUType_ptr.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))))); break;
    case 171: static_QUType_int.set(_o,write_contact_plot((QString)static_QUType_QString.get(_o+1),(vector<PDB_atom>*)static_QUType_ptr.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))))); break;
    case 172: static_QUType_int.set(_o,create_anaflex_files()); break;
    case 173: static_QUType_int.set(_o,create_anaflex_files((int)static_QUType_int.get(_o+1))); break;
    case 174: static_QUType_int.set(_o,create_anaflex_files((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2))); break;
    case 175: static_QUType_int.set(_o,run_anaflex()); break;
    case 176: static_QUType_int.set(_o,run_anaflex((int)static_QUType_int.get(_o+1))); break;
    case 177: static_QUType_int.set(_o,run_anaflex((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2))); break;
    case 178: run_us_config(); break;
    case 179: run_us_admin(); break;
    case 180: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
