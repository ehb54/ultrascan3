/****************************************************************************
** US_Average_Legend_F meta object code from reading C++ file 'us_average.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_average.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Average_Legend_F::className() const
{
    return "US_Average_Legend_F";
}

QMetaObject *US_Average_Legend_F::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Average_Legend_F( "US_Average_Legend_F", &US_Average_Legend_F::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Average_Legend_F::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Average_Legend_F", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Average_Legend_F::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Average_Legend_F", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Average_Legend_F::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Average_Legend_F", parentObject,
	0, 0,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Average_Legend_F.setMetaObject( metaObj );
    return metaObj;
}

void* US_Average_Legend_F::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Average_Legend_F" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL clicked
void US_Average_Legend_F::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool US_Average_Legend_F::qt_invoke( int _id, QUObject* _o )
{
    return Q3Frame::qt_invoke(_id,_o);
}

bool US_Average_Legend_F::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Average_Legend_F::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Average_Legend_F::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_Average::className() const
{
    return "US_Average";
}

QMetaObject *US_Average::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Average( "US_Average", &US_Average::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Average::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Average", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Average::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Average", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Average::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"expand", 0, 0 };
    static const QUMethod slot_1 = {"compress", 0, 0 };
    static const QUMethod slot_2 = {"close_dis", 0, 0 };
    static const QUMethod slot_3 = {"save_avg", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"save_avg", 1, param_slot_4 };
    static const QUMethod slot_5 = {"calc", 0, 0 };
    static const QUMethod slot_6 = {"update_pixmap", 0, 0 };
    static const QUMethod slot_7 = {"print_plot", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ "e", &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_8 = {"resizeEvent", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_9 = {"closeEvent", 1, param_slot_9 };
    static const QMetaData slot_tbl[] = {
	{ "expand()", &slot_0, QMetaData::Public },
	{ "compress()", &slot_1, QMetaData::Public },
	{ "close_dis()", &slot_2, QMetaData::Public },
	{ "save_avg()", &slot_3, QMetaData::Public },
	{ "save_avg(const QString&)", &slot_4, QMetaData::Public },
	{ "calc()", &slot_5, QMetaData::Public },
	{ "update_pixmap()", &slot_6, QMetaData::Public },
	{ "print_plot()", &slot_7, QMetaData::Public },
	{ "resizeEvent(QResizeEvent*)", &slot_8, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_9, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"status", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "status(int)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Average", parentObject,
	slot_tbl, 10,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Average.setMetaObject( metaObj );
    return metaObj;
}

void* US_Average::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Average" ) )
	return this;
    return QWidget::qt_cast( clname );
}

// SIGNAL status
void US_Average::status( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool US_Average::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: expand(); break;
    case 1: compress(); break;
    case 2: close_dis(); break;
    case 3: save_avg(); break;
    case 4: save_avg((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: calc(); break;
    case 6: update_pixmap(); break;
    case 7: print_plot(); break;
    case 8: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    case 9: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Average::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: status((int)static_QUType_int.get(_o+1)); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Average::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_Average::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
