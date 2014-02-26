/****************************************************************************
** US_Hydrodyn_Hydro meta object code from reading C++ file 'us_hydrodyn_hydro.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_hydro.h"
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

const char *US_Hydrodyn_Hydro::className() const
{
    return "US_Hydrodyn_Hydro";
}

QMetaObject *US_Hydrodyn_Hydro::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Hydro( "US_Hydrodyn_Hydro", &US_Hydrodyn_Hydro::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Hydro::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Hydro", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Hydro::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Hydro", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Hydro::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_solvent_name", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_solvent_acronym", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_temperature", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_solvent_viscosity", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_solvent_density", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_unit", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_volume", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_mass", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_overlap", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"select_reference_system", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"select_boundary_cond", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"select_volume_correction", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"select_mass_correction", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"select_overlap", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"select_bead_inclusion", 1, param_slot_15 };
    static const QUMethod slot_16 = {"set_solvent_defaults", 0, 0 };
    static const QUMethod slot_17 = {"set_rotational", 0, 0 };
    static const QUMethod slot_18 = {"set_viscosity", 0, 0 };
    static const QUMethod slot_19 = {"check_solvent_defaults", 0, 0 };
    static const QUMethod slot_20 = {"cancel", 0, 0 };
    static const QUMethod slot_21 = {"help", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_22 = {"closeEvent", 1, param_slot_22 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_solvent_name(const QString&)", &slot_1, QMetaData::Private },
	{ "update_solvent_acronym(const QString&)", &slot_2, QMetaData::Private },
	{ "update_temperature(const QString&)", &slot_3, QMetaData::Private },
	{ "update_solvent_viscosity(const QString&)", &slot_4, QMetaData::Private },
	{ "update_solvent_density(const QString&)", &slot_5, QMetaData::Private },
	{ "update_unit(double)", &slot_6, QMetaData::Private },
	{ "update_volume(const QString&)", &slot_7, QMetaData::Private },
	{ "update_mass(const QString&)", &slot_8, QMetaData::Private },
	{ "update_overlap(const QString&)", &slot_9, QMetaData::Private },
	{ "select_reference_system(int)", &slot_10, QMetaData::Private },
	{ "select_boundary_cond(int)", &slot_11, QMetaData::Private },
	{ "select_volume_correction(int)", &slot_12, QMetaData::Private },
	{ "select_mass_correction(int)", &slot_13, QMetaData::Private },
	{ "select_overlap(int)", &slot_14, QMetaData::Private },
	{ "select_bead_inclusion(int)", &slot_15, QMetaData::Private },
	{ "set_solvent_defaults()", &slot_16, QMetaData::Private },
	{ "set_rotational()", &slot_17, QMetaData::Private },
	{ "set_viscosity()", &slot_18, QMetaData::Private },
	{ "check_solvent_defaults()", &slot_19, QMetaData::Private },
	{ "cancel()", &slot_20, QMetaData::Private },
	{ "help()", &slot_21, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_22, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Hydro", parentObject,
	slot_tbl, 23,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Hydro.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Hydro::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Hydro" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Hydro::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_solvent_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_solvent_acronym((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_temperature((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_solvent_viscosity((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_solvent_density((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_unit((double)static_QUType_double.get(_o+1)); break;
    case 7: update_volume((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_mass((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_overlap((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: select_reference_system((int)static_QUType_int.get(_o+1)); break;
    case 11: select_boundary_cond((int)static_QUType_int.get(_o+1)); break;
    case 12: select_volume_correction((int)static_QUType_int.get(_o+1)); break;
    case 13: select_mass_correction((int)static_QUType_int.get(_o+1)); break;
    case 14: select_overlap((int)static_QUType_int.get(_o+1)); break;
    case 15: select_bead_inclusion((int)static_QUType_int.get(_o+1)); break;
    case 16: set_solvent_defaults(); break;
    case 17: set_rotational(); break;
    case 18: set_viscosity(); break;
    case 19: check_solvent_defaults(); break;
    case 20: cancel(); break;
    case 21: help(); break;
    case 22: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Hydro::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Hydro::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Hydro::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
