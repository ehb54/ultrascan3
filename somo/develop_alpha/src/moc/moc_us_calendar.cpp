/****************************************************************************
** US_Calendar meta object code from reading C++ file 'us_calendar.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_calendar.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Calendar::className() const
{
    return "US_Calendar";
}

QMetaObject *US_Calendar::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Calendar( "US_Calendar", &US_Calendar::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Calendar::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Calendar", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Calendar::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Calendar", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Calendar::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotDayClassify", 3, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"slotDayLabel", 4, param_slot_1 };
    static const QUMethod slot_2 = {"quit", 0, 0 };
    static const QUMethod slot_3 = {"accept", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_Date", 1, param_slot_4 };
    static const QMetaData slot_tbl[] = {
	{ "slotDayClassify(int,int,char*)", &slot_0, QMetaData::Public },
	{ "slotDayLabel(int,int,int,char*)", &slot_1, QMetaData::Public },
	{ "quit()", &slot_2, QMetaData::Private },
	{ "accept()", &slot_3, QMetaData::Private },
	{ "update_Date(QString)", &slot_4, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"dateChanged", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "dateChanged(QString)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Calendar", parentObject,
	slot_tbl, 5,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Calendar.setMetaObject( metaObj );
    return metaObj;
}

void* US_Calendar::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Calendar" ) )
	return this;
    return QWidget::qt_cast( clname );
}

// SIGNAL dateChanged
void US_Calendar::dateChanged( QString t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool US_Calendar::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotDayClassify((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(char*)static_QUType_charstar.get(_o+3)); break;
    case 1: slotDayLabel((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(char*)static_QUType_charstar.get(_o+4)); break;
    case 2: quit(); break;
    case 3: accept(); break;
    case 4: update_Date((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Calendar::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: dateChanged((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Calendar::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_Calendar::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *Calendar::className() const
{
    return "Calendar";
}

QMetaObject *Calendar::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Calendar( "Calendar", &Calendar::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Calendar::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Calendar", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Calendar::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Calendar", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Calendar::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"timerEvent", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "timerEvent()", &slot_0, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ "mon", &static_QUType_int, 0, QUParameter::In },
	{ "yr", &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"classifyRequest", 3, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"dayLabel", 4, param_signal_1 };
    static const QMetaData signal_tbl[] = {
	{ "classifyRequest(int,int,char*)", &signal_0, QMetaData::Public },
	{ "dayLabel(int,int,int,char*)", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"Calendar", parentObject,
	slot_tbl, 1,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Calendar.setMetaObject( metaObj );
    return metaObj;
}

void* Calendar::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Calendar" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL classifyRequest
void Calendar::classifyRequest( int t0, int t1, char* t2 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[4];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_charstar.set(o+3,t2);
    activate_signal( clist, o );
}

// SIGNAL dayLabel
void Calendar::dayLabel( int t0, int t1, int t2, char* t3 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[5];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_int.set(o+3,t2);
    static_QUType_charstar.set(o+4,t3);
    activate_signal( clist, o );
}

bool Calendar::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: timerEvent(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool Calendar::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: classifyRequest((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(char*)static_QUType_charstar.get(_o+3)); break;
    case 1: dayLabel((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(char*)static_QUType_charstar.get(_o+4)); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool Calendar::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool Calendar::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *DateInput::className() const
{
    return "DateInput";
}

QMetaObject *DateInput::metaObj = 0;
static QMetaObjectCleanUp cleanUp_DateInput( "DateInput", &DateInput::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString DateInput::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DateInput", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString DateInput::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DateInput", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* DateInput::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotDayLabel", 4, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"slotClassifyRequest", 3, param_slot_1 };
    static const QUMethod slot_2 = {"slotNewText", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotDayLabel(int,int,int,char*)", &slot_0, QMetaData::Protected },
	{ "slotClassifyRequest(int,int,char*)", &slot_1, QMetaData::Protected },
	{ "slotNewText()", &slot_2, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"classifyRequest", 3, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"dayLabel", 4, param_signal_1 };
    static const QUMethod signal_2 = {"dateChanged", 0, 0 };
    static const QUParameter param_signal_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_3 = {"DisplayChanged", 1, param_signal_3 };
    static const QMetaData signal_tbl[] = {
	{ "classifyRequest(int,int,char*)", &signal_0, QMetaData::Public },
	{ "dayLabel(int,int,int,char*)", &signal_1, QMetaData::Public },
	{ "dateChanged()", &signal_2, QMetaData::Public },
	{ "DisplayChanged(QString)", &signal_3, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"DateInput", parentObject,
	slot_tbl, 3,
	signal_tbl, 4,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_DateInput.setMetaObject( metaObj );
    return metaObj;
}

void* DateInput::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "DateInput" ) )
	return this;
    return QWidget::qt_cast( clname );
}

// SIGNAL classifyRequest
void DateInput::classifyRequest( int t0, int t1, char* t2 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[4];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_charstar.set(o+3,t2);
    activate_signal( clist, o );
}

// SIGNAL dayLabel
void DateInput::dayLabel( int t0, int t1, int t2, char* t3 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[5];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_int.set(o+3,t2);
    static_QUType_charstar.set(o+4,t3);
    activate_signal( clist, o );
}

// SIGNAL dateChanged
void DateInput::dateChanged()
{
    activate_signal( staticMetaObject()->signalOffset() + 2 );
}

// SIGNAL DisplayChanged
void DateInput::DisplayChanged( QString t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 3, t0 );
}

bool DateInput::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotDayLabel((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(char*)static_QUType_charstar.get(_o+4)); break;
    case 1: slotClassifyRequest((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(char*)static_QUType_charstar.get(_o+3)); break;
    case 2: slotNewText(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool DateInput::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: classifyRequest((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(char*)static_QUType_charstar.get(_o+3)); break;
    case 1: dayLabel((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(char*)static_QUType_charstar.get(_o+4)); break;
    case 2: dateChanged(); break;
    case 3: DisplayChanged((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool DateInput::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool DateInput::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
