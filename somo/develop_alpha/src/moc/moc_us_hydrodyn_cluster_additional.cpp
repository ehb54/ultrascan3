/****************************************************************************
** US_Hydrodyn_Cluster_Additional meta object code from reading C++ file 'us_hydrodyn_cluster_additional.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_additional.h"
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

const char *US_Hydrodyn_Cluster_Additional::className() const
{
    return "US_Hydrodyn_Cluster_Additional";
}

QMetaObject *US_Hydrodyn_Cluster_Additional::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Additional( "US_Hydrodyn_Cluster_Additional", &US_Hydrodyn_Cluster_Additional::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Additional::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Additional", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Additional::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Additional", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Additional::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_bfnb", 0, 0 };
    static const QUMethod slot_2 = {"bfnb", 0, 0 };
    static const QUMethod slot_3 = {"set_bfnb_nsa", 0, 0 };
    static const QUMethod slot_4 = {"bfnb_nsa", 0, 0 };
    static const QUMethod slot_5 = {"set_best", 0, 0 };
    static const QUMethod slot_6 = {"best", 0, 0 };
    static const QUMethod slot_7 = {"set_oned", 0, 0 };
    static const QUMethod slot_8 = {"oned", 0, 0 };
    static const QUMethod slot_9 = {"set_csa", 0, 0 };
    static const QUMethod slot_10 = {"csa", 0, 0 };
    static const QUMethod slot_11 = {"set_dammin", 0, 0 };
    static const QUMethod slot_12 = {"dammin", 0, 0 };
    static const QUMethod slot_13 = {"set_dammif", 0, 0 };
    static const QUMethod slot_14 = {"dammif", 0, 0 };
    static const QUMethod slot_15 = {"set_gasbor", 0, 0 };
    static const QUMethod slot_16 = {"gasbor", 0, 0 };
    static const QUMethod slot_17 = {"ok", 0, 0 };
    static const QUMethod slot_18 = {"cancel", 0, 0 };
    static const QUMethod slot_19 = {"help", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_20 = {"closeEvent", 1, param_slot_20 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_bfnb()", &slot_1, QMetaData::Private },
	{ "bfnb()", &slot_2, QMetaData::Private },
	{ "set_bfnb_nsa()", &slot_3, QMetaData::Private },
	{ "bfnb_nsa()", &slot_4, QMetaData::Private },
	{ "set_best()", &slot_5, QMetaData::Private },
	{ "best()", &slot_6, QMetaData::Private },
	{ "set_oned()", &slot_7, QMetaData::Private },
	{ "oned()", &slot_8, QMetaData::Private },
	{ "set_csa()", &slot_9, QMetaData::Private },
	{ "csa()", &slot_10, QMetaData::Private },
	{ "set_dammin()", &slot_11, QMetaData::Private },
	{ "dammin()", &slot_12, QMetaData::Private },
	{ "set_dammif()", &slot_13, QMetaData::Private },
	{ "dammif()", &slot_14, QMetaData::Private },
	{ "set_gasbor()", &slot_15, QMetaData::Private },
	{ "gasbor()", &slot_16, QMetaData::Private },
	{ "ok()", &slot_17, QMetaData::Private },
	{ "cancel()", &slot_18, QMetaData::Private },
	{ "help()", &slot_19, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_20, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Additional", parentObject,
	slot_tbl, 21,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Additional.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Additional::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Additional" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Additional::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_bfnb(); break;
    case 2: bfnb(); break;
    case 3: set_bfnb_nsa(); break;
    case 4: bfnb_nsa(); break;
    case 5: set_best(); break;
    case 6: best(); break;
    case 7: set_oned(); break;
    case 8: oned(); break;
    case 9: set_csa(); break;
    case 10: csa(); break;
    case 11: set_dammin(); break;
    case 12: dammin(); break;
    case 13: set_dammif(); break;
    case 14: dammif(); break;
    case 15: set_gasbor(); break;
    case 16: gasbor(); break;
    case 17: ok(); break;
    case 18: cancel(); break;
    case 19: help(); break;
    case 20: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Additional::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Additional::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Additional::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
