/****************************************************************************
** US_GlobalEquil meta object code from reading C++ file 'us_globalequil.h'
**
** Created: Tue Feb 25 10:28:29 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_globalequil.h"
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

const char *US_GlobalEquil::className() const
{
    return "US_GlobalEquil";
}

QMetaObject *US_GlobalEquil::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_GlobalEquil( "US_GlobalEquil", &US_GlobalEquil::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_GlobalEquil::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GlobalEquil", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_GlobalEquil::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GlobalEquil", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_GlobalEquil::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"load", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"load", 2, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"assign_scanfit", 5, param_slot_2 };
    static const QUMethod slot_3 = {"unload", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_projectName", 1, param_slot_4 };
    static const QUMethod slot_5 = {"details", 0, 0 };
    static const QUMethod slot_6 = {"print", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_7 = {"createHtmlDir", 1, param_slot_7 };
    static const QUMethod slot_8 = {"diagnostics", 0, 0 };
    static const QUMethod slot_9 = {"scan_check", 0, 0 };
    static const QUMethod slot_10 = {"reset_limits", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_limit", 1, param_slot_11 };
    static const QUMethod slot_12 = {"help", 0, 0 };
    static const QUMethod slot_13 = {"quit", 0, 0 };
    static const QUMethod slot_14 = {"update_model_button", 0, 0 };
    static const QUMethod slot_15 = {"update_pixmap", 0, 0 };
    static const QUMethod slot_16 = {"updateModelWindow", 0, 0 };
    static const QUMethod slot_17 = {"updateViewer", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"change_FitFlag", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"show_scan", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"activate_scan", 1, param_slot_20 };
    static const QUMethod slot_21 = {"call_show_scan", 0, 0 };
    static const QUMethod slot_22 = {"select_model", 0, 0 };
    static const QUMethod slot_23 = {"fitcontrol", 0, 0 };
    static const QUMethod slot_24 = {"report", 0, 0 };
    static const QUMethod slot_25 = {"monte_carlo", 0, 0 };
    static const QUMethod slot_26 = {"monte_carlo_iterate", 0, 0 };
    static const QUMethod slot_27 = {"loadfit", 0, 0 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"loadfit", 1, param_slot_28 };
    static const QUMethod slot_29 = {"float_all", 0, 0 };
    static const QUMethod slot_30 = {"fix_all", 0, 0 };
    static const QUMethod slot_31 = {"histogram", 0, 0 };
    static const QUMethod slot_32 = {"initialize", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_33 = {"calc_testParameter", 1, param_slot_33 };
    static const QUMethod slot_34 = {"linesearch", 0, 0 };
    static const QUMethod slot_35 = {"select_model_control", 0, 0 };
    static const QUMethod slot_36 = {"setup_vectors", 0, 0 };
    static const QUParameter param_slot_37[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_37 = {"select_scan", 1, param_slot_37 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"select_scan", 1, param_slot_38 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_varptr, "\x11", QUParameter::In }
    };
    static const QUMethod slot_39 = {"select_scan", 1, param_slot_39 };
    static const QUMethod slot_40 = {"fit_converged", 0, 0 };
    static const QUMethod slot_41 = {"fit_suspended", 0, 0 };
    static const QUMethod slot_42 = {"fit_resumed", 0, 0 };
    static const QUParameter param_slot_43[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_43 = {"writeScanList", 2, param_slot_43 };
    static const QUMethod slot_44 = {"clean_runInfo", 0, 0 };
    static const QUMethod slot_45 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_46[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_46 = {"getPlotMouseMoved", 1, param_slot_46 };
    static const QUParameter param_slot_47[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_47 = {"getPlotMouseReleased", 1, param_slot_47 };
    static const QUParameter param_slot_48[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_48 = {"closeEvent", 1, param_slot_48 };
    static const QMetaData slot_tbl[] = {
	{ "load()", &slot_0, QMetaData::Private },
	{ "load(const QString&)", &slot_1, QMetaData::Private },
	{ "assign_scanfit(int,int,int,const QString)", &slot_2, QMetaData::Private },
	{ "unload()", &slot_3, QMetaData::Private },
	{ "update_projectName(const QString&)", &slot_4, QMetaData::Private },
	{ "details()", &slot_5, QMetaData::Private },
	{ "print()", &slot_6, QMetaData::Private },
	{ "createHtmlDir()", &slot_7, QMetaData::Private },
	{ "diagnostics()", &slot_8, QMetaData::Private },
	{ "scan_check()", &slot_9, QMetaData::Private },
	{ "reset_limits()", &slot_10, QMetaData::Private },
	{ "update_limit(float)", &slot_11, QMetaData::Private },
	{ "help()", &slot_12, QMetaData::Private },
	{ "quit()", &slot_13, QMetaData::Private },
	{ "update_model_button()", &slot_14, QMetaData::Private },
	{ "update_pixmap()", &slot_15, QMetaData::Private },
	{ "updateModelWindow()", &slot_16, QMetaData::Private },
	{ "updateViewer()", &slot_17, QMetaData::Private },
	{ "change_FitFlag(int)", &slot_18, QMetaData::Private },
	{ "show_scan(int)", &slot_19, QMetaData::Private },
	{ "activate_scan(double)", &slot_20, QMetaData::Private },
	{ "call_show_scan()", &slot_21, QMetaData::Private },
	{ "select_model()", &slot_22, QMetaData::Private },
	{ "fitcontrol()", &slot_23, QMetaData::Private },
	{ "report()", &slot_24, QMetaData::Private },
	{ "monte_carlo()", &slot_25, QMetaData::Private },
	{ "monte_carlo_iterate()", &slot_26, QMetaData::Private },
	{ "loadfit()", &slot_27, QMetaData::Private },
	{ "loadfit(const QString&)", &slot_28, QMetaData::Private },
	{ "float_all()", &slot_29, QMetaData::Private },
	{ "fix_all()", &slot_30, QMetaData::Private },
	{ "histogram()", &slot_31, QMetaData::Private },
	{ "initialize()", &slot_32, QMetaData::Private },
	{ "calc_testParameter(float)", &slot_33, QMetaData::Private },
	{ "linesearch()", &slot_34, QMetaData::Private },
	{ "select_model_control()", &slot_35, QMetaData::Private },
	{ "setup_vectors()", &slot_36, QMetaData::Private },
	{ "select_scan(double)", &slot_37, QMetaData::Private },
	{ "select_scan(int)", &slot_38, QMetaData::Private },
	{ "select_scan(uint)", &slot_39, QMetaData::Private },
	{ "fit_converged()", &slot_40, QMetaData::Private },
	{ "fit_suspended()", &slot_41, QMetaData::Private },
	{ "fit_resumed()", &slot_42, QMetaData::Private },
	{ "writeScanList(const QString&,const int)", &slot_43, QMetaData::Private },
	{ "clean_runInfo()", &slot_44, QMetaData::Private },
	{ "setup_GUI()", &slot_45, QMetaData::Protected },
	{ "getPlotMouseMoved(const QMouseEvent&)", &slot_46, QMetaData::Protected },
	{ "getPlotMouseReleased(const QMouseEvent&)", &slot_47, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_48, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_GlobalEquil", parentObject,
	slot_tbl, 49,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_GlobalEquil.setMetaObject( metaObj );
    return metaObj;
}

void* US_GlobalEquil::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_GlobalEquil" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_GlobalEquil::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load(); break;
    case 1: static_QUType_int.set(_o,load((const QString&)static_QUType_QString.get(_o+1))); break;
    case 2: static_QUType_int.set(_o,assign_scanfit((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(const QString)static_QUType_QString.get(_o+4))); break;
    case 3: unload(); break;
    case 4: update_projectName((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: details(); break;
    case 6: print(); break;
    case 7: static_QUType_bool.set(_o,createHtmlDir()); break;
    case 8: diagnostics(); break;
    case 9: scan_check(); break;
    case 10: reset_limits(); break;
    case 11: update_limit((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 12: help(); break;
    case 13: quit(); break;
    case 14: update_model_button(); break;
    case 15: update_pixmap(); break;
    case 16: updateModelWindow(); break;
    case 17: updateViewer(); break;
    case 18: change_FitFlag((int)static_QUType_int.get(_o+1)); break;
    case 19: show_scan((int)static_QUType_int.get(_o+1)); break;
    case 20: activate_scan((double)static_QUType_double.get(_o+1)); break;
    case 21: call_show_scan(); break;
    case 22: select_model(); break;
    case 23: fitcontrol(); break;
    case 24: report(); break;
    case 25: monte_carlo(); break;
    case 26: monte_carlo_iterate(); break;
    case 27: loadfit(); break;
    case 28: loadfit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 29: float_all(); break;
    case 30: fix_all(); break;
    case 31: histogram(); break;
    case 32: initialize(); break;
    case 33: calc_testParameter((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 34: linesearch(); break;
    case 35: select_model_control(); break;
    case 36: setup_vectors(); break;
    case 37: select_scan((double)static_QUType_double.get(_o+1)); break;
    case 38: select_scan((int)static_QUType_int.get(_o+1)); break;
    case 39: select_scan((uint)(*((uint*)static_QUType_ptr.get(_o+1)))); break;
    case 40: fit_converged(); break;
    case 41: fit_suspended(); break;
    case 42: fit_resumed(); break;
    case 43: writeScanList((const QString&)static_QUType_QString.get(_o+1),(const int)static_QUType_int.get(_o+2)); break;
    case 44: clean_runInfo(); break;
    case 45: setup_GUI(); break;
    case 46: getPlotMouseMoved((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 47: getPlotMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 48: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_GlobalEquil::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_GlobalEquil::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_GlobalEquil::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
