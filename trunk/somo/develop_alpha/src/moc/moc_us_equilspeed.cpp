/****************************************************************************
** US_EquilSpeed meta object code from reading C++ file 'us_equilspeed.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_equilspeed.h"
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

const char *US_EquilSpeed::className() const
{
    return "US_EquilSpeed";
}

QMetaObject *US_EquilSpeed::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_EquilSpeed( "US_EquilSpeed", &US_EquilSpeed::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_EquilSpeed::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EquilSpeed", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_EquilSpeed::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EquilSpeed", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_EquilSpeed::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"quit", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"read_vbar", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_vbar", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_density", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_vbar_lbl", 2, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_buffer_lbl", 2, param_slot_6 };
    static const QUMethod slot_7 = {"get_buffer", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_mw", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_volume", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_temperature", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_sigma", 1, param_slot_11 };
    static const QUMethod slot_12 = {"calc_correction", 0, 0 };
    static const QUMethod slot_13 = {"animate1", 0, 0 };
    static const QUMethod slot_14 = {"animate2", 0, 0 };
    static const QUMethod slot_15 = {"calc", 0, 0 };
    static const QUMethod slot_16 = {"valid", 0, 0 };
    static const QUMethod slot_17 = {"update_epon", 0, 0 };
    static const QUMethod slot_18 = {"update_aluminum", 0, 0 };
    static const QUMethod slot_19 = {"update_an50", 0, 0 };
    static const QUMethod slot_20 = {"update_an60", 0, 0 };
    static const QUMethod slot_21 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_22 = {"closeEvent", 1, param_slot_22 };
    static const QMetaData slot_tbl[] = {
	{ "quit()", &slot_0, QMetaData::Private },
	{ "help()", &slot_1, QMetaData::Private },
	{ "read_vbar()", &slot_2, QMetaData::Private },
	{ "update_vbar(const QString&)", &slot_3, QMetaData::Private },
	{ "update_density(const QString&)", &slot_4, QMetaData::Private },
	{ "update_vbar_lbl(float,float)", &slot_5, QMetaData::Private },
	{ "update_buffer_lbl(float,float)", &slot_6, QMetaData::Private },
	{ "get_buffer()", &slot_7, QMetaData::Private },
	{ "update_mw(double)", &slot_8, QMetaData::Private },
	{ "update_volume(const QString&)", &slot_9, QMetaData::Private },
	{ "update_temperature(const QString&)", &slot_10, QMetaData::Private },
	{ "update_sigma(double)", &slot_11, QMetaData::Private },
	{ "calc_correction()", &slot_12, QMetaData::Private },
	{ "animate1()", &slot_13, QMetaData::Private },
	{ "animate2()", &slot_14, QMetaData::Private },
	{ "calc()", &slot_15, QMetaData::Private },
	{ "valid()", &slot_16, QMetaData::Private },
	{ "update_epon()", &slot_17, QMetaData::Private },
	{ "update_aluminum()", &slot_18, QMetaData::Private },
	{ "update_an50()", &slot_19, QMetaData::Private },
	{ "update_an60()", &slot_20, QMetaData::Private },
	{ "setup_GUI()", &slot_21, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_22, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_EquilSpeed", parentObject,
	slot_tbl, 23,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_EquilSpeed.setMetaObject( metaObj );
    return metaObj;
}

void* US_EquilSpeed::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_EquilSpeed" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_EquilSpeed::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: quit(); break;
    case 1: help(); break;
    case 2: read_vbar(); break;
    case 3: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_density((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_vbar_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 6: update_buffer_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 7: get_buffer(); break;
    case 8: update_mw((double)static_QUType_double.get(_o+1)); break;
    case 9: update_volume((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_temperature((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_sigma((double)static_QUType_double.get(_o+1)); break;
    case 12: calc_correction(); break;
    case 13: animate1(); break;
    case 14: animate2(); break;
    case 15: calc(); break;
    case 16: valid(); break;
    case 17: update_epon(); break;
    case 18: update_aluminum(); break;
    case 19: update_an50(); break;
    case 20: update_an60(); break;
    case 21: setup_GUI(); break;
    case 22: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_EquilSpeed::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_EquilSpeed::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_EquilSpeed::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
