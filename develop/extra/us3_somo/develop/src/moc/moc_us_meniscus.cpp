/****************************************************************************
** US_Meniscus meta object code from reading C++ file 'us_meniscus.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_meniscus.h"
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

const char *US_Meniscus::className() const
{
    return "US_Meniscus";
}

QMetaObject *US_Meniscus::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Meniscus( "US_Meniscus", &US_Meniscus::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Meniscus::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Meniscus", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Meniscus::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Meniscus", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Meniscus::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_order", 1, param_slot_1 };
    static const QUMethod slot_2 = {"update_plot", 0, 0 };
    static const QUMethod slot_3 = {"reset", 0, 0 };
    static const QUMethod slot_4 = {"help", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_5 = {"find_index", 5, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_order(double)", &slot_1, QMetaData::Private },
	{ "update_plot()", &slot_2, QMetaData::Private },
	{ "reset()", &slot_3, QMetaData::Private },
	{ "help()", &slot_4, QMetaData::Private },
	{ "find_index(double,int,int,double*)", &slot_5, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Meniscus", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Meniscus.setMetaObject( metaObj );
    return metaObj;
}

void* US_Meniscus::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Meniscus" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Meniscus::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_order((double)static_QUType_double.get(_o+1)); break;
    case 2: update_plot(); break;
    case 3: reset(); break;
    case 4: help(); break;
    case 5: static_QUType_int.set(_o,find_index((double)static_QUType_double.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(double*)static_QUType_varptr.get(_o+4))); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Meniscus::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Meniscus::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Meniscus::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
