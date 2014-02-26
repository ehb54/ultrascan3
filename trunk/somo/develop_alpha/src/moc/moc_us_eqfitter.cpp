/****************************************************************************
** US_EqFitter meta object code from reading C++ file 'us_eqfitter.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_eqfitter.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_EqFitter::className() const
{
    return "US_EqFitter";
}

QMetaObject *US_EqFitter::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_EqFitter( "US_EqFitter", &US_EqFitter::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_EqFitter::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EqFitter", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_EqFitter::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EqFitter", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_EqFitter::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_Minimize::staticMetaObject();
    static const QUMethod slot_0 = {"cleanup", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"updateRange", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"setRange", 1, param_slot_2 };
    static const QUMethod slot_3 = {"saveFit", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"saveFit", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_5 = {"calc_jacobian", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_6 = {"try_adjustment", 1, param_slot_6 };
    static const QUMethod slot_7 = {"view_report", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_8 = {"calc_model", 2, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_9 = {"fit_init", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_10 = {"guess_mapForward", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_11 = {"parameter_mapBackward", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_12 = {"parameter_addRandomNoise", 2, param_slot_12 };
    static const QUMethod slot_13 = {"write_data", 0, 0 };
    static const QUMethod slot_14 = {"write_report", 0, 0 };
    static const QUMethod slot_15 = {"plot_overlays", 0, 0 };
    static const QUMethod slot_16 = {"plot_residuals", 0, 0 };
    static const QUMethod slot_17 = {"plot_two", 0, 0 };
    static const QUMethod slot_18 = {"plot_three", 0, 0 };
    static const QUMethod slot_19 = {"plot_five", 0, 0 };
    static const QUMethod slot_20 = {"plot_four", 0, 0 };
    static const QUMethod slot_21 = {"plot_six", 0, 0 };
    static const QUMethod slot_22 = {"print", 0, 0 };
    static const QUMethod slot_23 = {"calc_integral", 0, 0 };
    static const QUMethod slot_24 = {"endFit", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_25 = {"createHtmlDir", 1, param_slot_25 };
    static const QUMethod slot_26 = {"calc_dlncr2", 0, 0 };
    static const QUMethod slot_27 = {"print_histogram", 0, 0 };
    static const QUMethod slot_28 = {"startFit", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_29 = {"GLLS_global", 1, param_slot_29 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_30 = {"GLLS_individual", 1, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_31 = {"NNLS_individual", 1, param_slot_31 };
    static const QMetaData slot_tbl[] = {
	{ "cleanup()", &slot_0, QMetaData::Private },
	{ "updateRange(double)", &slot_1, QMetaData::Private },
	{ "setRange(double)", &slot_2, QMetaData::Private },
	{ "saveFit()", &slot_3, QMetaData::Private },
	{ "saveFit(const QString&)", &slot_4, QMetaData::Private },
	{ "calc_jacobian()", &slot_5, QMetaData::Private },
	{ "try_adjustment()", &slot_6, QMetaData::Private },
	{ "view_report()", &slot_7, QMetaData::Public },
	{ "calc_model(double*)", &slot_8, QMetaData::Public },
	{ "fit_init()", &slot_9, QMetaData::Public },
	{ "guess_mapForward(double*)", &slot_10, QMetaData::Public },
	{ "parameter_mapBackward(double*)", &slot_11, QMetaData::Public },
	{ "parameter_addRandomNoise(double*,float)", &slot_12, QMetaData::Public },
	{ "write_data()", &slot_13, QMetaData::Public },
	{ "write_report()", &slot_14, QMetaData::Public },
	{ "plot_overlays()", &slot_15, QMetaData::Public },
	{ "plot_residuals()", &slot_16, QMetaData::Public },
	{ "plot_two()", &slot_17, QMetaData::Public },
	{ "plot_three()", &slot_18, QMetaData::Public },
	{ "plot_five()", &slot_19, QMetaData::Public },
	{ "plot_four()", &slot_20, QMetaData::Public },
	{ "plot_six()", &slot_21, QMetaData::Public },
	{ "print()", &slot_22, QMetaData::Public },
	{ "calc_integral()", &slot_23, QMetaData::Public },
	{ "endFit()", &slot_24, QMetaData::Public },
	{ "createHtmlDir()", &slot_25, QMetaData::Public },
	{ "calc_dlncr2()", &slot_26, QMetaData::Public },
	{ "print_histogram()", &slot_27, QMetaData::Public },
	{ "startFit()", &slot_28, QMetaData::Public },
	{ "GLLS_global()", &slot_29, QMetaData::Public },
	{ "GLLS_individual()", &slot_30, QMetaData::Public },
	{ "NNLS_individual()", &slot_31, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"dataSaved", 2, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "dataSaved(const QString&,const int)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_EqFitter", parentObject,
	slot_tbl, 32,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_EqFitter.setMetaObject( metaObj );
    return metaObj;
}

void* US_EqFitter::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_EqFitter" ) )
	return this;
    return US_Minimize::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL dataSaved
void US_EqFitter::dataSaved( const QString& t0, const int t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_QString.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    activate_signal( clist, o );
}

bool US_EqFitter::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cleanup(); break;
    case 1: updateRange((double)static_QUType_double.get(_o+1)); break;
    case 2: setRange((double)static_QUType_double.get(_o+1)); break;
    case 3: saveFit(); break;
    case 4: saveFit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: static_QUType_int.set(_o,calc_jacobian()); break;
    case 6: static_QUType_bool.set(_o,try_adjustment()); break;
    case 7: view_report(); break;
    case 8: static_QUType_int.set(_o,calc_model((double*)static_QUType_varptr.get(_o+1))); break;
    case 9: static_QUType_bool.set(_o,fit_init()); break;
    case 10: guess_mapForward((double*)static_QUType_varptr.get(_o+1)); break;
    case 11: parameter_mapBackward((double*)static_QUType_varptr.get(_o+1)); break;
    case 12: parameter_addRandomNoise((double*)static_QUType_varptr.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 13: write_data(); break;
    case 14: write_report(); break;
    case 15: plot_overlays(); break;
    case 16: plot_residuals(); break;
    case 17: plot_two(); break;
    case 18: plot_three(); break;
    case 19: plot_five(); break;
    case 20: plot_four(); break;
    case 21: plot_six(); break;
    case 22: print(); break;
    case 23: calc_integral(); break;
    case 24: endFit(); break;
    case 25: static_QUType_bool.set(_o,createHtmlDir()); break;
    case 26: calc_dlncr2(); break;
    case 27: print_histogram(); break;
    case 28: startFit(); break;
    case 29: static_QUType_int.set(_o,GLLS_global()); break;
    case 30: static_QUType_int.set(_o,GLLS_individual()); break;
    case 31: static_QUType_int.set(_o,NNLS_individual()); break;
    default:
	return US_Minimize::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_EqFitter::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: dataSaved((const QString&)static_QUType_QString.get(_o+1),(const int)static_QUType_int.get(_o+2)); break;
    default:
	return US_Minimize::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_EqFitter::qt_property( int id, int f, QVariant* v)
{
    return US_Minimize::qt_property( id, f, v);
}

bool US_EqFitter::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
