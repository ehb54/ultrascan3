/****************************************************************************
** US_Hydrodyn_Hydro_Zeno meta object code from reading C++ file 'us_hydrodyn_hydro_zeno.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_hydro_zeno.h"
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

const char *US_Hydrodyn_Hydro_Zeno::className() const
{
    return "US_Hydrodyn_Hydro_Zeno";
}

QMetaObject *US_Hydrodyn_Hydro_Zeno::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Hydro_Zeno( "US_Hydrodyn_Hydro_Zeno", &US_Hydrodyn_Hydro_Zeno::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Hydro_Zeno::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Hydro_Zeno", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Hydro_Zeno::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Hydro_Zeno", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Hydro_Zeno::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_zeno_zeno", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_zeno_zeno_steps", 1, param_slot_2 };
    static const QUMethod slot_3 = {"set_zeno_interior", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_zeno_interior_steps", 1, param_slot_4 };
    static const QUMethod slot_5 = {"set_zeno_surface", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_zeno_surface_steps", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_zeno_surface_thickness", 1, param_slot_7 };
    static const QUMethod slot_8 = {"cancel", 0, 0 };
    static const QUMethod slot_9 = {"help", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_10 = {"closeEvent", 1, param_slot_10 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_zeno_zeno()", &slot_1, QMetaData::Private },
	{ "update_zeno_zeno_steps(const QString&)", &slot_2, QMetaData::Private },
	{ "set_zeno_interior()", &slot_3, QMetaData::Private },
	{ "update_zeno_interior_steps(const QString&)", &slot_4, QMetaData::Private },
	{ "set_zeno_surface()", &slot_5, QMetaData::Private },
	{ "update_zeno_surface_steps(const QString&)", &slot_6, QMetaData::Private },
	{ "update_zeno_surface_thickness(const QString&)", &slot_7, QMetaData::Private },
	{ "cancel()", &slot_8, QMetaData::Private },
	{ "help()", &slot_9, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_10, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Hydro_Zeno", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Hydro_Zeno.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Hydro_Zeno::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Hydro_Zeno" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Hydro_Zeno::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_zeno_zeno(); break;
    case 2: update_zeno_zeno_steps((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: set_zeno_interior(); break;
    case 4: update_zeno_interior_steps((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: set_zeno_surface(); break;
    case 6: update_zeno_surface_steps((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_zeno_surface_thickness((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: cancel(); break;
    case 9: help(); break;
    case 10: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Hydro_Zeno::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Hydro_Zeno::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Hydro_Zeno::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
