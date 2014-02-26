/****************************************************************************
** US_Hydrodyn_Cluster_Bfnb_Nsa meta object code from reading C++ file 'us_hydrodyn_cluster_bfnb_nsa.h'
**
** Created: Tue Feb 25 10:28:31 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_bfnb_nsa.h"
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

const char *US_Hydrodyn_Cluster_Bfnb_Nsa::className() const
{
    return "US_Hydrodyn_Cluster_Bfnb_Nsa";
}

QMetaObject *US_Hydrodyn_Cluster_Bfnb_Nsa::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Bfnb_Nsa( "US_Hydrodyn_Cluster_Bfnb_Nsa", &US_Hydrodyn_Cluster_Bfnb_Nsa::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Bfnb_Nsa::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Bfnb_Nsa", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Bfnb_Nsa::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Bfnb_Nsa", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Bfnb_Nsa::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_nsaspheres", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_nsaess", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_nsaexcl", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_nsaga", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_nsasga", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_nsaiterations", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_nsaepsilon", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_nsagsm", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_nsascale", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_nsagenerations", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_nsapopulation", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_nsaelitism", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_nsamutate", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_nsacrossover", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_nsaearlytermination", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_sgpdistancequantum", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_sgpdistancemin", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_sgpdistancemax", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_sgpradiusmin", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_sgpradiusmax", 1, param_slot_19 };
    static const QUMethod slot_20 = {"save", 0, 0 };
    static const QUMethod slot_21 = {"load", 0, 0 };
    static const QUMethod slot_22 = {"help", 0, 0 };
    static const QUMethod slot_23 = {"cancel", 0, 0 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_24 = {"closeEvent", 1, param_slot_24 };
    static const QMetaData slot_tbl[] = {
	{ "update_nsaspheres(const QString&)", &slot_0, QMetaData::Private },
	{ "update_nsaess(const QString&)", &slot_1, QMetaData::Private },
	{ "update_nsaexcl(const QString&)", &slot_2, QMetaData::Private },
	{ "update_nsaga(const QString&)", &slot_3, QMetaData::Private },
	{ "update_nsasga(const QString&)", &slot_4, QMetaData::Private },
	{ "update_nsaiterations(const QString&)", &slot_5, QMetaData::Private },
	{ "update_nsaepsilon(const QString&)", &slot_6, QMetaData::Private },
	{ "update_nsagsm(const QString&)", &slot_7, QMetaData::Private },
	{ "update_nsascale(const QString&)", &slot_8, QMetaData::Private },
	{ "update_nsagenerations(const QString&)", &slot_9, QMetaData::Private },
	{ "update_nsapopulation(const QString&)", &slot_10, QMetaData::Private },
	{ "update_nsaelitism(const QString&)", &slot_11, QMetaData::Private },
	{ "update_nsamutate(const QString&)", &slot_12, QMetaData::Private },
	{ "update_nsacrossover(const QString&)", &slot_13, QMetaData::Private },
	{ "update_nsaearlytermination(const QString&)", &slot_14, QMetaData::Private },
	{ "update_sgpdistancequantum(const QString&)", &slot_15, QMetaData::Private },
	{ "update_sgpdistancemin(const QString&)", &slot_16, QMetaData::Private },
	{ "update_sgpdistancemax(const QString&)", &slot_17, QMetaData::Private },
	{ "update_sgpradiusmin(const QString&)", &slot_18, QMetaData::Private },
	{ "update_sgpradiusmax(const QString&)", &slot_19, QMetaData::Private },
	{ "save()", &slot_20, QMetaData::Private },
	{ "load()", &slot_21, QMetaData::Private },
	{ "help()", &slot_22, QMetaData::Private },
	{ "cancel()", &slot_23, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_24, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Bfnb_Nsa", parentObject,
	slot_tbl, 25,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Bfnb_Nsa.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Bfnb_Nsa::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Bfnb_Nsa" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Bfnb_Nsa::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_nsaspheres((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_nsaess((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_nsaexcl((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_nsaga((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_nsasga((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_nsaiterations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_nsaepsilon((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_nsagsm((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_nsascale((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_nsagenerations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_nsapopulation((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_nsaelitism((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_nsamutate((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_nsacrossover((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_nsaearlytermination((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: update_sgpdistancequantum((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_sgpdistancemin((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: update_sgpdistancemax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_sgpradiusmin((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: update_sgpradiusmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: save(); break;
    case 21: load(); break;
    case 22: help(); break;
    case 23: cancel(); break;
    case 24: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Bfnb_Nsa::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Bfnb_Nsa::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Bfnb_Nsa::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
