/****************************************************************************
** ScrollZoomer meta object code from reading C++ file 'scrollzoomer.h'
**
** Created: Wed Dec 4 19:14:47 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/qwt/scrollzoomer.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *ScrollZoomer::className() const
{
    return "ScrollZoomer";
}

QMetaObject *ScrollZoomer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_ScrollZoomer( "ScrollZoomer", &ScrollZoomer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString ScrollZoomer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ScrollZoomer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString ScrollZoomer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ScrollZoomer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* ScrollZoomer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QwtPlotZoomer::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "o", &static_QUType_ptr, "Qt::Orientation", QUParameter::In },
	{ "min", &static_QUType_double, 0, QUParameter::In },
	{ "max", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"scrollBarMoved", 3, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "scrollBarMoved(Qt::Orientation,double,double)", &slot_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"ScrollZoomer", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_ScrollZoomer.setMetaObject( metaObj );
    return metaObj;
}

void* ScrollZoomer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "ScrollZoomer" ) )
	return this;
    return QwtPlotZoomer::qt_cast( clname );
}

bool ScrollZoomer::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: scrollBarMoved((Qt::Orientation)(*((Qt::Orientation*)static_QUType_ptr.get(_o+1))),(double)static_QUType_double.get(_o+2),(double)static_QUType_double.get(_o+3)); break;
    default:
	return QwtPlotZoomer::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool ScrollZoomer::qt_emit( int _id, QUObject* _o )
{
    return QwtPlotZoomer::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool ScrollZoomer::qt_property( int id, int f, QVariant* v)
{
    return QwtPlotZoomer::qt_property( id, f, v);
}

bool ScrollZoomer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
