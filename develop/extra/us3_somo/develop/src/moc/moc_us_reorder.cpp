/****************************************************************************
** US_ReOrder meta object code from reading C++ file 'us_reorder.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_reorder.h"
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

const char *US_ReOrder::className() const
{
    return "US_ReOrder";
}

QMetaObject *US_ReOrder::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ReOrder( "US_ReOrder", &US_ReOrder::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ReOrder::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ReOrder", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ReOrder::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ReOrder", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ReOrder::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"closeEvent", 1, param_slot_1 };
    static const QUMethod slot_2 = {"select_dir", 0, 0 };
    static const QUMethod slot_3 = {"order_all", 0, 0 };
    static const QUMethod slot_4 = {"order_cell", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"show_cell", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_datatype", 1, param_slot_6 };
    static const QUMethod slot_7 = {"check_lambda", 0, 0 };
    static const QUMethod slot_8 = {"reset", 0, 0 };
    static const QUMethod slot_9 = {"help", 0, 0 };
    static const QUMethod slot_10 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_1, QMetaData::Protected },
	{ "select_dir()", &slot_2, QMetaData::Private },
	{ "order_all()", &slot_3, QMetaData::Private },
	{ "order_cell()", &slot_4, QMetaData::Private },
	{ "show_cell(int)", &slot_5, QMetaData::Private },
	{ "update_datatype(int)", &slot_6, QMetaData::Private },
	{ "check_lambda()", &slot_7, QMetaData::Private },
	{ "reset()", &slot_8, QMetaData::Private },
	{ "help()", &slot_9, QMetaData::Private },
	{ "quit()", &slot_10, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ReOrder", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ReOrder.setMetaObject( metaObj );
    return metaObj;
}

void* US_ReOrder::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ReOrder" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_ReOrder::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: select_dir(); break;
    case 3: order_all(); break;
    case 4: order_cell(); break;
    case 5: show_cell((int)static_QUType_int.get(_o+1)); break;
    case 6: update_datatype((int)static_QUType_int.get(_o+1)); break;
    case 7: check_lambda(); break;
    case 8: reset(); break;
    case 9: help(); break;
    case 10: quit(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ReOrder::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_ReOrder::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_ReOrder::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
