/****************************************************************************
** US_Hydrodyn_Saxs_Search meta object code from reading C++ file 'us_hydrodyn_saxs_search.h'
**
** Created: Tue Feb 25 10:28:33 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_search.h"
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

const char *US_Hydrodyn_Saxs_Search::className() const
{
    return "US_Hydrodyn_Saxs_Search";
}

QMetaObject *US_Hydrodyn_Saxs_Search::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Search( "US_Hydrodyn_Saxs_Search", &US_Hydrodyn_Saxs_Search::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Search::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Search", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Search::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Search", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Search::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"table_value", 2, param_slot_1 };
    static const QUMethod slot_2 = {"save_to_csv", 0, 0 };
    static const QUMethod slot_3 = {"replot_saxs", 0, 0 };
    static const QUMethod slot_4 = {"save_saxs_plot", 0, 0 };
    static const QUMethod slot_5 = {"set_target", 0, 0 };
    static const QUMethod slot_6 = {"start", 0, 0 };
    static const QUMethod slot_7 = {"run_current", 0, 0 };
    static const QUMethod slot_8 = {"run_best", 0, 0 };
    static const QUMethod slot_9 = {"stop", 0, 0 };
    static const QUMethod slot_10 = {"clear_display", 0, 0 };
    static const QUMethod slot_11 = {"update_font", 0, 0 };
    static const QUMethod slot_12 = {"save", 0, 0 };
    static const QUMethod slot_13 = {"cancel", 0, 0 };
    static const QUMethod slot_14 = {"help", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_15 = {"closeEvent", 1, param_slot_15 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "table_value(int,int)", &slot_1, QMetaData::Private },
	{ "save_to_csv()", &slot_2, QMetaData::Private },
	{ "replot_saxs()", &slot_3, QMetaData::Private },
	{ "save_saxs_plot()", &slot_4, QMetaData::Private },
	{ "set_target()", &slot_5, QMetaData::Private },
	{ "start()", &slot_6, QMetaData::Private },
	{ "run_current()", &slot_7, QMetaData::Private },
	{ "run_best()", &slot_8, QMetaData::Private },
	{ "stop()", &slot_9, QMetaData::Private },
	{ "clear_display()", &slot_10, QMetaData::Private },
	{ "update_font()", &slot_11, QMetaData::Private },
	{ "save()", &slot_12, QMetaData::Private },
	{ "cancel()", &slot_13, QMetaData::Private },
	{ "help()", &slot_14, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_15, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Search", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Search.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Search::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Search" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Search::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: table_value((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 2: save_to_csv(); break;
    case 3: replot_saxs(); break;
    case 4: save_saxs_plot(); break;
    case 5: set_target(); break;
    case 6: start(); break;
    case 7: run_current(); break;
    case 8: run_best(); break;
    case 9: stop(); break;
    case 10: clear_display(); break;
    case 11: update_font(); break;
    case 12: save(); break;
    case 13: cancel(); break;
    case 14: help(); break;
    case 15: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Search::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Search::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Search::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
