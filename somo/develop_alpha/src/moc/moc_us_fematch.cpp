/****************************************************************************
** US_FeMatch_W meta object code from reading C++ file 'us_fematch.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_fematch.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_FeMatch_W::className() const
{
    return "US_FeMatch_W";
}

QMetaObject *US_FeMatch_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_FeMatch_W( "US_FeMatch_W", &US_FeMatch_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_FeMatch_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_FeMatch_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_FeMatch_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_FeMatch_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_FeMatch_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_component", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_s", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_D", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_C", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_baseline", 1, param_slot_5 };
    static const QUMethod slot_6 = {"cofs_GUI", 0, 0 };
    static const QUMethod slot_7 = {"enableButtons", 0, 0 };
    static const QUMethod slot_8 = {"update_distribution", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_ptr, "struct mfem_data", QUParameter::In }
    };
    static const QUMethod slot_9 = {"clear_data", 1, param_slot_9 };
    static const QUMethod slot_10 = {"updateParameters", 0, 0 };
    static const QUMethod slot_11 = {"fit", 0, 0 };
    static const QUMethod slot_12 = {"calc_residuals", 0, 0 };
    static const QUMethod slot_13 = {"calc_residuals_ra", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"second_plot", 1, param_slot_14 };
    static const QUMethod slot_15 = {"write_cofs", 0, 0 };
    static const QUMethod slot_16 = {"write_res", 0, 0 };
    static const QUMethod slot_17 = {"calc_distros", 0, 0 };
    static const QUMethod slot_18 = {"load_model", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"load_model", 1, param_slot_19 };
    static const QUMethod slot_20 = {"create_modelsystems", 0, 0 };
    static const QUMethod slot_21 = {"update_labels", 0, 0 };
    static const QUMethod slot_22 = {"clearDisplay", 0, 0 };
    static const QUMethod slot_23 = {"view", 0, 0 };
    static const QUMethod slot_24 = {"help", 0, 0 };
    static const QUMethod slot_25 = {"save", 0, 0 };
    static const QUMethod slot_26 = {"second_plot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "update_component(double)", &slot_1, QMetaData::Private },
	{ "update_s(const QString&)", &slot_2, QMetaData::Private },
	{ "update_D(const QString&)", &slot_3, QMetaData::Private },
	{ "update_C(const QString&)", &slot_4, QMetaData::Private },
	{ "update_baseline(const QString&)", &slot_5, QMetaData::Private },
	{ "cofs_GUI()", &slot_6, QMetaData::Private },
	{ "enableButtons()", &slot_7, QMetaData::Private },
	{ "update_distribution()", &slot_8, QMetaData::Private },
	{ "clear_data(struct mfem_data*)", &slot_9, QMetaData::Private },
	{ "updateParameters()", &slot_10, QMetaData::Private },
	{ "fit()", &slot_11, QMetaData::Private },
	{ "calc_residuals()", &slot_12, QMetaData::Private },
	{ "calc_residuals_ra()", &slot_13, QMetaData::Private },
	{ "second_plot(int)", &slot_14, QMetaData::Private },
	{ "write_cofs()", &slot_15, QMetaData::Private },
	{ "write_res()", &slot_16, QMetaData::Private },
	{ "calc_distros()", &slot_17, QMetaData::Private },
	{ "load_model()", &slot_18, QMetaData::Private },
	{ "load_model(const QString&)", &slot_19, QMetaData::Private },
	{ "create_modelsystems()", &slot_20, QMetaData::Private },
	{ "update_labels()", &slot_21, QMetaData::Private },
	{ "clearDisplay()", &slot_22, QMetaData::Private },
	{ "view()", &slot_23, QMetaData::Private },
	{ "help()", &slot_24, QMetaData::Private },
	{ "save()", &slot_25, QMetaData::Private },
	{ "second_plot()", &slot_26, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_FeMatch_W", parentObject,
	slot_tbl, 27,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_FeMatch_W.setMetaObject( metaObj );
    return metaObj;
}

void* US_FeMatch_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_FeMatch_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool US_FeMatch_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: update_component((double)static_QUType_double.get(_o+1)); break;
    case 2: update_s((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_D((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_C((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_baseline((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: cofs_GUI(); break;
    case 7: enableButtons(); break;
    case 8: update_distribution(); break;
    case 9: clear_data((struct mfem_data*)static_QUType_ptr.get(_o+1)); break;
    case 10: updateParameters(); break;
    case 11: fit(); break;
    case 12: calc_residuals(); break;
    case 13: calc_residuals_ra(); break;
    case 14: second_plot((int)static_QUType_int.get(_o+1)); break;
    case 15: write_cofs(); break;
    case 16: write_res(); break;
    case 17: calc_distros(); break;
    case 18: load_model(); break;
    case 19: load_model((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: create_modelsystems(); break;
    case 21: update_labels(); break;
    case 22: clearDisplay(); break;
    case 23: view(); break;
    case 24: help(); break;
    case 25: save(); break;
    case 26: second_plot(); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_FeMatch_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_FeMatch_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool US_FeMatch_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
