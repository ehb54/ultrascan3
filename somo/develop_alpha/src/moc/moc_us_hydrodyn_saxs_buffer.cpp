/****************************************************************************
** US_Hydrodyn_Saxs_Buffer meta object code from reading C++ file 'us_hydrodyn_saxs_buffer.h'
**
** Created: Tue Feb 25 10:28:33 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_buffer.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3Frame>
#include <QMouseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Saxs_Buffer::className() const
{
    return "US_Hydrodyn_Saxs_Buffer";
}

QMetaObject *US_Hydrodyn_Saxs_Buffer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Buffer( "US_Hydrodyn_Saxs_Buffer", &US_Hydrodyn_Saxs_Buffer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Buffer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Buffer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Buffer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Buffer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Buffer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"color_rotate", 0, 0 };
    static const QUMethod slot_2 = {"dir_pressed", 0, 0 };
    static const QUMethod slot_3 = {"created_dir_pressed", 0, 0 };
    static const QUMethod slot_4 = {"update_enables", 0, 0 };
    static const QUMethod slot_5 = {"update_files", 0, 0 };
    static const QUMethod slot_6 = {"update_created_files", 0, 0 };
    static const QUMethod slot_7 = {"add_files", 0, 0 };
    static const QUMethod slot_8 = {"similar_files", 0, 0 };
    static const QUMethod slot_9 = {"conc", 0, 0 };
    static const QUMethod slot_10 = {"clear_files", 0, 0 };
    static const QUMethod slot_11 = {"regex_load", 0, 0 };
    static const QUMethod slot_12 = {"select_all", 0, 0 };
    static const QUMethod slot_13 = {"invert", 0, 0 };
    static const QUMethod slot_14 = {"join", 0, 0 };
    static const QUMethod slot_15 = {"adjacent", 0, 0 };
    static const QUMethod slot_16 = {"to_saxs", 0, 0 };
    static const QUMethod slot_17 = {"view", 0, 0 };
    static const QUMethod slot_18 = {"rescale", 0, 0 };
    static const QUMethod slot_19 = {"avg", 0, 0 };
    static const QUMethod slot_20 = {"asum", 0, 0 };
    static const QUMethod slot_21 = {"normalize", 0, 0 };
    static const QUMethod slot_22 = {"conc_avg", 0, 0 };
    static const QUMethod slot_23 = {"set_buffer", 0, 0 };
    static const QUMethod slot_24 = {"set_empty", 0, 0 };
    static const QUMethod slot_25 = {"set_signal", 0, 0 };
    static const QUMethod slot_26 = {"select_all_created", 0, 0 };
    static const QUMethod slot_27 = {"adjacent_created", 0, 0 };
    static const QUMethod slot_28 = {"save_created_csv", 0, 0 };
    static const QUMethod slot_29 = {"save_created", 0, 0 };
    static const QUMethod slot_30 = {"show_created", 0, 0 };
    static const QUMethod slot_31 = {"show_only_created", 0, 0 };
    static const QUParameter param_slot_32[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"table_value", 2, param_slot_32 };
    static const QUMethod slot_33 = {"start", 0, 0 };
    static const QUMethod slot_34 = {"run_current", 0, 0 };
    static const QUMethod slot_35 = {"run_divide", 0, 0 };
    static const QUMethod slot_36 = {"run_best", 0, 0 };
    static const QUMethod slot_37 = {"stop", 0, 0 };
    static const QUMethod slot_38 = {"clear_display", 0, 0 };
    static const QUMethod slot_39 = {"update_font", 0, 0 };
    static const QUMethod slot_40 = {"save", 0, 0 };
    static const QUMethod slot_41 = {"cancel", 0, 0 };
    static const QUMethod slot_42 = {"help", 0, 0 };
    static const QUParameter param_slot_43[] = {
	{ "rect", &static_QUType_ptr, "QwtDoubleRect", QUParameter::In }
    };
    static const QUMethod slot_43 = {"plot_zoomed", 1, param_slot_43 };
    static const QUParameter param_slot_44[] = {
	{ "me", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_44 = {"plot_mouse", 1, param_slot_44 };
    static const QUParameter param_slot_45[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_45 = {"adjust_wheel", 1, param_slot_45 };
    static const QUMethod slot_46 = {"wheel_cancel", 0, 0 };
    static const QUMethod slot_47 = {"wheel_save", 0, 0 };
    static const QUMethod slot_48 = {"join_start", 0, 0 };
    static const QUMethod slot_49 = {"join_swap", 0, 0 };
    static const QUMethod slot_50 = {"join_fit_scaling", 0, 0 };
    static const QUMethod slot_51 = {"join_fit_linear", 0, 0 };
    static const QUParameter param_slot_52[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_52 = {"join_offset_text", 1, param_slot_52 };
    static const QUParameter param_slot_53[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_53 = {"join_mult_text", 1, param_slot_53 };
    static const QUParameter param_slot_54[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_54 = {"join_start_text", 1, param_slot_54 };
    static const QUParameter param_slot_55[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_55 = {"join_point_text", 1, param_slot_55 };
    static const QUParameter param_slot_56[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_56 = {"join_end_text", 1, param_slot_56 };
    static const QUParameter param_slot_57[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_57 = {"join_offset_focus", 1, param_slot_57 };
    static const QUParameter param_slot_58[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_58 = {"join_mult_focus", 1, param_slot_58 };
    static const QUParameter param_slot_59[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_59 = {"join_start_focus", 1, param_slot_59 };
    static const QUParameter param_slot_60[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_60 = {"join_point_focus", 1, param_slot_60 };
    static const QUParameter param_slot_61[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_61 = {"join_end_focus", 1, param_slot_61 };
    static const QUMethod slot_62 = {"select_vis", 0, 0 };
    static const QUMethod slot_63 = {"remove_vis", 0, 0 };
    static const QUMethod slot_64 = {"crop_left", 0, 0 };
    static const QUMethod slot_65 = {"crop_common", 0, 0 };
    static const QUMethod slot_66 = {"crop_vis", 0, 0 };
    static const QUMethod slot_67 = {"crop_zero", 0, 0 };
    static const QUMethod slot_68 = {"crop_undo", 0, 0 };
    static const QUMethod slot_69 = {"crop_right", 0, 0 };
    static const QUMethod slot_70 = {"legend", 0, 0 };
    static const QUMethod slot_71 = {"axis_x", 0, 0 };
    static const QUMethod slot_72 = {"axis_y", 0, 0 };
    static const QUMethod slot_73 = {"legend_set", 0, 0 };
    static const QUMethod slot_74 = {"guinier", 0, 0 };
    static const QUParameter param_slot_75[] = {
	{ 0, &static_QUType_ptr, "QListBoxItem", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_75 = {"rename_created", 2, param_slot_75 };
    static const QUParameter param_slot_76[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_76 = {"closeEvent", 1, param_slot_76 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "color_rotate()", &slot_1, QMetaData::Private },
	{ "dir_pressed()", &slot_2, QMetaData::Private },
	{ "created_dir_pressed()", &slot_3, QMetaData::Private },
	{ "update_enables()", &slot_4, QMetaData::Private },
	{ "update_files()", &slot_5, QMetaData::Private },
	{ "update_created_files()", &slot_6, QMetaData::Private },
	{ "add_files()", &slot_7, QMetaData::Private },
	{ "similar_files()", &slot_8, QMetaData::Private },
	{ "conc()", &slot_9, QMetaData::Private },
	{ "clear_files()", &slot_10, QMetaData::Private },
	{ "regex_load()", &slot_11, QMetaData::Private },
	{ "select_all()", &slot_12, QMetaData::Private },
	{ "invert()", &slot_13, QMetaData::Private },
	{ "join()", &slot_14, QMetaData::Private },
	{ "adjacent()", &slot_15, QMetaData::Private },
	{ "to_saxs()", &slot_16, QMetaData::Private },
	{ "view()", &slot_17, QMetaData::Private },
	{ "rescale()", &slot_18, QMetaData::Private },
	{ "avg()", &slot_19, QMetaData::Private },
	{ "asum()", &slot_20, QMetaData::Private },
	{ "normalize()", &slot_21, QMetaData::Private },
	{ "conc_avg()", &slot_22, QMetaData::Private },
	{ "set_buffer()", &slot_23, QMetaData::Private },
	{ "set_empty()", &slot_24, QMetaData::Private },
	{ "set_signal()", &slot_25, QMetaData::Private },
	{ "select_all_created()", &slot_26, QMetaData::Private },
	{ "adjacent_created()", &slot_27, QMetaData::Private },
	{ "save_created_csv()", &slot_28, QMetaData::Private },
	{ "save_created()", &slot_29, QMetaData::Private },
	{ "show_created()", &slot_30, QMetaData::Private },
	{ "show_only_created()", &slot_31, QMetaData::Private },
	{ "table_value(int,int)", &slot_32, QMetaData::Private },
	{ "start()", &slot_33, QMetaData::Private },
	{ "run_current()", &slot_34, QMetaData::Private },
	{ "run_divide()", &slot_35, QMetaData::Private },
	{ "run_best()", &slot_36, QMetaData::Private },
	{ "stop()", &slot_37, QMetaData::Private },
	{ "clear_display()", &slot_38, QMetaData::Private },
	{ "update_font()", &slot_39, QMetaData::Private },
	{ "save()", &slot_40, QMetaData::Private },
	{ "cancel()", &slot_41, QMetaData::Private },
	{ "help()", &slot_42, QMetaData::Private },
	{ "plot_zoomed(const QwtDoubleRect&)", &slot_43, QMetaData::Private },
	{ "plot_mouse(const QMouseEvent&)", &slot_44, QMetaData::Private },
	{ "adjust_wheel(double)", &slot_45, QMetaData::Private },
	{ "wheel_cancel()", &slot_46, QMetaData::Private },
	{ "wheel_save()", &slot_47, QMetaData::Private },
	{ "join_start()", &slot_48, QMetaData::Private },
	{ "join_swap()", &slot_49, QMetaData::Private },
	{ "join_fit_scaling()", &slot_50, QMetaData::Private },
	{ "join_fit_linear()", &slot_51, QMetaData::Private },
	{ "join_offset_text(const QString&)", &slot_52, QMetaData::Private },
	{ "join_mult_text(const QString&)", &slot_53, QMetaData::Private },
	{ "join_start_text(const QString&)", &slot_54, QMetaData::Private },
	{ "join_point_text(const QString&)", &slot_55, QMetaData::Private },
	{ "join_end_text(const QString&)", &slot_56, QMetaData::Private },
	{ "join_offset_focus(bool)", &slot_57, QMetaData::Private },
	{ "join_mult_focus(bool)", &slot_58, QMetaData::Private },
	{ "join_start_focus(bool)", &slot_59, QMetaData::Private },
	{ "join_point_focus(bool)", &slot_60, QMetaData::Private },
	{ "join_end_focus(bool)", &slot_61, QMetaData::Private },
	{ "select_vis()", &slot_62, QMetaData::Private },
	{ "remove_vis()", &slot_63, QMetaData::Private },
	{ "crop_left()", &slot_64, QMetaData::Private },
	{ "crop_common()", &slot_65, QMetaData::Private },
	{ "crop_vis()", &slot_66, QMetaData::Private },
	{ "crop_zero()", &slot_67, QMetaData::Private },
	{ "crop_undo()", &slot_68, QMetaData::Private },
	{ "crop_right()", &slot_69, QMetaData::Private },
	{ "legend()", &slot_70, QMetaData::Private },
	{ "axis_x()", &slot_71, QMetaData::Private },
	{ "axis_y()", &slot_72, QMetaData::Private },
	{ "legend_set()", &slot_73, QMetaData::Private },
	{ "guinier()", &slot_74, QMetaData::Private },
	{ "rename_created(QListBoxItem*,const QPoint&)", &slot_75, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_76, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Buffer", parentObject,
	slot_tbl, 77,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Buffer.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Buffer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Buffer" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Buffer::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: color_rotate(); break;
    case 2: dir_pressed(); break;
    case 3: created_dir_pressed(); break;
    case 4: update_enables(); break;
    case 5: update_files(); break;
    case 6: update_created_files(); break;
    case 7: add_files(); break;
    case 8: similar_files(); break;
    case 9: conc(); break;
    case 10: clear_files(); break;
    case 11: regex_load(); break;
    case 12: select_all(); break;
    case 13: invert(); break;
    case 14: join(); break;
    case 15: adjacent(); break;
    case 16: to_saxs(); break;
    case 17: view(); break;
    case 18: rescale(); break;
    case 19: avg(); break;
    case 20: asum(); break;
    case 21: normalize(); break;
    case 22: conc_avg(); break;
    case 23: set_buffer(); break;
    case 24: set_empty(); break;
    case 25: set_signal(); break;
    case 26: select_all_created(); break;
    case 27: adjacent_created(); break;
    case 28: save_created_csv(); break;
    case 29: save_created(); break;
    case 30: show_created(); break;
    case 31: show_only_created(); break;
    case 32: table_value((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 33: start(); break;
    case 34: run_current(); break;
    case 35: run_divide(); break;
    case 36: run_best(); break;
    case 37: stop(); break;
    case 38: clear_display(); break;
    case 39: update_font(); break;
    case 40: save(); break;
    case 41: cancel(); break;
    case 42: help(); break;
    case 43: plot_zoomed((const QwtDoubleRect&)*((const QwtDoubleRect*)static_QUType_ptr.get(_o+1))); break;
    case 44: plot_mouse((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 45: adjust_wheel((double)static_QUType_double.get(_o+1)); break;
    case 46: wheel_cancel(); break;
    case 47: wheel_save(); break;
    case 48: join_start(); break;
    case 49: join_swap(); break;
    case 50: join_fit_scaling(); break;
    case 51: join_fit_linear(); break;
    case 52: join_offset_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 53: join_mult_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 54: join_start_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 55: join_point_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 56: join_end_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 57: join_offset_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 58: join_mult_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 59: join_start_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 60: join_point_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 61: join_end_focus((bool)static_QUType_bool.get(_o+1)); break;
    case 62: select_vis(); break;
    case 63: remove_vis(); break;
    case 64: crop_left(); break;
    case 65: crop_common(); break;
    case 66: crop_vis(); break;
    case 67: crop_zero(); break;
    case 68: crop_undo(); break;
    case 69: crop_right(); break;
    case 70: legend(); break;
    case 71: axis_x(); break;
    case 72: axis_y(); break;
    case 73: legend_set(); break;
    case 74: guinier(); break;
    case 75: rename_created((Q3ListBoxItem*)static_QUType_ptr.get(_o+1),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+2))); break;
    case 76: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Buffer::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Buffer::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Buffer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
