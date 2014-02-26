/****************************************************************************
** US_ExtinctionLegend meta object code from reading C++ file 'us_extinction.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_extinction.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3Frame>
#include <QMouseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_ExtinctionLegend::className() const
{
    return "US_ExtinctionLegend";
}

QMetaObject *US_ExtinctionLegend::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ExtinctionLegend( "US_ExtinctionLegend", &US_ExtinctionLegend::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ExtinctionLegend::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ExtinctionLegend", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ExtinctionLegend::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ExtinctionLegend", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ExtinctionLegend::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"set_pathlength", 0, 0 };
    static const QUMethod slot_2 = {"set_normalized", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Public },
	{ "set_pathlength()", &slot_1, QMetaData::Public },
	{ "set_normalized()", &slot_2, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"scaleChanged", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "scaleChanged()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ExtinctionLegend", parentObject,
	slot_tbl, 3,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ExtinctionLegend.setMetaObject( metaObj );
    return metaObj;
}

void* US_ExtinctionLegend::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ExtinctionLegend" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL scaleChanged
void US_ExtinctionLegend::scaleChanged()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_ExtinctionLegend::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: set_pathlength(); break;
    case 2: set_normalized(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ExtinctionLegend::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: scaleChanged(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_ExtinctionLegend::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_ExtinctionLegend::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_Extinction::className() const
{
    return "US_Extinction";
}

QMetaObject *US_Extinction::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Extinction( "US_Extinction", &US_Extinction::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Extinction::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Extinction", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Extinction::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Extinction", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Extinction::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"closeEvent", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_2 = {"mouseMoved", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_3 = {"mousePressed", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_4 = {"mouseReleased", 1, param_slot_4 };
    static const QUMethod slot_5 = {"selectScans", 0, 0 };
    static const QUMethod slot_6 = {"reset", 0, 0 };
    static const QUMethod slot_7 = {"plot", 0, 0 };
    static const QUMethod slot_8 = {"fit", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut }
    };
    static const QUMethod slot_9 = {"initialize", 1, param_slot_9 };
    static const QUMethod slot_10 = {"selectPeptide", 0, 0 };
    static const QUMethod slot_11 = {"save", 0, 0 };
    static const QUMethod slot_12 = {"print", 0, 0 };
    static const QUMethod slot_13 = {"view", 0, 0 };
    static const QUMethod slot_14 = {"help", 0, 0 };
    static const QUMethod slot_15 = {"cancel", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"loadScan", 2, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_odCutoff", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_lambdaCutoff", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_pathlength", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_extinction", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_extinction_coefficient", 1, param_slot_21 };
    static const QUMethod slot_22 = {"calc_extinction", 0, 0 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_project", 1, param_slot_23 };
    static const QUMethod slot_24 = {"update_scale", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_order", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_wavelength", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_27 = {"createHtmlDir", 1, param_slot_27 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_1, QMetaData::Protected },
	{ "mouseMoved(const QMouseEvent&)", &slot_2, QMetaData::Protected },
	{ "mousePressed(const QMouseEvent&)", &slot_3, QMetaData::Protected },
	{ "mouseReleased(const QMouseEvent&)", &slot_4, QMetaData::Protected },
	{ "selectScans()", &slot_5, QMetaData::Public },
	{ "reset()", &slot_6, QMetaData::Public },
	{ "plot()", &slot_7, QMetaData::Public },
	{ "fit()", &slot_8, QMetaData::Public },
	{ "initialize(double**)", &slot_9, QMetaData::Public },
	{ "selectPeptide()", &slot_10, QMetaData::Public },
	{ "save()", &slot_11, QMetaData::Public },
	{ "print()", &slot_12, QMetaData::Public },
	{ "view()", &slot_13, QMetaData::Public },
	{ "help()", &slot_14, QMetaData::Public },
	{ "cancel()", &slot_15, QMetaData::Public },
	{ "loadScan(const QString&)", &slot_16, QMetaData::Public },
	{ "update_odCutoff(const QString&)", &slot_17, QMetaData::Public },
	{ "update_lambdaCutoff(const QString&)", &slot_18, QMetaData::Public },
	{ "update_pathlength(const QString&)", &slot_19, QMetaData::Public },
	{ "update_extinction(const QString&)", &slot_20, QMetaData::Public },
	{ "update_extinction_coefficient(float)", &slot_21, QMetaData::Public },
	{ "calc_extinction()", &slot_22, QMetaData::Public },
	{ "update_project(const QString&)", &slot_23, QMetaData::Public },
	{ "update_scale()", &slot_24, QMetaData::Public },
	{ "update_order(double)", &slot_25, QMetaData::Public },
	{ "update_wavelength(double)", &slot_26, QMetaData::Public },
	{ "createHtmlDir()", &slot_27, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod signal_0 = {"fitUpdated", 3, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"projectChanged", 1, param_signal_1 };
    static const QUMethod signal_2 = {"extinctionClosed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "fitUpdated(double*,double*,unsigned int)", &signal_0, QMetaData::Public },
	{ "projectChanged(const QString&)", &signal_1, QMetaData::Public },
	{ "extinctionClosed()", &signal_2, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Extinction", parentObject,
	slot_tbl, 28,
	signal_tbl, 3,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Extinction.setMetaObject( metaObj );
    return metaObj;
}

void* US_Extinction::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Extinction" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL fitUpdated
void US_Extinction::fitUpdated( double* t0, double* t1, unsigned int t2 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[4];
    static_QUType_varptr.set(o+1,t0);
    static_QUType_varptr.set(o+2,t1);
    static_QUType_ptr.set(o+3,&t2);
    activate_signal( clist, o );
}

// SIGNAL projectChanged
void US_Extinction::projectChanged( const QString& t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 1, t0 );
}

// SIGNAL extinctionClosed
void US_Extinction::extinctionClosed()
{
    activate_signal( staticMetaObject()->signalOffset() + 2 );
}

bool US_Extinction::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: mouseMoved((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 3: mousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 4: mouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 5: selectScans(); break;
    case 6: reset(); break;
    case 7: plot(); break;
    case 8: fit(); break;
    case 9: initialize((double**)static_QUType_ptr.get(_o+1)); break;
    case 10: selectPeptide(); break;
    case 11: save(); break;
    case 12: print(); break;
    case 13: view(); break;
    case 14: help(); break;
    case 15: cancel(); break;
    case 16: static_QUType_bool.set(_o,loadScan((const QString&)static_QUType_QString.get(_o+1))); break;
    case 17: update_odCutoff((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_lambdaCutoff((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: update_pathlength((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: update_extinction((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: update_extinction_coefficient((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 22: calc_extinction(); break;
    case 23: update_project((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: update_scale(); break;
    case 25: update_order((double)static_QUType_double.get(_o+1)); break;
    case 26: update_wavelength((double)static_QUType_double.get(_o+1)); break;
    case 27: static_QUType_bool.set(_o,createHtmlDir()); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Extinction::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: fitUpdated((double*)static_QUType_varptr.get(_o+1),(double*)static_QUType_varptr.get(_o+2),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+3)))); break;
    case 1: projectChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: extinctionClosed(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Extinction::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Extinction::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
