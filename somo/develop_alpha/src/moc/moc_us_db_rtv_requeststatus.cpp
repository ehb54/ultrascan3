/****************************************************************************
** US_DB_RequestStatus meta object code from reading C++ file 'us_db_rtv_requeststatus.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_rtv_requeststatus.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_DB_RequestStatus::className() const
{
    return "US_DB_RequestStatus";
}

QMetaObject *US_DB_RequestStatus::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_DB_RequestStatus( "US_DB_RequestStatus", &US_DB_RequestStatus::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_DB_RequestStatus::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RequestStatus", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_DB_RequestStatus::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_DB_RequestStatus", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_DB_RequestStatus::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUMethod slot_0 = {"select_cb1", 0, 0 };
    static const QUMethod slot_1 = {"select_cb2", 0, 0 };
    static const QUMethod slot_2 = {"select_cb3", 0, 0 };
    static const QUMethod slot_3 = {"select_cb4", 0, 0 };
    static const QUMethod slot_4 = {"select_cb5", 0, 0 };
    static const QUMethod slot_5 = {"select_cb6", 0, 0 };
    static const QUMethod slot_6 = {"select_cb7", 0, 0 };
    static const QUMethod slot_7 = {"show_status", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"select_status", 1, param_slot_8 };
    static const QUMethod slot_9 = {"update_status", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_display", 1, param_slot_10 };
    static const QUMethod slot_11 = {"reset", 0, 0 };
    static const QUMethod slot_12 = {"help", 0, 0 };
    static const QUMethod slot_13 = {"quit", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ "e", &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_14 = {"resizeEvent", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_15 = {"closeEvent", 1, param_slot_15 };
    static const QMetaData slot_tbl[] = {
	{ "select_cb1()", &slot_0, QMetaData::Protected },
	{ "select_cb2()", &slot_1, QMetaData::Protected },
	{ "select_cb3()", &slot_2, QMetaData::Protected },
	{ "select_cb4()", &slot_3, QMetaData::Protected },
	{ "select_cb5()", &slot_4, QMetaData::Protected },
	{ "select_cb6()", &slot_5, QMetaData::Protected },
	{ "select_cb7()", &slot_6, QMetaData::Protected },
	{ "show_status()", &slot_7, QMetaData::Protected },
	{ "select_status(int)", &slot_8, QMetaData::Protected },
	{ "update_status()", &slot_9, QMetaData::Protected },
	{ "update_display(QString)", &slot_10, QMetaData::Protected },
	{ "reset()", &slot_11, QMetaData::Protected },
	{ "help()", &slot_12, QMetaData::Protected },
	{ "quit()", &slot_13, QMetaData::Protected },
	{ "resizeEvent(QResizeEvent*)", &slot_14, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_15, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_DB_RequestStatus", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_DB_RequestStatus.setMetaObject( metaObj );
    return metaObj;
}

void* US_DB_RequestStatus::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_DB_RequestStatus" ) )
	return this;
    return US_DB::qt_cast( clname );
}

bool US_DB_RequestStatus::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: select_cb1(); break;
    case 1: select_cb2(); break;
    case 2: select_cb3(); break;
    case 3: select_cb4(); break;
    case 4: select_cb5(); break;
    case 5: select_cb6(); break;
    case 6: select_cb7(); break;
    case 7: show_status(); break;
    case 8: select_status((int)static_QUType_int.get(_o+1)); break;
    case 9: update_status(); break;
    case 10: update_display((QString)static_QUType_QString.get(_o+1)); break;
    case 11: reset(); break;
    case 12: help(); break;
    case 13: quit(); break;
    case 14: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    case 15: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_DB_RequestStatus::qt_emit( int _id, QUObject* _o )
{
    return US_DB::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_DB_RequestStatus::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_DB_RequestStatus::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
