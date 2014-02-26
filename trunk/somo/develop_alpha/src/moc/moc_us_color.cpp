/****************************************************************************
** US_Color meta object code from reading C++ file 'us_color.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_color.h"
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

const char *US_Color::className() const
{
    return "US_Color";
}

QMetaObject *US_Color::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Color( "US_Color", &US_Color::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Color::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Color", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Color::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Color", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Color::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"set_default", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"sel_margin", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_widgets", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_save_str", 1, param_slot_5 };
    static const QUMethod slot_6 = {"apply", 0, 0 };
    static const QUMethod slot_7 = {"reset", 0, 0 };
    static const QUMethod slot_8 = {"quit", 0, 0 };
    static const QUMethod slot_9 = {"save_as", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"selected_scheme", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"selected_item", 1, param_slot_11 };
    static const QUMethod slot_12 = {"pick_color1", 0, 0 };
    static const QUMethod slot_13 = {"pick_color2", 0, 0 };
    static const QUMethod slot_14 = {"pick_color3", 0, 0 };
    static const QUMethod slot_15 = {"pick_color4", 0, 0 };
    static const QUMethod slot_16 = {"pick_color5", 0, 0 };
    static const QUMethod slot_17 = {"pick_color6", 0, 0 };
    static const QUMethod slot_18 = {"delete_scheme", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_19 = {"closeEvent", 1, param_slot_19 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Public },
	{ "help()", &slot_1, QMetaData::Public },
	{ "set_default()", &slot_2, QMetaData::Public },
	{ "sel_margin(int)", &slot_3, QMetaData::Public },
	{ "update_widgets(double)", &slot_4, QMetaData::Public },
	{ "update_save_str(const QString&)", &slot_5, QMetaData::Public },
	{ "apply()", &slot_6, QMetaData::Public },
	{ "reset()", &slot_7, QMetaData::Public },
	{ "quit()", &slot_8, QMetaData::Public },
	{ "save_as()", &slot_9, QMetaData::Public },
	{ "selected_scheme(int)", &slot_10, QMetaData::Public },
	{ "selected_item(int)", &slot_11, QMetaData::Public },
	{ "pick_color1()", &slot_12, QMetaData::Public },
	{ "pick_color2()", &slot_13, QMetaData::Public },
	{ "pick_color3()", &slot_14, QMetaData::Public },
	{ "pick_color4()", &slot_15, QMetaData::Public },
	{ "pick_color5()", &slot_16, QMetaData::Public },
	{ "pick_color6()", &slot_17, QMetaData::Public },
	{ "delete_scheme()", &slot_18, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_19, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"marginChanged", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "marginChanged(int)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Color", parentObject,
	slot_tbl, 20,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Color.setMetaObject( metaObj );
    return metaObj;
}

void* US_Color::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Color" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL marginChanged
void US_Color::marginChanged( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool US_Color::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: help(); break;
    case 2: set_default(); break;
    case 3: sel_margin((int)static_QUType_int.get(_o+1)); break;
    case 4: update_widgets((double)static_QUType_double.get(_o+1)); break;
    case 5: update_save_str((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: apply(); break;
    case 7: reset(); break;
    case 8: quit(); break;
    case 9: save_as(); break;
    case 10: selected_scheme((int)static_QUType_int.get(_o+1)); break;
    case 11: selected_item((int)static_QUType_int.get(_o+1)); break;
    case 12: pick_color1(); break;
    case 13: pick_color2(); break;
    case 14: pick_color3(); break;
    case 15: pick_color4(); break;
    case 16: pick_color5(); break;
    case 17: pick_color6(); break;
    case 18: delete_scheme(); break;
    case 19: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Color::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: marginChanged((int)static_QUType_int.get(_o+1)); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Color::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Color::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
