/****************************************************************************
** US_DB_TblInvestigator meta object code from reading C++ file 'us_db_tbl_investigator.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_tbl_investigator.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_TblInvestigator::className() const
{
    return "US_DB_TblInvestigator";
}

QMetaObject *US_DB_TblInvestigator::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_TblInvestigator( "US_DB_TblInvestigator", &US_DB_TblInvestigator::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_TblInvestigator::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_TblInvestigator", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_TblInvestigator::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_TblInvestigator", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_TblInvestigator::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUMethod slot_0 = {"GUI", 0, 0 };
    static const QUMethod slot_1 = {"checkname", 0, 0 };
    static const QUMethod slot_2 = {"update", 0, 0 };
    static const QUMethod slot_3 = {"save", 0, 0 };
    static const QUMethod slot_4 = {"reset", 0, 0 };
    static const QUMethod slot_5 = {"check_permission", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"del", 1, param_slot_6 };
    static const QUMethod slot_7 = {"help", 0, 0 };
    static const QUMethod slot_8 = {"quit", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_firstname", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_lastname", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_address", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_city", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_state", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_zip", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_phone", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_email", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"select_name", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_18 = {"check_fields", 1, param_slot_18 };
    static const QUMethod slot_19 = {"clear", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "GUI()", &slot_0, QMetaData::Private },
	{ "checkname()", &slot_1, QMetaData::Private },
	{ "update()", &slot_2, QMetaData::Private },
	{ "save()", &slot_3, QMetaData::Private },
	{ "reset()", &slot_4, QMetaData::Private },
	{ "check_permission()", &slot_5, QMetaData::Private },
	{ "del(bool)", &slot_6, QMetaData::Private },
	{ "help()", &slot_7, QMetaData::Private },
	{ "quit()", &slot_8, QMetaData::Private },
	{ "update_firstname(const QString&)", &slot_9, QMetaData::Private },
	{ "update_lastname(const QString&)", &slot_10, QMetaData::Private },
	{ "update_address(const QString&)", &slot_11, QMetaData::Private },
	{ "update_city(const QString&)", &slot_12, QMetaData::Private },
	{ "update_state(const QString&)", &slot_13, QMetaData::Private },
	{ "update_zip(const QString&)", &slot_14, QMetaData::Private },
	{ "update_phone(const QString&)", &slot_15, QMetaData::Private },
	{ "update_email(const QString&)", &slot_16, QMetaData::Private },
	{ "select_name(int)", &slot_17, QMetaData::Private },
	{ "check_fields()", &slot_18, QMetaData::Private },
	{ "clear()", &slot_19, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ "Display", &static_QUType_QString, 0, QUParameter::In },
	{ "InvID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"valueChanged", 2, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "valueChanged(QString,int)", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_TblInvestigator", parentObject,
	slot_tbl, 20,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_TblInvestigator.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_TblInvestigator::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_TblInvestigator" ) )
	return this;
    return US_DB::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL valueChanged
void US_DB_TblInvestigator::valueChanged( QString t0, int t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_QString.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    activate_signal( clist, o );
}

bool US_DB_TblInvestigator::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: GUI(); break;
    case 1: checkname(); break;
    case 2: update(); break;
    case 3: save(); break;
    case 4: reset(); break;
    case 5: check_permission(); break;
    case 6: del((bool)static_QUType_bool.get(_o+1)); break;
    case 7: help(); break;
    case 8: quit(); break;
    case 9: update_firstname((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_lastname((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_address((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_city((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_state((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_zip((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: update_phone((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_email((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: select_name((int)static_QUType_int.get(_o+1)); break;
    case 18: static_QUType_bool.set(_o,check_fields()); break;
    case 19: clear(); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_TblInvestigator::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: valueChanged((QString)static_QUType_QString.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    default:
	return US_DB::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_DB_TblInvestigator::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_DB_TblInvestigator::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
