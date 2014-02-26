/****************************************************************************
** US_Hydrodyn_Saxs_Hplc_Movie meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_movie.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_hplc_movie.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Saxs_Hplc_Movie::className() const
{
    return "US_Hydrodyn_Saxs_Hplc_Movie";
}

QMetaObject *US_Hydrodyn_Saxs_Hplc_Movie::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Hplc_Movie( "US_Hydrodyn_Saxs_Hplc_Movie", &US_Hydrodyn_Saxs_Hplc_Movie::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Hplc_Movie::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Movie", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Hplc_Movie::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Movie", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Hplc_Movie::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"front", 0, 0 };
    static const QUMethod slot_1 = {"prev", 0, 0 };
    static const QUMethod slot_2 = {"slower", 0, 0 };
    static const QUMethod slot_3 = {"start", 0, 0 };
    static const QUMethod slot_4 = {"faster", 0, 0 };
    static const QUMethod slot_5 = {"next", 0, 0 };
    static const QUMethod slot_6 = {"end", 0, 0 };
    static const QUMethod slot_7 = {"set_show_gauss", 0, 0 };
    static const QUMethod slot_8 = {"set_show_ref", 0, 0 };
    static const QUMethod slot_9 = {"set_mono", 0, 0 };
    static const QUMethod slot_10 = {"set_save", 0, 0 };
    static const QUMethod slot_11 = {"help", 0, 0 };
    static const QUMethod slot_12 = {"cancel", 0, 0 };
    static const QUMethod slot_13 = {"setupGUI", 0, 0 };
    static const QUMethod slot_14 = {"update_enables", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_15 = {"closeEvent", 1, param_slot_15 };
    static const QMetaData slot_tbl[] = {
	{ "front()", &slot_0, QMetaData::Private },
	{ "prev()", &slot_1, QMetaData::Private },
	{ "slower()", &slot_2, QMetaData::Private },
	{ "start()", &slot_3, QMetaData::Private },
	{ "faster()", &slot_4, QMetaData::Private },
	{ "next()", &slot_5, QMetaData::Private },
	{ "end()", &slot_6, QMetaData::Private },
	{ "set_show_gauss()", &slot_7, QMetaData::Private },
	{ "set_show_ref()", &slot_8, QMetaData::Private },
	{ "set_mono()", &slot_9, QMetaData::Private },
	{ "set_save()", &slot_10, QMetaData::Private },
	{ "help()", &slot_11, QMetaData::Private },
	{ "cancel()", &slot_12, QMetaData::Private },
	{ "setupGUI()", &slot_13, QMetaData::Private },
	{ "update_enables()", &slot_14, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_15, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Hplc_Movie", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Hplc_Movie.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Hplc_Movie::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Hplc_Movie" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Hplc_Movie::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: front(); break;
    case 1: prev(); break;
    case 2: slower(); break;
    case 3: start(); break;
    case 4: faster(); break;
    case 5: next(); break;
    case 6: end(); break;
    case 7: set_show_gauss(); break;
    case 8: set_show_ref(); break;
    case 9: set_mono(); break;
    case 10: set_save(); break;
    case 11: help(); break;
    case 12: cancel(); break;
    case 13: setupGUI(); break;
    case 14: update_enables(); break;
    case 15: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Hplc_Movie::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Hplc_Movie::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Hplc_Movie::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
