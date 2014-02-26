/****************************************************************************
** US_Hydrodyn_Batch_Movie_Opts meta object code from reading C++ file 'us_hydrodyn_batch_movie_opts.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_batch_movie_opts.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_Batch_Movie_Opts::className() const
{
    return "US_Hydrodyn_Batch_Movie_Opts";
}

QMetaObject *US_Hydrodyn_Batch_Movie_Opts::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Batch_Movie_Opts( "US_Hydrodyn_Batch_Movie_Opts", &US_Hydrodyn_Batch_Movie_Opts::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Batch_Movie_Opts::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Batch_Movie_Opts", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Batch_Movie_Opts::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Batch_Movie_Opts", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Batch_Movie_Opts::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_title", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_dir", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_file", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_fps", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_scale", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_tc_unit", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_tc_start", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_tc_delta", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_tc_pointsize", 1, param_slot_8 };
    static const QUMethod slot_9 = {"update_dir_msg", 0, 0 };
    static const QUMethod slot_10 = {"update_enables", 0, 0 };
    static const QUMethod slot_11 = {"set_clean_up", 0, 0 };
    static const QUMethod slot_12 = {"set_use_tc", 0, 0 };
    static const QUMethod slot_13 = {"set_black_background", 0, 0 };
    static const QUMethod slot_14 = {"cancel", 0, 0 };
    static const QUMethod slot_15 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update_title(const QString&)", &slot_0, QMetaData::Private },
	{ "update_dir(const QString&)", &slot_1, QMetaData::Private },
	{ "update_file(const QString&)", &slot_2, QMetaData::Private },
	{ "update_fps(const QString&)", &slot_3, QMetaData::Private },
	{ "update_scale(const QString&)", &slot_4, QMetaData::Private },
	{ "update_tc_unit(const QString&)", &slot_5, QMetaData::Private },
	{ "update_tc_start(const QString&)", &slot_6, QMetaData::Private },
	{ "update_tc_delta(const QString&)", &slot_7, QMetaData::Private },
	{ "update_tc_pointsize(const QString&)", &slot_8, QMetaData::Private },
	{ "update_dir_msg()", &slot_9, QMetaData::Private },
	{ "update_enables()", &slot_10, QMetaData::Private },
	{ "set_clean_up()", &slot_11, QMetaData::Private },
	{ "set_use_tc()", &slot_12, QMetaData::Private },
	{ "set_black_background()", &slot_13, QMetaData::Private },
	{ "cancel()", &slot_14, QMetaData::Private },
	{ "help()", &slot_15, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Batch_Movie_Opts", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Batch_Movie_Opts.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Batch_Movie_Opts::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Batch_Movie_Opts" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Batch_Movie_Opts::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_title((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_fps((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_scale((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_tc_unit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_tc_start((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_tc_delta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_tc_pointsize((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_dir_msg(); break;
    case 10: update_enables(); break;
    case 11: set_clean_up(); break;
    case 12: set_use_tc(); break;
    case 13: set_black_background(); break;
    case 14: cancel(); break;
    case 15: help(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Batch_Movie_Opts::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Batch_Movie_Opts::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Batch_Movie_Opts::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
