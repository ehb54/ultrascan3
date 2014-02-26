/****************************************************************************
** sm_dat_W meta object code from reading C++ file 'us_smdat.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_smdat.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *sm_dat_W::className() const
{
    return "sm_dat_W";
}

QMetaObject *sm_dat_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_sm_dat_W( "sm_dat_W", &sm_dat_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString sm_dat_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "sm_dat_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString sm_dat_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "sm_dat_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* sm_dat_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"write_sm", 0, 0 };
    static const QUMethod slot_2 = {"write_res", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_3 = {"plot_analysis", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_4 = {"setups", 1, param_slot_4 };
    static const QUMethod slot_5 = {"view", 0, 0 };
    static const QUMethod slot_6 = {"help", 0, 0 };
    static const QUMethod slot_7 = {"save", 0, 0 };
    static const QUMethod slot_8 = {"second_plot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "write_sm()", &slot_1, QMetaData::Private },
	{ "write_res()", &slot_2, QMetaData::Private },
	{ "plot_analysis()", &slot_3, QMetaData::Private },
	{ "setups()", &slot_4, QMetaData::Private },
	{ "view()", &slot_5, QMetaData::Private },
	{ "help()", &slot_6, QMetaData::Private },
	{ "save()", &slot_7, QMetaData::Private },
	{ "second_plot()", &slot_8, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"sm_dat_W", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_sm_dat_W.setMetaObject( metaObj );
    return metaObj;
}

void* sm_dat_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "sm_dat_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool sm_dat_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: write_sm(); break;
    case 2: write_res(); break;
    case 3: static_QUType_int.set(_o,plot_analysis()); break;
    case 4: static_QUType_int.set(_o,setups()); break;
    case 5: view(); break;
    case 6: help(); break;
    case 7: save(); break;
    case 8: second_plot(); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool sm_dat_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool sm_dat_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool sm_dat_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
