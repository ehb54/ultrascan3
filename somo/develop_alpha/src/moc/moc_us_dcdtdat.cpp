/****************************************************************************
** dcdt_dat_W meta object code from reading C++ file 'us_dcdtdat.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_dcdtdat.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *dcdt_dat_W::className() const
{
    return "dcdt_dat_W";
}

QMetaObject *dcdt_dat_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_dcdt_dat_W( "dcdt_dat_W", &dcdt_dat_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString dcdt_dat_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "dcdt_dat_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString dcdt_dat_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "dcdt_dat_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* dcdt_dat_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"write_dcdt_avg", 0, 0 };
    static const QUMethod slot_2 = {"write_dcdt_scans", 0, 0 };
    static const QUMethod slot_3 = {"write_dcdt_sval", 0, 0 };
    static const QUMethod slot_4 = {"write_res", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_sval", 1, param_slot_5 };
    static const QUMethod slot_6 = {"update_r_button", 0, 0 };
    static const QUMethod slot_7 = {"update_s_button", 0, 0 };
    static const QUMethod slot_8 = {"cleanup", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_9 = {"plot_analysis", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_10 = {"setups", 1, param_slot_10 };
    static const QUMethod slot_11 = {"view", 0, 0 };
    static const QUMethod slot_12 = {"help", 0, 0 };
    static const QUMethod slot_13 = {"save", 0, 0 };
    static const QUMethod slot_14 = {"second_plot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "write_dcdt_avg()", &slot_1, QMetaData::Private },
	{ "write_dcdt_scans()", &slot_2, QMetaData::Private },
	{ "write_dcdt_sval()", &slot_3, QMetaData::Private },
	{ "write_res()", &slot_4, QMetaData::Private },
	{ "update_sval(double)", &slot_5, QMetaData::Private },
	{ "update_r_button()", &slot_6, QMetaData::Private },
	{ "update_s_button()", &slot_7, QMetaData::Private },
	{ "cleanup()", &slot_8, QMetaData::Private },
	{ "plot_analysis()", &slot_9, QMetaData::Private },
	{ "setups()", &slot_10, QMetaData::Private },
	{ "view()", &slot_11, QMetaData::Private },
	{ "help()", &slot_12, QMetaData::Private },
	{ "save()", &slot_13, QMetaData::Private },
	{ "second_plot()", &slot_14, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"dcdt_dat_W", parentObject,
	slot_tbl, 15,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_dcdt_dat_W.setMetaObject( metaObj );
    return metaObj;
}

void* dcdt_dat_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "dcdt_dat_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool dcdt_dat_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: write_dcdt_avg(); break;
    case 2: write_dcdt_scans(); break;
    case 3: write_dcdt_sval(); break;
    case 4: write_res(); break;
    case 5: update_sval((double)static_QUType_double.get(_o+1)); break;
    case 6: update_r_button(); break;
    case 7: update_s_button(); break;
    case 8: cleanup(); break;
    case 9: static_QUType_int.set(_o,plot_analysis()); break;
    case 10: static_QUType_int.set(_o,setups()); break;
    case 11: view(); break;
    case 12: help(); break;
    case 13: save(); break;
    case 14: second_plot(); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool dcdt_dat_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool dcdt_dat_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool dcdt_dat_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
