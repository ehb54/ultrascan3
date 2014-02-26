/****************************************************************************
** US_FeMatchRa_W meta object code from reading C++ file 'us_fematch_ra.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_fematch_ra.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_FeMatchRa_W::className() const
{
    return "US_FeMatchRa_W";
}

QMetaObject *US_FeMatchRa_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_FeMatchRa_W( "US_FeMatchRa_W", &US_FeMatchRa_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_FeMatchRa_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_FeMatchRa_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_FeMatchRa_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_FeMatchRa_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_FeMatchRa_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"enableButtons", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_ptr, "struct mfem_data", QUParameter::In }
    };
    static const QUMethod slot_2 = {"clear_data", 1, param_slot_2 };
    static const QUMethod slot_3 = {"fit", 0, 0 };
    static const QUMethod slot_4 = {"write_res", 0, 0 };
    static const QUMethod slot_5 = {"load_model", 0, 0 };
    static const QUMethod slot_6 = {"clearDisplay", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"printError", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_radialGrid", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_timeGrid", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ "val", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_simpoints", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ "val", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_band_volume", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ "val", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_model", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ "val", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_parameter", 1, param_slot_13 };
    static const QUMethod slot_14 = {"assign_parameters", 0, 0 };
    static const QUMethod slot_15 = {"assign_model", 0, 0 };
    static const QUMethod slot_16 = {"show_model", 0, 0 };
    static const QUMethod slot_17 = {"show_parameter", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"select_plotmode", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_ptr, "list<Parameter>", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_19 = {"reduce", 4, param_slot_19 };
    static const QUMethod slot_20 = {"view", 0, 0 };
    static const QUMethod slot_21 = {"help", 0, 0 };
    static const QUMethod slot_22 = {"save", 0, 0 };
    static const QUMethod slot_23 = {"second_plot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "enableButtons()", &slot_1, QMetaData::Private },
	{ "clear_data(struct mfem_data*)", &slot_2, QMetaData::Private },
	{ "fit()", &slot_3, QMetaData::Private },
	{ "write_res()", &slot_4, QMetaData::Private },
	{ "load_model()", &slot_5, QMetaData::Private },
	{ "clearDisplay()", &slot_6, QMetaData::Private },
	{ "printError(const int&)", &slot_7, QMetaData::Private },
	{ "update_radialGrid(int)", &slot_8, QMetaData::Private },
	{ "update_timeGrid(int)", &slot_9, QMetaData::Private },
	{ "update_simpoints(double)", &slot_10, QMetaData::Private },
	{ "update_band_volume(double)", &slot_11, QMetaData::Private },
	{ "update_model(double)", &slot_12, QMetaData::Private },
	{ "update_parameter(double)", &slot_13, QMetaData::Private },
	{ "assign_parameters()", &slot_14, QMetaData::Private },
	{ "assign_model()", &slot_15, QMetaData::Private },
	{ "show_model()", &slot_16, QMetaData::Private },
	{ "show_parameter()", &slot_17, QMetaData::Private },
	{ "select_plotmode(int)", &slot_18, QMetaData::Private },
	{ "reduce(list<Parameter>*,double*,double*,double*)", &slot_19, QMetaData::Private },
	{ "view()", &slot_20, QMetaData::Private },
	{ "help()", &slot_21, QMetaData::Private },
	{ "save()", &slot_22, QMetaData::Private },
	{ "second_plot()", &slot_23, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_FeMatchRa_W", parentObject,
	slot_tbl, 24,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_FeMatchRa_W.setMetaObject( metaObj );
    return metaObj;
}

void* US_FeMatchRa_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_FeMatchRa_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool US_FeMatchRa_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: enableButtons(); break;
    case 2: clear_data((struct mfem_data*)static_QUType_ptr.get(_o+1)); break;
    case 3: fit(); break;
    case 4: write_res(); break;
    case 5: load_model(); break;
    case 6: clearDisplay(); break;
    case 7: printError((const int&)static_QUType_int.get(_o+1)); break;
    case 8: update_radialGrid((int)static_QUType_int.get(_o+1)); break;
    case 9: update_timeGrid((int)static_QUType_int.get(_o+1)); break;
    case 10: update_simpoints((double)static_QUType_double.get(_o+1)); break;
    case 11: update_band_volume((double)static_QUType_double.get(_o+1)); break;
    case 12: update_model((double)static_QUType_double.get(_o+1)); break;
    case 13: update_parameter((double)static_QUType_double.get(_o+1)); break;
    case 14: assign_parameters(); break;
    case 15: assign_model(); break;
    case 16: show_model(); break;
    case 17: show_parameter(); break;
    case 18: select_plotmode((int)static_QUType_int.get(_o+1)); break;
    case 19: reduce((list<Parameter>*)static_QUType_ptr.get(_o+1),(double*)static_QUType_varptr.get(_o+2),(double*)static_QUType_varptr.get(_o+3),(double*)static_QUType_varptr.get(_o+4)); break;
    case 20: view(); break;
    case 21: help(); break;
    case 22: save(); break;
    case 23: second_plot(); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_FeMatchRa_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_FeMatchRa_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool US_FeMatchRa_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
