/****************************************************************************
** US_lncr2_W meta object code from reading C++ file 'us_lncr2.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_lncr2.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_lncr2_W::className() const
{
    return "US_lncr2_W";
}

QMetaObject *US_lncr2_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_lncr2_W( "US_lncr2_W", &US_lncr2_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_lncr2_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_lncr2_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_lncr2_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_lncr2_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_lncr2_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"write_lncr2", 0, 0 };
    static const QUMethod slot_2 = {"write_res", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_baseline", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_4 = {"plot_analysis", 1, param_slot_4 };
    static const QUMethod slot_5 = {"view", 0, 0 };
    static const QUMethod slot_6 = {"help", 0, 0 };
    static const QUMethod slot_7 = {"save", 0, 0 };
    static const QUMethod slot_8 = {"second_plot", 0, 0 };
    static const QUMethod slot_9 = {"reset", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "write_lncr2()", &slot_1, QMetaData::Private },
	{ "write_res()", &slot_2, QMetaData::Private },
	{ "update_baseline(double)", &slot_3, QMetaData::Private },
	{ "plot_analysis()", &slot_4, QMetaData::Private },
	{ "view()", &slot_5, QMetaData::Private },
	{ "help()", &slot_6, QMetaData::Private },
	{ "save()", &slot_7, QMetaData::Private },
	{ "second_plot()", &slot_8, QMetaData::Private },
	{ "reset()", &slot_9, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_lncr2_W", parentObject,
	slot_tbl, 10,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_lncr2_W.setMetaObject( metaObj );
    return metaObj;
}

void* US_lncr2_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_lncr2_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool US_lncr2_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: write_lncr2(); break;
    case 2: write_res(); break;
    case 3: update_baseline((double)static_QUType_double.get(_o+1)); break;
    case 4: static_QUType_int.set(_o,plot_analysis()); break;
    case 5: view(); break;
    case 6: help(); break;
    case 7: save(); break;
    case 8: second_plot(); break;
    case 9: reset(); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_lncr2_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_lncr2_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool US_lncr2_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
