/****************************************************************************
** US_Hydrodyn_SasOptionsCurve meta object code from reading C++ file 'us_hydrodyn_sas_options_curve.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_sas_options_curve.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_SasOptionsCurve::className() const
{
    return "US_Hydrodyn_SasOptionsCurve";
}

QMetaObject *US_Hydrodyn_SasOptionsCurve::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SasOptionsCurve( "US_Hydrodyn_SasOptionsCurve", &US_Hydrodyn_SasOptionsCurve::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SasOptionsCurve::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsCurve", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SasOptionsCurve::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsCurve", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SasOptionsCurve::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_wavelength", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_start_angle", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_end_angle", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_delta_angle", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_start_q", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_end_q", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_delta_q", 1, param_slot_7 };
    static const QUMethod slot_8 = {"set_normalize_by_mw", 0, 0 };
    static const QUMethod slot_9 = {"cancel", 0, 0 };
    static const QUMethod slot_10 = {"help", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_11 = {"closeEvent", 1, param_slot_11 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_wavelength(double)", &slot_1, QMetaData::Private },
	{ "update_start_angle(double)", &slot_2, QMetaData::Private },
	{ "update_end_angle(double)", &slot_3, QMetaData::Private },
	{ "update_delta_angle(double)", &slot_4, QMetaData::Private },
	{ "update_start_q(double)", &slot_5, QMetaData::Private },
	{ "update_end_q(double)", &slot_6, QMetaData::Private },
	{ "update_delta_q(double)", &slot_7, QMetaData::Private },
	{ "set_normalize_by_mw()", &slot_8, QMetaData::Private },
	{ "cancel()", &slot_9, QMetaData::Private },
	{ "help()", &slot_10, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_11, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SasOptionsCurve", parentObject,
	slot_tbl, 12,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SasOptionsCurve.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SasOptionsCurve::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SasOptionsCurve" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SasOptionsCurve::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_wavelength((double)static_QUType_double.get(_o+1)); break;
    case 2: update_start_angle((double)static_QUType_double.get(_o+1)); break;
    case 3: update_end_angle((double)static_QUType_double.get(_o+1)); break;
    case 4: update_delta_angle((double)static_QUType_double.get(_o+1)); break;
    case 5: update_start_q((double)static_QUType_double.get(_o+1)); break;
    case 6: update_end_q((double)static_QUType_double.get(_o+1)); break;
    case 7: update_delta_q((double)static_QUType_double.get(_o+1)); break;
    case 8: set_normalize_by_mw(); break;
    case 9: cancel(); break;
    case 10: help(); break;
    case 11: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SasOptionsCurve::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SasOptionsCurve::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SasOptionsCurve::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
