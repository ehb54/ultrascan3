/****************************************************************************
** US_2dPlot meta object code from reading C++ file 'us_2dplot.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_2dplot.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_2dPlot::className() const
{
    return "US_2dPlot";
}

QMetaObject *US_2dPlot::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_2dPlot( "US_2dPlot", &US_2dPlot::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_2dPlot::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_2dPlot", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_2dPlot::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_2dPlot", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_2dPlot::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QwtPlot::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"closeEvent", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "closeEvent(QCloseEvent*)", &slot_0, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"plotClosed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "plotClosed()", &signal_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_2dPlot", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_2dPlot.setMetaObject( metaObj );
    return metaObj;
}

void* US_2dPlot::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_2dPlot" ) )
	return this;
    return QwtPlot::qt_cast( clname );
}

// SIGNAL plotClosed
void US_2dPlot::plotClosed()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_2dPlot::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QwtPlot::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_2dPlot::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: plotClosed(); break;
    default:
	return QwtPlot::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_2dPlot::qt_property( int id, int f, QVariant* v)
{
    return QwtPlot::qt_property( id, f, v);
}

bool US_2dPlot::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
