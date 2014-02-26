/****************************************************************************
** US_Hydrodyn_Misc meta object code from reading C++ file 'us_hydrodyn_misc.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_misc.h"
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

const char *US_Hydrodyn_Misc::className() const
{
    return "US_Hydrodyn_Misc";
}

QMetaObject *US_Hydrodyn_Misc::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Misc( "US_Hydrodyn_Misc", &US_Hydrodyn_Misc::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Misc::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Misc", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Misc::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Misc", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Misc::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_vbar", 0, 0 };
    static const QUMethod slot_2 = {"set_pb_rule_on", 0, 0 };
    static const QUMethod slot_3 = {"select_vbar", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_vbar_signal", 2, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_vbar", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_vbar_temperature", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_hydrovol", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_avg_radius", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_avg_mass", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_avg_hydration", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_avg_volume", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_avg_vbar", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_target_e_density", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_target_volume", 1, param_slot_14 };
    static const QUMethod slot_15 = {"set_set_target_on_load_pdb", 0, 0 };
    static const QUMethod slot_16 = {"set_equalize_radii", 0, 0 };
    static const QUMethod slot_17 = {"set_hydro_supc", 0, 0 };
    static const QUMethod slot_18 = {"set_hydro_zeno", 0, 0 };
    static const QUMethod slot_19 = {"set_export_msroll", 0, 0 };
    static const QUMethod slot_20 = {"cancel", 0, 0 };
    static const QUMethod slot_21 = {"help", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_22 = {"closeEvent", 1, param_slot_22 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_vbar()", &slot_1, QMetaData::Private },
	{ "set_pb_rule_on()", &slot_2, QMetaData::Private },
	{ "select_vbar()", &slot_3, QMetaData::Private },
	{ "update_vbar_signal(float,float)", &slot_4, QMetaData::Private },
	{ "update_vbar(const QString&)", &slot_5, QMetaData::Private },
	{ "update_vbar_temperature(const QString&)", &slot_6, QMetaData::Private },
	{ "update_hydrovol(double)", &slot_7, QMetaData::Private },
	{ "update_avg_radius(double)", &slot_8, QMetaData::Private },
	{ "update_avg_mass(double)", &slot_9, QMetaData::Private },
	{ "update_avg_hydration(double)", &slot_10, QMetaData::Private },
	{ "update_avg_volume(double)", &slot_11, QMetaData::Private },
	{ "update_avg_vbar(double)", &slot_12, QMetaData::Private },
	{ "update_target_e_density(const QString&)", &slot_13, QMetaData::Private },
	{ "update_target_volume(const QString&)", &slot_14, QMetaData::Private },
	{ "set_set_target_on_load_pdb()", &slot_15, QMetaData::Private },
	{ "set_equalize_radii()", &slot_16, QMetaData::Private },
	{ "set_hydro_supc()", &slot_17, QMetaData::Private },
	{ "set_hydro_zeno()", &slot_18, QMetaData::Private },
	{ "set_export_msroll()", &slot_19, QMetaData::Private },
	{ "cancel()", &slot_20, QMetaData::Private },
	{ "help()", &slot_21, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_22, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"vbar_changed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "vbar_changed()", &signal_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Misc", parentObject,
	slot_tbl, 23,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Misc.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Misc::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Misc" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL vbar_changed
void US_Hydrodyn_Misc::vbar_changed()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_Hydrodyn_Misc::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_vbar(); break;
    case 2: set_pb_rule_on(); break;
    case 3: select_vbar(); break;
    case 4: update_vbar_signal((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 5: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_vbar_temperature((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_hydrovol((double)static_QUType_double.get(_o+1)); break;
    case 8: update_avg_radius((double)static_QUType_double.get(_o+1)); break;
    case 9: update_avg_mass((double)static_QUType_double.get(_o+1)); break;
    case 10: update_avg_hydration((double)static_QUType_double.get(_o+1)); break;
    case 11: update_avg_volume((double)static_QUType_double.get(_o+1)); break;
    case 12: update_avg_vbar((double)static_QUType_double.get(_o+1)); break;
    case 13: update_target_e_density((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_target_volume((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: set_set_target_on_load_pdb(); break;
    case 16: set_equalize_radii(); break;
    case 17: set_hydro_supc(); break;
    case 18: set_hydro_zeno(); break;
    case 19: set_export_msroll(); break;
    case 20: cancel(); break;
    case 21: help(); break;
    case 22: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Misc::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: vbar_changed(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Misc::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Misc::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
