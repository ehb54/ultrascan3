/****************************************************************************
** US_MeniscusDialog meta object code from reading C++ file 'us_meniscus_dlg.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_meniscus_dlg.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_MeniscusDialog::className() const
{
    return "US_MeniscusDialog";
}

QMetaObject *US_MeniscusDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_MeniscusDialog( "US_MeniscusDialog", &US_MeniscusDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_MeniscusDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_MeniscusDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_MeniscusDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_MeniscusDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_MeniscusDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"ok", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_meniscus1", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_meniscus2", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_meniscus3", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_meniscus4", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_meniscus5", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_meniscus6", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_meniscus7", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_meniscus8", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_10 = {"check_meniscus", 1, param_slot_10 };
    static const QUMethod slot_11 = {"setup_GUI", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Private },
	{ "ok()", &slot_1, QMetaData::Private },
	{ "update_meniscus1(const QString&)", &slot_2, QMetaData::Private },
	{ "update_meniscus2(const QString&)", &slot_3, QMetaData::Private },
	{ "update_meniscus3(const QString&)", &slot_4, QMetaData::Private },
	{ "update_meniscus4(const QString&)", &slot_5, QMetaData::Private },
	{ "update_meniscus5(const QString&)", &slot_6, QMetaData::Private },
	{ "update_meniscus6(const QString&)", &slot_7, QMetaData::Private },
	{ "update_meniscus7(const QString&)", &slot_8, QMetaData::Private },
	{ "update_meniscus8(const QString&)", &slot_9, QMetaData::Private },
	{ "check_meniscus()", &slot_10, QMetaData::Private },
	{ "setup_GUI()", &slot_11, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_MeniscusDialog", parentObject,
	slot_tbl, 12,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_MeniscusDialog.setMetaObject( metaObj );
    return metaObj;
}

void* US_MeniscusDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_MeniscusDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_MeniscusDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: ok(); break;
    case 2: update_meniscus1((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_meniscus2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_meniscus3((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_meniscus4((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_meniscus5((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_meniscus6((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_meniscus7((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_meniscus8((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: static_QUType_bool.set(_o,check_meniscus()); break;
    case 11: setup_GUI(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_MeniscusDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_MeniscusDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_MeniscusDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
