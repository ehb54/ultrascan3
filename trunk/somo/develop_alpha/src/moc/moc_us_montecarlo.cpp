/****************************************************************************
** US_MonteCarlo meta object code from reading C++ file 'us_montecarlo.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_montecarlo.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <QResizeEvent>
#include <Q3Frame>
#include <QMouseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_MonteCarlo::className() const
{
    return "US_MonteCarlo";
}

QMetaObject *US_MonteCarlo::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_MonteCarlo( "US_MonteCarlo", &US_MonteCarlo::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_MonteCarlo::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_MonteCarlo", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_MonteCarlo::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_MonteCarlo", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_MonteCarlo::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"update_iteration", 0, 0 };
    static const QUMethod slot_1 = {"beowulf", 0, 0 };
    static const QUMethod slot_2 = {"set_new", 0, 0 };
    static const QUMethod slot_3 = {"set_append", 0, 0 };
    static const QUMethod slot_4 = {"select_file", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_file", 1, param_slot_5 };
    static const QUMethod slot_6 = {"set_gaussian", 0, 0 };
    static const QUMethod slot_7 = {"set_bootstrap", 0, 0 };
    static const QUMethod slot_8 = {"set_mixed", 0, 0 };
    static const QUMethod slot_9 = {"set_random", 0, 0 };
    static const QUMethod slot_10 = {"set_originalFit", 0, 0 };
    static const QUMethod slot_11 = {"set_lastFit", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_noiseLevel", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_ignoreVariance", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_percent_bootstrap", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_percent_gaussian", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"select_seed", 1, param_slot_16 };
    static const QUMethod slot_17 = {"update_parameter", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_rule", 1, param_slot_18 };
    static const QUMethod slot_19 = {"show_parameter", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"show_parameter", 1, param_slot_20 };
    static const QUMethod slot_21 = {"statistics", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_total_iterations", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_bins", 1, param_slot_23 };
    static const QUMethod slot_24 = {"print", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_25 = {"createHtmlDir", 1, param_slot_25 };
    static const QUMethod slot_26 = {"start", 0, 0 };
    static const QUMethod slot_27 = {"stop", 0, 0 };
    static const QUMethod slot_28 = {"quit", 0, 0 };
    static const QUMethod slot_29 = {"file_error", 0, 0 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_30 = {"plotMousePressed", 1, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_31 = {"plotMouseReleased", 1, param_slot_31 };
    static const QUMethod slot_32 = {"help", 0, 0 };
    static const QUMethod slot_33 = {"save", 0, 0 };
    static const QUParameter param_slot_34[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_34 = {"closeEvent", 1, param_slot_34 };
    static const QUParameter param_slot_35[] = {
	{ "e", &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_35 = {"resizeEvent", 1, param_slot_35 };
    static const QMetaData slot_tbl[] = {
	{ "update_iteration()", &slot_0, QMetaData::Public },
	{ "beowulf()", &slot_1, QMetaData::Public },
	{ "set_new()", &slot_2, QMetaData::Protected },
	{ "set_append()", &slot_3, QMetaData::Protected },
	{ "select_file()", &slot_4, QMetaData::Protected },
	{ "update_file(const QString&)", &slot_5, QMetaData::Protected },
	{ "set_gaussian()", &slot_6, QMetaData::Protected },
	{ "set_bootstrap()", &slot_7, QMetaData::Protected },
	{ "set_mixed()", &slot_8, QMetaData::Protected },
	{ "set_random()", &slot_9, QMetaData::Protected },
	{ "set_originalFit()", &slot_10, QMetaData::Protected },
	{ "set_lastFit()", &slot_11, QMetaData::Protected },
	{ "update_noiseLevel(const QString&)", &slot_12, QMetaData::Protected },
	{ "update_ignoreVariance(const QString&)", &slot_13, QMetaData::Protected },
	{ "update_percent_bootstrap(double)", &slot_14, QMetaData::Protected },
	{ "update_percent_gaussian(double)", &slot_15, QMetaData::Protected },
	{ "select_seed(const QString&)", &slot_16, QMetaData::Protected },
	{ "update_parameter()", &slot_17, QMetaData::Protected },
	{ "update_rule(int)", &slot_18, QMetaData::Protected },
	{ "show_parameter()", &slot_19, QMetaData::Protected },
	{ "show_parameter(int)", &slot_20, QMetaData::Protected },
	{ "statistics()", &slot_21, QMetaData::Protected },
	{ "update_total_iterations(const QString&)", &slot_22, QMetaData::Protected },
	{ "update_bins(const QString&)", &slot_23, QMetaData::Protected },
	{ "print()", &slot_24, QMetaData::Protected },
	{ "createHtmlDir()", &slot_25, QMetaData::Protected },
	{ "start()", &slot_26, QMetaData::Protected },
	{ "stop()", &slot_27, QMetaData::Protected },
	{ "quit()", &slot_28, QMetaData::Protected },
	{ "file_error()", &slot_29, QMetaData::Protected },
	{ "plotMousePressed(const QMouseEvent&)", &slot_30, QMetaData::Protected },
	{ "plotMouseReleased(const QMouseEvent&)", &slot_31, QMetaData::Protected },
	{ "help()", &slot_32, QMetaData::Protected },
	{ "save()", &slot_33, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_34, QMetaData::Protected },
	{ "resizeEvent(QResizeEvent*)", &slot_35, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"iterate", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "iterate()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_MonteCarlo", parentObject,
	slot_tbl, 36,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_MonteCarlo.setMetaObject( metaObj );
    return metaObj;
}

void* US_MonteCarlo::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_MonteCarlo" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL iterate
void US_MonteCarlo::iterate()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_MonteCarlo::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_iteration(); break;
    case 1: beowulf(); break;
    case 2: set_new(); break;
    case 3: set_append(); break;
    case 4: select_file(); break;
    case 5: update_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: set_gaussian(); break;
    case 7: set_bootstrap(); break;
    case 8: set_mixed(); break;
    case 9: set_random(); break;
    case 10: set_originalFit(); break;
    case 11: set_lastFit(); break;
    case 12: update_noiseLevel((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_ignoreVariance((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_percent_bootstrap((double)static_QUType_double.get(_o+1)); break;
    case 15: update_percent_gaussian((double)static_QUType_double.get(_o+1)); break;
    case 16: select_seed((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: update_parameter(); break;
    case 18: update_rule((int)static_QUType_int.get(_o+1)); break;
    case 19: show_parameter(); break;
    case 20: show_parameter((int)static_QUType_int.get(_o+1)); break;
    case 21: statistics(); break;
    case 22: update_total_iterations((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: update_bins((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: print(); break;
    case 25: static_QUType_bool.set(_o,createHtmlDir()); break;
    case 26: start(); break;
    case 27: stop(); break;
    case 28: quit(); break;
    case 29: file_error(); break;
    case 30: plotMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 31: plotMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 32: help(); break;
    case 33: save(); break;
    case 34: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 35: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_MonteCarlo::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: iterate(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_MonteCarlo::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_MonteCarlo::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
