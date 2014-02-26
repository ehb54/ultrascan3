/****************************************************************************
** ScrollBar meta object code from reading C++ file 'scrollbar.h'
**
** Created: Wed Dec 4 19:14:47 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/qwt/scrollbar.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *ScrollBar::className() const
{
    return "ScrollBar";
}

QMetaObject *ScrollBar::metaObj = 0;
static QMetaObjectCleanUp cleanUp_ScrollBar( "ScrollBar", &ScrollBar::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString ScrollBar::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ScrollBar", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString ScrollBar::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ScrollBar", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* ScrollBar::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QScrollBar::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "min", &static_QUType_double, 0, QUParameter::In },
	{ "max", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"setBase", 2, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "min", &static_QUType_double, 0, QUParameter::In },
	{ "max", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"moveSlider", 2, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "value", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"catchValueChanged", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "value", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"catchSliderMoved", 1, param_slot_3 };
    static const QMetaData slot_tbl[] = {
	{ "setBase(double,double)", &slot_0, QMetaData::Public },
	{ "moveSlider(double,double)", &slot_1, QMetaData::Public },
	{ "catchValueChanged(int)", &slot_2, QMetaData::Private },
	{ "catchSliderMoved(int)", &slot_3, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "Qt::Orientation", QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"sliderMoved", 3, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_ptr, "Qt::Orientation", QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"valueChanged", 3, param_signal_1 };
    static const QMetaData signal_tbl[] = {
	{ "sliderMoved(Qt::Orientation,double,double)", &signal_0, QMetaData::Public },
	{ "valueChanged(Qt::Orientation,double,double)", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"ScrollBar", parentObject,
	slot_tbl, 4,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_ScrollBar.setMetaObject( metaObj );
    return metaObj;
}

void* ScrollBar::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "ScrollBar" ) )
	return this;
    return QScrollBar::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL sliderMoved
void ScrollBar::sliderMoved( Qt::Orientation t0, double t1, double t2 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[4];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_double.set(o+2,t1);
    static_QUType_double.set(o+3,t2);
    activate_signal( clist, o );
}

// SIGNAL valueChanged
void ScrollBar::valueChanged( Qt::Orientation t0, double t1, double t2 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[4];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_double.set(o+2,t1);
    static_QUType_double.set(o+3,t2);
    activate_signal( clist, o );
}

bool ScrollBar::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setBase((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2)); break;
    case 1: moveSlider((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2)); break;
    case 2: catchValueChanged((int)static_QUType_int.get(_o+1)); break;
    case 3: catchSliderMoved((int)static_QUType_int.get(_o+1)); break;
    default:
	return QScrollBar::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool ScrollBar::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: sliderMoved((Qt::Orientation)(*((Qt::Orientation*)static_QUType_ptr.get(_o+1))),(double)static_QUType_double.get(_o+2),(double)static_QUType_double.get(_o+3)); break;
    case 1: valueChanged((Qt::Orientation)(*((Qt::Orientation*)static_QUType_ptr.get(_o+1))),(double)static_QUType_double.get(_o+2),(double)static_QUType_double.get(_o+3)); break;
    default:
	return QScrollBar::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool ScrollBar::qt_property( int id, int f, QVariant* v)
{
    return QScrollBar::qt_property( id, f, v);
}

bool ScrollBar::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
