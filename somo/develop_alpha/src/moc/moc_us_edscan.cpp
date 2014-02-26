/****************************************************************************
** edit_single_F meta object code from reading C++ file 'us_edscan.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_edscan.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *edit_single_F::className() const
{
    return "edit_single_F";
}

QMetaObject *edit_single_F::metaObj = 0;
static QMetaObjectCleanUp cleanUp_edit_single_F( "edit_single_F", &edit_single_F::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString edit_single_F::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "edit_single_F", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString edit_single_F::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "edit_single_F", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* edit_single_F::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "help()", &slot_0, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"edit_single_F", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_edit_single_F.setMetaObject( metaObj );
    return metaObj;
}

void* edit_single_F::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "edit_single_F" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL clicked
void edit_single_F::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool edit_single_F::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: help(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool edit_single_F::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool edit_single_F::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool edit_single_F::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *edit_single_Win::className() const
{
    return "edit_single_Win";
}

QMetaObject *edit_single_Win::metaObj = 0;
static QMetaObjectCleanUp cleanUp_edit_single_Win( "edit_single_Win", &edit_single_Win::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString edit_single_Win::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "edit_single_Win", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString edit_single_Win::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "edit_single_Win", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* edit_single_Win::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"plotMousePressed", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"plotMouseReleased", 1, param_slot_1 };
    static const QUMethod slot_2 = {"zoom", 0, 0 };
    static const QUMethod slot_3 = {"update_plot", 0, 0 };
    static const QUMethod slot_4 = {"accept", 0, 0 };
    static const QUMethod slot_5 = {"cancel", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_6 = {"closeEvent", 1, param_slot_6 };
    static const QMetaData slot_tbl[] = {
	{ "plotMousePressed(const QMouseEvent&)", &slot_0, QMetaData::Private },
	{ "plotMouseReleased(const QMouseEvent&)", &slot_1, QMetaData::Private },
	{ "zoom()", &slot_2, QMetaData::Private },
	{ "update_plot()", &slot_3, QMetaData::Private },
	{ "accept()", &slot_4, QMetaData::Private },
	{ "cancel()", &slot_5, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_6, QMetaData::Private }
    };
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"edit_single_Win", parentObject,
	slot_tbl, 7,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_edit_single_Win.setMetaObject( metaObj );
    return metaObj;
}

void* edit_single_Win::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "edit_single_Win" ) )
	return this;
    return QWidget::qt_cast( clname );
}

// SIGNAL clicked
void edit_single_Win::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool edit_single_Win::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: plotMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 1: plotMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 2: zoom(); break;
    case 3: update_plot(); break;
    case 4: accept(); break;
    case 5: cancel(); break;
    case 6: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool edit_single_Win::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool edit_single_Win::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool edit_single_Win::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_ZoomFrame::className() const
{
    return "US_ZoomFrame";
}

QMetaObject *US_ZoomFrame::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ZoomFrame( "US_ZoomFrame", &US_ZoomFrame::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ZoomFrame::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ZoomFrame", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ZoomFrame::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ZoomFrame", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ZoomFrame::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "help()", &slot_0, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ZoomFrame", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ZoomFrame.setMetaObject( metaObj );
    return metaObj;
}

void* US_ZoomFrame::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ZoomFrame" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL clicked
void US_ZoomFrame::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_ZoomFrame::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: help(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ZoomFrame::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_ZoomFrame::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_ZoomFrame::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_ZoomWin::className() const
{
    return "US_ZoomWin";
}

QMetaObject *US_ZoomWin::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ZoomWin( "US_ZoomWin", &US_ZoomWin::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ZoomWin::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ZoomWin", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ZoomWin::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ZoomWin", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ZoomWin::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"plotMousePressed", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"plotMouseReleased", 1, param_slot_1 };
    static const QUMethod slot_2 = {"zoom", 0, 0 };
    static const QUMethod slot_3 = {"update_plot", 0, 0 };
    static const QUMethod slot_4 = {"accept", 0, 0 };
    static const QUMethod slot_5 = {"cancel", 0, 0 };
    static const QUMethod slot_6 = {"reset_zoom", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "plotMousePressed(const QMouseEvent&)", &slot_0, QMetaData::Private },
	{ "plotMouseReleased(const QMouseEvent&)", &slot_1, QMetaData::Private },
	{ "zoom()", &slot_2, QMetaData::Private },
	{ "update_plot()", &slot_3, QMetaData::Private },
	{ "accept()", &slot_4, QMetaData::Private },
	{ "cancel()", &slot_5, QMetaData::Private },
	{ "reset_zoom()", &slot_6, QMetaData::Private }
    };
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ZoomWin", parentObject,
	slot_tbl, 7,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ZoomWin.setMetaObject( metaObj );
    return metaObj;
}

void* US_ZoomWin::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ZoomWin" ) )
	return this;
    return QWidget::qt_cast( clname );
}

// SIGNAL clicked
void US_ZoomWin::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_ZoomWin::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: plotMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 1: plotMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 2: zoom(); break;
    case 3: update_plot(); break;
    case 4: accept(); break;
    case 5: cancel(); break;
    case 6: reset_zoom(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ZoomWin::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_ZoomWin::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_ZoomWin::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
