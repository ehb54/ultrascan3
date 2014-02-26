/****************************************************************************
** US_Hydrodyn_Cluster_Dammin meta object code from reading C++ file 'us_hydrodyn_cluster_dammin.h'
**
** Created: Tue Feb 25 10:28:32 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_dammin.h"
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

const char *US_Hydrodyn_Cluster_Dammin::className() const
{
    return "US_Hydrodyn_Cluster_Dammin";
}

QMetaObject *US_Hydrodyn_Cluster_Dammin::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Dammin( "US_Hydrodyn_Cluster_Dammin", &US_Hydrodyn_Cluster_Dammin::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Dammin::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Dammin", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Dammin::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Dammin", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Dammin::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_dammingnomfile", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_damminmode", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_dammindescription", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_damminangularunits", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_damminfitcurvelimit", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_damminknotstofit", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_damminconstantsubtractionprocedure", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_damminmaxharmonics", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_dammininitialdamtype", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_damminsymmetry", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_damminspherediameter", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_damminpackingradius", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_damminradius1stcoordinationsphere", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_damminloosenesspenaltyweight", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_dammindisconnectivitypenaltyweight", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_damminperipheralpenaltyweight", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_damminfixingthersholdsLosandRf", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_damminrandomizestructure", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_damminweight", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_dammininitialscalefactor", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_damminfixscalefactor", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_dammininitialannealingtemperature", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_damminannealingschedulefactor", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_damminnumberofindependentatomstomodify", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_damminmaxnumberiterationseachT", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_damminmaxnumbersuccesseseachT", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_damminminnumbersuccessestocontinue", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_damminmaxnumberannealingsteps", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_damminexpectedshape", 1, param_slot_28 };
    static const QUMethod slot_29 = {"save", 0, 0 };
    static const QUMethod slot_30 = {"load", 0, 0 };
    static const QUMethod slot_31 = {"help", 0, 0 };
    static const QUMethod slot_32 = {"cancel", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_33 = {"closeEvent", 1, param_slot_33 };
    static const QMetaData slot_tbl[] = {
	{ "update_dammingnomfile(const QString&)", &slot_0, QMetaData::Private },
	{ "update_damminmode(const QString&)", &slot_1, QMetaData::Private },
	{ "update_dammindescription(const QString&)", &slot_2, QMetaData::Private },
	{ "update_damminangularunits(const QString&)", &slot_3, QMetaData::Private },
	{ "update_damminfitcurvelimit(const QString&)", &slot_4, QMetaData::Private },
	{ "update_damminknotstofit(const QString&)", &slot_5, QMetaData::Private },
	{ "update_damminconstantsubtractionprocedure(const QString&)", &slot_6, QMetaData::Private },
	{ "update_damminmaxharmonics(const QString&)", &slot_7, QMetaData::Private },
	{ "update_dammininitialdamtype(const QString&)", &slot_8, QMetaData::Private },
	{ "update_damminsymmetry(const QString&)", &slot_9, QMetaData::Private },
	{ "update_damminspherediameter(const QString&)", &slot_10, QMetaData::Private },
	{ "update_damminpackingradius(const QString&)", &slot_11, QMetaData::Private },
	{ "update_damminradius1stcoordinationsphere(const QString&)", &slot_12, QMetaData::Private },
	{ "update_damminloosenesspenaltyweight(const QString&)", &slot_13, QMetaData::Private },
	{ "update_dammindisconnectivitypenaltyweight(const QString&)", &slot_14, QMetaData::Private },
	{ "update_damminperipheralpenaltyweight(const QString&)", &slot_15, QMetaData::Private },
	{ "update_damminfixingthersholdsLosandRf(const QString&)", &slot_16, QMetaData::Private },
	{ "update_damminrandomizestructure(const QString&)", &slot_17, QMetaData::Private },
	{ "update_damminweight(const QString&)", &slot_18, QMetaData::Private },
	{ "update_dammininitialscalefactor(const QString&)", &slot_19, QMetaData::Private },
	{ "update_damminfixscalefactor(const QString&)", &slot_20, QMetaData::Private },
	{ "update_dammininitialannealingtemperature(const QString&)", &slot_21, QMetaData::Private },
	{ "update_damminannealingschedulefactor(const QString&)", &slot_22, QMetaData::Private },
	{ "update_damminnumberofindependentatomstomodify(const QString&)", &slot_23, QMetaData::Private },
	{ "update_damminmaxnumberiterationseachT(const QString&)", &slot_24, QMetaData::Private },
	{ "update_damminmaxnumbersuccesseseachT(const QString&)", &slot_25, QMetaData::Private },
	{ "update_damminminnumbersuccessestocontinue(const QString&)", &slot_26, QMetaData::Private },
	{ "update_damminmaxnumberannealingsteps(const QString&)", &slot_27, QMetaData::Private },
	{ "update_damminexpectedshape(const QString&)", &slot_28, QMetaData::Private },
	{ "save()", &slot_29, QMetaData::Private },
	{ "load()", &slot_30, QMetaData::Private },
	{ "help()", &slot_31, QMetaData::Private },
	{ "cancel()", &slot_32, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_33, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Dammin", parentObject,
	slot_tbl, 34,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Dammin.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Dammin::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Dammin" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Dammin::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_dammingnomfile((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_damminmode((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_dammindescription((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_damminangularunits((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_damminfitcurvelimit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_damminknotstofit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_damminconstantsubtractionprocedure((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_damminmaxharmonics((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_dammininitialdamtype((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_damminsymmetry((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_damminspherediameter((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_damminpackingradius((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_damminradius1stcoordinationsphere((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_damminloosenesspenaltyweight((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_dammindisconnectivitypenaltyweight((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: update_damminperipheralpenaltyweight((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_damminfixingthersholdsLosandRf((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: update_damminrandomizestructure((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_damminweight((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: update_dammininitialscalefactor((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: update_damminfixscalefactor((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: update_dammininitialannealingtemperature((const QString&)static_QUType_QString.get(_o+1)); break;
    case 22: update_damminannealingschedulefactor((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: update_damminnumberofindependentatomstomodify((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: update_damminmaxnumberiterationseachT((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: update_damminmaxnumbersuccesseseachT((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: update_damminminnumbersuccessestocontinue((const QString&)static_QUType_QString.get(_o+1)); break;
    case 27: update_damminmaxnumberannealingsteps((const QString&)static_QUType_QString.get(_o+1)); break;
    case 28: update_damminexpectedshape((const QString&)static_QUType_QString.get(_o+1)); break;
    case 29: save(); break;
    case 30: load(); break;
    case 31: help(); break;
    case 32: cancel(); break;
    case 33: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Dammin::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Dammin::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Dammin::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
