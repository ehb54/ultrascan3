/****************************************************************************
** US_Hydrodyn_OR meta object code from reading C++ file 'us_hydrodyn_overlap_reduction.h'
**
** Created: Tue Feb 25 10:28:32 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_overlap_reduction.h"
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

const char *US_Hydrodyn_OR::className() const
{
    return "US_Hydrodyn_OR";
}

QMetaObject *US_Hydrodyn_OR::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_OR( "US_Hydrodyn_OR", &US_Hydrodyn_OR::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_OR::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_OR", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_OR::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_OR", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_OR::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_fuse", 0, 0 };
    static const QUMethod slot_2 = {"set_hierarch", 0, 0 };
    static const QUMethod slot_3 = {"set_sync", 0, 0 };
    static const QUMethod slot_4 = {"set_translate", 0, 0 };
    static const QUMethod slot_5 = {"set_remove", 0, 0 };
    static const QUMethod slot_6 = {"set_replicate_method", 0, 0 };
    static const QUMethod slot_7 = {"replicate", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_fuse", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_sync", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_hierarch", 1, param_slot_10 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_fuse()", &slot_1, QMetaData::Private },
	{ "set_hierarch()", &slot_2, QMetaData::Private },
	{ "set_sync()", &slot_3, QMetaData::Private },
	{ "set_translate()", &slot_4, QMetaData::Private },
	{ "set_remove()", &slot_5, QMetaData::Private },
	{ "set_replicate_method()", &slot_6, QMetaData::Private },
	{ "replicate()", &slot_7, QMetaData::Private },
	{ "update_fuse(double)", &slot_8, QMetaData::Private },
	{ "update_sync(double)", &slot_9, QMetaData::Private },
	{ "update_hierarch(double)", &slot_10, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_OR", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_OR.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_OR::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_OR" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_OR::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_fuse(); break;
    case 2: set_hierarch(); break;
    case 3: set_sync(); break;
    case 4: set_translate(); break;
    case 5: set_remove(); break;
    case 6: set_replicate_method(); break;
    case 7: replicate(); break;
    case 8: update_fuse((double)static_QUType_double.get(_o+1)); break;
    case 9: update_sync((double)static_QUType_double.get(_o+1)); break;
    case 10: update_hierarch((double)static_QUType_double.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_OR::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_OR::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_OR::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
