/****************************************************************************
** US_GridControl_T meta object code from reading C++ file 'us_gridcontrol_t.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_gridcontrol_t.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_GridControl_T::className() const
{
    return "US_GridControl_T";
}

QMetaObject *US_GridControl_T::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_GridControl_T( "US_GridControl_T", &US_GridControl_T::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_GridControl_T::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GridControl_T", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_GridControl_T::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GridControl_T", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_GridControl_T::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUMethod slot_0 = {"add_experiment", 0, 0 };
    static const QUMethod slot_1 = {"write_experiment", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"write_solutes", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_email", 1, param_slot_3 };
    static const QUMethod slot_4 = {"write_ga_experiment", 0, 0 };
    static const QUMethod slot_5 = {"write_sa2d_experiment", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "add_experiment()", &slot_0, QMetaData::Private },
	{ "write_experiment()", &slot_1, QMetaData::Private },
	{ "write_solutes(const QString&)", &slot_2, QMetaData::Private },
	{ "update_email(const QString&)", &slot_3, QMetaData::Private },
	{ "write_ga_experiment()", &slot_4, QMetaData::Private },
	{ "write_sa2d_experiment()", &slot_5, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_GridControl_T", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_GridControl_T.setMetaObject( metaObj );
    return metaObj;
}

void* US_GridControl_T::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_GridControl_T" ) )
	return this;
    return QObject::qt_cast( clname );
}

bool US_GridControl_T::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: add_experiment(); break;
    case 1: write_experiment(); break;
    case 2: write_solutes((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_email((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: write_ga_experiment(); break;
    case 5: write_sa2d_experiment(); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_GridControl_T::qt_emit( int _id, QUObject* _o )
{
    return QObject::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_GridControl_T::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool US_GridControl_T::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
