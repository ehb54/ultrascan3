/****************************************************************************
** US_Beowulf meta object code from reading C++ file 'us_beowulf.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_beowulf.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Beowulf::className() const
{
    return "US_Beowulf";
}

QMetaObject *US_Beowulf::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Beowulf( "US_Beowulf", &US_Beowulf::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Beowulf::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Beowulf", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Beowulf::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Beowulf", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Beowulf::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"add_host", 0, 0 };
    static const QUMethod slot_1 = {"mergeAll", 0, 0 };
    static const QUMethod slot_2 = {"killAll", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"edit_host", 1, param_slot_3 };
    static const QUMethod slot_4 = {"delete_host", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"show_host", 1, param_slot_5 };
    static const QUMethod slot_6 = {"quit", 0, 0 };
    static const QUMethod slot_7 = {"help", 0, 0 };
    static const QUMethod slot_8 = {"save", 0, 0 };
    static const QUMethod slot_9 = {"run", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_10 = {"get_localhost", 1, param_slot_10 };
    static const QUMethod slot_11 = {"change_ssh", 0, 0 };
    static const QUMethod slot_12 = {"change_rsh", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"change_display", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_14 = {"closeEvent", 1, param_slot_14 };
    static const QMetaData slot_tbl[] = {
	{ "add_host()", &slot_0, QMetaData::Public },
	{ "mergeAll()", &slot_1, QMetaData::Public },
	{ "killAll()", &slot_2, QMetaData::Public },
	{ "edit_host(const QString&)", &slot_3, QMetaData::Public },
	{ "delete_host()", &slot_4, QMetaData::Public },
	{ "show_host(int)", &slot_5, QMetaData::Public },
	{ "quit()", &slot_6, QMetaData::Public },
	{ "help()", &slot_7, QMetaData::Public },
	{ "save()", &slot_8, QMetaData::Public },
	{ "run()", &slot_9, QMetaData::Public },
	{ "get_localhost()", &slot_10, QMetaData::Public },
	{ "change_ssh()", &slot_11, QMetaData::Public },
	{ "change_rsh()", &slot_12, QMetaData::Public },
	{ "change_display(const QString&)", &slot_13, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_14, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Beowulf", parentObject,
	slot_tbl, 15,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Beowulf.setMetaObject( metaObj );
    return metaObj;
}

void* US_Beowulf::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Beowulf" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Beowulf::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: add_host(); break;
    case 1: mergeAll(); break;
    case 2: killAll(); break;
    case 3: edit_host((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: delete_host(); break;
    case 5: show_host((int)static_QUType_int.get(_o+1)); break;
    case 6: quit(); break;
    case 7: help(); break;
    case 8: save(); break;
    case 9: run(); break;
    case 10: static_QUType_bool.set(_o,get_localhost()); break;
    case 11: change_ssh(); break;
    case 12: change_rsh(); break;
    case 13: change_display((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Beowulf::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Beowulf::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Beowulf::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
