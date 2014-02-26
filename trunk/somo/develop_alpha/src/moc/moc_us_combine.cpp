/****************************************************************************
** US_Combine meta object code from reading C++ file 'us_combine.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_combine.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3Frame>
#include <QMouseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Combine::className() const
{
    return "US_Combine";
}

QMetaObject *US_Combine::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Combine( "US_Combine", &US_Combine::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Combine::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Combine", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Combine::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Combine", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Combine::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"show_cell", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"show_lambda", 1, param_slot_1 };
    static const QUMethod slot_2 = {"load_data", 0, 0 };
    static const QUMethod slot_3 = {"details", 0, 0 };
    static const QUMethod slot_4 = {"plotDistros", 0, 0 };
    static const QUMethod slot_5 = {"quit", 0, 0 };
    static const QUMethod slot_6 = {"set_colors", 0, 0 };
    static const QUMethod slot_7 = {"print", 0, 0 };
    static const QUMethod slot_8 = {"update_screen", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"add_distribution", 1, param_slot_9 };
    static const QUMethod slot_10 = {"reset", 0, 0 };
    static const QUMethod slot_11 = {"help", 0, 0 };
    static const QUMethod slot_12 = {"view", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_distribName", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_vbar", 1, param_slot_14 };
    static const QUMethod slot_15 = {"zoom_in", 0, 0 };
    static const QUMethod slot_16 = {"zoom_out", 0, 0 };
    static const QUMethod slot_17 = {"save", 0, 0 };
    static const QUMethod slot_18 = {"set_symbol", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_Xmin", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_Xmax", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_ff0", 1, param_slot_21 };
    static const QUMethod slot_22 = {"calc_limits", 0, 0 };
    static const QUMethod slot_23 = {"enable_all", 0, 0 };
    static const QUMethod slot_24 = {"disable_all", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"convert", 2, param_slot_25 };
    static const QUMethod slot_26 = {"read_vbar", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_vbar_lbl", 2, param_slot_27 };
    static const QUMethod slot_28 = {"select_integral", 0, 0 };
    static const QUMethod slot_29 = {"select_envelope", 0, 0 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_30 = {"select_method", 1, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_31 = {"setup_GUI", 1, param_slot_31 };
    static const QUParameter param_slot_32[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_32 = {"mousePressed", 1, param_slot_32 };
    static const QUParameter param_slot_33[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_33 = {"closeEvent", 1, param_slot_33 };
    static const QMetaData slot_tbl[] = {
	{ "show_cell(int)", &slot_0, QMetaData::Public },
	{ "show_lambda(int)", &slot_1, QMetaData::Public },
	{ "load_data()", &slot_2, QMetaData::Public },
	{ "details()", &slot_3, QMetaData::Public },
	{ "plotDistros()", &slot_4, QMetaData::Public },
	{ "quit()", &slot_5, QMetaData::Public },
	{ "set_colors()", &slot_6, QMetaData::Public },
	{ "print()", &slot_7, QMetaData::Public },
	{ "update_screen()", &slot_8, QMetaData::Public },
	{ "add_distribution(int)", &slot_9, QMetaData::Public },
	{ "reset()", &slot_10, QMetaData::Public },
	{ "help()", &slot_11, QMetaData::Public },
	{ "view()", &slot_12, QMetaData::Public },
	{ "update_distribName(const QString&)", &slot_13, QMetaData::Public },
	{ "update_vbar(const QString&)", &slot_14, QMetaData::Public },
	{ "zoom_in()", &slot_15, QMetaData::Public },
	{ "zoom_out()", &slot_16, QMetaData::Public },
	{ "save()", &slot_17, QMetaData::Public },
	{ "set_symbol()", &slot_18, QMetaData::Public },
	{ "update_Xmin(double)", &slot_19, QMetaData::Public },
	{ "update_Xmax(double)", &slot_20, QMetaData::Public },
	{ "update_ff0(double)", &slot_21, QMetaData::Public },
	{ "calc_limits()", &slot_22, QMetaData::Public },
	{ "enable_all()", &slot_23, QMetaData::Public },
	{ "disable_all()", &slot_24, QMetaData::Public },
	{ "convert(double)", &slot_25, QMetaData::Public },
	{ "read_vbar()", &slot_26, QMetaData::Public },
	{ "update_vbar_lbl(float,float)", &slot_27, QMetaData::Public },
	{ "select_integral()", &slot_28, QMetaData::Public },
	{ "select_envelope()", &slot_29, QMetaData::Public },
	{ "select_method(int)", &slot_30, QMetaData::Public },
	{ "setup_GUI(int)", &slot_31, QMetaData::Protected },
	{ "mousePressed(const QMouseEvent&)", &slot_32, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_33, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Combine", parentObject,
	slot_tbl, 34,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Combine.setMetaObject( metaObj );
    return metaObj;
}

void* US_Combine::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Combine" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Combine::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: show_cell((int)static_QUType_int.get(_o+1)); break;
    case 1: show_lambda((int)static_QUType_int.get(_o+1)); break;
    case 2: load_data(); break;
    case 3: details(); break;
    case 4: plotDistros(); break;
    case 5: quit(); break;
    case 6: set_colors(); break;
    case 7: print(); break;
    case 8: update_screen(); break;
    case 9: add_distribution((int)static_QUType_int.get(_o+1)); break;
    case 10: reset(); break;
    case 11: help(); break;
    case 12: view(); break;
    case 13: update_distribName((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: zoom_in(); break;
    case 16: zoom_out(); break;
    case 17: save(); break;
    case 18: set_symbol(); break;
    case 19: update_Xmin((double)static_QUType_double.get(_o+1)); break;
    case 20: update_Xmax((double)static_QUType_double.get(_o+1)); break;
    case 21: update_ff0((double)static_QUType_double.get(_o+1)); break;
    case 22: calc_limits(); break;
    case 23: enable_all(); break;
    case 24: disable_all(); break;
    case 25: static_QUType_double.set(_o,convert((double)static_QUType_double.get(_o+1))); break;
    case 26: read_vbar(); break;
    case 27: update_vbar_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 28: select_integral(); break;
    case 29: select_envelope(); break;
    case 30: select_method((int)static_QUType_int.get(_o+1)); break;
    case 31: setup_GUI((int)static_QUType_int.get(_o+1)); break;
    case 32: mousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 33: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Combine::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Combine::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Combine::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
