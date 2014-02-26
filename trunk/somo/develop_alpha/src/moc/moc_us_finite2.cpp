/****************************************************************************
** US_SetModel meta object code from reading C++ file 'us_finite2.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_finite2.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_SetModel::className() const
{
    return "US_SetModel";
}

QMetaObject *US_SetModel::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_SetModel( "US_SetModel", &US_SetModel::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_SetModel::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SetModel", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_SetModel::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SetModel", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_SetModel::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"assign_component", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"select_model", 1, param_slot_1 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "assign_component(double)", &slot_0, QMetaData::Private },
	{ "select_model(int)", &slot_1, QMetaData::Private },
	{ "help()", &slot_2, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_SetModel", parentObject,
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_SetModel.setMetaObject( metaObj );
    return metaObj;
}

void* US_SetModel::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_SetModel" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_SetModel::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: assign_component((double)static_QUType_double.get(_o+1)); break;
    case 1: select_model((int)static_QUType_int.get(_o+1)); break;
    case 2: help(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_SetModel::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_SetModel::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_SetModel::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
