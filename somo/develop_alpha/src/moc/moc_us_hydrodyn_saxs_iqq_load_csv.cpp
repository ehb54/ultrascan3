/****************************************************************************
** US_Hydrodyn_Saxs_Iqq_Load_Csv meta object code from reading C++ file 'us_hydrodyn_saxs_iqq_load_csv.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_iqq_load_csv.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Saxs_Iqq_Load_Csv::className() const
{
    return "US_Hydrodyn_Saxs_Iqq_Load_Csv";
}

QMetaObject *US_Hydrodyn_Saxs_Iqq_Load_Csv::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Iqq_Load_Csv( "US_Hydrodyn_Saxs_Iqq_Load_Csv", &US_Hydrodyn_Saxs_Iqq_Load_Csv::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Iqq_Load_Csv::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Iqq_Load_Csv", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Iqq_Load_Csv::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Iqq_Load_Csv", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Iqq_Load_Csv::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"update_selected", 0, 0 };
    static const QUMethod slot_1 = {"set_create_avg", 0, 0 };
    static const QUMethod slot_2 = {"set_create_std_dev", 0, 0 };
    static const QUMethod slot_3 = {"set_only_plot_stats", 0, 0 };
    static const QUMethod slot_4 = {"set_save_to_csv", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_csv_filename", 1, param_slot_5 };
    static const QUMethod slot_6 = {"set_save_original_data", 0, 0 };
    static const QUMethod slot_7 = {"set_run_nnls", 0, 0 };
    static const QUMethod slot_8 = {"set_run_best_fit", 0, 0 };
    static const QUMethod slot_9 = {"select_all", 0, 0 };
    static const QUMethod slot_10 = {"select_target", 0, 0 };
    static const QUMethod slot_11 = {"transpose", 0, 0 };
    static const QUMethod slot_12 = {"save_as_dat", 0, 0 };
    static const QUMethod slot_13 = {"save_selected", 0, 0 };
    static const QUMethod slot_14 = {"ok", 0, 0 };
    static const QUMethod slot_15 = {"cancel", 0, 0 };
    static const QUMethod slot_16 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_selected()", &slot_0, QMetaData::Private },
	{ "set_create_avg()", &slot_1, QMetaData::Private },
	{ "set_create_std_dev()", &slot_2, QMetaData::Private },
	{ "set_only_plot_stats()", &slot_3, QMetaData::Private },
	{ "set_save_to_csv()", &slot_4, QMetaData::Private },
	{ "update_csv_filename(const QString&)", &slot_5, QMetaData::Private },
	{ "set_save_original_data()", &slot_6, QMetaData::Private },
	{ "set_run_nnls()", &slot_7, QMetaData::Private },
	{ "set_run_best_fit()", &slot_8, QMetaData::Private },
	{ "select_all()", &slot_9, QMetaData::Private },
	{ "select_target()", &slot_10, QMetaData::Private },
	{ "transpose()", &slot_11, QMetaData::Private },
	{ "save_as_dat()", &slot_12, QMetaData::Private },
	{ "save_selected()", &slot_13, QMetaData::Private },
	{ "ok()", &slot_14, QMetaData::Private },
	{ "cancel()", &slot_15, QMetaData::Private },
	{ "help()", &slot_16, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Iqq_Load_Csv", parentObject,
	slot_tbl, 17,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Iqq_Load_Csv.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Iqq_Load_Csv::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Iqq_Load_Csv" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Iqq_Load_Csv::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_selected(); break;
    case 1: set_create_avg(); break;
    case 2: set_create_std_dev(); break;
    case 3: set_only_plot_stats(); break;
    case 4: set_save_to_csv(); break;
    case 5: update_csv_filename((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: set_save_original_data(); break;
    case 7: set_run_nnls(); break;
    case 8: set_run_best_fit(); break;
    case 9: select_all(); break;
    case 10: select_target(); break;
    case 11: transpose(); break;
    case 12: save_as_dat(); break;
    case 13: save_selected(); break;
    case 14: ok(); break;
    case 15: cancel(); break;
    case 16: help(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Iqq_Load_Csv::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Iqq_Load_Csv::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Iqq_Load_Csv::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
