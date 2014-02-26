/****************************************************************************
** US_Finite_W meta object code from reading C++ file 'us_finite1.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_finite1.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Finite_W::className() const
{
    return "US_Finite_W";
}

QMetaObject *US_Finite_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Finite_W( "US_Finite_W", &US_Finite_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Finite_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Finite_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Finite_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Finite_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Finite_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"write_fef", 0, 0 };
    static const QUMethod slot_2 = {"write_res", 0, 0 };
    static const QUMethod slot_3 = {"reset", 0, 0 };
    static const QUMethod slot_4 = {"create_model", 0, 0 };
    static const QUMethod slot_5 = {"setup_model", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_6 = {"calc_deltas", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_7 = {"calc_residuals", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_8 = {"calc_alpha", 1, param_slot_8 };
    static const QUMethod slot_9 = {"select_model_control", 0, 0 };
    static const QUMethod slot_10 = {"select_fit_control", 0, 0 };
    static const QUMethod slot_11 = {"start_fit", 0, 0 };
    static const QUMethod slot_12 = {"suspend_fit", 0, 0 };
    static const QUMethod slot_13 = {"resume_fit", 0, 0 };
    static const QUMethod slot_14 = {"cancel_fit", 0, 0 };
    static const QUMethod slot_15 = {"load_fit", 0, 0 };
    static const QUMethod slot_16 = {"save_fit", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"load_fit", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_residuals", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_simulation_parameters_constrained", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_simulation_parameters_unconstrained", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_21 = {"plot_analysis", 1, param_slot_21 };
    static const QUMethod slot_22 = {"dud", 0, 0 };
    static const QUMethod slot_23 = {"swap", 0, 0 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_plot", 1, param_slot_24 };
    static const QUMethod slot_25 = {"shift_all", 0, 0 };
    static const QUMethod slot_26 = {"dud_init", 0, 0 };
    static const QUMethod slot_27 = {"order_variance", 0, 0 };
    static const QUMethod slot_28 = {"view", 0, 0 };
    static const QUMethod slot_29 = {"help", 0, 0 };
    static const QUMethod slot_30 = {"save", 0, 0 };
    static const QUMethod slot_31 = {"second_plot", 0, 0 };
    static const QUMethod slot_32 = {"F_init", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_33 = {"assign_F", 1, param_slot_33 };
    static const QUParameter param_slot_34[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_34 = {"non_interacting_model", 1, param_slot_34 };
    static const QUMethod slot_35 = {"monomer_dimer_model", 0, 0 };
    static const QUMethod slot_36 = {"isomerizing_model", 0, 0 };
    static const QUMethod slot_37 = {"init_finite_element", 0, 0 };
    static const QUMethod slot_38 = {"cleanup_finite", 0, 0 };
    static const QUMethod slot_39 = {"cleanup_dud", 0, 0 };
    static const QUMethod slot_40 = {"updateButtons", 0, 0 };
    static const QUMethod slot_41 = {"monte_carlo", 0, 0 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_42 = {"closeEvent", 1, param_slot_42 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "write_fef()", &slot_1, QMetaData::Private },
	{ "write_res()", &slot_2, QMetaData::Private },
	{ "reset()", &slot_3, QMetaData::Private },
	{ "create_model()", &slot_4, QMetaData::Private },
	{ "setup_model()", &slot_5, QMetaData::Private },
	{ "calc_deltas(const unsigned int)", &slot_6, QMetaData::Private },
	{ "calc_residuals(const unsigned int)", &slot_7, QMetaData::Private },
	{ "calc_alpha()", &slot_8, QMetaData::Private },
	{ "select_model_control()", &slot_9, QMetaData::Private },
	{ "select_fit_control()", &slot_10, QMetaData::Private },
	{ "start_fit()", &slot_11, QMetaData::Private },
	{ "suspend_fit()", &slot_12, QMetaData::Private },
	{ "resume_fit()", &slot_13, QMetaData::Private },
	{ "cancel_fit()", &slot_14, QMetaData::Private },
	{ "load_fit()", &slot_15, QMetaData::Private },
	{ "save_fit()", &slot_16, QMetaData::Private },
	{ "load_fit(const QString&)", &slot_17, QMetaData::Private },
	{ "update_residuals(double)", &slot_18, QMetaData::Private },
	{ "update_simulation_parameters_constrained(const unsigned int)", &slot_19, QMetaData::Private },
	{ "update_simulation_parameters_unconstrained(const unsigned int)", &slot_20, QMetaData::Private },
	{ "plot_analysis()", &slot_21, QMetaData::Private },
	{ "dud()", &slot_22, QMetaData::Private },
	{ "swap()", &slot_23, QMetaData::Private },
	{ "update_plot(const unsigned int)", &slot_24, QMetaData::Private },
	{ "shift_all()", &slot_25, QMetaData::Private },
	{ "dud_init()", &slot_26, QMetaData::Private },
	{ "order_variance()", &slot_27, QMetaData::Private },
	{ "view()", &slot_28, QMetaData::Private },
	{ "help()", &slot_29, QMetaData::Private },
	{ "save()", &slot_30, QMetaData::Private },
	{ "second_plot()", &slot_31, QMetaData::Private },
	{ "F_init()", &slot_32, QMetaData::Private },
	{ "assign_F(const unsigned int)", &slot_33, QMetaData::Private },
	{ "non_interacting_model()", &slot_34, QMetaData::Private },
	{ "monomer_dimer_model()", &slot_35, QMetaData::Private },
	{ "isomerizing_model()", &slot_36, QMetaData::Private },
	{ "init_finite_element()", &slot_37, QMetaData::Private },
	{ "cleanup_finite()", &slot_38, QMetaData::Private },
	{ "cleanup_dud()", &slot_39, QMetaData::Private },
	{ "updateButtons()", &slot_40, QMetaData::Private },
	{ "monte_carlo()", &slot_41, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_42, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Finite_W", parentObject,
	slot_tbl, 43,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Finite_W.setMetaObject( metaObj );
    return metaObj;
}

void* US_Finite_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Finite_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool US_Finite_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: write_fef(); break;
    case 2: write_res(); break;
    case 3: reset(); break;
    case 4: create_model(); break;
    case 5: setup_model(); break;
    case 6: calc_deltas((const unsigned int)(*((const unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 7: calc_residuals((const unsigned int)(*((const unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 8: static_QUType_bool.set(_o,calc_alpha()); break;
    case 9: select_model_control(); break;
    case 10: select_fit_control(); break;
    case 11: start_fit(); break;
    case 12: suspend_fit(); break;
    case 13: resume_fit(); break;
    case 14: cancel_fit(); break;
    case 15: load_fit(); break;
    case 16: save_fit(); break;
    case 17: load_fit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_residuals((double)static_QUType_double.get(_o+1)); break;
    case 19: update_simulation_parameters_constrained((const unsigned int)(*((const unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 20: update_simulation_parameters_unconstrained((const unsigned int)(*((const unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 21: static_QUType_int.set(_o,plot_analysis()); break;
    case 22: dud(); break;
    case 23: swap(); break;
    case 24: update_plot((const unsigned int)(*((const unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 25: shift_all(); break;
    case 26: dud_init(); break;
    case 27: order_variance(); break;
    case 28: view(); break;
    case 29: help(); break;
    case 30: save(); break;
    case 31: second_plot(); break;
    case 32: F_init(); break;
    case 33: assign_F((const unsigned int)(*((const unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 34: static_QUType_int.set(_o,non_interacting_model()); break;
    case 35: monomer_dimer_model(); break;
    case 36: isomerizing_model(); break;
    case 37: init_finite_element(); break;
    case 38: cleanup_finite(); break;
    case 39: cleanup_dud(); break;
    case 40: updateButtons(); break;
    case 41: monte_carlo(); break;
    case 42: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Finite_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Finite_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool US_Finite_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
