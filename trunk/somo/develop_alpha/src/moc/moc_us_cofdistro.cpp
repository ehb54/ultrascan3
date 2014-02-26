/****************************************************************************
** US_CofDistro_W meta object code from reading C++ file 'us_cofdistro.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_cofdistro.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_CofDistro_W::className() const
{
    return "US_CofDistro_W";
}

QMetaObject *US_CofDistro_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_CofDistro_W( "US_CofDistro_W", &US_CofDistro_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_CofDistro_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_CofDistro_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_CofDistro_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_CofDistro_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_CofDistro_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"linesearch", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_resolution", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_s_max", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_s_min", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_ff0_min", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_ff0_max", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_par_c", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_shape_distro", 1, param_slot_8 };
    static const QUMethod slot_9 = {"set_par_c", 0, 0 };
    static const QUMethod slot_10 = {"setupGUI", 0, 0 };
    static const QUMethod slot_11 = {"enableButtons", 0, 0 };
    static const QUMethod slot_12 = {"update_distribution", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_ptr, "struct mfem_data", QUParameter::In }
    };
    static const QUMethod slot_13 = {"clear_data", 1, param_slot_13 };
    static const QUMethod slot_14 = {"fit", 0, 0 };
    static const QUMethod slot_15 = {"calc_residuals", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_16 = {"calc_testParameter", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"second_plot", 1, param_slot_17 };
    static const QUMethod slot_18 = {"write_cofdistro", 0, 0 };
    static const QUMethod slot_19 = {"write_res", 0, 0 };
    static const QUMethod slot_20 = {"calc_distros", 0, 0 };
    static const QUMethod slot_21 = {"set_abc", 0, 0 };
    static const QUMethod slot_22 = {"view", 0, 0 };
    static const QUMethod slot_23 = {"help", 0, 0 };
    static const QUMethod slot_24 = {"save", 0, 0 };
    static const QUMethod slot_25 = {"second_plot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "linesearch()", &slot_1, QMetaData::Private },
	{ "update_resolution(double)", &slot_2, QMetaData::Private },
	{ "update_s_max(double)", &slot_3, QMetaData::Private },
	{ "update_s_min(double)", &slot_4, QMetaData::Private },
	{ "update_ff0_min(double)", &slot_5, QMetaData::Private },
	{ "update_ff0_max(double)", &slot_6, QMetaData::Private },
	{ "update_par_c(double)", &slot_7, QMetaData::Private },
	{ "update_shape_distro(int)", &slot_8, QMetaData::Private },
	{ "set_par_c()", &slot_9, QMetaData::Private },
	{ "setupGUI()", &slot_10, QMetaData::Private },
	{ "enableButtons()", &slot_11, QMetaData::Private },
	{ "update_distribution()", &slot_12, QMetaData::Private },
	{ "clear_data(struct mfem_data*)", &slot_13, QMetaData::Private },
	{ "fit()", &slot_14, QMetaData::Private },
	{ "calc_residuals()", &slot_15, QMetaData::Private },
	{ "calc_testParameter(float)", &slot_16, QMetaData::Private },
	{ "second_plot(int)", &slot_17, QMetaData::Private },
	{ "write_cofdistro()", &slot_18, QMetaData::Private },
	{ "write_res()", &slot_19, QMetaData::Private },
	{ "calc_distros()", &slot_20, QMetaData::Private },
	{ "set_abc()", &slot_21, QMetaData::Private },
	{ "view()", &slot_22, QMetaData::Private },
	{ "help()", &slot_23, QMetaData::Private },
	{ "save()", &slot_24, QMetaData::Private },
	{ "second_plot()", &slot_25, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_CofDistro_W", parentObject,
	slot_tbl, 26,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_CofDistro_W.setMetaObject( metaObj );
    return metaObj;
}

void* US_CofDistro_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_CofDistro_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool US_CofDistro_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: linesearch(); break;
    case 2: update_resolution((double)static_QUType_double.get(_o+1)); break;
    case 3: update_s_max((double)static_QUType_double.get(_o+1)); break;
    case 4: update_s_min((double)static_QUType_double.get(_o+1)); break;
    case 5: update_ff0_min((double)static_QUType_double.get(_o+1)); break;
    case 6: update_ff0_max((double)static_QUType_double.get(_o+1)); break;
    case 7: update_par_c((double)static_QUType_double.get(_o+1)); break;
    case 8: update_shape_distro((int)static_QUType_int.get(_o+1)); break;
    case 9: set_par_c(); break;
    case 10: setupGUI(); break;
    case 11: enableButtons(); break;
    case 12: update_distribution(); break;
    case 13: clear_data((struct mfem_data*)static_QUType_ptr.get(_o+1)); break;
    case 14: fit(); break;
    case 15: calc_residuals(); break;
    case 16: calc_testParameter((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 17: second_plot((int)static_QUType_int.get(_o+1)); break;
    case 18: write_cofdistro(); break;
    case 19: write_res(); break;
    case 20: calc_distros(); break;
    case 21: set_abc(); break;
    case 22: view(); break;
    case 23: help(); break;
    case 24: save(); break;
    case 25: second_plot(); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_CofDistro_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_CofDistro_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool US_CofDistro_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
