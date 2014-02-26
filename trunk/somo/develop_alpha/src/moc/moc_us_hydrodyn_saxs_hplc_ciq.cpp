/****************************************************************************
** US_Hydrodyn_Saxs_Hplc_Ciq meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_ciq.h'
**
** Created: Tue Feb 25 10:28:33 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_hplc_ciq.h"
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

const char *US_Hydrodyn_Saxs_Hplc_Ciq::className() const
{
    return "US_Hydrodyn_Saxs_Hplc_Ciq";
}

QMetaObject *US_Hydrodyn_Saxs_Hplc_Ciq::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Hplc_Ciq( "US_Hydrodyn_Saxs_Hplc_Ciq", &US_Hydrodyn_Saxs_Hplc_Ciq::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Hplc_Ciq::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Ciq", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Hplc_Ciq::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Ciq", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Hplc_Ciq::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"set_add_bl", 0, 0 };
    static const QUMethod slot_1 = {"set_save_as_pct_iq", 0, 0 };
    static const QUMethod slot_2 = {"set_sd_source", 0, 0 };
    static const QUMethod slot_3 = {"set_sd_zero_avg_local_sd", 0, 0 };
    static const QUMethod slot_4 = {"set_sd_zero_keep_as_zeros", 0, 0 };
    static const QUMethod slot_5 = {"set_sd_zero_set_to_pt1pct", 0, 0 };
    static const QUMethod slot_6 = {"zeros_found", 0, 0 };
    static const QUMethod slot_7 = {"set_zero_drop_points", 0, 0 };
    static const QUMethod slot_8 = {"set_zero_avg_local_sd", 0, 0 };
    static const QUMethod slot_9 = {"set_zero_keep_as_zeros", 0, 0 };
    static const QUMethod slot_10 = {"set_normalize", 0, 0 };
    static const QUMethod slot_11 = {"set_I0se", 0, 0 };
    static const QUMethod slot_12 = {"update_enables", 0, 0 };
    static const QUMethod slot_13 = {"global", 0, 0 };
    static const QUMethod slot_14 = {"help", 0, 0 };
    static const QUMethod slot_15 = {"quit", 0, 0 };
    static const QUMethod slot_16 = {"create_ng", 0, 0 };
    static const QUMethod slot_17 = {"go", 0, 0 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_18 = {"closeEvent", 1, param_slot_18 };
    static const QMetaData slot_tbl[] = {
	{ "set_add_bl()", &slot_0, QMetaData::Private },
	{ "set_save_as_pct_iq()", &slot_1, QMetaData::Private },
	{ "set_sd_source()", &slot_2, QMetaData::Private },
	{ "set_sd_zero_avg_local_sd()", &slot_3, QMetaData::Private },
	{ "set_sd_zero_keep_as_zeros()", &slot_4, QMetaData::Private },
	{ "set_sd_zero_set_to_pt1pct()", &slot_5, QMetaData::Private },
	{ "zeros_found()", &slot_6, QMetaData::Private },
	{ "set_zero_drop_points()", &slot_7, QMetaData::Private },
	{ "set_zero_avg_local_sd()", &slot_8, QMetaData::Private },
	{ "set_zero_keep_as_zeros()", &slot_9, QMetaData::Private },
	{ "set_normalize()", &slot_10, QMetaData::Private },
	{ "set_I0se()", &slot_11, QMetaData::Private },
	{ "update_enables()", &slot_12, QMetaData::Private },
	{ "global()", &slot_13, QMetaData::Private },
	{ "help()", &slot_14, QMetaData::Private },
	{ "quit()", &slot_15, QMetaData::Private },
	{ "create_ng()", &slot_16, QMetaData::Private },
	{ "go()", &slot_17, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_18, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Hplc_Ciq", parentObject,
	slot_tbl, 19,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Hplc_Ciq.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Hplc_Ciq::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Hplc_Ciq" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Hplc_Ciq::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: set_add_bl(); break;
    case 1: set_save_as_pct_iq(); break;
    case 2: set_sd_source(); break;
    case 3: set_sd_zero_avg_local_sd(); break;
    case 4: set_sd_zero_keep_as_zeros(); break;
    case 5: set_sd_zero_set_to_pt1pct(); break;
    case 6: zeros_found(); break;
    case 7: set_zero_drop_points(); break;
    case 8: set_zero_avg_local_sd(); break;
    case 9: set_zero_keep_as_zeros(); break;
    case 10: set_normalize(); break;
    case 11: set_I0se(); break;
    case 12: update_enables(); break;
    case 13: global(); break;
    case 14: help(); break;
    case 15: quit(); break;
    case 16: create_ng(); break;
    case 17: go(); break;
    case 18: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Hplc_Ciq::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Hplc_Ciq::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Hplc_Ciq::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
