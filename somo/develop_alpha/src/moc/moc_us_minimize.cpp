/****************************************************************************
** US_Minimize meta object code from reading C++ file 'us_minimize.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_minimize.h"
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

const char *US_Minimize::className() const
{
    return "US_Minimize";
}

QMetaObject *US_Minimize::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Minimize( "US_Minimize", &US_Minimize::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Minimize::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Minimize", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Minimize::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Minimize", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Minimize::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"setup_GUI2", 0, 0 };
    static const QUMethod slot_2 = {"cancelFit", 0, 0 };
    static const QUMethod slot_3 = {"update_plotGroup", 0, 0 };
    static const QUMethod slot_4 = {"update_plotAll", 0, 0 };
    static const QUMethod slot_5 = {"update_plotSingle", 0, 0 };
    static const QUMethod slot_6 = {"update_constrained", 0, 0 };
    static const QUMethod slot_7 = {"update_unconstrained", 0, 0 };
    static const QUMethod slot_8 = {"update_autoconverge", 0, 0 };
    static const QUMethod slot_9 = {"update_manualconverge", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_maxIterations", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_lambdaStart", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_lambdaStep", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_tolerance", 1, param_slot_13 };
    static const QUMethod slot_14 = {"change_showGuiFit", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_15 = {"closeEvent", 1, param_slot_15 };
    static const QUMethod slot_16 = {"resumeFit", 0, 0 };
    static const QUMethod slot_17 = {"suspendFit", 0, 0 };
    static const QUMethod slot_18 = {"help", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_19 = {"calc_B", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_ptr, "float*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_20 = {"calc_testParameter", 2, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_ptr, "float*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_21 = {"linesearch", 2, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_ptr, "float*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "float*", QUParameter::InOut }
    };
    static const QUMethod slot_22 = {"updateQN", 2, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_nlsMethod", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_24 = {"Fit", 1, param_slot_24 };
    static const QUMethod slot_25 = {"update_fitDialog", 0, 0 };
    static const QUMethod slot_26 = {"cancel", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"setGUI", 1, param_slot_27 };
    static const QUMethod slot_28 = {"calc_residuals", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_29 = {"fit_init", 1, param_slot_29 };
    static const QUMethod slot_30 = {"cleanup", 0, 0 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_31 = {"calc_model", 2, param_slot_31 };
    static const QUMethod slot_32 = {"write_data", 0, 0 };
    static const QUMethod slot_33 = {"plot_overlays", 0, 0 };
    static const QUMethod slot_34 = {"plot_residuals", 0, 0 };
    static const QUMethod slot_35 = {"plot_two", 0, 0 };
    static const QUMethod slot_36 = {"plot_three", 0, 0 };
    static const QUMethod slot_37 = {"plot_four", 0, 0 };
    static const QUMethod slot_38 = {"plot_five", 0, 0 };
    static const QUMethod slot_39 = {"saveFit", 0, 0 };
    static const QUMethod slot_40 = {"print", 0, 0 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_41 = {"updateRange", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_42 = {"calc_jacobian", 1, param_slot_42 };
    static const QUMethod slot_43 = {"endFit", 0, 0 };
    static const QUParameter param_slot_44[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_44 = {"try_adjustment", 1, param_slot_44 };
    static const QUMethod slot_45 = {"view_report", 0, 0 };
    static const QUMethod slot_46 = {"startFit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Protected },
	{ "setup_GUI2()", &slot_1, QMetaData::Protected },
	{ "cancelFit()", &slot_2, QMetaData::Protected },
	{ "update_plotGroup()", &slot_3, QMetaData::Protected },
	{ "update_plotAll()", &slot_4, QMetaData::Protected },
	{ "update_plotSingle()", &slot_5, QMetaData::Protected },
	{ "update_constrained()", &slot_6, QMetaData::Protected },
	{ "update_unconstrained()", &slot_7, QMetaData::Protected },
	{ "update_autoconverge()", &slot_8, QMetaData::Protected },
	{ "update_manualconverge()", &slot_9, QMetaData::Protected },
	{ "update_maxIterations(const QString&)", &slot_10, QMetaData::Protected },
	{ "update_lambdaStart(const QString&)", &slot_11, QMetaData::Protected },
	{ "update_lambdaStep(const QString&)", &slot_12, QMetaData::Protected },
	{ "update_tolerance(const QString&)", &slot_13, QMetaData::Protected },
	{ "change_showGuiFit()", &slot_14, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_15, QMetaData::Protected },
	{ "resumeFit()", &slot_16, QMetaData::Protected },
	{ "suspendFit()", &slot_17, QMetaData::Protected },
	{ "help()", &slot_18, QMetaData::Protected },
	{ "calc_B()", &slot_19, QMetaData::Protected },
	{ "calc_testParameter(float**,float)", &slot_20, QMetaData::Protected },
	{ "linesearch(float**,float)", &slot_21, QMetaData::Protected },
	{ "updateQN(float**,float**)", &slot_22, QMetaData::Protected },
	{ "update_nlsMethod(int)", &slot_23, QMetaData::Public },
	{ "Fit()", &slot_24, QMetaData::Public },
	{ "update_fitDialog()", &slot_25, QMetaData::Public },
	{ "cancel()", &slot_26, QMetaData::Public },
	{ "setGUI(bool)", &slot_27, QMetaData::Public },
	{ "calc_residuals()", &slot_28, QMetaData::Protected },
	{ "fit_init()", &slot_29, QMetaData::Protected },
	{ "cleanup()", &slot_30, QMetaData::Protected },
	{ "calc_model(double*)", &slot_31, QMetaData::Protected },
	{ "write_data()", &slot_32, QMetaData::Protected },
	{ "plot_overlays()", &slot_33, QMetaData::Protected },
	{ "plot_residuals()", &slot_34, QMetaData::Protected },
	{ "plot_two()", &slot_35, QMetaData::Protected },
	{ "plot_three()", &slot_36, QMetaData::Protected },
	{ "plot_four()", &slot_37, QMetaData::Protected },
	{ "plot_five()", &slot_38, QMetaData::Protected },
	{ "saveFit()", &slot_39, QMetaData::Protected },
	{ "print()", &slot_40, QMetaData::Protected },
	{ "updateRange(double)", &slot_41, QMetaData::Protected },
	{ "calc_jacobian()", &slot_42, QMetaData::Protected },
	{ "endFit()", &slot_43, QMetaData::Protected },
	{ "try_adjustment()", &slot_44, QMetaData::Protected },
	{ "view_report()", &slot_45, QMetaData::Public },
	{ "startFit()", &slot_46, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"hasConverged", 0, 0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"currentStatus", 1, param_signal_1 };
    static const QUMethod signal_2 = {"parametersUpdated", 0, 0 };
    static const QUMethod signal_3 = {"newParameters", 0, 0 };
    static const QUMethod signal_4 = {"fitSuspended", 0, 0 };
    static const QUMethod signal_5 = {"fitAborted", 0, 0 };
    static const QUMethod signal_6 = {"fitResumed", 0, 0 };
    static const QUMethod signal_7 = {"fitStarted", 0, 0 };
    static const QUMethod signal_8 = {"fittingWidgetClosed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "hasConverged()", &signal_0, QMetaData::Public },
	{ "currentStatus(const QString&)", &signal_1, QMetaData::Public },
	{ "parametersUpdated()", &signal_2, QMetaData::Public },
	{ "newParameters()", &signal_3, QMetaData::Public },
	{ "fitSuspended()", &signal_4, QMetaData::Public },
	{ "fitAborted()", &signal_5, QMetaData::Public },
	{ "fitResumed()", &signal_6, QMetaData::Public },
	{ "fitStarted()", &signal_7, QMetaData::Public },
	{ "fittingWidgetClosed()", &signal_8, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Minimize", parentObject,
	slot_tbl, 47,
	signal_tbl, 9,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Minimize.setMetaObject( metaObj );
    return metaObj;
}

void* US_Minimize::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Minimize" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL hasConverged
void US_Minimize::hasConverged()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL currentStatus
void US_Minimize::currentStatus( const QString& t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 1, t0 );
}

// SIGNAL parametersUpdated
void US_Minimize::parametersUpdated()
{
    activate_signal( staticMetaObject()->signalOffset() + 2 );
}

// SIGNAL newParameters
void US_Minimize::newParameters()
{
    activate_signal( staticMetaObject()->signalOffset() + 3 );
}

// SIGNAL fitSuspended
void US_Minimize::fitSuspended()
{
    activate_signal( staticMetaObject()->signalOffset() + 4 );
}

// SIGNAL fitAborted
void US_Minimize::fitAborted()
{
    activate_signal( staticMetaObject()->signalOffset() + 5 );
}

// SIGNAL fitResumed
void US_Minimize::fitResumed()
{
    activate_signal( staticMetaObject()->signalOffset() + 6 );
}

// SIGNAL fitStarted
void US_Minimize::fitStarted()
{
    activate_signal( staticMetaObject()->signalOffset() + 7 );
}

// SIGNAL fittingWidgetClosed
void US_Minimize::fittingWidgetClosed()
{
    activate_signal( staticMetaObject()->signalOffset() + 8 );
}

bool US_Minimize::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: setup_GUI2(); break;
    case 2: cancelFit(); break;
    case 3: update_plotGroup(); break;
    case 4: update_plotAll(); break;
    case 5: update_plotSingle(); break;
    case 6: update_constrained(); break;
    case 7: update_unconstrained(); break;
    case 8: update_autoconverge(); break;
    case 9: update_manualconverge(); break;
    case 10: update_maxIterations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_lambdaStart((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_lambdaStep((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_tolerance((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: change_showGuiFit(); break;
    case 15: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 16: resumeFit(); break;
    case 17: suspendFit(); break;
    case 18: help(); break;
    case 19: static_QUType_int.set(_o,calc_B()); break;
    case 20: calc_testParameter((float**)static_QUType_ptr.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 21: linesearch((float**)static_QUType_ptr.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 22: updateQN((float**)static_QUType_ptr.get(_o+1),(float**)static_QUType_ptr.get(_o+2)); break;
    case 23: update_nlsMethod((int)static_QUType_int.get(_o+1)); break;
    case 24: static_QUType_int.set(_o,Fit()); break;
    case 25: update_fitDialog(); break;
    case 26: cancel(); break;
    case 27: setGUI((bool)static_QUType_bool.get(_o+1)); break;
    case 28: calc_residuals(); break;
    case 29: static_QUType_bool.set(_o,fit_init()); break;
    case 30: cleanup(); break;
    case 31: static_QUType_int.set(_o,calc_model((double*)static_QUType_varptr.get(_o+1))); break;
    case 32: write_data(); break;
    case 33: plot_overlays(); break;
    case 34: plot_residuals(); break;
    case 35: plot_two(); break;
    case 36: plot_three(); break;
    case 37: plot_four(); break;
    case 38: plot_five(); break;
    case 39: saveFit(); break;
    case 40: print(); break;
    case 41: updateRange((double)static_QUType_double.get(_o+1)); break;
    case 42: static_QUType_int.set(_o,calc_jacobian()); break;
    case 43: endFit(); break;
    case 44: static_QUType_bool.set(_o,try_adjustment()); break;
    case 45: view_report(); break;
    case 46: startFit(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Minimize::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: hasConverged(); break;
    case 1: currentStatus((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: parametersUpdated(); break;
    case 3: newParameters(); break;
    case 4: fitSuspended(); break;
    case 5: fitAborted(); break;
    case 6: fitResumed(); break;
    case 7: fitStarted(); break;
    case 8: fittingWidgetClosed(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Minimize::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Minimize::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
