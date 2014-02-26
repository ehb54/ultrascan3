/****************************************************************************
** US_DB_RtvInvestigator meta object code from reading C++ file 'us_db_rtv_investigator.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_rtv_investigator.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_RtvInvestigator::className() const
{
    return "US_DB_RtvInvestigator";
}

QMetaObject *US_DB_RtvInvestigator::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RtvInvestigator( "US_DB_RtvInvestigator", &US_DB_RtvInvestigator::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RtvInvestigator::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvInvestigator", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RtvInvestigator::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RtvInvestigator", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RtvInvestigator::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"checkname", 0, 0 };
    static const QUMethod slot_2 = {"checkID", 0, 0 };
    static const QUMethod slot_3 = {"reset", 0, 0 };
    static const QUMethod slot_4 = {"retrieve", 0, 0 };
    static const QUMethod slot_5 = {"help", 0, 0 };
    static const QUMethod slot_6 = {"quit", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_lastname", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"select_name", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"check_data", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"select_data", 1, param_slot_10 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Protected },
	{ "checkname()", &slot_1, QMetaData::Protected },
	{ "checkID()", &slot_2, QMetaData::Protected },
	{ "reset()", &slot_3, QMetaData::Protected },
	{ "retrieve()", &slot_4, QMetaData::Protected },
	{ "help()", &slot_5, QMetaData::Protected },
	{ "quit()", &slot_6, QMetaData::Protected },
	{ "update_lastname(const QString&)", &slot_7, QMetaData::Protected },
	{ "select_name(int)", &slot_8, QMetaData::Protected },
	{ "check_data(int)", &slot_9, QMetaData::Protected },
	{ "select_data(int)", &slot_10, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RtvInvestigator", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RtvInvestigator.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RtvInvestigator::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RtvInvestigator" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_DB_RtvInvestigator::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: checkname(); break;
    case 2: checkID(); break;
    case 3: reset(); break;
    case 4: retrieve(); break;
    case 5: help(); break;
    case 6: quit(); break;
    case 7: update_lastname((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: select_name((int)static_QUType_int.get(_o+1)); break;
    case 9: check_data((int)static_QUType_int.get(_o+1)); break;
    case 10: select_data((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RtvInvestigator::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RtvInvestigator::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_DB_RtvInvestigator::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
