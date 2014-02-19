/****************************************************************************
** US_ViewMWL meta object code from reading C++ file 'us_viewmwl.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_viewmwl.h"
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

const char *US_ViewMWL::className() const
{
    return "US_ViewMWL";
}

QMetaObject *US_ViewMWL::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ViewMWL( "US_ViewMWL", &US_ViewMWL::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ViewMWL::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ViewMWL", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ViewMWL::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ViewMWL", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ViewMWL::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"load", 0, 0 };
    static const QUMethod slot_1 = {"save", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_max_lambda", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_min_lambda", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_max_time", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_min_time", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_max_radius", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_min_radius", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_average", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_xscaling", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_yscaling", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_zscaling", 1, param_slot_11 };
    static const QUMethod slot_12 = {"set_radius", 0, 0 };
    static const QUMethod slot_13 = {"set_model", 0, 0 };
    static const QUMethod slot_14 = {"set_wavelength", 0, 0 };
    static const QUMethod slot_15 = {"set_absorbance", 0, 0 };
    static const QUMethod slot_16 = {"set_intensity", 0, 0 };
    static const QUMethod slot_17 = {"set_ascii", 0, 0 };
    static const QUMethod slot_18 = {"set_binary", 0, 0 };
    static const QUMethod slot_19 = {"set_2d", 0, 0 };
    static const QUMethod slot_20 = {"set_3d", 0, 0 };
    static const QUMethod slot_21 = {"set_pngs", 0, 0 };
    static const QUMethod slot_22 = {"setLoading", 0, 0 };
    static const QUMethod slot_23 = {"find_elements", 0, 0 };
    static const QUMethod slot_24 = {"find_minmax_od", 0, 0 };
    static const QUMethod slot_25 = {"cancel", 0, 0 };
    static const QUMethod slot_26 = {"load_model", 0, 0 };
    static const QUMethod slot_27 = {"help", 0, 0 };
    static const QUMethod slot_28 = {"export_data", 0, 0 };
    static const QUMethod slot_29 = {"print", 0, 0 };
    static const QUMethod slot_30 = {"update", 0, 0 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_31 = {"update", 1, param_slot_31 };
    static const QUParameter param_slot_32[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"update", 2, param_slot_32 };
    static const QUMethod slot_33 = {"movie", 0, 0 };
    static const QUParameter param_slot_34[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_34 = {"select_cell", 1, param_slot_34 };
    static const QUParameter param_slot_35[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_35 = {"select_channel", 1, param_slot_35 };
    static const QUMethod slot_36 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_37[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_37 = {"closeEvent", 1, param_slot_37 };
    static const QMetaData slot_tbl[] = {
	{ "load()", &slot_0, QMetaData::Public },
	{ "save()", &slot_1, QMetaData::Public },
	{ "update_max_lambda(double)", &slot_2, QMetaData::Private },
	{ "update_min_lambda(double)", &slot_3, QMetaData::Private },
	{ "update_max_time(double)", &slot_4, QMetaData::Private },
	{ "update_min_time(double)", &slot_5, QMetaData::Private },
	{ "update_max_radius(double)", &slot_6, QMetaData::Private },
	{ "update_min_radius(double)", &slot_7, QMetaData::Private },
	{ "update_average(double)", &slot_8, QMetaData::Private },
	{ "update_xscaling(double)", &slot_9, QMetaData::Private },
	{ "update_yscaling(double)", &slot_10, QMetaData::Private },
	{ "update_zscaling(double)", &slot_11, QMetaData::Private },
	{ "set_radius()", &slot_12, QMetaData::Private },
	{ "set_model()", &slot_13, QMetaData::Private },
	{ "set_wavelength()", &slot_14, QMetaData::Private },
	{ "set_absorbance()", &slot_15, QMetaData::Private },
	{ "set_intensity()", &slot_16, QMetaData::Private },
	{ "set_ascii()", &slot_17, QMetaData::Private },
	{ "set_binary()", &slot_18, QMetaData::Private },
	{ "set_2d()", &slot_19, QMetaData::Private },
	{ "set_3d()", &slot_20, QMetaData::Private },
	{ "set_pngs()", &slot_21, QMetaData::Private },
	{ "setLoading()", &slot_22, QMetaData::Private },
	{ "find_elements()", &slot_23, QMetaData::Private },
	{ "find_minmax_od()", &slot_24, QMetaData::Private },
	{ "cancel()", &slot_25, QMetaData::Private },
	{ "load_model()", &slot_26, QMetaData::Private },
	{ "help()", &slot_27, QMetaData::Private },
	{ "export_data()", &slot_28, QMetaData::Private },
	{ "print()", &slot_29, QMetaData::Private },
	{ "update()", &slot_30, QMetaData::Private },
	{ "update(unsigned int)", &slot_31, QMetaData::Private },
	{ "update(unsigned int,QString)", &slot_32, QMetaData::Private },
	{ "movie()", &slot_33, QMetaData::Private },
	{ "select_cell(int)", &slot_34, QMetaData::Private },
	{ "select_channel(int)", &slot_35, QMetaData::Private },
	{ "setup_GUI()", &slot_36, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_37, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ViewMWL", parentObject,
	slot_tbl, 38,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ViewMWL.setMetaObject( metaObj );
    return metaObj;
}

void* US_ViewMWL::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ViewMWL" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_ViewMWL::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load(); break;
    case 1: save(); break;
    case 2: update_max_lambda((double)static_QUType_double.get(_o+1)); break;
    case 3: update_min_lambda((double)static_QUType_double.get(_o+1)); break;
    case 4: update_max_time((double)static_QUType_double.get(_o+1)); break;
    case 5: update_min_time((double)static_QUType_double.get(_o+1)); break;
    case 6: update_max_radius((double)static_QUType_double.get(_o+1)); break;
    case 7: update_min_radius((double)static_QUType_double.get(_o+1)); break;
    case 8: update_average((double)static_QUType_double.get(_o+1)); break;
    case 9: update_xscaling((double)static_QUType_double.get(_o+1)); break;
    case 10: update_yscaling((double)static_QUType_double.get(_o+1)); break;
    case 11: update_zscaling((double)static_QUType_double.get(_o+1)); break;
    case 12: set_radius(); break;
    case 13: set_model(); break;
    case 14: set_wavelength(); break;
    case 15: set_absorbance(); break;
    case 16: set_intensity(); break;
    case 17: set_ascii(); break;
    case 18: set_binary(); break;
    case 19: set_2d(); break;
    case 20: set_3d(); break;
    case 21: set_pngs(); break;
    case 22: setLoading(); break;
    case 23: find_elements(); break;
    case 24: find_minmax_od(); break;
    case 25: cancel(); break;
    case 26: load_model(); break;
    case 27: help(); break;
    case 28: export_data(); break;
    case 29: print(); break;
    case 30: update(); break;
    case 31: update((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 32: update((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))),(QString)static_QUType_QString.get(_o+2)); break;
    case 33: movie(); break;
    case 34: select_cell((int)static_QUType_int.get(_o+1)); break;
    case 35: select_channel((int)static_QUType_int.get(_o+1)); break;
    case 36: setup_GUI(); break;
    case 37: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ViewMWL::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_ViewMWL::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_ViewMWL::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
