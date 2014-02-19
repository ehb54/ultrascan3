/****************************************************************************
** US_vhwEnhanced meta object code from reading C++ file 'us_vhwenhanced.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_vhwenhanced.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QMouseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_vhwEnhanced::className() const
{
    return "US_vhwEnhanced";
}

QMetaObject *US_vhwEnhanced::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_vhwEnhanced( "US_vhwEnhanced", &US_vhwEnhanced::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_vhwEnhanced::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_vhwEnhanced", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_vhwEnhanced::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_vhwEnhanced", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_vhwEnhanced::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"write_vhw", 0, 0 };
    static const QUMethod slot_2 = {"write_dis", 0, 0 };
    static const QUMethod slot_3 = {"write_res", 0, 0 };
    static const QUMethod slot_4 = {"select_groups", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_divisions", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_tolerance", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_7 = {"getAnalysisPlotMouseReleased", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_8 = {"getAnalysisPlotMousePressed", 1, param_slot_8 };
    static const QUMethod slot_9 = {"cleanup", 0, 0 };
    static const QUMethod slot_10 = {"save_model", 0, 0 };
    static const QUMethod slot_11 = {"cofs", 0, 0 };
    static const QUMethod slot_12 = {"sa2d", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_13 = {"calc_sed", 2, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"find_root", 2, param_slot_14 };
    static const QUMethod slot_15 = {"activate_reset", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_16 = {"plot_analysis", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_17 = {"setups", 1, param_slot_17 };
    static const QUMethod slot_18 = {"view", 0, 0 };
    static const QUMethod slot_19 = {"help", 0, 0 };
    static const QUMethod slot_20 = {"save", 0, 0 };
    static const QUMethod slot_21 = {"second_plot", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_22 = {"excludeSingleSignal", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_23 = {"excludeRangeSignal", 1, param_slot_23 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "write_vhw()", &slot_1, QMetaData::Private },
	{ "write_dis()", &slot_2, QMetaData::Private },
	{ "write_res()", &slot_3, QMetaData::Private },
	{ "select_groups()", &slot_4, QMetaData::Private },
	{ "update_divisions(double)", &slot_5, QMetaData::Private },
	{ "update_tolerance(double)", &slot_6, QMetaData::Private },
	{ "getAnalysisPlotMouseReleased(const QMouseEvent&)", &slot_7, QMetaData::Private },
	{ "getAnalysisPlotMousePressed(const QMouseEvent&)", &slot_8, QMetaData::Private },
	{ "cleanup()", &slot_9, QMetaData::Private },
	{ "save_model()", &slot_10, QMetaData::Private },
	{ "cofs()", &slot_11, QMetaData::Private },
	{ "sa2d()", &slot_12, QMetaData::Private },
	{ "calc_sed(unsigned int)", &slot_13, QMetaData::Private },
	{ "find_root(double)", &slot_14, QMetaData::Private },
	{ "activate_reset()", &slot_15, QMetaData::Private },
	{ "plot_analysis()", &slot_16, QMetaData::Private },
	{ "setups()", &slot_17, QMetaData::Private },
	{ "view()", &slot_18, QMetaData::Private },
	{ "help()", &slot_19, QMetaData::Private },
	{ "save()", &slot_20, QMetaData::Private },
	{ "second_plot()", &slot_21, QMetaData::Private },
	{ "excludeSingleSignal(unsigned int)", &slot_22, QMetaData::Private },
	{ "excludeRangeSignal(unsigned int)", &slot_23, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_vhwEnhanced", parentObject,
	slot_tbl, 24,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_vhwEnhanced.setMetaObject( metaObj );
    return metaObj;
}

void* US_vhwEnhanced::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_vhwEnhanced" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool US_vhwEnhanced::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: write_vhw(); break;
    case 2: write_dis(); break;
    case 3: write_res(); break;
    case 4: select_groups(); break;
    case 5: update_divisions((double)static_QUType_double.get(_o+1)); break;
    case 6: update_tolerance((double)static_QUType_double.get(_o+1)); break;
    case 7: getAnalysisPlotMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 8: getAnalysisPlotMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 9: cleanup(); break;
    case 10: save_model(); break;
    case 11: cofs(); break;
    case 12: sa2d(); break;
    case 13: static_QUType_int.set(_o,calc_sed((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))))); break;
    case 14: static_QUType_double.set(_o,find_root((double)static_QUType_double.get(_o+1))); break;
    case 15: activate_reset(); break;
    case 16: static_QUType_int.set(_o,plot_analysis()); break;
    case 17: static_QUType_int.set(_o,setups()); break;
    case 18: view(); break;
    case 19: help(); break;
    case 20: save(); break;
    case 21: second_plot(); break;
    case 22: excludeSingleSignal((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 23: excludeRangeSignal((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_vhwEnhanced::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_vhwEnhanced::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool US_vhwEnhanced::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
