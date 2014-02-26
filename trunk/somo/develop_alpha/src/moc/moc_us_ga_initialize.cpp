/****************************************************************************
** US_GA_Initialize meta object code from reading C++ file 'us_ga_initialize.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_ga_initialize.h"
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

const char *US_GA_Initialize::className() const
{
    return "US_GA_Initialize";
}

QMetaObject *US_GA_Initialize::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_GA_Initialize( "US_GA_Initialize", &US_GA_Initialize::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_GA_Initialize::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GA_Initialize", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_GA_Initialize::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GA_Initialize", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_GA_Initialize::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct MonteCarloStats", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"calc_stats", 5, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct MonteCarloStats", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"calc_stats", 4, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_k_range", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_s_range", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_resolution", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_initial_solutes", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_ff0_max", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_ff0_min", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_plot_smax", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_plot_smin", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_plot_fmax", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_plot_fmin", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_x_resolution", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_y_resolution", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_x_pixel", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_y_pixel", 1, param_slot_15 };
    static const QUMethod slot_16 = {"plot_1dim", 0, 0 };
    static const QUMethod slot_17 = {"plot_2dim", 0, 0 };
    static const QUMethod slot_18 = {"plot_3dim", 0, 0 };
    static const QUMethod slot_19 = {"select_3dim", 0, 0 };
    static const QUMethod slot_20 = {"select_1dim", 0, 0 };
    static const QUMethod slot_21 = {"select_2dim", 0, 0 };
    static const QUMethod slot_22 = {"select_autolimit", 0, 0 };
    static const QUMethod slot_23 = {"load_distro", 0, 0 };
    static const QUMethod slot_24 = {"make_pngs", 0, 0 };
    static const QUMethod slot_25 = {"load_color", 0, 0 };
    static const QUMethod slot_26 = {"calc_distro", 0, 0 };
    static const QUMethod slot_27 = {"reset_peaks_replot", 0, 0 };
    static const QUMethod slot_28 = {"reset_peaks", 0, 0 };
    static const QUMethod slot_29 = {"assign_peaks", 0, 0 };
    static const QUMethod slot_30 = {"help", 0, 0 };
    static const QUMethod slot_31 = {"save", 0, 0 };
    static const QUMethod slot_32 = {"print", 0, 0 };
    static const QUMethod slot_33 = {"draw", 0, 0 };
    static const QUMethod slot_34 = {"plot_buckets", 0, 0 };
    static const QUMethod slot_35 = {"shrink", 0, 0 };
    static const QUParameter param_slot_36[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_36 = {"edit_solute", 1, param_slot_36 };
    static const QUParameter param_slot_37[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_37 = {"highlight_solute", 1, param_slot_37 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"highlight_solute", 2, param_slot_38 };
    static const QUMethod slot_39 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_40[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_40 = {"closeEvent", 1, param_slot_40 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_41 = {"getMouseReleased", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_42 = {"getMousePressed", 1, param_slot_42 };
    static const QMetaData slot_tbl[] = {
	{ "calc_stats(struct MonteCarloStats*,const vector<double>,const vector<double>,QString)", &slot_0, QMetaData::Public },
	{ "calc_stats(struct MonteCarloStats*,const vector<double>,QString)", &slot_1, QMetaData::Public },
	{ "update_k_range(double)", &slot_2, QMetaData::Private },
	{ "update_s_range(double)", &slot_3, QMetaData::Private },
	{ "update_resolution(double)", &slot_4, QMetaData::Private },
	{ "update_initial_solutes(double)", &slot_5, QMetaData::Private },
	{ "update_ff0_max(double)", &slot_6, QMetaData::Private },
	{ "update_ff0_min(double)", &slot_7, QMetaData::Private },
	{ "update_plot_smax(double)", &slot_8, QMetaData::Private },
	{ "update_plot_smin(double)", &slot_9, QMetaData::Private },
	{ "update_plot_fmax(double)", &slot_10, QMetaData::Private },
	{ "update_plot_fmin(double)", &slot_11, QMetaData::Private },
	{ "update_x_resolution(double)", &slot_12, QMetaData::Private },
	{ "update_y_resolution(double)", &slot_13, QMetaData::Private },
	{ "update_x_pixel(double)", &slot_14, QMetaData::Private },
	{ "update_y_pixel(double)", &slot_15, QMetaData::Private },
	{ "plot_1dim()", &slot_16, QMetaData::Private },
	{ "plot_2dim()", &slot_17, QMetaData::Private },
	{ "plot_3dim()", &slot_18, QMetaData::Private },
	{ "select_3dim()", &slot_19, QMetaData::Private },
	{ "select_1dim()", &slot_20, QMetaData::Private },
	{ "select_2dim()", &slot_21, QMetaData::Private },
	{ "select_autolimit()", &slot_22, QMetaData::Private },
	{ "load_distro()", &slot_23, QMetaData::Private },
	{ "make_pngs()", &slot_24, QMetaData::Private },
	{ "load_color()", &slot_25, QMetaData::Private },
	{ "calc_distro()", &slot_26, QMetaData::Private },
	{ "reset_peaks_replot()", &slot_27, QMetaData::Private },
	{ "reset_peaks()", &slot_28, QMetaData::Private },
	{ "assign_peaks()", &slot_29, QMetaData::Private },
	{ "help()", &slot_30, QMetaData::Private },
	{ "save()", &slot_31, QMetaData::Private },
	{ "print()", &slot_32, QMetaData::Private },
	{ "draw()", &slot_33, QMetaData::Private },
	{ "plot_buckets()", &slot_34, QMetaData::Private },
	{ "shrink()", &slot_35, QMetaData::Private },
	{ "edit_solute(int)", &slot_36, QMetaData::Private },
	{ "highlight_solute(int)", &slot_37, QMetaData::Private },
	{ "highlight_solute(int,bool)", &slot_38, QMetaData::Private },
	{ "setup_GUI()", &slot_39, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_40, QMetaData::Private },
	{ "getMouseReleased(const QMouseEvent&)", &slot_41, QMetaData::Private },
	{ "getMousePressed(const QMouseEvent&)", &slot_42, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_GA_Initialize", parentObject,
	slot_tbl, 43,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_GA_Initialize.setMetaObject( metaObj );
    return metaObj;
}

void* US_GA_Initialize::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_GA_Initialize" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_GA_Initialize::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: static_QUType_QString.set(_o,calc_stats((struct MonteCarloStats*)static_QUType_ptr.get(_o+1),(const vector<double>)(*((const vector<double>*)static_QUType_ptr.get(_o+2))),(const vector<double>)(*((const vector<double>*)static_QUType_ptr.get(_o+3))),(QString)static_QUType_QString.get(_o+4))); break;
    case 1: static_QUType_QString.set(_o,calc_stats((struct MonteCarloStats*)static_QUType_ptr.get(_o+1),(const vector<double>)(*((const vector<double>*)static_QUType_ptr.get(_o+2))),(QString)static_QUType_QString.get(_o+3))); break;
    case 2: update_k_range((double)static_QUType_double.get(_o+1)); break;
    case 3: update_s_range((double)static_QUType_double.get(_o+1)); break;
    case 4: update_resolution((double)static_QUType_double.get(_o+1)); break;
    case 5: update_initial_solutes((double)static_QUType_double.get(_o+1)); break;
    case 6: update_ff0_max((double)static_QUType_double.get(_o+1)); break;
    case 7: update_ff0_min((double)static_QUType_double.get(_o+1)); break;
    case 8: update_plot_smax((double)static_QUType_double.get(_o+1)); break;
    case 9: update_plot_smin((double)static_QUType_double.get(_o+1)); break;
    case 10: update_plot_fmax((double)static_QUType_double.get(_o+1)); break;
    case 11: update_plot_fmin((double)static_QUType_double.get(_o+1)); break;
    case 12: update_x_resolution((double)static_QUType_double.get(_o+1)); break;
    case 13: update_y_resolution((double)static_QUType_double.get(_o+1)); break;
    case 14: update_x_pixel((double)static_QUType_double.get(_o+1)); break;
    case 15: update_y_pixel((double)static_QUType_double.get(_o+1)); break;
    case 16: plot_1dim(); break;
    case 17: plot_2dim(); break;
    case 18: plot_3dim(); break;
    case 19: select_3dim(); break;
    case 20: select_1dim(); break;
    case 21: select_2dim(); break;
    case 22: select_autolimit(); break;
    case 23: load_distro(); break;
    case 24: make_pngs(); break;
    case 25: load_color(); break;
    case 26: calc_distro(); break;
    case 27: reset_peaks_replot(); break;
    case 28: reset_peaks(); break;
    case 29: assign_peaks(); break;
    case 30: help(); break;
    case 31: save(); break;
    case 32: print(); break;
    case 33: draw(); break;
    case 34: plot_buckets(); break;
    case 35: shrink(); break;
    case 36: edit_solute((int)static_QUType_int.get(_o+1)); break;
    case 37: highlight_solute((int)static_QUType_int.get(_o+1)); break;
    case 38: highlight_solute((int)static_QUType_int.get(_o+1),(bool)static_QUType_bool.get(_o+2)); break;
    case 39: setup_GUI(); break;
    case 40: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 41: getMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 42: getMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_GA_Initialize::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_GA_Initialize::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_GA_Initialize::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
