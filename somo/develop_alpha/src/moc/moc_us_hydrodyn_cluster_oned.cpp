/****************************************************************************
** US_Hydrodyn_Cluster_Oned meta object code from reading C++ file 'us_hydrodyn_cluster_oned.h'
**
** Created: Tue Feb 25 10:28:32 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_cluster_oned.h"
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

const char *US_Hydrodyn_Cluster_Oned::className() const
{
    return "US_Hydrodyn_Cluster_Oned";
}

QMetaObject *US_Hydrodyn_Cluster_Oned::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Cluster_Oned( "US_Hydrodyn_Cluster_Oned", &US_Hydrodyn_Cluster_Oned::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Cluster_Oned::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Oned", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Cluster_Oned::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Cluster_Oned", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Cluster_Oned::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_1drotationfile", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_1drotationsuserand", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_1dlambda", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_1ddetectordistance", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_1ddetectorwidth", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_1ddetectorpixelswidth", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_1drho0", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_1ddeltar", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_1dproberadius", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_1dthreshold", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_1dsamplerotations", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_1daxisrotations", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_1dintermediatesaves", 1, param_slot_12 };
    static const QUMethod slot_13 = {"save", 0, 0 };
    static const QUMethod slot_14 = {"load", 0, 0 };
    static const QUMethod slot_15 = {"help", 0, 0 };
    static const QUMethod slot_16 = {"cancel", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_17 = {"closeEvent", 1, param_slot_17 };
    static const QMetaData slot_tbl[] = {
	{ "update_1drotationfile(const QString&)", &slot_0, QMetaData::Private },
	{ "update_1drotationsuserand(const QString&)", &slot_1, QMetaData::Private },
	{ "update_1dlambda(const QString&)", &slot_2, QMetaData::Private },
	{ "update_1ddetectordistance(const QString&)", &slot_3, QMetaData::Private },
	{ "update_1ddetectorwidth(const QString&)", &slot_4, QMetaData::Private },
	{ "update_1ddetectorpixelswidth(const QString&)", &slot_5, QMetaData::Private },
	{ "update_1drho0(const QString&)", &slot_6, QMetaData::Private },
	{ "update_1ddeltar(const QString&)", &slot_7, QMetaData::Private },
	{ "update_1dproberadius(const QString&)", &slot_8, QMetaData::Private },
	{ "update_1dthreshold(const QString&)", &slot_9, QMetaData::Private },
	{ "update_1dsamplerotations(const QString&)", &slot_10, QMetaData::Private },
	{ "update_1daxisrotations(const QString&)", &slot_11, QMetaData::Private },
	{ "update_1dintermediatesaves(const QString&)", &slot_12, QMetaData::Private },
	{ "save()", &slot_13, QMetaData::Private },
	{ "load()", &slot_14, QMetaData::Private },
	{ "help()", &slot_15, QMetaData::Private },
	{ "cancel()", &slot_16, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_17, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Cluster_Oned", parentObject,
	slot_tbl, 18,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Cluster_Oned.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Cluster_Oned::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Cluster_Oned" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Cluster_Oned::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_1drotationfile((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_1drotationsuserand((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_1dlambda((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_1ddetectordistance((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_1ddetectorwidth((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_1ddetectorpixelswidth((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_1drho0((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_1ddeltar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_1dproberadius((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_1dthreshold((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_1dsamplerotations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_1daxisrotations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_1dintermediatesaves((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: save(); break;
    case 14: load(); break;
    case 15: help(); break;
    case 16: cancel(); break;
    case 17: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Cluster_Oned::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Cluster_Oned::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Cluster_Oned::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
