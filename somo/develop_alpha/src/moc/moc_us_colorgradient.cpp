/****************************************************************************
** US_GradientWidget meta object code from reading C++ file 'us_colorgradient.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_colorgradient.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QLabel>
#include <QCloseEvent>
#include <QPaintEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_GradientWidget::className() const
{
    return "US_GradientWidget";
}

QMetaObject *US_GradientWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_GradientWidget( "US_GradientWidget", &US_GradientWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_GradientWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GradientWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_GradientWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GradientWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_GradientWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QLabel::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "QPaintEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"paintEvent", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "paintEvent(QPaintEvent*)", &slot_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_GradientWidget", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_GradientWidget.setMetaObject( metaObj );
    return metaObj;
}

void* US_GradientWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_GradientWidget" ) )
	return this;
    return QLabel::qt_cast( clname );
}

bool US_GradientWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: paintEvent((QPaintEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QLabel::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_GradientWidget::qt_emit( int _id, QUObject* _o )
{
    return QLabel::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_GradientWidget::qt_property( int id, int f, QVariant* v)
{
    return QLabel::qt_property( id, f, v);
}

bool US_GradientWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_ColorGradient::className() const
{
    return "US_ColorGradient";
}

QMetaObject *US_ColorGradient::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ColorGradient( "US_ColorGradient", &US_ColorGradient::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ColorGradient::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ColorGradient", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ColorGradient::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ColorGradient", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ColorGradient::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"select_startcolor", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_colors", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_step", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_points", 1, param_slot_3 };
    static const QUMethod slot_4 = {"update_stepcolor", 0, 0 };
    static const QUMethod slot_5 = {"update_array", 0, 0 };
    static const QUMethod slot_6 = {"help", 0, 0 };
    static const QUMethod slot_7 = {"reset", 0, 0 };
    static const QUMethod slot_8 = {"save", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_9 = {"closeEvent", 1, param_slot_9 };
    static const QMetaData slot_tbl[] = {
	{ "select_startcolor()", &slot_0, QMetaData::Public },
	{ "update_colors(double)", &slot_1, QMetaData::Public },
	{ "update_step(double)", &slot_2, QMetaData::Public },
	{ "update_points(double)", &slot_3, QMetaData::Public },
	{ "update_stepcolor()", &slot_4, QMetaData::Public },
	{ "update_array()", &slot_5, QMetaData::Public },
	{ "help()", &slot_6, QMetaData::Public },
	{ "reset()", &slot_7, QMetaData::Public },
	{ "save()", &slot_8, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_9, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ColorGradient", parentObject,
	slot_tbl, 10,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ColorGradient.setMetaObject( metaObj );
    return metaObj;
}

void* US_ColorGradient::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ColorGradient" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_ColorGradient::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: select_startcolor(); break;
    case 1: update_colors((double)static_QUType_double.get(_o+1)); break;
    case 2: update_step((double)static_QUType_double.get(_o+1)); break;
    case 3: update_points((double)static_QUType_double.get(_o+1)); break;
    case 4: update_stepcolor(); break;
    case 5: update_array(); break;
    case 6: help(); break;
    case 7: reset(); break;
    case 8: save(); break;
    case 9: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ColorGradient::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_ColorGradient::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_ColorGradient::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
