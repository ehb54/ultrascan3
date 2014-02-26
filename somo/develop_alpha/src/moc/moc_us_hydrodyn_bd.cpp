/****************************************************************************
** US_Hydrodyn_BD meta object code from reading C++ file 'us_hydrodyn_bd.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_bd.h"
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

const char *US_Hydrodyn_BD::className() const
{
    return "US_Hydrodyn_BD";
}

QMetaObject *US_Hydrodyn_BD::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_BD( "US_Hydrodyn_BD", &US_Hydrodyn_BD::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_BD::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_BD", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_BD::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_BD", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_BD::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"cancel", 0, 0 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QUMethod slot_3 = {"bd_options", 0, 0 };
    static const QUMethod slot_4 = {"anaflex_options", 0, 0 };
    static const QUMethod slot_5 = {"stop", 0, 0 };
    static const QUMethod slot_6 = {"bd_prepare", 0, 0 };
    static const QUMethod slot_7 = {"bd_load", 0, 0 };
    static const QUMethod slot_8 = {"bd_edit", 0, 0 };
    static const QUMethod slot_9 = {"bd_run", 0, 0 };
    static const QUMethod slot_10 = {"bd_load_results", 0, 0 };
    static const QUMethod slot_11 = {"anaflex_prepare", 0, 0 };
    static const QUMethod slot_12 = {"anaflex_load", 0, 0 };
    static const QUMethod slot_13 = {"anaflex_edit", 0, 0 };
    static const QUMethod slot_14 = {"anaflex_run", 0, 0 };
    static const QUMethod slot_15 = {"anaflex_load_results", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_16 = {"closeEvent", 1, param_slot_16 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "cancel()", &slot_1, QMetaData::Private },
	{ "help()", &slot_2, QMetaData::Private },
	{ "bd_options()", &slot_3, QMetaData::Private },
	{ "anaflex_options()", &slot_4, QMetaData::Private },
	{ "stop()", &slot_5, QMetaData::Private },
	{ "bd_prepare()", &slot_6, QMetaData::Private },
	{ "bd_load()", &slot_7, QMetaData::Private },
	{ "bd_edit()", &slot_8, QMetaData::Private },
	{ "bd_run()", &slot_9, QMetaData::Private },
	{ "bd_load_results()", &slot_10, QMetaData::Private },
	{ "anaflex_prepare()", &slot_11, QMetaData::Private },
	{ "anaflex_load()", &slot_12, QMetaData::Private },
	{ "anaflex_edit()", &slot_13, QMetaData::Private },
	{ "anaflex_run()", &slot_14, QMetaData::Private },
	{ "anaflex_load_results()", &slot_15, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_16, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_BD", parentObject,
	slot_tbl, 17,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_BD.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_BD::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_BD" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_BD::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: cancel(); break;
    case 2: help(); break;
    case 3: bd_options(); break;
    case 4: anaflex_options(); break;
    case 5: stop(); break;
    case 6: bd_prepare(); break;
    case 7: bd_load(); break;
    case 8: bd_edit(); break;
    case 9: bd_run(); break;
    case 10: bd_load_results(); break;
    case 11: anaflex_prepare(); break;
    case 12: anaflex_load(); break;
    case 13: anaflex_edit(); break;
    case 14: anaflex_run(); break;
    case 15: anaflex_load_results(); break;
    case 16: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_BD::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_BD::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_BD::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
