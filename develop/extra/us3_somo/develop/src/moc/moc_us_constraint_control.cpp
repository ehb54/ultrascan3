/****************************************************************************
** US_ConstraintControl meta object code from reading C++ file 'us_constraint_control.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_constraint_control.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_ConstraintControl::className() const
{
    return "US_ConstraintControl";
}

QMetaObject *US_ConstraintControl::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ConstraintControl( "US_ConstraintControl", &US_ConstraintControl::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ConstraintControl::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ConstraintControl", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ConstraintControl::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ConstraintControl", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ConstraintControl::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_0 = {"setDefault", 2, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ "constant", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_1 = {"setDefault", 3, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_2 = {"update", 4, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ "constant", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_3 = {"update", 5, param_slot_3 };
    static const QUMethod slot_4 = {"clear", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"setFit", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_ptr, "struct constraint", QUParameter::In }
    };
    static const QUMethod slot_6 = {"update", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"setHigh", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"setLow", 1, param_slot_8 };
    static const QUMethod slot_9 = {"setFit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setDefault(float,float)", &slot_0, QMetaData::Public },
	{ "setDefault(float,float,float)", &slot_1, QMetaData::Public },
	{ "update(float,float,float*,float*)", &slot_2, QMetaData::Public },
	{ "update(float,float,float*,float*,float)", &slot_3, QMetaData::Public },
	{ "clear()", &slot_4, QMetaData::Public },
	{ "setFit(bool)", &slot_5, QMetaData::Public },
	{ "update(struct constraint)", &slot_6, QMetaData::Public },
	{ "setHigh(const QString&)", &slot_7, QMetaData::Private },
	{ "setLow(const QString&)", &slot_8, QMetaData::Private },
	{ "setFit()", &slot_9, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "struct constraint", QUParameter::In }
    };
    static const QUMethod signal_0 = {"constraintChanged", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "constraintChanged(struct constraint)", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ConstraintControl", parentObject,
	slot_tbl, 10,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ConstraintControl.setMetaObject( metaObj );
    return metaObj;
}

void* US_ConstraintControl::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ConstraintControl" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL constraintChanged
void US_ConstraintControl::constraintChanged( struct constraint t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

bool US_ConstraintControl::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setDefault((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 1: setDefault((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3)))); break;
    case 2: update((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float*)static_QUType_ptr.get(_o+3),(float*)static_QUType_ptr.get(_o+4)); break;
    case 3: update((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float*)static_QUType_ptr.get(_o+3),(float*)static_QUType_ptr.get(_o+4),(float)(*((float*)static_QUType_ptr.get(_o+5)))); break;
    case 4: clear(); break;
    case 5: setFit((bool)static_QUType_bool.get(_o+1)); break;
    case 6: update((struct constraint)(*((struct constraint*)static_QUType_ptr.get(_o+1)))); break;
    case 7: setHigh((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: setLow((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: setFit(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ConstraintControl::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: constraintChanged((struct constraint)(*((struct constraint*)static_QUType_ptr.get(_o+1)))); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_ConstraintControl::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_ConstraintControl::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
