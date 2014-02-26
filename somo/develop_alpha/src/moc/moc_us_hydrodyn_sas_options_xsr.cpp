/****************************************************************************
** US_Hydrodyn_SasOptionsXsr meta object code from reading C++ file 'us_hydrodyn_sas_options_xsr.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_sas_options_xsr.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_SasOptionsXsr::className() const
{
    return "US_Hydrodyn_SasOptionsXsr";
}

QMetaObject *US_Hydrodyn_SasOptionsXsr::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SasOptionsXsr( "US_Hydrodyn_SasOptionsXsr", &US_Hydrodyn_SasOptionsXsr::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SasOptionsXsr::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsXsr", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SasOptionsXsr::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SasOptionsXsr", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SasOptionsXsr::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_xsr_symmop", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_xsr_nx", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_xsr_ny", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_xsr_griddistance", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_xsr_ncomponents", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_xsr_compactness_weight", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_xsr_looseness_weight", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_xsr_temperature", 1, param_slot_7 };
    static const QUMethod slot_8 = {"cancel", 0, 0 };
    static const QUMethod slot_9 = {"help", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_10 = {"closeEvent", 1, param_slot_10 };
    static const QMetaData slot_tbl[] = {
	{ "update_xsr_symmop(const QString&)", &slot_0, QMetaData::Private },
	{ "update_xsr_nx(const QString&)", &slot_1, QMetaData::Private },
	{ "update_xsr_ny(const QString&)", &slot_2, QMetaData::Private },
	{ "update_xsr_griddistance(const QString&)", &slot_3, QMetaData::Private },
	{ "update_xsr_ncomponents(const QString&)", &slot_4, QMetaData::Private },
	{ "update_xsr_compactness_weight(const QString&)", &slot_5, QMetaData::Private },
	{ "update_xsr_looseness_weight(const QString&)", &slot_6, QMetaData::Private },
	{ "update_xsr_temperature(const QString&)", &slot_7, QMetaData::Private },
	{ "cancel()", &slot_8, QMetaData::Private },
	{ "help()", &slot_9, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_10, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SasOptionsXsr", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SasOptionsXsr.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SasOptionsXsr::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SasOptionsXsr" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SasOptionsXsr::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_xsr_symmop((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_xsr_nx((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_xsr_ny((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_xsr_griddistance((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_xsr_ncomponents((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_xsr_compactness_weight((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_xsr_looseness_weight((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_xsr_temperature((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: cancel(); break;
    case 9: help(); break;
    case 10: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SasOptionsXsr::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SasOptionsXsr::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SasOptionsXsr::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
