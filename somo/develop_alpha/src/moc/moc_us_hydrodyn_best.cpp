/****************************************************************************
** US_Hydrodyn_Best meta object code from reading C++ file 'us_hydrodyn_best.h'
**
** Created: Fri Feb 21 14:56:57 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_best.h"
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

const char *US_Hydrodyn_Best::className() const
{
    return "US_Hydrodyn_Best";
}

QMetaObject *US_Hydrodyn_Best::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Best( "US_Hydrodyn_Best", &US_Hydrodyn_Best::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Best::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Best", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Best::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Best", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Best::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"load", 0, 0 };
    static const QUMethod slot_1 = {"join_results", 0, 0 };
    static const QUMethod slot_2 = {"save_results", 0, 0 };
    static const QUMethod slot_3 = {"hide_input", 0, 0 };
    static const QUMethod slot_4 = {"clear_display", 0, 0 };
    static const QUMethod slot_5 = {"update_font", 0, 0 };
    static const QUMethod slot_6 = {"save", 0, 0 };
    static const QUMethod slot_7 = {"hide_editor", 0, 0 };
    static const QUMethod slot_8 = {"help", 0, 0 };
    static const QUMethod slot_9 = {"cancel", 0, 0 };
    static const QUMethod slot_10 = {"data_selected", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ "do_recompute_tau", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"data_selected", 1, param_slot_11 };
    static const QUMethod slot_12 = {"cb_changed", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ "do_data", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"cb_changed", 1, param_slot_13 };
    static const QUMethod slot_14 = {"cb_changed_ln", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ "do_data", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"cb_changed_ln", 1, param_slot_15 };
    static const QUMethod slot_16 = {"cb_changed_exp", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ "do_data", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"cb_changed_exp", 1, param_slot_17 };
    static const QUMethod slot_18 = {"toggle_points", 0, 0 };
    static const QUMethod slot_19 = {"toggle_points_ln", 0, 0 };
    static const QUMethod slot_20 = {"toggle_points_exp", 0, 0 };
    static const QUParameter param_slot_21[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"set_last_file", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_22 = {"closeEvent", 1, param_slot_22 };
    static const QMetaData slot_tbl[] = {
	{ "load()", &slot_0, QMetaData::Private },
	{ "join_results()", &slot_1, QMetaData::Private },
	{ "save_results()", &slot_2, QMetaData::Private },
	{ "hide_input()", &slot_3, QMetaData::Private },
	{ "clear_display()", &slot_4, QMetaData::Private },
	{ "update_font()", &slot_5, QMetaData::Private },
	{ "save()", &slot_6, QMetaData::Private },
	{ "hide_editor()", &slot_7, QMetaData::Private },
	{ "help()", &slot_8, QMetaData::Private },
	{ "cancel()", &slot_9, QMetaData::Private },
	{ "data_selected()", &slot_10, QMetaData::Private },
	{ "data_selected(bool)", &slot_11, QMetaData::Private },
	{ "cb_changed()", &slot_12, QMetaData::Private },
	{ "cb_changed(bool)", &slot_13, QMetaData::Private },
	{ "cb_changed_ln()", &slot_14, QMetaData::Private },
	{ "cb_changed_ln(bool)", &slot_15, QMetaData::Private },
	{ "cb_changed_exp()", &slot_16, QMetaData::Private },
	{ "cb_changed_exp(bool)", &slot_17, QMetaData::Private },
	{ "toggle_points()", &slot_18, QMetaData::Private },
	{ "toggle_points_ln()", &slot_19, QMetaData::Private },
	{ "toggle_points_exp()", &slot_20, QMetaData::Private },
	{ "set_last_file(const QString&)", &slot_21, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_22, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Best", parentObject,
	slot_tbl, 23,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Best.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Best::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Best" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Best::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load(); break;
    case 1: join_results(); break;
    case 2: save_results(); break;
    case 3: hide_input(); break;
    case 4: clear_display(); break;
    case 5: update_font(); break;
    case 6: save(); break;
    case 7: hide_editor(); break;
    case 8: help(); break;
    case 9: cancel(); break;
    case 10: data_selected(); break;
    case 11: data_selected((bool)static_QUType_bool.get(_o+1)); break;
    case 12: cb_changed(); break;
    case 13: cb_changed((bool)static_QUType_bool.get(_o+1)); break;
    case 14: cb_changed_ln(); break;
    case 15: cb_changed_ln((bool)static_QUType_bool.get(_o+1)); break;
    case 16: cb_changed_exp(); break;
    case 17: cb_changed_exp((bool)static_QUType_bool.get(_o+1)); break;
    case 18: toggle_points(); break;
    case 19: toggle_points_ln(); break;
    case 20: toggle_points_exp(); break;
    case 21: set_last_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 22: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Best::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Best::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Best::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
