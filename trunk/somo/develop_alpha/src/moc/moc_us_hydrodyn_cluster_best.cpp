/****************************************************************************
** US_Hydrodyn_Cluster_Best meta object code from reading C++ file 'us_hydrodyn_cluster_best.h'
**
** Created: Tue Feb 25 10:35:27 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_best.h"
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

const char *US_Hydrodyn_Cluster_Best::className() const
{
    return "US_Hydrodyn_Cluster_Best";
}

QMetaObject *US_Hydrodyn_Cluster_Best::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Best( "US_Hydrodyn_Cluster_Best", &US_Hydrodyn_Cluster_Best::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Best::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Best", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Best::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Best", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Best::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_bestmsrprober", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_bestmsrfinenessangle", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_bestmsrmaxtriangles", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_bestrcoalnmin", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_bestrcoalnmax", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_bestrcoaln", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_bestbestmw", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_bestbestwatr", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_bestexpand", 1, param_slot_8 };
    static const QUMethod slot_9 = {"set_bestbestvc", 0, 0 };
    static const QUMethod slot_10 = {"hide_opt_label", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_bestmsrcoalescer", 1, param_slot_11 };
    static const QUMethod slot_12 = {"set_bestbestv", 0, 0 };
    static const QUMethod slot_13 = {"set_bestbestp", 0, 0 };
    static const QUMethod slot_14 = {"set_bestbestna", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_bestmsrradiifile", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_bestmsrpatternfile", 1, param_slot_16 };
    static const QUMethod slot_17 = {"save", 0, 0 };
    static const QUMethod slot_18 = {"load", 0, 0 };
    static const QUMethod slot_19 = {"help", 0, 0 };
    static const QUMethod slot_20 = {"cancel", 0, 0 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_21 = {"closeEvent", 1, param_slot_21 };
    static const QMetaData slot_tbl[] = {
	{ "update_bestmsrprober(const QString&)", &slot_0, QMetaData::Private },
	{ "update_bestmsrfinenessangle(const QString&)", &slot_1, QMetaData::Private },
	{ "update_bestmsrmaxtriangles(const QString&)", &slot_2, QMetaData::Private },
	{ "update_bestrcoalnmin(const QString&)", &slot_3, QMetaData::Private },
	{ "update_bestrcoalnmax(const QString&)", &slot_4, QMetaData::Private },
	{ "update_bestrcoaln(const QString&)", &slot_5, QMetaData::Private },
	{ "update_bestbestmw(const QString&)", &slot_6, QMetaData::Private },
	{ "update_bestbestwatr(const QString&)", &slot_7, QMetaData::Private },
	{ "update_bestexpand(const QString&)", &slot_8, QMetaData::Private },
	{ "set_bestbestvc()", &slot_9, QMetaData::Private },
	{ "hide_opt_label()", &slot_10, QMetaData::Private },
	{ "update_bestmsrcoalescer(const QString&)", &slot_11, QMetaData::Private },
	{ "set_bestbestv()", &slot_12, QMetaData::Private },
	{ "set_bestbestp()", &slot_13, QMetaData::Private },
	{ "set_bestbestna()", &slot_14, QMetaData::Private },
	{ "update_bestmsrradiifile(const QString&)", &slot_15, QMetaData::Private },
	{ "update_bestmsrpatternfile(const QString&)", &slot_16, QMetaData::Private },
	{ "save()", &slot_17, QMetaData::Private },
	{ "load()", &slot_18, QMetaData::Private },
	{ "help()", &slot_19, QMetaData::Private },
	{ "cancel()", &slot_20, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_21, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Best", parentObject,
	slot_tbl, 22,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Best.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Best::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Best" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Best::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_bestmsrprober((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_bestmsrfinenessangle((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_bestmsrmaxtriangles((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_bestrcoalnmin((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_bestrcoalnmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_bestrcoaln((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_bestbestmw((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_bestbestwatr((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_bestexpand((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: set_bestbestvc(); break;
    case 10: hide_opt_label(); break;
    case 11: update_bestmsrcoalescer((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: set_bestbestv(); break;
    case 13: set_bestbestp(); break;
    case 14: set_bestbestna(); break;
    case 15: update_bestmsrradiifile((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_bestmsrpatternfile((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: save(); break;
    case 18: load(); break;
    case 19: help(); break;
    case 20: cancel(); break;
    case 21: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Best::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Best::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Best::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
