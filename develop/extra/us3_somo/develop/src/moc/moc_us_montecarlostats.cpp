/****************************************************************************
** US_MonteCarloStats_W meta object code from reading C++ file 'us_montecarlostats.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_montecarlostats.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_MonteCarloStats_W::className() const
{
    return "US_MonteCarloStats_W";
}

QMetaObject *US_MonteCarloStats_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_MonteCarloStats_W( "US_MonteCarloStats_W", &US_MonteCarloStats_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_MonteCarloStats_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_MonteCarloStats_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_MonteCarloStats_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_MonteCarloStats_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_MonteCarloStats_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"update_labels", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"quit", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_3 = {"closeEvent", 1, param_slot_3 };
    static const QMetaData slot_tbl[] = {
	{ "update_labels()", &slot_0, QMetaData::Public },
	{ "help()", &slot_1, QMetaData::Public },
	{ "quit()", &slot_2, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_3, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_MonteCarloStats_W", parentObject,
	slot_tbl, 4,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_MonteCarloStats_W.setMetaObject( metaObj );
    return metaObj;
}

void* US_MonteCarloStats_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_MonteCarloStats_W" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_MonteCarloStats_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_labels(); break;
    case 1: help(); break;
    case 2: quit(); break;
    case 3: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_MonteCarloStats_W::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_MonteCarloStats_W::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_MonteCarloStats_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
