/****************************************************************************
** US_Hydrodyn_Cluster_Results meta object code from reading C++ file 'us_hydrodyn_cluster_results.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_results.h"
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

const char *US_Hydrodyn_Cluster_Results::className() const
{
    return "US_Hydrodyn_Cluster_Results";
}

QMetaObject *US_Hydrodyn_Cluster_Results::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Results( "US_Hydrodyn_Cluster_Results", &US_Hydrodyn_Cluster_Results::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Results::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Results", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Results::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Results", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Results::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"update_enables", 0, 0 };
    static const QUMethod slot_2 = {"select_all", 0, 0 };
    static const QUMethod slot_3 = {"purge", 0, 0 };
    static const QUMethod slot_4 = {"load_results", 0, 0 };
    static const QUMethod slot_5 = {"clear_display", 0, 0 };
    static const QUMethod slot_6 = {"update_font", 0, 0 };
    static const QUMethod slot_7 = {"save", 0, 0 };
    static const QUMethod slot_8 = {"cancel", 0, 0 };
    static const QUMethod slot_9 = {"help", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_10 = {"closeEvent", 1, param_slot_10 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_enables()", &slot_1, QMetaData::Private },
	{ "select_all()", &slot_2, QMetaData::Private },
	{ "purge()", &slot_3, QMetaData::Private },
	{ "load_results()", &slot_4, QMetaData::Private },
	{ "clear_display()", &slot_5, QMetaData::Private },
	{ "update_font()", &slot_6, QMetaData::Private },
	{ "save()", &slot_7, QMetaData::Private },
	{ "cancel()", &slot_8, QMetaData::Private },
	{ "help()", &slot_9, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_10, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Results", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Results.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Results::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Results" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Results::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_enables(); break;
    case 2: select_all(); break;
    case 3: purge(); break;
    case 4: load_results(); break;
    case 5: clear_display(); break;
    case 6: update_font(); break;
    case 7: save(); break;
    case 8: cancel(); break;
    case 9: help(); break;
    case 10: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Results::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Results::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Results::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
