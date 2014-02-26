/****************************************************************************
** US_Hydrodyn_Saxs_Screen meta object code from reading C++ file 'us_hydrodyn_saxs_screen.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_screen.h"
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

const char *US_Hydrodyn_Saxs_Screen::className() const
{
    return "US_Hydrodyn_Saxs_Screen";
}

QMetaObject *US_Hydrodyn_Saxs_Screen::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Screen( "US_Hydrodyn_Saxs_Screen", &US_Hydrodyn_Saxs_Screen::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Screen::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Screen", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Screen::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Screen", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Screen::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"table_value", 2, param_slot_1 };
    static const QUMethod slot_2 = {"push", 0, 0 };
    static const QUMethod slot_3 = {"clear_plot_row", 0, 0 };
    static const QUMethod slot_4 = {"clear_plot_all", 0, 0 };
    static const QUMethod slot_5 = {"save_plot", 0, 0 };
    static const QUMethod slot_6 = {"load_plot", 0, 0 };
    static const QUMethod slot_7 = {"replot", 0, 0 };
    static const QUMethod slot_8 = {"replot_saxs", 0, 0 };
    static const QUMethod slot_9 = {"save_saxs_plot", 0, 0 };
    static const QUMethod slot_10 = {"set_target", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"adjust_wheel", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"adjust_wheel2", 1, param_slot_12 };
    static const QUMethod slot_13 = {"start", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ "already_running", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"start", 1, param_slot_14 };
    static const QUMethod slot_15 = {"run_all_targets", 0, 0 };
    static const QUMethod slot_16 = {"stop", 0, 0 };
    static const QUMethod slot_17 = {"clear_display", 0, 0 };
    static const QUMethod slot_18 = {"update_font", 0, 0 };
    static const QUMethod slot_19 = {"save", 0, 0 };
    static const QUMethod slot_20 = {"cancel", 0, 0 };
    static const QUMethod slot_21 = {"help", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_22 = {"closeEvent", 1, param_slot_22 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "table_value(int,int)", &slot_1, QMetaData::Private },
	{ "push()", &slot_2, QMetaData::Private },
	{ "clear_plot_row()", &slot_3, QMetaData::Private },
	{ "clear_plot_all()", &slot_4, QMetaData::Private },
	{ "save_plot()", &slot_5, QMetaData::Private },
	{ "load_plot()", &slot_6, QMetaData::Private },
	{ "replot()", &slot_7, QMetaData::Private },
	{ "replot_saxs()", &slot_8, QMetaData::Private },
	{ "save_saxs_plot()", &slot_9, QMetaData::Private },
	{ "set_target()", &slot_10, QMetaData::Private },
	{ "adjust_wheel(double)", &slot_11, QMetaData::Private },
	{ "adjust_wheel2(double)", &slot_12, QMetaData::Private },
	{ "start()", &slot_13, QMetaData::Private },
	{ "start(bool)", &slot_14, QMetaData::Private },
	{ "run_all_targets()", &slot_15, QMetaData::Private },
	{ "stop()", &slot_16, QMetaData::Private },
	{ "clear_display()", &slot_17, QMetaData::Private },
	{ "update_font()", &slot_18, QMetaData::Private },
	{ "save()", &slot_19, QMetaData::Private },
	{ "cancel()", &slot_20, QMetaData::Private },
	{ "help()", &slot_21, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_22, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Screen", parentObject,
	slot_tbl, 23,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Screen.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Screen::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Screen" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Screen::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: table_value((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 2: push(); break;
    case 3: clear_plot_row(); break;
    case 4: clear_plot_all(); break;
    case 5: save_plot(); break;
    case 6: load_plot(); break;
    case 7: replot(); break;
    case 8: replot_saxs(); break;
    case 9: save_saxs_plot(); break;
    case 10: set_target(); break;
    case 11: adjust_wheel((double)static_QUType_double.get(_o+1)); break;
    case 12: adjust_wheel2((double)static_QUType_double.get(_o+1)); break;
    case 13: start(); break;
    case 14: start((bool)static_QUType_bool.get(_o+1)); break;
    case 15: run_all_targets(); break;
    case 16: stop(); break;
    case 17: clear_display(); break;
    case 18: update_font(); break;
    case 19: save(); break;
    case 20: cancel(); break;
    case 21: help(); break;
    case 22: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Screen::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Screen::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Screen::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
