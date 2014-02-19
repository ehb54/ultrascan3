/****************************************************************************
** US_Enter_DNA_DB meta object code from reading C++ file 'us_enter_dna.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_enter_dna.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Enter_DNA_DB::className() const
{
    return "US_Enter_DNA_DB";
}

QMetaObject *US_Enter_DNA_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Enter_DNA_DB( "US_Enter_DNA_DB", &US_Enter_DNA_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Enter_DNA_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Enter_DNA_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Enter_DNA_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Enter_DNA_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Enter_DNA_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_description", 1, param_slot_0 };
    static const QUMethod slot_1 = {"update_sequence", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_vbar", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_e280", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_e260", 1, param_slot_4 };
    static const QUMethod slot_5 = {"save_HD", 0, 0 };
    static const QUMethod slot_6 = {"save_DB", 0, 0 };
    static const QUMethod slot_7 = {"help", 0, 0 };
    static const QUMethod slot_8 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_description(const QString&)", &slot_0, QMetaData::Protected },
	{ "update_sequence()", &slot_1, QMetaData::Protected },
	{ "update_vbar(const QString&)", &slot_2, QMetaData::Protected },
	{ "update_e280(const QString&)", &slot_3, QMetaData::Protected },
	{ "update_e260(const QString&)", &slot_4, QMetaData::Protected },
	{ "save_HD()", &slot_5, QMetaData::Protected },
	{ "save_DB()", &slot_6, QMetaData::Protected },
	{ "help()", &slot_7, QMetaData::Protected },
	{ "quit()", &slot_8, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Enter_DNA_DB", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Enter_DNA_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_Enter_DNA_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Enter_DNA_DB" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_Enter_DNA_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_description((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_sequence(); break;
    case 2: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_e280((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_e260((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: save_HD(); break;
    case 6: save_DB(); break;
    case 7: help(); break;
    case 8: quit(); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Enter_DNA_DB::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Enter_DNA_DB::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_Enter_DNA_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
