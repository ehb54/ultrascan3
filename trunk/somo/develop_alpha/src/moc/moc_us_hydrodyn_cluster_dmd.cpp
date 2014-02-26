/****************************************************************************
** US_Hydrodyn_Cluster_Dmd meta object code from reading C++ file 'us_hydrodyn_cluster_dmd.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_dmd.h"
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

const char *US_Hydrodyn_Cluster_Dmd::className() const
{
    return "US_Hydrodyn_Cluster_Dmd";
}

QMetaObject *US_Hydrodyn_Cluster_Dmd::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Dmd( "US_Hydrodyn_Cluster_Dmd", &US_Hydrodyn_Cluster_Dmd::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Dmd::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Dmd", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Dmd::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Dmd", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Dmd::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"table_value", 2, param_slot_1 };
    static const QUMethod slot_2 = {"update_enables", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"row_header_released", 1, param_slot_3 };
    static const QUMethod slot_4 = {"select_all", 0, 0 };
    static const QUMethod slot_5 = {"copy", 0, 0 };
    static const QUMethod slot_6 = {"paste", 0, 0 };
    static const QUMethod slot_7 = {"paste_all", 0, 0 };
    static const QUMethod slot_8 = {"dup", 0, 0 };
    static const QUMethod slot_9 = {"delete_rows", 0, 0 };
    static const QUMethod slot_10 = {"load", 0, 0 };
    static const QUMethod slot_11 = {"reset", 0, 0 };
    static const QUMethod slot_12 = {"save_csv", 0, 0 };
    static const QUMethod slot_13 = {"clear_display", 0, 0 };
    static const QUMethod slot_14 = {"update_font", 0, 0 };
    static const QUMethod slot_15 = {"save", 0, 0 };
    static const QUMethod slot_16 = {"ok", 0, 0 };
    static const QUMethod slot_17 = {"cancel", 0, 0 };
    static const QUMethod slot_18 = {"help", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_19 = {"closeEvent", 1, param_slot_19 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "table_value(int,int)", &slot_1, QMetaData::Private },
	{ "update_enables()", &slot_2, QMetaData::Private },
	{ "row_header_released(int)", &slot_3, QMetaData::Private },
	{ "select_all()", &slot_4, QMetaData::Private },
	{ "copy()", &slot_5, QMetaData::Private },
	{ "paste()", &slot_6, QMetaData::Private },
	{ "paste_all()", &slot_7, QMetaData::Private },
	{ "dup()", &slot_8, QMetaData::Private },
	{ "delete_rows()", &slot_9, QMetaData::Private },
	{ "load()", &slot_10, QMetaData::Private },
	{ "reset()", &slot_11, QMetaData::Private },
	{ "save_csv()", &slot_12, QMetaData::Private },
	{ "clear_display()", &slot_13, QMetaData::Private },
	{ "update_font()", &slot_14, QMetaData::Private },
	{ "save()", &slot_15, QMetaData::Private },
	{ "ok()", &slot_16, QMetaData::Private },
	{ "cancel()", &slot_17, QMetaData::Private },
	{ "help()", &slot_18, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_19, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Dmd", parentObject,
	slot_tbl, 20,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Dmd.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Dmd::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Dmd" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Dmd::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: table_value((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 2: update_enables(); break;
    case 3: row_header_released((int)static_QUType_int.get(_o+1)); break;
    case 4: select_all(); break;
    case 5: copy(); break;
    case 6: paste(); break;
    case 7: paste_all(); break;
    case 8: dup(); break;
    case 9: delete_rows(); break;
    case 10: load(); break;
    case 11: reset(); break;
    case 12: save_csv(); break;
    case 13: clear_display(); break;
    case 14: update_font(); break;
    case 15: save(); break;
    case 16: ok(); break;
    case 17: cancel(); break;
    case 18: help(); break;
    case 19: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Dmd::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Dmd::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Dmd::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
