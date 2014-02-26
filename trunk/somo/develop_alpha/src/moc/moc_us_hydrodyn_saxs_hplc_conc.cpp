/****************************************************************************
** US_Hydrodyn_Saxs_Hplc_Conc meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_conc.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_hplc_conc.h"
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

const char *US_Hydrodyn_Saxs_Hplc_Conc::className() const
{
    return "US_Hydrodyn_Saxs_Hplc_Conc";
}

QMetaObject *US_Hydrodyn_Saxs_Hplc_Conc::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Hplc_Conc( "US_Hydrodyn_Saxs_Hplc_Conc", &US_Hydrodyn_Saxs_Hplc_Conc::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Hplc_Conc::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Conc", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Hplc_Conc::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Conc", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Hplc_Conc::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"update_enables", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"row_header_released", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"sort_column", 1, param_slot_2 };
    static const QUMethod slot_3 = {"load", 0, 0 };
    static const QUMethod slot_4 = {"save", 0, 0 };
    static const QUMethod slot_5 = {"copy", 0, 0 };
    static const QUMethod slot_6 = {"paste", 0, 0 };
    static const QUMethod slot_7 = {"paste_all", 0, 0 };
    static const QUMethod slot_8 = {"cancel", 0, 0 };
    static const QUMethod slot_9 = {"help", 0, 0 };
    static const QUMethod slot_10 = {"set_ok", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_11 = {"closeEvent", 1, param_slot_11 };
    static const QMetaData slot_tbl[] = {
	{ "update_enables()", &slot_0, QMetaData::Private },
	{ "row_header_released(int)", &slot_1, QMetaData::Private },
	{ "sort_column(int)", &slot_2, QMetaData::Private },
	{ "load()", &slot_3, QMetaData::Private },
	{ "save()", &slot_4, QMetaData::Private },
	{ "copy()", &slot_5, QMetaData::Private },
	{ "paste()", &slot_6, QMetaData::Private },
	{ "paste_all()", &slot_7, QMetaData::Private },
	{ "cancel()", &slot_8, QMetaData::Private },
	{ "help()", &slot_9, QMetaData::Private },
	{ "set_ok()", &slot_10, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_11, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Hplc_Conc", parentObject,
	slot_tbl, 12,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Hplc_Conc.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Hplc_Conc::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Hplc_Conc" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Hplc_Conc::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_enables(); break;
    case 1: row_header_released((int)static_QUType_int.get(_o+1)); break;
    case 2: sort_column((int)static_QUType_int.get(_o+1)); break;
    case 3: load(); break;
    case 4: save(); break;
    case 5: copy(); break;
    case 6: paste(); break;
    case 7: paste_all(); break;
    case 8: cancel(); break;
    case 9: help(); break;
    case 10: set_ok(); break;
    case 11: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Hplc_Conc::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Hplc_Conc::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Hplc_Conc::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
