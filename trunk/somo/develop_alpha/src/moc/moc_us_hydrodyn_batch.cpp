/****************************************************************************
** US_Hydrodyn_Batch meta object code from reading C++ file 'us_hydrodyn_batch.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_batch.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <QResizeEvent>
#include <Q3Frame>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Batch::className() const
{
    return "US_Hydrodyn_Batch";
}

QMetaObject *US_Hydrodyn_Batch::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Batch( "US_Hydrodyn_Batch", &US_Hydrodyn_Batch::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Batch::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Batch", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Batch::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Batch", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Batch::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"add_files", 0, 0 };
    static const QUMethod slot_2 = {"select_all", 0, 0 };
    static const QUMethod slot_3 = {"remove_files", 0, 0 };
    static const QUMethod slot_4 = {"load_somo", 0, 0 };
    static const QUMethod slot_5 = {"load_saxs", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"residue", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"atom", 1, param_slot_7 };
    static const QUMethod slot_8 = {"screen", 0, 0 };
    static const QUMethod slot_9 = {"start", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ "quiet", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"start", 1, param_slot_10 };
    static const QUMethod slot_11 = {"set_mm_first", 0, 0 };
    static const QUMethod slot_12 = {"set_mm_all", 0, 0 };
    static const QUMethod slot_13 = {"set_dmd", 0, 0 };
    static const QUMethod slot_14 = {"set_somo", 0, 0 };
    static const QUMethod slot_15 = {"set_grid", 0, 0 };
    static const QUMethod slot_16 = {"set_equi_grid", 0, 0 };
    static const QUMethod slot_17 = {"set_hydro", 0, 0 };
    static const QUMethod slot_18 = {"set_zeno", 0, 0 };
    static const QUMethod slot_19 = {"set_iqq", 0, 0 };
    static const QUMethod slot_20 = {"set_saxs_search", 0, 0 };
    static const QUMethod slot_21 = {"set_prr", 0, 0 };
    static const QUMethod slot_22 = {"set_avg_hydro", 0, 0 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_avg_hydro_name", 1, param_slot_23 };
    static const QUMethod slot_24 = {"set_csv_saxs", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_csv_saxs_name", 1, param_slot_25 };
    static const QUMethod slot_26 = {"set_create_native_saxs", 0, 0 };
    static const QUMethod slot_27 = {"set_hydrate", 0, 0 };
    static const QUMethod slot_28 = {"set_compute_iq_avg", 0, 0 };
    static const QUMethod slot_29 = {"set_compute_iq_only_avg", 0, 0 };
    static const QUMethod slot_30 = {"set_compute_iq_std_dev", 0, 0 };
    static const QUMethod slot_31 = {"set_compute_prr_avg", 0, 0 };
    static const QUMethod slot_32 = {"set_compute_prr_std_dev", 0, 0 };
    static const QUMethod slot_33 = {"select_save_params", 0, 0 };
    static const QUMethod slot_34 = {"set_saveParams", 0, 0 };
    static const QUMethod slot_35 = {"stop", 0, 0 };
    static const QUMethod slot_36 = {"make_movie", 0, 0 };
    static const QUMethod slot_37 = {"cancel", 0, 0 };
    static const QUMethod slot_38 = {"cluster", 0, 0 };
    static const QUMethod slot_39 = {"open_saxs_options", 0, 0 };
    static const QUMethod slot_40 = {"help", 0, 0 };
    static const QUMethod slot_41 = {"save", 0, 0 };
    static const QUMethod slot_42 = {"print", 0, 0 };
    static const QUMethod slot_43 = {"update_font", 0, 0 };
    static const QUMethod slot_44 = {"clear_display", 0, 0 };
    static const QUParameter param_slot_45[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ "file", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_45 = {"screen_pdb", 2, param_slot_45 };
    static const QUParameter param_slot_46[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ "file", &static_QUType_QString, 0, QUParameter::In },
	{ "display_pdb", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_46 = {"screen_pdb", 3, param_slot_46 };
    static const QUParameter param_slot_47[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ "file", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_47 = {"screen_bead_model", 2, param_slot_47 };
    static const QUMethod slot_48 = {"save_us_hydrodyn_settings", 0, 0 };
    static const QUMethod slot_49 = {"restore_us_hydrodyn_settings", 0, 0 };
    static const QUMethod slot_50 = {"update_enables", 0, 0 };
    static const QUMethod slot_51 = {"disable_after_start", 0, 0 };
    static const QUMethod slot_52 = {"enable_after_stop", 0, 0 };
    static const QUMethod slot_53 = {"set_counts", 0, 0 };
    static const QUParameter param_slot_54[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out },
	{ "i", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_54 = {"get_file_name", 2, param_slot_54 };
    static const QUParameter param_slot_55[] = {
	{ "display_messages", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_55 = {"check_for_missing_files", 1, param_slot_55 };
    static const QUParameter param_slot_56[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_56 = {"closeEvent", 1, param_slot_56 };
    static const QUParameter param_slot_57[] = {
	{ 0, &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_57 = {"resizeEvent", 1, param_slot_57 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "add_files()", &slot_1, QMetaData::Private },
	{ "select_all()", &slot_2, QMetaData::Private },
	{ "remove_files()", &slot_3, QMetaData::Private },
	{ "load_somo()", &slot_4, QMetaData::Private },
	{ "load_saxs()", &slot_5, QMetaData::Private },
	{ "residue(int)", &slot_6, QMetaData::Private },
	{ "atom(int)", &slot_7, QMetaData::Private },
	{ "screen()", &slot_8, QMetaData::Private },
	{ "start()", &slot_9, QMetaData::Private },
	{ "start(bool)", &slot_10, QMetaData::Private },
	{ "set_mm_first()", &slot_11, QMetaData::Private },
	{ "set_mm_all()", &slot_12, QMetaData::Private },
	{ "set_dmd()", &slot_13, QMetaData::Private },
	{ "set_somo()", &slot_14, QMetaData::Private },
	{ "set_grid()", &slot_15, QMetaData::Private },
	{ "set_equi_grid()", &slot_16, QMetaData::Private },
	{ "set_hydro()", &slot_17, QMetaData::Private },
	{ "set_zeno()", &slot_18, QMetaData::Private },
	{ "set_iqq()", &slot_19, QMetaData::Private },
	{ "set_saxs_search()", &slot_20, QMetaData::Private },
	{ "set_prr()", &slot_21, QMetaData::Private },
	{ "set_avg_hydro()", &slot_22, QMetaData::Private },
	{ "update_avg_hydro_name(const QString&)", &slot_23, QMetaData::Private },
	{ "set_csv_saxs()", &slot_24, QMetaData::Private },
	{ "update_csv_saxs_name(const QString&)", &slot_25, QMetaData::Private },
	{ "set_create_native_saxs()", &slot_26, QMetaData::Private },
	{ "set_hydrate()", &slot_27, QMetaData::Private },
	{ "set_compute_iq_avg()", &slot_28, QMetaData::Private },
	{ "set_compute_iq_only_avg()", &slot_29, QMetaData::Private },
	{ "set_compute_iq_std_dev()", &slot_30, QMetaData::Private },
	{ "set_compute_prr_avg()", &slot_31, QMetaData::Private },
	{ "set_compute_prr_std_dev()", &slot_32, QMetaData::Private },
	{ "select_save_params()", &slot_33, QMetaData::Private },
	{ "set_saveParams()", &slot_34, QMetaData::Private },
	{ "stop()", &slot_35, QMetaData::Private },
	{ "make_movie()", &slot_36, QMetaData::Private },
	{ "cancel()", &slot_37, QMetaData::Private },
	{ "cluster()", &slot_38, QMetaData::Private },
	{ "open_saxs_options()", &slot_39, QMetaData::Private },
	{ "help()", &slot_40, QMetaData::Private },
	{ "save()", &slot_41, QMetaData::Private },
	{ "print()", &slot_42, QMetaData::Private },
	{ "update_font()", &slot_43, QMetaData::Private },
	{ "clear_display()", &slot_44, QMetaData::Private },
	{ "screen_pdb(QString)", &slot_45, QMetaData::Private },
	{ "screen_pdb(QString,bool)", &slot_46, QMetaData::Private },
	{ "screen_bead_model(QString)", &slot_47, QMetaData::Private },
	{ "save_us_hydrodyn_settings()", &slot_48, QMetaData::Private },
	{ "restore_us_hydrodyn_settings()", &slot_49, QMetaData::Private },
	{ "update_enables()", &slot_50, QMetaData::Private },
	{ "disable_after_start()", &slot_51, QMetaData::Private },
	{ "enable_after_stop()", &slot_52, QMetaData::Private },
	{ "set_counts()", &slot_53, QMetaData::Private },
	{ "get_file_name(int)", &slot_54, QMetaData::Private },
	{ "check_for_missing_files(bool)", &slot_55, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_56, QMetaData::Protected },
	{ "resizeEvent(QResizeEvent*)", &slot_57, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Batch", parentObject,
	slot_tbl, 58,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Batch.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Batch::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Batch" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Batch::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: add_files(); break;
    case 2: select_all(); break;
    case 3: remove_files(); break;
    case 4: load_somo(); break;
    case 5: load_saxs(); break;
    case 6: residue((int)static_QUType_int.get(_o+1)); break;
    case 7: atom((int)static_QUType_int.get(_o+1)); break;
    case 8: screen(); break;
    case 9: start(); break;
    case 10: start((bool)static_QUType_bool.get(_o+1)); break;
    case 11: set_mm_first(); break;
    case 12: set_mm_all(); break;
    case 13: set_dmd(); break;
    case 14: set_somo(); break;
    case 15: set_grid(); break;
    case 16: set_equi_grid(); break;
    case 17: set_hydro(); break;
    case 18: set_zeno(); break;
    case 19: set_iqq(); break;
    case 20: set_saxs_search(); break;
    case 21: set_prr(); break;
    case 22: set_avg_hydro(); break;
    case 23: update_avg_hydro_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: set_csv_saxs(); break;
    case 25: update_csv_saxs_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: set_create_native_saxs(); break;
    case 27: set_hydrate(); break;
    case 28: set_compute_iq_avg(); break;
    case 29: set_compute_iq_only_avg(); break;
    case 30: set_compute_iq_std_dev(); break;
    case 31: set_compute_prr_avg(); break;
    case 32: set_compute_prr_std_dev(); break;
    case 33: select_save_params(); break;
    case 34: set_saveParams(); break;
    case 35: stop(); break;
    case 36: make_movie(); break;
    case 37: cancel(); break;
    case 38: cluster(); break;
    case 39: open_saxs_options(); break;
    case 40: help(); break;
    case 41: save(); break;
    case 42: print(); break;
    case 43: update_font(); break;
    case 44: clear_display(); break;
    case 45: static_QUType_bool.set(_o,screen_pdb((QString)static_QUType_QString.get(_o+1))); break;
    case 46: static_QUType_bool.set(_o,screen_pdb((QString)static_QUType_QString.get(_o+1),(bool)static_QUType_bool.get(_o+2))); break;
    case 47: static_QUType_bool.set(_o,screen_bead_model((QString)static_QUType_QString.get(_o+1))); break;
    case 48: save_us_hydrodyn_settings(); break;
    case 49: restore_us_hydrodyn_settings(); break;
    case 50: update_enables(); break;
    case 51: disable_after_start(); break;
    case 52: enable_after_stop(); break;
    case 53: set_counts(); break;
    case 54: static_QUType_QString.set(_o,get_file_name((int)static_QUType_int.get(_o+1))); break;
    case 55: check_for_missing_files((bool)static_QUType_bool.get(_o+1)); break;
    case 56: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 57: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Batch::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Batch::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Batch::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
