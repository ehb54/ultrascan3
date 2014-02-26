/****************************************************************************
** US_Hydrodyn_Cluster_Bfnb meta object code from reading C++ file 'us_hydrodyn_cluster_bfnb.h'
**
** Created: Tue Feb 25 10:28:31 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_bfnb.h"
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

const char *US_Hydrodyn_Cluster_Bfnb::className() const
{
    return "US_Hydrodyn_Cluster_Bfnb";
}

QMetaObject *US_Hydrodyn_Cluster_Bfnb::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Bfnb( "US_Hydrodyn_Cluster_Bfnb", &US_Hydrodyn_Cluster_Bfnb::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Bfnb::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Bfnb", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Bfnb::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Bfnb", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Bfnb::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"hide_main_label", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_pmtypes", 1, param_slot_1 };
    static const QUMethod slot_2 = {"set_pmincrementally", 0, 0 };
    static const QUMethod slot_3 = {"set_pmallcombinations", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_pmrayleighdrho", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_pmbufferedensity", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_pmoutname", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_pmgridsize", 1, param_slot_7 };
    static const QUMethod slot_8 = {"set_pmapproxmaxdimension", 0, 0 };
    static const QUMethod slot_9 = {"hide_q_label", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_pmminq", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_pmmaxq", 1, param_slot_11 };
    static const QUMethod slot_12 = {"set_pmlogqbin", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_pmqpoints", 1, param_slot_13 };
    static const QUMethod slot_14 = {"hide_supp_label", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_pmharmonics", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_pmseed", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_pmmemory", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_pmbestfinestconversion", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_pmbestcoarseconversion", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_pmbestconversiondivisor", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_pmbestrefinementrangepct", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_pmmaxdimension", 1, param_slot_22 };
    static const QUMethod slot_23 = {"hide_ga_label", 0, 0 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_pmgapopulation", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_pmgagenerations", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_pmgamutate", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_pmgasamutate", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_pmgacrossover", 1, param_slot_28 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"update_pmgaelitism", 1, param_slot_29 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_30 = {"update_pmgaearlytermination", 1, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_31 = {"update_pmgapointsmax", 1, param_slot_31 };
    static const QUMethod slot_32 = {"hide_misc_label", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_33 = {"update_pmbestdeltastart", 1, param_slot_33 };
    static const QUParameter param_slot_34[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_34 = {"update_pmbestdeltadivisor", 1, param_slot_34 };
    static const QUParameter param_slot_35[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_35 = {"update_pmbestdeltamin", 1, param_slot_35 };
    static const QUMethod slot_36 = {"set_pmcsv", 0, 0 };
    static const QUParameter param_slot_37[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_37 = {"update_pmdebug", 1, param_slot_37 };
    static const QUMethod slot_38 = {"save", 0, 0 };
    static const QUMethod slot_39 = {"load", 0, 0 };
    static const QUMethod slot_40 = {"help", 0, 0 };
    static const QUMethod slot_41 = {"cancel", 0, 0 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_42 = {"closeEvent", 1, param_slot_42 };
    static const QMetaData slot_tbl[] = {
	{ "hide_main_label()", &slot_0, QMetaData::Private },
	{ "update_pmtypes(const QString&)", &slot_1, QMetaData::Private },
	{ "set_pmincrementally()", &slot_2, QMetaData::Private },
	{ "set_pmallcombinations()", &slot_3, QMetaData::Private },
	{ "update_pmrayleighdrho(const QString&)", &slot_4, QMetaData::Private },
	{ "update_pmbufferedensity(const QString&)", &slot_5, QMetaData::Private },
	{ "update_pmoutname(const QString&)", &slot_6, QMetaData::Private },
	{ "update_pmgridsize(const QString&)", &slot_7, QMetaData::Private },
	{ "set_pmapproxmaxdimension()", &slot_8, QMetaData::Private },
	{ "hide_q_label()", &slot_9, QMetaData::Private },
	{ "update_pmminq(const QString&)", &slot_10, QMetaData::Private },
	{ "update_pmmaxq(const QString&)", &slot_11, QMetaData::Private },
	{ "set_pmlogqbin()", &slot_12, QMetaData::Private },
	{ "update_pmqpoints(const QString&)", &slot_13, QMetaData::Private },
	{ "hide_supp_label()", &slot_14, QMetaData::Private },
	{ "update_pmharmonics(const QString&)", &slot_15, QMetaData::Private },
	{ "update_pmseed(const QString&)", &slot_16, QMetaData::Private },
	{ "update_pmmemory(const QString&)", &slot_17, QMetaData::Private },
	{ "update_pmbestfinestconversion(const QString&)", &slot_18, QMetaData::Private },
	{ "update_pmbestcoarseconversion(const QString&)", &slot_19, QMetaData::Private },
	{ "update_pmbestconversiondivisor(const QString&)", &slot_20, QMetaData::Private },
	{ "update_pmbestrefinementrangepct(const QString&)", &slot_21, QMetaData::Private },
	{ "update_pmmaxdimension(const QString&)", &slot_22, QMetaData::Private },
	{ "hide_ga_label()", &slot_23, QMetaData::Private },
	{ "update_pmgapopulation(const QString&)", &slot_24, QMetaData::Private },
	{ "update_pmgagenerations(const QString&)", &slot_25, QMetaData::Private },
	{ "update_pmgamutate(const QString&)", &slot_26, QMetaData::Private },
	{ "update_pmgasamutate(const QString&)", &slot_27, QMetaData::Private },
	{ "update_pmgacrossover(const QString&)", &slot_28, QMetaData::Private },
	{ "update_pmgaelitism(const QString&)", &slot_29, QMetaData::Private },
	{ "update_pmgaearlytermination(const QString&)", &slot_30, QMetaData::Private },
	{ "update_pmgapointsmax(const QString&)", &slot_31, QMetaData::Private },
	{ "hide_misc_label()", &slot_32, QMetaData::Private },
	{ "update_pmbestdeltastart(const QString&)", &slot_33, QMetaData::Private },
	{ "update_pmbestdeltadivisor(const QString&)", &slot_34, QMetaData::Private },
	{ "update_pmbestdeltamin(const QString&)", &slot_35, QMetaData::Private },
	{ "set_pmcsv()", &slot_36, QMetaData::Private },
	{ "update_pmdebug(const QString&)", &slot_37, QMetaData::Private },
	{ "save()", &slot_38, QMetaData::Private },
	{ "load()", &slot_39, QMetaData::Private },
	{ "help()", &slot_40, QMetaData::Private },
	{ "cancel()", &slot_41, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_42, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Bfnb", parentObject,
	slot_tbl, 43,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Bfnb.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Bfnb::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Bfnb" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Bfnb::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: hide_main_label(); break;
    case 1: update_pmtypes((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: set_pmincrementally(); break;
    case 3: set_pmallcombinations(); break;
    case 4: update_pmrayleighdrho((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_pmbufferedensity((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_pmoutname((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_pmgridsize((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: set_pmapproxmaxdimension(); break;
    case 9: hide_q_label(); break;
    case 10: update_pmminq((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_pmmaxq((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: set_pmlogqbin(); break;
    case 13: update_pmqpoints((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: hide_supp_label(); break;
    case 15: update_pmharmonics((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_pmseed((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: update_pmmemory((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_pmbestfinestconversion((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: update_pmbestcoarseconversion((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: update_pmbestconversiondivisor((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: update_pmbestrefinementrangepct((const QString&)static_QUType_QString.get(_o+1)); break;
    case 22: update_pmmaxdimension((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: hide_ga_label(); break;
    case 24: update_pmgapopulation((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: update_pmgagenerations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: update_pmgamutate((const QString&)static_QUType_QString.get(_o+1)); break;
    case 27: update_pmgasamutate((const QString&)static_QUType_QString.get(_o+1)); break;
    case 28: update_pmgacrossover((const QString&)static_QUType_QString.get(_o+1)); break;
    case 29: update_pmgaelitism((const QString&)static_QUType_QString.get(_o+1)); break;
    case 30: update_pmgaearlytermination((const QString&)static_QUType_QString.get(_o+1)); break;
    case 31: update_pmgapointsmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 32: hide_misc_label(); break;
    case 33: update_pmbestdeltastart((const QString&)static_QUType_QString.get(_o+1)); break;
    case 34: update_pmbestdeltadivisor((const QString&)static_QUType_QString.get(_o+1)); break;
    case 35: update_pmbestdeltamin((const QString&)static_QUType_QString.get(_o+1)); break;
    case 36: set_pmcsv(); break;
    case 37: update_pmdebug((const QString&)static_QUType_QString.get(_o+1)); break;
    case 38: save(); break;
    case 39: load(); break;
    case 40: help(); break;
    case 41: cancel(); break;
    case 42: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Bfnb::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Bfnb::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Bfnb::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
