/****************************************************************************
** US_Vbar meta object code from reading C++ file 'us_vbar.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_vbar.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Vbar::className() const
{
    return "US_Vbar";
}

QMetaObject *US_Vbar::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Vbar( "US_Vbar", &US_Vbar::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Vbar::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Vbar", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Vbar::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Vbar", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Vbar::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB_T::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"read_file", 2, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"result_output", 2, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_2 = {"read_db", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"select_vbar", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"retrieve_vbar", 2, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"setInvestigator", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"export_vbar", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"export_DNA_vbar", 1, param_slot_7 };
    static const QMetaData slot_tbl[] = {
	{ "read_file(const QString&)", &slot_0, QMetaData::Public },
	{ "result_output(const QString&)", &slot_1, QMetaData::Public },
	{ "read_db()", &slot_2, QMetaData::Public },
	{ "select_vbar(int)", &slot_3, QMetaData::Public },
	{ "retrieve_vbar(int)", &slot_4, QMetaData::Public },
	{ "setInvestigator(const int)", &slot_5, QMetaData::Public },
	{ "export_vbar(int)", &slot_6, QMetaData::Public },
	{ "export_DNA_vbar(int)", &slot_7, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"valueChanged", 2, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_1 = {"e280Changed", 1, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ "PepID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_2 = {"idChanged", 1, param_signal_2 };
    static const QMetaData signal_tbl[] = {
	{ "valueChanged(float,float)", &signal_0, QMetaData::Public },
	{ "e280Changed(float)", &signal_1, QMetaData::Public },
	{ "idChanged(int)", &signal_2, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Vbar", parentObject,
	slot_tbl, 8,
	signal_tbl, 3,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Vbar.setMetaObject( metaObj );
    return metaObj;
}

void* US_Vbar::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Vbar" ) )
	return this;
    return US_DB_T::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL valueChanged
void US_Vbar::valueChanged( float t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL e280Changed
void US_Vbar::e280Changed( float t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

// SIGNAL idChanged
void US_Vbar::idChanged( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 2, t0 );
}

bool US_Vbar::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: static_QUType_bool.set(_o,read_file((const QString&)static_QUType_QString.get(_o+1))); break;
    case 1: static_QUType_bool.set(_o,result_output((const QString&)static_QUType_QString.get(_o+1))); break;
    case 2: static_QUType_bool.set(_o,read_db()); break;
    case 3: select_vbar((int)static_QUType_int.get(_o+1)); break;
    case 4: static_QUType_bool.set(_o,retrieve_vbar((int)static_QUType_int.get(_o+1))); break;
    case 5: setInvestigator((const int)static_QUType_int.get(_o+1)); break;
    case 6: export_vbar((int)static_QUType_int.get(_o+1)); break;
    case 7: export_DNA_vbar((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB_T::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Vbar::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: valueChanged((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 1: e280Changed((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 2: idChanged((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB_T::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Vbar::qt_property( int id, int f, QVariant* v)
{
    return US_DB_T::qt_property( id, f, v);
}

bool US_Vbar::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
