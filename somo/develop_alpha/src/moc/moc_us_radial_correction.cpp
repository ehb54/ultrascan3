/****************************************************************************
** US_RadialCorrection meta object code from reading C++ file 'us_radial_correction.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_radial_correction.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_RadialCorrection::className() const
{
    return "US_RadialCorrection";
}

QMetaObject *US_RadialCorrection::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_RadialCorrection( "US_RadialCorrection", &US_RadialCorrection::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_RadialCorrection::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_RadialCorrection", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_RadialCorrection::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_RadialCorrection", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_RadialCorrection::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"process", 0, 0 };
    static const QUMethod slot_2 = {"load", 0, 0 };
    static const QUMethod slot_3 = {"help", 0, 0 };
    static const QUMethod slot_4 = {"reset", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"set_correction", 1, param_slot_5 };
    static const QUMethod slot_6 = {"update", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"set_speed", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"set_top_measured", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"set_top_target", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"set_bottom_measured", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_scaled", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_rotor", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_centerpiece", 1, param_slot_13 };
    static const QUMethod slot_14 = {"update_trim", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Public },
	{ "process()", &slot_1, QMetaData::Public },
	{ "load()", &slot_2, QMetaData::Public },
	{ "help()", &slot_3, QMetaData::Public },
	{ "reset()", &slot_4, QMetaData::Public },
	{ "set_correction(double)", &slot_5, QMetaData::Public },
	{ "update()", &slot_6, QMetaData::Public },
	{ "set_speed(double)", &slot_7, QMetaData::Public },
	{ "set_top_measured(double)", &slot_8, QMetaData::Public },
	{ "set_top_target(double)", &slot_9, QMetaData::Public },
	{ "set_bottom_measured(double)", &slot_10, QMetaData::Public },
	{ "update_scaled(int)", &slot_11, QMetaData::Public },
	{ "update_rotor(int)", &slot_12, QMetaData::Public },
	{ "update_centerpiece(int)", &slot_13, QMetaData::Public },
	{ "update_trim()", &slot_14, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_RadialCorrection", parentObject,
	slot_tbl, 15,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_RadialCorrection.setMetaObject( metaObj );
    return metaObj;
}

void* US_RadialCorrection::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_RadialCorrection" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_RadialCorrection::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: process(); break;
    case 2: load(); break;
    case 3: help(); break;
    case 4: reset(); break;
    case 5: set_correction((double)static_QUType_double.get(_o+1)); break;
    case 6: update(); break;
    case 7: set_speed((double)static_QUType_double.get(_o+1)); break;
    case 8: set_top_measured((double)static_QUType_double.get(_o+1)); break;
    case 9: set_top_target((double)static_QUType_double.get(_o+1)); break;
    case 10: set_bottom_measured((double)static_QUType_double.get(_o+1)); break;
    case 11: update_scaled((int)static_QUType_int.get(_o+1)); break;
    case 12: update_rotor((int)static_QUType_int.get(_o+1)); break;
    case 13: update_centerpiece((int)static_QUType_int.get(_o+1)); break;
    case 14: update_trim(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_RadialCorrection::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_RadialCorrection::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_RadialCorrection::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
