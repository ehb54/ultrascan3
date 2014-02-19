/****************************************************************************
** US_Merge meta object code from reading C++ file 'us_merge.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_merge.h"
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

const char *US_Merge::className() const
{
    return "US_Merge";
}

QMetaObject *US_Merge::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Merge( "US_Merge", &US_Merge::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Merge::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Merge", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Merge::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Merge", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Merge::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"closeEvent", 1, param_slot_1 };
    static const QUMethod slot_2 = {"select_dir1", 0, 0 };
    static const QUMethod slot_3 = {"select_dir2", 0, 0 };
    static const QUMethod slot_4 = {"merge", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"show_dir1_cell", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"show_dir2_cell", 1, param_slot_6 };
    static const QUMethod slot_7 = {"order_dir1_cell", 0, 0 };
    static const QUMethod slot_8 = {"order_dir2_cell", 0, 0 };
    static const QUMethod slot_9 = {"order_dir1_all", 0, 0 };
    static const QUMethod slot_10 = {"order_dir2_all", 0, 0 };
    static const QUMethod slot_11 = {"update_if_button", 0, 0 };
    static const QUMethod slot_12 = {"update_wl_button", 0, 0 };
    static const QUMethod slot_13 = {"update_abs_button", 0, 0 };
    static const QUMethod slot_14 = {"update_copy_button", 0, 0 };
    static const QUMethod slot_15 = {"update_move_button", 0, 0 };
    static const QUMethod slot_16 = {"check_dir1_lambda", 0, 0 };
    static const QUMethod slot_17 = {"check_dir2_lambda", 0, 0 };
    static const QUMethod slot_18 = {"help", 0, 0 };
    static const QUMethod slot_19 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_1, QMetaData::Protected },
	{ "select_dir1()", &slot_2, QMetaData::Private },
	{ "select_dir2()", &slot_3, QMetaData::Private },
	{ "merge()", &slot_4, QMetaData::Private },
	{ "show_dir1_cell(int)", &slot_5, QMetaData::Private },
	{ "show_dir2_cell(int)", &slot_6, QMetaData::Private },
	{ "order_dir1_cell()", &slot_7, QMetaData::Private },
	{ "order_dir2_cell()", &slot_8, QMetaData::Private },
	{ "order_dir1_all()", &slot_9, QMetaData::Private },
	{ "order_dir2_all()", &slot_10, QMetaData::Private },
	{ "update_if_button()", &slot_11, QMetaData::Private },
	{ "update_wl_button()", &slot_12, QMetaData::Private },
	{ "update_abs_button()", &slot_13, QMetaData::Private },
	{ "update_copy_button()", &slot_14, QMetaData::Private },
	{ "update_move_button()", &slot_15, QMetaData::Private },
	{ "check_dir1_lambda()", &slot_16, QMetaData::Private },
	{ "check_dir2_lambda()", &slot_17, QMetaData::Private },
	{ "help()", &slot_18, QMetaData::Private },
	{ "quit()", &slot_19, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Merge", parentObject,
	slot_tbl, 20,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Merge.setMetaObject( metaObj );
    return metaObj;
}

void* US_Merge::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Merge" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Merge::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: select_dir1(); break;
    case 3: select_dir2(); break;
    case 4: merge(); break;
    case 5: show_dir1_cell((int)static_QUType_int.get(_o+1)); break;
    case 6: show_dir2_cell((int)static_QUType_int.get(_o+1)); break;
    case 7: order_dir1_cell(); break;
    case 8: order_dir2_cell(); break;
    case 9: order_dir1_all(); break;
    case 10: order_dir2_all(); break;
    case 11: update_if_button(); break;
    case 12: update_wl_button(); break;
    case 13: update_abs_button(); break;
    case 14: update_copy_button(); break;
    case 15: update_move_button(); break;
    case 16: check_dir1_lambda(); break;
    case 17: check_dir2_lambda(); break;
    case 18: help(); break;
    case 19: quit(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Merge::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Merge::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Merge::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
