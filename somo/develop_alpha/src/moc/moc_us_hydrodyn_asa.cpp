/****************************************************************************
** US_Hydrodyn_ASA meta object code from reading C++ file 'us_hydrodyn_asa.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_asa.h"
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

const char *US_Hydrodyn_ASA::className() const
{
    return "US_Hydrodyn_ASA";
}

QMetaObject *US_Hydrodyn_ASA::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_ASA( "US_Hydrodyn_ASA", &US_Hydrodyn_ASA::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_ASA::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_ASA", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_ASA::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_ASA", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_ASA::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_probe_radius", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_probe_recheck_radius", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_asa_threshold", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_asa_threshold_percent", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_asa_grid_threshold", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_asa_grid_threshold_percent", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_asab1_step", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_hydrate_probe_radius", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_hydrate_threshold", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_vvv_probe_radius", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_vvv_grid_dR", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"select_asa_method", 1, param_slot_12 };
    static const QUMethod slot_13 = {"set_asa_calculation", 0, 0 };
    static const QUMethod slot_14 = {"set_bead_check", 0, 0 };
    static const QUMethod slot_15 = {"set_vvv", 0, 0 };
    static const QUMethod slot_16 = {"cancel", 0, 0 };
    static const QUMethod slot_17 = {"help", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_18 = {"closeEvent", 1, param_slot_18 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_probe_radius(double)", &slot_1, QMetaData::Private },
	{ "update_probe_recheck_radius(double)", &slot_2, QMetaData::Private },
	{ "update_asa_threshold(double)", &slot_3, QMetaData::Private },
	{ "update_asa_threshold_percent(double)", &slot_4, QMetaData::Private },
	{ "update_asa_grid_threshold(double)", &slot_5, QMetaData::Private },
	{ "update_asa_grid_threshold_percent(double)", &slot_6, QMetaData::Private },
	{ "update_asab1_step(double)", &slot_7, QMetaData::Private },
	{ "update_hydrate_probe_radius(double)", &slot_8, QMetaData::Private },
	{ "update_hydrate_threshold(double)", &slot_9, QMetaData::Private },
	{ "update_vvv_probe_radius(double)", &slot_10, QMetaData::Private },
	{ "update_vvv_grid_dR(double)", &slot_11, QMetaData::Private },
	{ "select_asa_method(int)", &slot_12, QMetaData::Private },
	{ "set_asa_calculation()", &slot_13, QMetaData::Private },
	{ "set_bead_check()", &slot_14, QMetaData::Private },
	{ "set_vvv()", &slot_15, QMetaData::Private },
	{ "cancel()", &slot_16, QMetaData::Private },
	{ "help()", &slot_17, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_18, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_ASA", parentObject,
	slot_tbl, 19,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_ASA.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_ASA::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_ASA" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_ASA::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_probe_radius((double)static_QUType_double.get(_o+1)); break;
    case 2: update_probe_recheck_radius((double)static_QUType_double.get(_o+1)); break;
    case 3: update_asa_threshold((double)static_QUType_double.get(_o+1)); break;
    case 4: update_asa_threshold_percent((double)static_QUType_double.get(_o+1)); break;
    case 5: update_asa_grid_threshold((double)static_QUType_double.get(_o+1)); break;
    case 6: update_asa_grid_threshold_percent((double)static_QUType_double.get(_o+1)); break;
    case 7: update_asab1_step((double)static_QUType_double.get(_o+1)); break;
    case 8: update_hydrate_probe_radius((double)static_QUType_double.get(_o+1)); break;
    case 9: update_hydrate_threshold((double)static_QUType_double.get(_o+1)); break;
    case 10: update_vvv_probe_radius((double)static_QUType_double.get(_o+1)); break;
    case 11: update_vvv_grid_dR((double)static_QUType_double.get(_o+1)); break;
    case 12: select_asa_method((int)static_QUType_int.get(_o+1)); break;
    case 13: set_asa_calculation(); break;
    case 14: set_bead_check(); break;
    case 15: set_vvv(); break;
    case 16: cancel(); break;
    case 17: help(); break;
    case 18: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_ASA::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_ASA::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_ASA::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
