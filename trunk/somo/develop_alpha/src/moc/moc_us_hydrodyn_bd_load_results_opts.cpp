/****************************************************************************
** US_Hydrodyn_BD_Load_Results_Opts meta object code from reading C++ file 'us_hydrodyn_bd_load_results_opts.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_bd_load_results_opts.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_BD_Load_Results_Opts::className() const
{
    return "US_Hydrodyn_BD_Load_Results_Opts";
}

QMetaObject *US_Hydrodyn_BD_Load_Results_Opts::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_BD_Load_Results_Opts( "US_Hydrodyn_BD_Load_Results_Opts", &US_Hydrodyn_BD_Load_Results_Opts::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_BD_Load_Results_Opts::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_BD_Load_Results_Opts", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_BD_Load_Results_Opts::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_BD_Load_Results_Opts", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_BD_Load_Results_Opts::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_solvent_name", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_solvent_acronym", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_temperature", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_solvent_viscosity", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_solvent_density", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_psv", 1, param_slot_5 };
    static const QUMethod slot_6 = {"set_to_browflex", 0, 0 };
    static const QUMethod slot_7 = {"set_solvent_defaults", 0, 0 };
    static const QUMethod slot_8 = {"set_check_fix_overlaps", 0, 0 };
    static const QUMethod slot_9 = {"check_solvent_browflex_defaults", 0, 0 };
    static const QUMethod slot_10 = {"cancel", 0, 0 };
    static const QUMethod slot_11 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_solvent_name(const QString&)", &slot_0, QMetaData::Private },
	{ "update_solvent_acronym(const QString&)", &slot_1, QMetaData::Private },
	{ "update_temperature(const QString&)", &slot_2, QMetaData::Private },
	{ "update_solvent_viscosity(const QString&)", &slot_3, QMetaData::Private },
	{ "update_solvent_density(const QString&)", &slot_4, QMetaData::Private },
	{ "update_psv(const QString&)", &slot_5, QMetaData::Private },
	{ "set_to_browflex()", &slot_6, QMetaData::Private },
	{ "set_solvent_defaults()", &slot_7, QMetaData::Private },
	{ "set_check_fix_overlaps()", &slot_8, QMetaData::Private },
	{ "check_solvent_browflex_defaults()", &slot_9, QMetaData::Private },
	{ "cancel()", &slot_10, QMetaData::Private },
	{ "help()", &slot_11, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_BD_Load_Results_Opts", parentObject,
	slot_tbl, 12,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_BD_Load_Results_Opts.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_BD_Load_Results_Opts::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_BD_Load_Results_Opts" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_BD_Load_Results_Opts::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_solvent_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_solvent_acronym((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_temperature((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_solvent_viscosity((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_solvent_density((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_psv((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: set_to_browflex(); break;
    case 7: set_solvent_defaults(); break;
    case 8: set_check_fix_overlaps(); break;
    case 9: check_solvent_browflex_defaults(); break;
    case 10: cancel(); break;
    case 11: help(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_BD_Load_Results_Opts::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_BD_Load_Results_Opts::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_BD_Load_Results_Opts::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
