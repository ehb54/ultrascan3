/****************************************************************************
** US_PseudoAbs meta object code from reading C++ file 'us_pseudoabs.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_pseudoabs.h"
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

const char *US_PseudoAbs::className() const
{
    return "US_PseudoAbs";
}

QMetaObject *US_PseudoAbs::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_PseudoAbs( "US_PseudoAbs", &US_PseudoAbs::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_PseudoAbs::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_PseudoAbs", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_PseudoAbs::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_PseudoAbs", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_PseudoAbs::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"closeEvent", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_2 = {"plotMousePressed", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_3 = {"plotMouseReleased", 1, param_slot_3 };
    static const QUMethod slot_4 = {"select_dir", 0, 0 };
    static const QUMethod slot_5 = {"convert_cell", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"show_cell", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"show_channel", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_ch1txt", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_ch2txt", 1, param_slot_9 };
    static const QUMethod slot_10 = {"show1", 0, 0 };
    static const QUMethod slot_11 = {"show2", 0, 0 };
    static const QUMethod slot_12 = {"reset", 0, 0 };
    static const QUMethod slot_13 = {"markref", 0, 0 };
    static const QUMethod slot_14 = {"help", 0, 0 };
    static const QUMethod slot_15 = {"quit", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_1, QMetaData::Protected },
	{ "plotMousePressed(const QMouseEvent&)", &slot_2, QMetaData::Protected },
	{ "plotMouseReleased(const QMouseEvent&)", &slot_3, QMetaData::Protected },
	{ "select_dir()", &slot_4, QMetaData::Private },
	{ "convert_cell()", &slot_5, QMetaData::Private },
	{ "show_cell(int)", &slot_6, QMetaData::Private },
	{ "show_channel(int)", &slot_7, QMetaData::Private },
	{ "update_ch1txt(const QString&)", &slot_8, QMetaData::Private },
	{ "update_ch2txt(const QString&)", &slot_9, QMetaData::Private },
	{ "show1()", &slot_10, QMetaData::Private },
	{ "show2()", &slot_11, QMetaData::Private },
	{ "reset()", &slot_12, QMetaData::Private },
	{ "markref()", &slot_13, QMetaData::Private },
	{ "help()", &slot_14, QMetaData::Private },
	{ "quit()", &slot_15, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_PseudoAbs", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_PseudoAbs.setMetaObject( metaObj );
    return metaObj;
}

void* US_PseudoAbs::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_PseudoAbs" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_PseudoAbs::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: plotMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 3: plotMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 4: select_dir(); break;
    case 5: convert_cell(); break;
    case 6: show_cell((int)static_QUType_int.get(_o+1)); break;
    case 7: show_channel((int)static_QUType_int.get(_o+1)); break;
    case 8: update_ch1txt((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_ch2txt((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: show1(); break;
    case 11: show2(); break;
    case 12: reset(); break;
    case 13: markref(); break;
    case 14: help(); break;
    case 15: quit(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_PseudoAbs::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_PseudoAbs::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_PseudoAbs::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
