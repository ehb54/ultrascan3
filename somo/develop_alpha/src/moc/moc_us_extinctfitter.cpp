/****************************************************************************
** US_ExtinctionFitter meta object code from reading C++ file 'us_extinctfitter.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_extinctfitter.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_ExtinctionFitter::className() const
{
    return "US_ExtinctionFitter";
}

QMetaObject *US_ExtinctionFitter::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ExtinctionFitter( "US_ExtinctionFitter", &US_ExtinctionFitter::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ExtinctionFitter::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ExtinctionFitter", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ExtinctionFitter::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ExtinctionFitter", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ExtinctionFitter::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_Minimize::staticMetaObject();
    static const QUMethod slot_0 = {"cleanup", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_1 = {"calc_jacobian", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_2 = {"fit_init", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_3 = {"calc_model", 2, param_slot_3 };
    static const QUMethod slot_4 = {"view_report", 0, 0 };
    static const QUMethod slot_5 = {"write_report", 0, 0 };
    static const QUMethod slot_6 = {"plot_overlays", 0, 0 };
    static const QUMethod slot_7 = {"plot_residuals", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ "scan", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"updateRange", 1, param_slot_8 };
    static const QUMethod slot_9 = {"endFit", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_10 = {"createHtmlDir", 1, param_slot_10 };
    static const QUMethod slot_11 = {"saveFit", 0, 0 };
    static const QUMethod slot_12 = {"startFit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "cleanup()", &slot_0, QMetaData::Private },
	{ "calc_jacobian()", &slot_1, QMetaData::Private },
	{ "fit_init()", &slot_2, QMetaData::Public },
	{ "calc_model(double*)", &slot_3, QMetaData::Public },
	{ "view_report()", &slot_4, QMetaData::Public },
	{ "write_report()", &slot_5, QMetaData::Public },
	{ "plot_overlays()", &slot_6, QMetaData::Public },
	{ "plot_residuals()", &slot_7, QMetaData::Public },
	{ "updateRange(double)", &slot_8, QMetaData::Public },
	{ "endFit()", &slot_9, QMetaData::Public },
	{ "createHtmlDir()", &slot_10, QMetaData::Public },
	{ "saveFit()", &slot_11, QMetaData::Public },
	{ "startFit()", &slot_12, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"dataSaved", 2, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "dataSaved(const QString&,const int)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ExtinctionFitter", parentObject,
	slot_tbl, 13,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ExtinctionFitter.setMetaObject( metaObj );
    return metaObj;
}

void* US_ExtinctionFitter::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ExtinctionFitter" ) )
	return this;
    return US_Minimize::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL dataSaved
void US_ExtinctionFitter::dataSaved( const QString& t0, const int t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_QString.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    activate_signal( clist, o );
}

bool US_ExtinctionFitter::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cleanup(); break;
    case 1: static_QUType_int.set(_o,calc_jacobian()); break;
    case 2: static_QUType_bool.set(_o,fit_init()); break;
    case 3: static_QUType_int.set(_o,calc_model((double*)static_QUType_varptr.get(_o+1))); break;
    case 4: view_report(); break;
    case 5: write_report(); break;
    case 6: plot_overlays(); break;
    case 7: plot_residuals(); break;
    case 8: updateRange((double)static_QUType_double.get(_o+1)); break;
    case 9: endFit(); break;
    case 10: static_QUType_bool.set(_o,createHtmlDir()); break;
    case 11: saveFit(); break;
    case 12: startFit(); break;
    default:
	return US_Minimize::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ExtinctionFitter::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: dataSaved((const QString&)static_QUType_QString.get(_o+1),(const int)static_QUType_int.get(_o+2)); break;
    default:
	return US_Minimize::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_ExtinctionFitter::qt_property( int id, int f, QVariant* v)
{
    return US_Minimize::qt_property( id, f, v);
}

bool US_ExtinctionFitter::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
