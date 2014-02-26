/****************************************************************************
** US_Config meta object code from reading C++ file 'us_util.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_util.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Config::className() const
{
    return "US_Config";
}

QMetaObject *US_Config::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Config( "US_Config", &US_Config::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Config::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Config", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Config::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Config", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Config::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_0 = {"read", 1, param_slot_0 };
    static const QUMethod slot_1 = {"setModelString", 0, 0 };
    static const QUMethod slot_2 = {"color_defaults", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "read()", &slot_0, QMetaData::Public },
	{ "setModelString()", &slot_1, QMetaData::Public },
	{ "color_defaults()", &slot_2, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"errorMessage", 2, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "errorMessage(QString,QString)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Config", parentObject,
	slot_tbl, 3,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Config.setMetaObject( metaObj );
    return metaObj;
}

void* US_Config::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Config" ) )
	return this;
    return QObject::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL errorMessage
void US_Config::errorMessage( QString t0, QString t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_QString.set(o+1,t0);
    static_QUType_QString.set(o+2,t1);
    activate_signal( clist, o );
}

bool US_Config::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: static_QUType_bool.set(_o,read()); break;
    case 1: setModelString(); break;
    case 2: color_defaults(); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Config::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: errorMessage((QString)static_QUType_QString.get(_o+1),(QString)static_QUType_QString.get(_o+2)); break;
    default:
	return QObject::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Config::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool US_Config::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_Help::className() const
{
    return "US_Help";
}

QMetaObject *US_Help::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Help( "US_Help", &US_Help::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Help::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Help", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Help::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Help", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Help::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"show_URL", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"show_html_file", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"show_help", 1, param_slot_2 };
    static const QUMethod slot_3 = {"captureStdout", 0, 0 };
    static const QUMethod slot_4 = {"captureStderr", 0, 0 };
    static const QUMethod slot_5 = {"endProcess", 0, 0 };
    static const QUMethod slot_6 = {"openBrowser", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "show_URL(QString)", &slot_0, QMetaData::Public },
	{ "show_html_file(QString)", &slot_1, QMetaData::Public },
	{ "show_help(QString)", &slot_2, QMetaData::Public },
	{ "captureStdout()", &slot_3, QMetaData::Private },
	{ "captureStderr()", &slot_4, QMetaData::Private },
	{ "endProcess()", &slot_5, QMetaData::Private },
	{ "openBrowser()", &slot_6, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Help", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Help.setMetaObject( metaObj );
    return metaObj;
}

void* US_Help::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Help" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_Help::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: show_URL((QString)static_QUType_QString.get(_o+1)); break;
    case 1: show_html_file((QString)static_QUType_QString.get(_o+1)); break;
    case 2: show_help((QString)static_QUType_QString.get(_o+1)); break;
    case 3: captureStdout(); break;
    case 4: captureStderr(); break;
    case 5: endProcess(); break;
    case 6: openBrowser(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Help::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Help::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_Help::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *OneLiner::className() const
{
    return "OneLiner";
}

QMetaObject *OneLiner::metaObj = 0;
static QMetaObjectCleanUp cleanUp_OneLiner( "OneLiner", &OneLiner::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString OneLiner::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "OneLiner", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString OneLiner::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "OneLiner", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* OneLiner::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "update(const QString&)", &slot_0, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"textChanged", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "textChanged(const QString&)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"OneLiner", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_OneLiner.setMetaObject( metaObj );
    return metaObj;
}

void* OneLiner::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "OneLiner" ) )
	return this;
    return QDialog::qt_cast( clname );
}

// SIGNAL textChanged
void OneLiner::textChanged( const QString& t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool OneLiner::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update((const QString&)static_QUType_QString.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool OneLiner::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: textChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    default:
	return QDialog::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool OneLiner::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool OneLiner::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_FitParameter::className() const
{
    return "US_FitParameter";
}

QMetaObject *US_FitParameter::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_FitParameter( "US_FitParameter", &US_FitParameter::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_FitParameter::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_FitParameter", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_FitParameter::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_FitParameter", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_FitParameter::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"setEnabled", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_1 = {"updateValue", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_2 = {"updateRange", 1, param_slot_2 };
    static const QUMethod slot_3 = {"setFloatRed", 0, 0 };
    static const QUMethod slot_4 = {"setFloatGreen", 0, 0 };
    static const QUMethod slot_5 = {"setFloatDisabled", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"setUnit", 1, param_slot_6 };
    static const QMetaData slot_tbl[] = {
	{ "setEnabled(bool)", &slot_0, QMetaData::Public },
	{ "updateValue(float)", &slot_1, QMetaData::Public },
	{ "updateRange(float)", &slot_2, QMetaData::Public },
	{ "setFloatRed()", &slot_3, QMetaData::Public },
	{ "setFloatGreen()", &slot_4, QMetaData::Public },
	{ "setFloatDisabled()", &slot_5, QMetaData::Public },
	{ "setUnit(const QString&)", &slot_6, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_FitParameter", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_FitParameter.setMetaObject( metaObj );
    return metaObj;
}

void* US_FitParameter::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_FitParameter" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_FitParameter::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setEnabled((bool)static_QUType_bool.get(_o+1)); break;
    case 1: updateValue((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 2: updateRange((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 3: setFloatRed(); break;
    case 4: setFloatGreen(); break;
    case 5: setFloatDisabled(); break;
    case 6: setUnit((const QString&)static_QUType_QString.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_FitParameter::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_FitParameter::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_FitParameter::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
