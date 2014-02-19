/****************************************************************************
** US_Archive meta object code from reading C++ file 'us_archive.h'
**
** Created: Wed Dec 4 19:31:40 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_archive.h"
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

const char *US_Archive::className() const
{
    return "US_Archive";
}

QMetaObject *US_Archive::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Archive( "US_Archive", &US_Archive::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Archive::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Archive", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Archive::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Archive", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Archive::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_Widgets::staticMetaObject();
    static const QUMethod slot_0 = {"view", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"viewtargz", 1, param_slot_1 };
    static const QUMethod slot_2 = {"extract_archive", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"extract", 3, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"extract", 4, param_slot_4 };
    static const QUMethod slot_5 = {"quit", 0, 0 };
    static const QUMethod slot_6 = {"help", 0, 0 };
    static const QUMethod slot_7 = {"set_ultrascan", 0, 0 };
    static const QUMethod slot_8 = {"set_delete", 0, 0 };
    static const QUMethod slot_9 = {"set_reports", 0, 0 };
    static const QUMethod slot_10 = {"select_velocdata", 0, 0 };
    static const QUMethod slot_11 = {"select_equildata", 0, 0 };
    static const QUMethod slot_12 = {"select_eqilproj", 0, 0 };
    static const QUMethod slot_13 = {"select_montecarlo", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"select_create_archive", 1, param_slot_14 };
    static const QUMethod slot_15 = {"create_archive", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_varptr, "\x04", QUParameter::In }
    };
    static const QUMethod slot_16 = {"create_archive", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"move_file", 2, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_varptr, "\x04", QUParameter::InOut }
    };
    static const QUMethod slot_18 = {"create_tar", 4, param_slot_18 };
    static const QUMethod slot_19 = {"clean_temp_dir", 0, 0 };
    static const QUMethod slot_20 = {"disable_buttons", 0, 0 };
    static const QUMethod slot_21 = {"enable_buttons", 0, 0 };
    static const QUMethod slot_22 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_23 = {"closeEvent", 1, param_slot_23 };
    static const QMetaData slot_tbl[] = {
	{ "view()", &slot_0, QMetaData::Private },
	{ "viewtargz(const QString&)", &slot_1, QMetaData::Private },
	{ "extract_archive()", &slot_2, QMetaData::Private },
	{ "extract(const QString&,const QString&,const QString&)", &slot_3, QMetaData::Private },
	{ "extract(const QString&,const QString&,const QString&,const bool)", &slot_4, QMetaData::Private },
	{ "quit()", &slot_5, QMetaData::Private },
	{ "help()", &slot_6, QMetaData::Private },
	{ "set_ultrascan()", &slot_7, QMetaData::Private },
	{ "set_delete()", &slot_8, QMetaData::Private },
	{ "set_reports()", &slot_9, QMetaData::Private },
	{ "select_velocdata()", &slot_10, QMetaData::Private },
	{ "select_equildata()", &slot_11, QMetaData::Private },
	{ "select_eqilproj()", &slot_12, QMetaData::Private },
	{ "select_montecarlo()", &slot_13, QMetaData::Private },
	{ "select_create_archive(const int)", &slot_14, QMetaData::Private },
	{ "create_archive()", &slot_15, QMetaData::Private },
	{ "create_archive(QStringList*)", &slot_16, QMetaData::Private },
	{ "move_file(const QString&,const QString&)", &slot_17, QMetaData::Private },
	{ "create_tar(const QString&,const QString&,QStringList&)", &slot_18, QMetaData::Private },
	{ "clean_temp_dir()", &slot_19, QMetaData::Private },
	{ "disable_buttons()", &slot_20, QMetaData::Private },
	{ "enable_buttons()", &slot_21, QMetaData::Private },
	{ "setup_GUI()", &slot_22, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_23, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Archive", parentObject,
	slot_tbl, 24,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Archive.setMetaObject( metaObj );
    return metaObj;
}

void* US_Archive::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Archive" ) )
	return this;
    return US_Widgets::qt_cast( clname );
}

bool US_Archive::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: view(); break;
    case 1: viewtargz((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: extract_archive(); break;
    case 3: extract((const QString&)static_QUType_QString.get(_o+1),(const QString&)static_QUType_QString.get(_o+2),(const QString&)static_QUType_QString.get(_o+3)); break;
    case 4: extract((const QString&)static_QUType_QString.get(_o+1),(const QString&)static_QUType_QString.get(_o+2),(const QString&)static_QUType_QString.get(_o+3),(const bool)static_QUType_bool.get(_o+4)); break;
    case 5: quit(); break;
    case 6: help(); break;
    case 7: set_ultrascan(); break;
    case 8: set_delete(); break;
    case 9: set_reports(); break;
    case 10: select_velocdata(); break;
    case 11: select_equildata(); break;
    case 12: select_eqilproj(); break;
    case 13: select_montecarlo(); break;
    case 14: select_create_archive((const int)static_QUType_int.get(_o+1)); break;
    case 15: create_archive(); break;
    case 16: create_archive((QStringList*)static_QUType_varptr.get(_o+1)); break;
    case 17: move_file((const QString&)static_QUType_QString.get(_o+1),(const QString&)static_QUType_QString.get(_o+2)); break;
    case 18: static_QUType_bool.set(_o,create_tar((const QString&)static_QUType_QString.get(_o+1),(const QString&)static_QUType_QString.get(_o+2),(QStringList&)*((QStringList*)static_QUType_ptr.get(_o+3)))); break;
    case 19: clean_temp_dir(); break;
    case 20: disable_buttons(); break;
    case 21: enable_buttons(); break;
    case 22: setup_GUI(); break;
    case 23: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return US_Widgets::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Archive::qt_emit( int _id, QUObject* _o )
{
    return US_Widgets::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Archive::qt_property( int id, int f, QVariant* v)
{
    return US_Widgets::qt_property( id, f, v);
}

bool US_Archive::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
