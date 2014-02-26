/****************************************************************************
** US_Hydrodyn_Cluster meta object code from reading C++ file 'us_hydrodyn_cluster.h'
**
** Created: Sat Dec 21 19:45:16 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster.h"
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

const char *US_Hydrodyn_Cluster::className() const
{
    return "US_Hydrodyn_Cluster";
}

QMetaObject *US_Hydrodyn_Cluster::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster( "US_Hydrodyn_Cluster", &US_Hydrodyn_Cluster::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"add_target", 0, 0 };
    static const QUMethod slot_2 = {"clear_target", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_output_name", 1, param_slot_3 };
    static const QUMethod slot_4 = {"create_pkg", 0, 0 };
    static const QUMethod slot_5 = {"submit_pkg", 0, 0 };
    static const QUMethod slot_6 = {"check_status", 0, 0 };
    static const QUMethod slot_7 = {"load_results", 0, 0 };
    static const QUMethod slot_8 = {"for_mpi", 0, 0 };
    static const QUMethod slot_9 = {"split_grid", 0, 0 };
    static const QUMethod slot_10 = {"dmd", 0, 0 };
    static const QUMethod slot_11 = {"additional", 0, 0 };
    static const QUMethod slot_12 = {"advanced", 0, 0 };
    static const QUMethod slot_13 = {"clear_display", 0, 0 };
    static const QUMethod slot_14 = {"update_font", 0, 0 };
    static const QUMethod slot_15 = {"save", 0, 0 };
    static const QUMethod slot_16 = {"cancel", 0, 0 };
    static const QUMethod slot_17 = {"config", 0, 0 };
    static const QUMethod slot_18 = {"help", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_19 = {"closeEvent", 1, param_slot_19 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "add_target()", &slot_1, QMetaData::Private },
	{ "clear_target()", &slot_2, QMetaData::Private },
	{ "update_output_name(const QString&)", &slot_3, QMetaData::Private },
	{ "create_pkg()", &slot_4, QMetaData::Private },
	{ "submit_pkg()", &slot_5, QMetaData::Private },
	{ "check_status()", &slot_6, QMetaData::Private },
	{ "load_results()", &slot_7, QMetaData::Private },
	{ "for_mpi()", &slot_8, QMetaData::Private },
	{ "split_grid()", &slot_9, QMetaData::Private },
	{ "dmd()", &slot_10, QMetaData::Private },
	{ "additional()", &slot_11, QMetaData::Private },
	{ "advanced()", &slot_12, QMetaData::Private },
	{ "clear_display()", &slot_13, QMetaData::Private },
	{ "update_font()", &slot_14, QMetaData::Private },
	{ "save()", &slot_15, QMetaData::Private },
	{ "cancel()", &slot_16, QMetaData::Private },
	{ "config()", &slot_17, QMetaData::Private },
	{ "help()", &slot_18, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_19, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster", parentObject,
	slot_tbl, 20,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: add_target(); break;
    case 2: clear_target(); break;
    case 3: update_output_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: create_pkg(); break;
    case 5: submit_pkg(); break;
    case 6: check_status(); break;
    case 7: load_results(); break;
    case 8: for_mpi(); break;
    case 9: split_grid(); break;
    case 10: dmd(); break;
    case 11: additional(); break;
    case 12: advanced(); break;
    case 13: clear_display(); break;
    case 14: update_font(); break;
    case 15: save(); break;
    case 16: cancel(); break;
    case 17: config(); break;
    case 18: help(); break;
    case 19: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
