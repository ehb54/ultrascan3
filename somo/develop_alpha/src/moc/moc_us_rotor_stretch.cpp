/****************************************************************************
** US_RotorStretch meta object code from reading C++ file 'us_rotor_stretch.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_rotor_stretch.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_RotorStretch::className() const
{
    return "US_RotorStretch";
}

QMetaObject *US_RotorStretch::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_RotorStretch( "US_RotorStretch", &US_RotorStretch::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_RotorStretch::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_RotorStretch", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_RotorStretch::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_RotorStretch", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_RotorStretch::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"update", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"set_speed", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_rotor", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_centerpiece", 1, param_slot_4 };
    static const QUMethod slot_5 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Public },
	{ "update()", &slot_1, QMetaData::Public },
	{ "set_speed(double)", &slot_2, QMetaData::Public },
	{ "update_rotor(int)", &slot_3, QMetaData::Public },
	{ "update_centerpiece(int)", &slot_4, QMetaData::Public },
	{ "help()", &slot_5, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_RotorStretch", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_RotorStretch.setMetaObject( metaObj );
    return metaObj;
}

void* US_RotorStretch::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_RotorStretch" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_RotorStretch::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: update(); break;
    case 2: set_speed((double)static_QUType_double.get(_o+1)); break;
    case 3: update_rotor((int)static_QUType_int.get(_o+1)); break;
    case 4: update_centerpiece((int)static_QUType_int.get(_o+1)); break;
    case 5: help(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_RotorStretch::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_RotorStretch::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_RotorStretch::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
