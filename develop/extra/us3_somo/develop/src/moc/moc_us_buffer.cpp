/****************************************************************************
** US_Buffer meta object code from reading C++ file 'us_buffer.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_buffer.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Buffer::className() const
{
    return "US_Buffer";
}

QMetaObject *US_Buffer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Buffer( "US_Buffer", &US_Buffer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Buffer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Buffer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Buffer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Buffer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Buffer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB_T::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"export_buffer", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"read_buffer", 1, param_slot_1 };
    static const QUMethod slot_2 = {"buf_init", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_3 = {"read_template_file", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"setInvestigator", 1, param_slot_4 };
    static const QUMethod slot_5 = {"recalc_density", 0, 0 };
    static const QUMethod slot_6 = {"recalc_viscosity", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"result_output", 2, param_slot_7 };
    static const QMetaData slot_tbl[] = {
	{ "export_buffer(int)", &slot_0, QMetaData::Public },
	{ "read_buffer(QString)", &slot_1, QMetaData::Public },
	{ "buf_init()", &slot_2, QMetaData::Public },
	{ "read_template_file()", &slot_3, QMetaData::Public },
	{ "setInvestigator(const int)", &slot_4, QMetaData::Public },
	{ "recalc_density()", &slot_5, QMetaData::Public },
	{ "recalc_viscosity()", &slot_6, QMetaData::Public },
	{ "result_output(const QString&)", &slot_7, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ "density", &static_QUType_ptr, "float", QUParameter::In },
	{ "viscosity", &static_QUType_ptr, "float", QUParameter::In },
	{ "refractive_index", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"valueChanged", 3, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ "density", &static_QUType_ptr, "float", QUParameter::In },
	{ "viscosity", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_1 = {"valueChanged", 2, param_signal_1 };
    static const QMetaData signal_tbl[] = {
	{ "valueChanged(float,float,float)", &signal_0, QMetaData::Public },
	{ "valueChanged(float,float)", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Buffer", parentObject,
	slot_tbl, 8,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Buffer.setMetaObject( metaObj );
    return metaObj;
}

void* US_Buffer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Buffer" ) )
	return this;
    return US_DB_T::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL valueChanged
void US_Buffer::valueChanged( float t0, float t1, float t2 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[4];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    static_QUType_ptr.set(o+3,&t2);
    activate_signal( clist, o );
}

// SIGNAL valueChanged
void US_Buffer::valueChanged( float t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

bool US_Buffer::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: export_buffer((int)static_QUType_int.get(_o+1)); break;
    case 1: read_buffer((QString)static_QUType_QString.get(_o+1)); break;
    case 2: buf_init(); break;
    case 3: static_QUType_bool.set(_o,read_template_file()); break;
    case 4: setInvestigator((const int)static_QUType_int.get(_o+1)); break;
    case 5: recalc_density(); break;
    case 6: recalc_viscosity(); break;
    case 7: static_QUType_bool.set(_o,result_output((const QString&)static_QUType_QString.get(_o+1))); break;
    default:
	return US_DB_T::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Buffer::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: valueChanged((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3)))); break;
    case 1: valueChanged((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    default:
	return US_DB_T::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Buffer::qt_property( int id, int f, QVariant* v)
{
    return US_DB_T::qt_property( id, f, v);
}

bool US_Buffer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
