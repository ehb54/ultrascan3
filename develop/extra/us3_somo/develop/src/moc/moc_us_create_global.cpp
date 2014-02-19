/****************************************************************************
** US_CreateGlobal meta object code from reading C++ file 'us_create_global.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_create_global.h"
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

const char *US_CreateGlobal::className() const
{
    return "US_CreateGlobal";
}

QMetaObject *US_CreateGlobal::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_CreateGlobal( "US_CreateGlobal", &US_CreateGlobal::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_CreateGlobal::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_CreateGlobal", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_CreateGlobal::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_CreateGlobal", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_CreateGlobal::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"closeEvent", 1, param_slot_1 };
    static const QUMethod slot_2 = {"add", 0, 0 };
    static const QUMethod slot_3 = {"reset", 0, 0 };
    static const QUMethod slot_4 = {"help", 0, 0 };
    static const QUMethod slot_5 = {"save", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"remove", 1, param_slot_6 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_1, QMetaData::Protected },
	{ "add()", &slot_2, QMetaData::Private },
	{ "reset()", &slot_3, QMetaData::Private },
	{ "help()", &slot_4, QMetaData::Private },
	{ "save()", &slot_5, QMetaData::Private },
	{ "remove(int)", &slot_6, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_CreateGlobal", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_CreateGlobal.setMetaObject( metaObj );
    return metaObj;
}

void* US_CreateGlobal::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_CreateGlobal" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_CreateGlobal::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: add(); break;
    case 3: reset(); break;
    case 4: help(); break;
    case 5: save(); break;
    case 6: remove((int)static_QUType_int.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_CreateGlobal::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_CreateGlobal::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_CreateGlobal::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
