/****************************************************************************
** US_Hydrodyn_Dammin_Opts meta object code from reading C++ file 'us_hydrodyn_dammin_opts.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_dammin_opts.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Dammin_Opts::className() const
{
    return "US_Hydrodyn_Dammin_Opts";
}

QMetaObject *US_Hydrodyn_Dammin_Opts::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Dammin_Opts( "US_Hydrodyn_Dammin_Opts", &US_Hydrodyn_Dammin_Opts::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Dammin_Opts::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Dammin_Opts", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Dammin_Opts::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Dammin_Opts", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Dammin_Opts::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_psv", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_mw", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_partial", 1, param_slot_2 };
    static const QUMethod slot_3 = {"set_write_bead_model", 0, 0 };
    static const QUMethod slot_4 = {"set_remember", 0, 0 };
    static const QUMethod slot_5 = {"set_use_partial", 0, 0 };
    static const QUMethod slot_6 = {"cancel", 0, 0 };
    static const QUMethod slot_7 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_psv(const QString&)", &slot_0, QMetaData::Private },
	{ "update_mw(const QString&)", &slot_1, QMetaData::Private },
	{ "update_partial(const QString&)", &slot_2, QMetaData::Private },
	{ "set_write_bead_model()", &slot_3, QMetaData::Private },
	{ "set_remember()", &slot_4, QMetaData::Private },
	{ "set_use_partial()", &slot_5, QMetaData::Private },
	{ "cancel()", &slot_6, QMetaData::Private },
	{ "help()", &slot_7, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Dammin_Opts", parentObject,
	slot_tbl, 8,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Dammin_Opts.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Dammin_Opts::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Dammin_Opts" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Dammin_Opts::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_psv((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_mw((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_partial((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: set_write_bead_model(); break;
    case 4: set_remember(); break;
    case 5: set_use_partial(); break;
    case 6: cancel(); break;
    case 7: help(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Dammin_Opts::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Dammin_Opts::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Dammin_Opts::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
