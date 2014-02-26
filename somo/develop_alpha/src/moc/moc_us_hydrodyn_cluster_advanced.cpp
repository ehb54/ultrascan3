/****************************************************************************
** US_Hydrodyn_Cluster_Advanced meta object code from reading C++ file 'us_hydrodyn_cluster_advanced.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_advanced.h"
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

const char *US_Hydrodyn_Cluster_Advanced::className() const
{
    return "US_Hydrodyn_Cluster_Advanced";
}

QMetaObject *US_Hydrodyn_Cluster_Advanced::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Advanced( "US_Hydrodyn_Cluster_Advanced", &US_Hydrodyn_Cluster_Advanced::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Advanced::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Advanced", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Advanced::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Advanced", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Advanced::staticMetaObject()
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
    static const QUMethod slot_2 = {"clear_display", 0, 0 };
    static const QUMethod slot_3 = {"update_font", 0, 0 };
    static const QUMethod slot_4 = {"save", 0, 0 };
    static const QUMethod slot_5 = {"ok", 0, 0 };
    static const QUMethod slot_6 = {"cancel", 0, 0 };
    static const QUMethod slot_7 = {"help", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_8 = {"closeEvent", 1, param_slot_8 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "table_value(int,int)", &slot_1, QMetaData::Private },
	{ "clear_display()", &slot_2, QMetaData::Private },
	{ "update_font()", &slot_3, QMetaData::Private },
	{ "save()", &slot_4, QMetaData::Private },
	{ "ok()", &slot_5, QMetaData::Private },
	{ "cancel()", &slot_6, QMetaData::Private },
	{ "help()", &slot_7, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_8, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Advanced", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Advanced.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Advanced::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Advanced" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Advanced::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: table_value((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 2: clear_display(); break;
    case 3: update_font(); break;
    case 4: save(); break;
    case 5: ok(); break;
    case 6: cancel(); break;
    case 7: help(); break;
    case 8: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Advanced::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Advanced::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Advanced::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
