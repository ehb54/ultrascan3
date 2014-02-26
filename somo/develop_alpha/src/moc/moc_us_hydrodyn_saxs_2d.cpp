/****************************************************************************
** US_Hydrodyn_Saxs_2d meta object code from reading C++ file 'us_hydrodyn_saxs_2d.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_2d.h"
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

const char *US_Hydrodyn_Saxs_2d::className() const
{
    return "US_Hydrodyn_Saxs_2d";
}

QMetaObject *US_Hydrodyn_Saxs_2d::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_2d( "US_Hydrodyn_Saxs_2d", &US_Hydrodyn_Saxs_2d::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_2d::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_2d", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_2d::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_2d", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_2d::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_lambda", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_detector_distance", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_detector_height", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_detector_width", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_detector_pixels_height", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_detector_pixels_width", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_beam_center_pixels_height", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_beam_center_pixels_width", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_sample_rotations", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"adjust_wheel", 1, param_slot_10 };
    static const QUMethod slot_11 = {"info", 0, 0 };
    static const QUMethod slot_12 = {"start", 0, 0 };
    static const QUMethod slot_13 = {"stop", 0, 0 };
    static const QUMethod slot_14 = {"integrate", 0, 0 };
    static const QUMethod slot_15 = {"clear_display", 0, 0 };
    static const QUMethod slot_16 = {"update_font", 0, 0 };
    static const QUMethod slot_17 = {"save", 0, 0 };
    static const QUMethod slot_18 = {"cancel", 0, 0 };
    static const QUMethod slot_19 = {"help", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_20 = {"closeEvent", 1, param_slot_20 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_lambda(const QString&)", &slot_1, QMetaData::Private },
	{ "update_detector_distance(const QString&)", &slot_2, QMetaData::Private },
	{ "update_detector_height(const QString&)", &slot_3, QMetaData::Private },
	{ "update_detector_width(const QString&)", &slot_4, QMetaData::Private },
	{ "update_detector_pixels_height(const QString&)", &slot_5, QMetaData::Private },
	{ "update_detector_pixels_width(const QString&)", &slot_6, QMetaData::Private },
	{ "update_beam_center_pixels_height(const QString&)", &slot_7, QMetaData::Private },
	{ "update_beam_center_pixels_width(const QString&)", &slot_8, QMetaData::Private },
	{ "update_sample_rotations(const QString&)", &slot_9, QMetaData::Private },
	{ "adjust_wheel(double)", &slot_10, QMetaData::Private },
	{ "info()", &slot_11, QMetaData::Private },
	{ "start()", &slot_12, QMetaData::Private },
	{ "stop()", &slot_13, QMetaData::Private },
	{ "integrate()", &slot_14, QMetaData::Private },
	{ "clear_display()", &slot_15, QMetaData::Private },
	{ "update_font()", &slot_16, QMetaData::Private },
	{ "save()", &slot_17, QMetaData::Private },
	{ "cancel()", &slot_18, QMetaData::Private },
	{ "help()", &slot_19, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_20, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_2d", parentObject,
	slot_tbl, 21,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_2d.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_2d::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_2d" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_2d::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_lambda((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_detector_distance((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_detector_height((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_detector_width((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_detector_pixels_height((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_detector_pixels_width((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_beam_center_pixels_height((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_beam_center_pixels_width((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_sample_rotations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: adjust_wheel((double)static_QUType_double.get(_o+1)); break;
    case 11: info(); break;
    case 12: start(); break;
    case 13: stop(); break;
    case 14: integrate(); break;
    case 15: clear_display(); break;
    case 16: update_font(); break;
    case 17: save(); break;
    case 18: cancel(); break;
    case 19: help(); break;
    case 20: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_2d::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_2d::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_2d::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
