/****************************************************************************
** US_Hydrodyn_Saxs_Mw meta object code from reading C++ file 'us_hydrodyn_saxs_mw.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_mw.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Saxs_Mw::className() const
{
    return "US_Hydrodyn_Saxs_Mw";
}

QMetaObject *US_Hydrodyn_Saxs_Mw::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Mw( "US_Hydrodyn_Saxs_Mw", &US_Hydrodyn_Saxs_Mw::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Mw::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Mw", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Mw::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Mw", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Mw::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_mw", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_partial", 1, param_slot_1 };
    static const QUMethod slot_2 = {"set_to_last_used_mw", 0, 0 };
    static const QUMethod slot_3 = {"set_remember", 0, 0 };
    static const QUMethod slot_4 = {"set_use_partial", 0, 0 };
    static const QUMethod slot_5 = {"cancel", 0, 0 };
    static const QUMethod slot_6 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_mw(const QString&)", &slot_0, QMetaData::Private },
	{ "update_partial(const QString&)", &slot_1, QMetaData::Private },
	{ "set_to_last_used_mw()", &slot_2, QMetaData::Private },
	{ "set_remember()", &slot_3, QMetaData::Private },
	{ "set_use_partial()", &slot_4, QMetaData::Private },
	{ "cancel()", &slot_5, QMetaData::Private },
	{ "help()", &slot_6, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Mw", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Mw.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Mw::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Mw" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Mw::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_mw((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_partial((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: set_to_last_used_mw(); break;
    case 3: set_remember(); break;
    case 4: set_use_partial(); break;
    case 5: cancel(); break;
    case 6: help(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Mw::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Mw::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Mw::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
