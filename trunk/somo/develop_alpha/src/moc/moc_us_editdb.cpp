/****************************************************************************
** US_Edit_DB meta object code from reading C++ file 'us_editdb.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_editdb.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Edit_DB::className() const
{
    return "US_Edit_DB";
}

QMetaObject *US_Edit_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Edit_DB( "US_Edit_DB", &US_Edit_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Edit_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Edit_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Edit_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Edit_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Edit_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = EditData_Win::staticMetaObject();
    static const QUMethod slot_0 = {"help", 0, 0 };
    static const QUMethod slot_1 = {"plot_dataset", 0, 0 };
    static const QUMethod slot_2 = {"review", 0, 0 };
    static const QUMethod slot_3 = {"load_dataset", 0, 0 };
    static const QUMethod slot_4 = {"update_data_uv", 0, 0 };
    static const QUMethod slot_5 = {"update_data_if", 0, 0 };
    static const QUMethod slot_6 = {"update_data_fl", 0, 0 };
    static const QUMethod slot_7 = {"update_data_veloc", 0, 0 };
    static const QUMethod slot_8 = {"update_data_equil", 0, 0 };
    static const QUMethod slot_9 = {"update_data_diff", 0, 0 };
    static const QUMethod slot_10 = {"update_data_intensity", 0, 0 };
    static const QUMethod slot_11 = {"update_data_wv", 0, 0 };
    static const QUMethod slot_12 = {"set_plot_axes", 0, 0 };
    static const QUMethod slot_13 = {"setup_GUI", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "help()", &slot_0, QMetaData::Public },
	{ "plot_dataset()", &slot_1, QMetaData::Public },
	{ "review()", &slot_2, QMetaData::Public },
	{ "load_dataset()", &slot_3, QMetaData::Public },
	{ "update_data_uv()", &slot_4, QMetaData::Public },
	{ "update_data_if()", &slot_5, QMetaData::Public },
	{ "update_data_fl()", &slot_6, QMetaData::Public },
	{ "update_data_veloc()", &slot_7, QMetaData::Public },
	{ "update_data_equil()", &slot_8, QMetaData::Public },
	{ "update_data_diff()", &slot_9, QMetaData::Public },
	{ "update_data_intensity()", &slot_10, QMetaData::Public },
	{ "update_data_wv()", &slot_11, QMetaData::Public },
	{ "set_plot_axes()", &slot_12, QMetaData::Public },
	{ "setup_GUI()", &slot_13, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"variablesUpdated", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "variablesUpdated()", &signal_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Edit_DB", parentObject,
	slot_tbl, 14,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Edit_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_Edit_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Edit_DB" ) )
	return this;
    return EditData_Win::qt_cast( clname );
}

// SIGNAL variablesUpdated
void US_Edit_DB::variablesUpdated()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_Edit_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: help(); break;
    case 1: plot_dataset(); break;
    case 2: review(); break;
    case 3: load_dataset(); break;
    case 4: update_data_uv(); break;
    case 5: update_data_if(); break;
    case 6: update_data_fl(); break;
    case 7: update_data_veloc(); break;
    case 8: update_data_equil(); break;
    case 9: update_data_diff(); break;
    case 10: update_data_intensity(); break;
    case 11: update_data_wv(); break;
    case 12: set_plot_axes(); break;
    case 13: setup_GUI(); break;
    default:
	return EditData_Win::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Edit_DB::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: variablesUpdated(); break;
    default:
	return EditData_Win::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Edit_DB::qt_property( int id, int f, QVariant* v)
{
    return EditData_Win::qt_property( id, f, v);
}

bool US_Edit_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
