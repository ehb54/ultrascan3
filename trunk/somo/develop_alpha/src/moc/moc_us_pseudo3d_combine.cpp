/****************************************************************************
** US_Pseudo3D_Combine meta object code from reading C++ file 'us_pseudo3d_combine.h'
**
** Created: Tue Feb 25 10:28:36 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_pseudo3d_combine.h"
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

const char *US_Pseudo3D_Combine::className() const
{
    return "US_Pseudo3D_Combine";
}

QMetaObject *US_Pseudo3D_Combine::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Pseudo3D_Combine( "US_Pseudo3D_Combine", &US_Pseudo3D_Combine::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Pseudo3D_Combine::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Pseudo3D_Combine", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Pseudo3D_Combine::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Pseudo3D_Combine", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Pseudo3D_Combine::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_plot_smax", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_plot_smin", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_plot_fmax", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_plot_fmin", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_resolution", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_x_resolution", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_y_resolution", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_x_pixel", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_y_pixel", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_current_distro", 1, param_slot_9 };
    static const QUMethod slot_10 = {"plot_3dim", 0, 0 };
    static const QUMethod slot_11 = {"loop", 0, 0 };
    static const QUMethod slot_12 = {"select_autolimit", 0, 0 };
    static const QUMethod slot_13 = {"select_plot_s", 0, 0 };
    static const QUMethod slot_14 = {"select_plot_mw", 0, 0 };
    static const QUMethod slot_15 = {"load_distro", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"load_distro", 1, param_slot_16 };
    static const QUMethod slot_17 = {"load_color", 0, 0 };
    static const QUMethod slot_18 = {"help", 0, 0 };
    static const QUMethod slot_19 = {"save", 0, 0 };
    static const QUMethod slot_20 = {"stop", 0, 0 };
    static const QUMethod slot_21 = {"print", 0, 0 };
    static const QUMethod slot_22 = {"reset", 0, 0 };
    static const QUMethod slot_23 = {"set_limits", 0, 0 };
    static const QUMethod slot_24 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_25 = {"closeEvent", 1, param_slot_25 };
    static const QMetaData slot_tbl[] = {
	{ "update_plot_smax(double)", &slot_0, QMetaData::Private },
	{ "update_plot_smin(double)", &slot_1, QMetaData::Private },
	{ "update_plot_fmax(double)", &slot_2, QMetaData::Private },
	{ "update_plot_fmin(double)", &slot_3, QMetaData::Private },
	{ "update_resolution(double)", &slot_4, QMetaData::Private },
	{ "update_x_resolution(double)", &slot_5, QMetaData::Private },
	{ "update_y_resolution(double)", &slot_6, QMetaData::Private },
	{ "update_x_pixel(double)", &slot_7, QMetaData::Private },
	{ "update_y_pixel(double)", &slot_8, QMetaData::Private },
	{ "update_current_distro(double)", &slot_9, QMetaData::Private },
	{ "plot_3dim()", &slot_10, QMetaData::Private },
	{ "loop()", &slot_11, QMetaData::Private },
	{ "select_autolimit()", &slot_12, QMetaData::Private },
	{ "select_plot_s()", &slot_13, QMetaData::Private },
	{ "select_plot_mw()", &slot_14, QMetaData::Private },
	{ "load_distro()", &slot_15, QMetaData::Private },
	{ "load_distro(const QString&)", &slot_16, QMetaData::Private },
	{ "load_color()", &slot_17, QMetaData::Private },
	{ "help()", &slot_18, QMetaData::Private },
	{ "save()", &slot_19, QMetaData::Private },
	{ "stop()", &slot_20, QMetaData::Private },
	{ "print()", &slot_21, QMetaData::Private },
	{ "reset()", &slot_22, QMetaData::Private },
	{ "set_limits()", &slot_23, QMetaData::Private },
	{ "setup_GUI()", &slot_24, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_25, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Pseudo3D_Combine", parentObject,
	slot_tbl, 26,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Pseudo3D_Combine.setMetaObject( metaObj );
    return metaObj;
}

void* US_Pseudo3D_Combine::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Pseudo3D_Combine" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Pseudo3D_Combine::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_plot_smax((double)static_QUType_double.get(_o+1)); break;
    case 1: update_plot_smin((double)static_QUType_double.get(_o+1)); break;
    case 2: update_plot_fmax((double)static_QUType_double.get(_o+1)); break;
    case 3: update_plot_fmin((double)static_QUType_double.get(_o+1)); break;
    case 4: update_resolution((double)static_QUType_double.get(_o+1)); break;
    case 5: update_x_resolution((double)static_QUType_double.get(_o+1)); break;
    case 6: update_y_resolution((double)static_QUType_double.get(_o+1)); break;
    case 7: update_x_pixel((double)static_QUType_double.get(_o+1)); break;
    case 8: update_y_pixel((double)static_QUType_double.get(_o+1)); break;
    case 9: update_current_distro((double)static_QUType_double.get(_o+1)); break;
    case 10: plot_3dim(); break;
    case 11: loop(); break;
    case 12: select_autolimit(); break;
    case 13: select_plot_s(); break;
    case 14: select_plot_mw(); break;
    case 15: load_distro(); break;
    case 16: load_distro((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: load_color(); break;
    case 18: help(); break;
    case 19: save(); break;
    case 20: stop(); break;
    case 21: print(); break;
    case 22: reset(); break;
    case 23: set_limits(); break;
    case 24: setup_GUI(); break;
    case 25: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Pseudo3D_Combine::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Pseudo3D_Combine::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Pseudo3D_Combine::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
