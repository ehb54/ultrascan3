/****************************************************************************
** US_Hydrodyn_Pdb_Tool meta object code from reading C++ file 'us_hydrodyn_pdb_tool.h'
**
** Created: Tue Feb 4 09:09:19 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_pdb_tool.h"
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

const char *US_Hydrodyn_Pdb_Tool::className() const
{
    return "US_Hydrodyn_Pdb_Tool";
}

QMetaObject *US_Hydrodyn_Pdb_Tool::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Pdb_Tool( "US_Hydrodyn_Pdb_Tool", &US_Hydrodyn_Pdb_Tool::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Pdb_Tool::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Pdb_Tool", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Pdb_Tool::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Pdb_Tool", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Pdb_Tool::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"clear_display", 0, 0 };
    static const QUMethod slot_2 = {"update_font", 0, 0 };
    static const QUMethod slot_3 = {"save", 0, 0 };
    static const QUMethod slot_4 = {"split_pdb", 0, 0 };
    static const QUMethod slot_5 = {"join_pdbs", 0, 0 };
    static const QUMethod slot_6 = {"merge", 0, 0 };
    static const QUMethod slot_7 = {"renum_pdb", 0, 0 };
    static const QUMethod slot_8 = {"hybrid_split", 0, 0 };
    static const QUMethod slot_9 = {"h_to_chainX", 0, 0 };
    static const QUMethod slot_10 = {"hide_csv", 0, 0 };
    static const QUMethod slot_11 = {"csv_selection_changed", 0, 0 };
    static const QUMethod slot_12 = {"csv_load_1", 0, 0 };
    static const QUMethod slot_13 = {"csv_load", 0, 0 };
    static const QUMethod slot_14 = {"csv_save", 0, 0 };
    static const QUMethod slot_15 = {"csv_cut", 0, 0 };
    static const QUMethod slot_16 = {"csv_undo", 0, 0 };
    static const QUMethod slot_17 = {"csv_clear", 0, 0 };
    static const QUMethod slot_18 = {"csv_copy", 0, 0 };
    static const QUMethod slot_19 = {"csv_paste", 0, 0 };
    static const QUMethod slot_20 = {"csv_paste_new", 0, 0 };
    static const QUMethod slot_21 = {"csv_merge", 0, 0 };
    static const QUMethod slot_22 = {"csv_angle", 0, 0 };
    static const QUMethod slot_23 = {"csv_reseq", 0, 0 };
    static const QUMethod slot_24 = {"csv_check", 0, 0 };
    static const QUMethod slot_25 = {"csv_sort", 0, 0 };
    static const QUMethod slot_26 = {"csv_find_alt", 0, 0 };
    static const QUMethod slot_27 = {"csv_bm", 0, 0 };
    static const QUMethod slot_28 = {"csv_clash_report", 0, 0 };
    static const QUMethod slot_29 = {"csv_visualize", 0, 0 };
    static const QUMethod slot_30 = {"csv_sel", 0, 0 };
    static const QUMethod slot_31 = {"csv_sel_clear", 0, 0 };
    static const QUMethod slot_32 = {"csv_sel_clean", 0, 0 };
    static const QUMethod slot_33 = {"csv_sel_invert", 0, 0 };
    static const QUMethod slot_34 = {"csv_sel_chain", 0, 0 };
    static const QUMethod slot_35 = {"csv_sel_nearest_atoms", 0, 0 };
    static const QUMethod slot_36 = {"csv_sel_nearest_residues", 0, 0 };
    static const QUMethod slot_37 = {"hide_csv2", 0, 0 };
    static const QUMethod slot_38 = {"csv2_selection_changed", 0, 0 };
    static const QUMethod slot_39 = {"csv2_load_1", 0, 0 };
    static const QUMethod slot_40 = {"csv2_load", 0, 0 };
    static const QUMethod slot_41 = {"csv2_dup", 0, 0 };
    static const QUMethod slot_42 = {"csv2_save", 0, 0 };
    static const QUMethod slot_43 = {"csv2_undo", 0, 0 };
    static const QUMethod slot_44 = {"csv2_clear", 0, 0 };
    static const QUMethod slot_45 = {"csv2_cut", 0, 0 };
    static const QUMethod slot_46 = {"csv2_copy", 0, 0 };
    static const QUMethod slot_47 = {"csv2_paste", 0, 0 };
    static const QUMethod slot_48 = {"csv2_paste_new", 0, 0 };
    static const QUMethod slot_49 = {"csv2_merge", 0, 0 };
    static const QUMethod slot_50 = {"csv2_angle", 0, 0 };
    static const QUMethod slot_51 = {"csv2_reseq", 0, 0 };
    static const QUMethod slot_52 = {"csv2_check", 0, 0 };
    static const QUMethod slot_53 = {"csv2_sort", 0, 0 };
    static const QUMethod slot_54 = {"csv2_find_alt", 0, 0 };
    static const QUMethod slot_55 = {"csv2_bm", 0, 0 };
    static const QUMethod slot_56 = {"csv2_clash_report", 0, 0 };
    static const QUMethod slot_57 = {"csv2_visualize", 0, 0 };
    static const QUMethod slot_58 = {"csv2_sel", 0, 0 };
    static const QUMethod slot_59 = {"csv2_sel_clear", 0, 0 };
    static const QUMethod slot_60 = {"csv2_sel_clean", 0, 0 };
    static const QUMethod slot_61 = {"csv2_sel_invert", 0, 0 };
    static const QUMethod slot_62 = {"csv2_sel_chain", 0, 0 };
    static const QUMethod slot_63 = {"csv2_sel_nearest_atoms", 0, 0 };
    static const QUMethod slot_64 = {"csv2_sel_nearest_residues", 0, 0 };
    static const QUParameter param_slot_65[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_65 = {"adjust_wheel", 1, param_slot_65 };
    static const QUMethod slot_66 = {"naccess_readFromStdout", 0, 0 };
    static const QUMethod slot_67 = {"naccess_readFromStderr", 0, 0 };
    static const QUMethod slot_68 = {"naccess_launchFinished", 0, 0 };
    static const QUMethod slot_69 = {"naccess_processExited", 0, 0 };
    static const QUMethod slot_70 = {"cancel", 0, 0 };
    static const QUMethod slot_71 = {"help", 0, 0 };
    static const QUParameter param_slot_72[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_72 = {"closeEvent", 1, param_slot_72 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "clear_display()", &slot_1, QMetaData::Private },
	{ "update_font()", &slot_2, QMetaData::Private },
	{ "save()", &slot_3, QMetaData::Private },
	{ "split_pdb()", &slot_4, QMetaData::Private },
	{ "join_pdbs()", &slot_5, QMetaData::Private },
	{ "merge()", &slot_6, QMetaData::Private },
	{ "renum_pdb()", &slot_7, QMetaData::Private },
	{ "hybrid_split()", &slot_8, QMetaData::Private },
	{ "h_to_chainX()", &slot_9, QMetaData::Private },
	{ "hide_csv()", &slot_10, QMetaData::Private },
	{ "csv_selection_changed()", &slot_11, QMetaData::Private },
	{ "csv_load_1()", &slot_12, QMetaData::Private },
	{ "csv_load()", &slot_13, QMetaData::Private },
	{ "csv_save()", &slot_14, QMetaData::Private },
	{ "csv_cut()", &slot_15, QMetaData::Private },
	{ "csv_undo()", &slot_16, QMetaData::Private },
	{ "csv_clear()", &slot_17, QMetaData::Private },
	{ "csv_copy()", &slot_18, QMetaData::Private },
	{ "csv_paste()", &slot_19, QMetaData::Private },
	{ "csv_paste_new()", &slot_20, QMetaData::Private },
	{ "csv_merge()", &slot_21, QMetaData::Private },
	{ "csv_angle()", &slot_22, QMetaData::Private },
	{ "csv_reseq()", &slot_23, QMetaData::Private },
	{ "csv_check()", &slot_24, QMetaData::Private },
	{ "csv_sort()", &slot_25, QMetaData::Private },
	{ "csv_find_alt()", &slot_26, QMetaData::Private },
	{ "csv_bm()", &slot_27, QMetaData::Private },
	{ "csv_clash_report()", &slot_28, QMetaData::Private },
	{ "csv_visualize()", &slot_29, QMetaData::Private },
	{ "csv_sel()", &slot_30, QMetaData::Private },
	{ "csv_sel_clear()", &slot_31, QMetaData::Private },
	{ "csv_sel_clean()", &slot_32, QMetaData::Private },
	{ "csv_sel_invert()", &slot_33, QMetaData::Private },
	{ "csv_sel_chain()", &slot_34, QMetaData::Private },
	{ "csv_sel_nearest_atoms()", &slot_35, QMetaData::Private },
	{ "csv_sel_nearest_residues()", &slot_36, QMetaData::Private },
	{ "hide_csv2()", &slot_37, QMetaData::Private },
	{ "csv2_selection_changed()", &slot_38, QMetaData::Private },
	{ "csv2_load_1()", &slot_39, QMetaData::Private },
	{ "csv2_load()", &slot_40, QMetaData::Private },
	{ "csv2_dup()", &slot_41, QMetaData::Private },
	{ "csv2_save()", &slot_42, QMetaData::Private },
	{ "csv2_undo()", &slot_43, QMetaData::Private },
	{ "csv2_clear()", &slot_44, QMetaData::Private },
	{ "csv2_cut()", &slot_45, QMetaData::Private },
	{ "csv2_copy()", &slot_46, QMetaData::Private },
	{ "csv2_paste()", &slot_47, QMetaData::Private },
	{ "csv2_paste_new()", &slot_48, QMetaData::Private },
	{ "csv2_merge()", &slot_49, QMetaData::Private },
	{ "csv2_angle()", &slot_50, QMetaData::Private },
	{ "csv2_reseq()", &slot_51, QMetaData::Private },
	{ "csv2_check()", &slot_52, QMetaData::Private },
	{ "csv2_sort()", &slot_53, QMetaData::Private },
	{ "csv2_find_alt()", &slot_54, QMetaData::Private },
	{ "csv2_bm()", &slot_55, QMetaData::Private },
	{ "csv2_clash_report()", &slot_56, QMetaData::Private },
	{ "csv2_visualize()", &slot_57, QMetaData::Private },
	{ "csv2_sel()", &slot_58, QMetaData::Private },
	{ "csv2_sel_clear()", &slot_59, QMetaData::Private },
	{ "csv2_sel_clean()", &slot_60, QMetaData::Private },
	{ "csv2_sel_invert()", &slot_61, QMetaData::Private },
	{ "csv2_sel_chain()", &slot_62, QMetaData::Private },
	{ "csv2_sel_nearest_atoms()", &slot_63, QMetaData::Private },
	{ "csv2_sel_nearest_residues()", &slot_64, QMetaData::Private },
	{ "adjust_wheel(double)", &slot_65, QMetaData::Private },
	{ "naccess_readFromStdout()", &slot_66, QMetaData::Private },
	{ "naccess_readFromStderr()", &slot_67, QMetaData::Private },
	{ "naccess_launchFinished()", &slot_68, QMetaData::Private },
	{ "naccess_processExited()", &slot_69, QMetaData::Private },
	{ "cancel()", &slot_70, QMetaData::Private },
	{ "help()", &slot_71, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_72, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Pdb_Tool", parentObject,
	slot_tbl, 73,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Pdb_Tool.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Pdb_Tool::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Pdb_Tool" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Pdb_Tool::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: clear_display(); break;
    case 2: update_font(); break;
    case 3: save(); break;
    case 4: split_pdb(); break;
    case 5: join_pdbs(); break;
    case 6: merge(); break;
    case 7: renum_pdb(); break;
    case 8: hybrid_split(); break;
    case 9: h_to_chainX(); break;
    case 10: hide_csv(); break;
    case 11: csv_selection_changed(); break;
    case 12: csv_load_1(); break;
    case 13: csv_load(); break;
    case 14: csv_save(); break;
    case 15: csv_cut(); break;
    case 16: csv_undo(); break;
    case 17: csv_clear(); break;
    case 18: csv_copy(); break;
    case 19: csv_paste(); break;
    case 20: csv_paste_new(); break;
    case 21: csv_merge(); break;
    case 22: csv_angle(); break;
    case 23: csv_reseq(); break;
    case 24: csv_check(); break;
    case 25: csv_sort(); break;
    case 26: csv_find_alt(); break;
    case 27: csv_bm(); break;
    case 28: csv_clash_report(); break;
    case 29: csv_visualize(); break;
    case 30: csv_sel(); break;
    case 31: csv_sel_clear(); break;
    case 32: csv_sel_clean(); break;
    case 33: csv_sel_invert(); break;
    case 34: csv_sel_chain(); break;
    case 35: csv_sel_nearest_atoms(); break;
    case 36: csv_sel_nearest_residues(); break;
    case 37: hide_csv2(); break;
    case 38: csv2_selection_changed(); break;
    case 39: csv2_load_1(); break;
    case 40: csv2_load(); break;
    case 41: csv2_dup(); break;
    case 42: csv2_save(); break;
    case 43: csv2_undo(); break;
    case 44: csv2_clear(); break;
    case 45: csv2_cut(); break;
    case 46: csv2_copy(); break;
    case 47: csv2_paste(); break;
    case 48: csv2_paste_new(); break;
    case 49: csv2_merge(); break;
    case 50: csv2_angle(); break;
    case 51: csv2_reseq(); break;
    case 52: csv2_check(); break;
    case 53: csv2_sort(); break;
    case 54: csv2_find_alt(); break;
    case 55: csv2_bm(); break;
    case 56: csv2_clash_report(); break;
    case 57: csv2_visualize(); break;
    case 58: csv2_sel(); break;
    case 59: csv2_sel_clear(); break;
    case 60: csv2_sel_clean(); break;
    case 61: csv2_sel_invert(); break;
    case 62: csv2_sel_chain(); break;
    case 63: csv2_sel_nearest_atoms(); break;
    case 64: csv2_sel_nearest_residues(); break;
    case 65: adjust_wheel((double)static_QUType_double.get(_o+1)); break;
    case 66: naccess_readFromStdout(); break;
    case 67: naccess_readFromStderr(); break;
    case 68: naccess_launchFinished(); break;
    case 69: naccess_processExited(); break;
    case 70: cancel(); break;
    case 71: help(); break;
    case 72: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Pdb_Tool::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Pdb_Tool::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Pdb_Tool::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
