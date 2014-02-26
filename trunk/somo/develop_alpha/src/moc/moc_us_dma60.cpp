/****************************************************************************
** US_DMA60 meta object code from reading C++ file 'us_dma60.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_dma60.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DMA60::className() const
{
    return "US_DMA60";
}

QMetaObject *US_DMA60::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DMA60( "US_DMA60", &US_DMA60::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DMA60::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DMA60", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DMA60::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DMA60", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DMA60::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_temperature", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_pressure", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_T_air", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_T_water", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_description", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_units", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_concentration", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_T_sample", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_conc_limit_low", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_conc_limit_high", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_steps", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_select_conc", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_select_density", 1, param_slot_12 };
    static const QUMethod slot_13 = {"extrapolate_single_value", 0, 0 };
    static const QUMethod slot_14 = {"calibrate", 0, 0 };
    static const QUMethod slot_15 = {"save_k", 0, 0 };
    static const QUMethod slot_16 = {"load_k", 0, 0 };
    static const QUMethod slot_17 = {"reset_k", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"select_k", 1, param_slot_18 };
    static const QUMethod slot_19 = {"measurement", 0, 0 };
    static const QUMethod slot_20 = {"save_measurement", 0, 0 };
    static const QUMethod slot_21 = {"load_measurement", 0, 0 };
    static const QUMethod slot_22 = {"reset_measurement", 0, 0 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"select_measurement", 1, param_slot_23 };
    static const QUMethod slot_24 = {"extrapolate", 0, 0 };
    static const QUMethod slot_25 = {"help", 0, 0 };
    static const QUMethod slot_26 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_temperature(const QString&)", &slot_0, QMetaData::Protected },
	{ "update_pressure(const QString&)", &slot_1, QMetaData::Protected },
	{ "update_T_air(const QString&)", &slot_2, QMetaData::Protected },
	{ "update_T_water(const QString&)", &slot_3, QMetaData::Protected },
	{ "update_description(const QString&)", &slot_4, QMetaData::Protected },
	{ "update_units(const QString&)", &slot_5, QMetaData::Protected },
	{ "update_concentration(const QString&)", &slot_6, QMetaData::Protected },
	{ "update_T_sample(const QString&)", &slot_7, QMetaData::Protected },
	{ "update_conc_limit_low(const QString&)", &slot_8, QMetaData::Protected },
	{ "update_conc_limit_high(const QString&)", &slot_9, QMetaData::Protected },
	{ "update_steps(const QString&)", &slot_10, QMetaData::Protected },
	{ "update_select_conc(const QString&)", &slot_11, QMetaData::Protected },
	{ "update_select_density(const QString&)", &slot_12, QMetaData::Protected },
	{ "extrapolate_single_value()", &slot_13, QMetaData::Protected },
	{ "calibrate()", &slot_14, QMetaData::Protected },
	{ "save_k()", &slot_15, QMetaData::Protected },
	{ "load_k()", &slot_16, QMetaData::Protected },
	{ "reset_k()", &slot_17, QMetaData::Protected },
	{ "select_k(int)", &slot_18, QMetaData::Protected },
	{ "measurement()", &slot_19, QMetaData::Protected },
	{ "save_measurement()", &slot_20, QMetaData::Protected },
	{ "load_measurement()", &slot_21, QMetaData::Protected },
	{ "reset_measurement()", &slot_22, QMetaData::Protected },
	{ "select_measurement(int)", &slot_23, QMetaData::Protected },
	{ "extrapolate()", &slot_24, QMetaData::Protected },
	{ "help()", &slot_25, QMetaData::Protected },
	{ "quit()", &slot_26, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DMA60", parentObject,
	slot_tbl, 27,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DMA60.setMetaObject( metaObj );
    return metaObj;
}

void* US_DMA60::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DMA60" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_DMA60::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_temperature((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_pressure((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_T_air((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_T_water((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_description((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_units((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_concentration((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_T_sample((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_conc_limit_low((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_conc_limit_high((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_steps((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_select_conc((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_select_density((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: extrapolate_single_value(); break;
    case 14: calibrate(); break;
    case 15: save_k(); break;
    case 16: load_k(); break;
    case 17: reset_k(); break;
    case 18: select_k((int)static_QUType_int.get(_o+1)); break;
    case 19: measurement(); break;
    case 20: save_measurement(); break;
    case 21: load_measurement(); break;
    case 22: reset_measurement(); break;
    case 23: select_measurement((int)static_QUType_int.get(_o+1)); break;
    case 24: extrapolate(); break;
    case 25: help(); break;
    case 26: quit(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DMA60::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DMA60::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_DMA60::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
