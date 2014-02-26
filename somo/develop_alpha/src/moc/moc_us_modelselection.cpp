/****************************************************************************
** US_ModelSelection meta object code from reading C++ file 'us_modelselection.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_modelselection.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_ModelSelection::className() const
{
    return "US_ModelSelection";
}

QMetaObject *US_ModelSelection::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ModelSelection( "US_ModelSelection", &US_ModelSelection::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ModelSelection::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ModelSelection", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ModelSelection::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ModelSelection", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ModelSelection::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In }
    };
    static const QUMethod slot_0 = {"select_model", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_1 = {"initializeAssociation1", 4, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In }
    };
    static const QUMethod slot_2 = {"initializeAssociation2", 1, param_slot_2 };
    static const QMetaData slot_tbl[] = {
	{ "select_model(struct ModelSystem*)", &slot_0, QMetaData::Public },
	{ "initializeAssociation1(struct ModelSystem*,unsigned int,unsigned int,unsigned int)", &slot_1, QMetaData::Public },
	{ "initializeAssociation2(struct ModelSystem*)", &slot_2, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ModelSelection", parentObject,
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ModelSelection.setMetaObject( metaObj );
    return metaObj;
}

void* US_ModelSelection::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ModelSelection" ) )
	return this;
    return QObject::qt_cast( clname );
}

bool US_ModelSelection::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: select_model((struct ModelSystem*)static_QUType_ptr.get(_o+1)); break;
    case 1: initializeAssociation1((struct ModelSystem*)static_QUType_ptr.get(_o+1),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+2))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+3))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+4)))); break;
    case 2: initializeAssociation2((struct ModelSystem*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ModelSelection::qt_emit( int _id, QUObject* _o )
{
    return QObject::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_ModelSelection::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool US_ModelSelection::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
