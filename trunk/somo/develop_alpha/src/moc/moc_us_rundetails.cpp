/****************************************************************************
** RunDetails_F meta object code from reading C++ file 'us_rundetails.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_rundetails.h"
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

const char *RunDetails_F::className() const
{
    return "RunDetails_F";
}

QMetaObject *RunDetails_F::metaObj = 0;
static QMetaObjectCleanUp cleanUp_RunDetails_F( "RunDetails_F", &RunDetails_F::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString RunDetails_F::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "RunDetails_F", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString RunDetails_F::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "RunDetails_F", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* RunDetails_F::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"show_cell", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"show_speed", 1, param_slot_1 };
    static const QUMethod slot_2 = {"update_screen", 0, 0 };
    static const QUMethod slot_3 = {"help", 0, 0 };
    static const QUMethod slot_4 = {"cancel", 0, 0 };
    static const QUMethod slot_5 = {"accept", 0, 0 };
    static const QUMethod slot_6 = {"plot_type", 0, 0 };
    static const QUMethod slot_7 = {"animate", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_cell", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_id", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_10 = {"closeEvent", 1, param_slot_10 };
    static const QUMethod slot_11 = {"create_gui", 0, 0 };
    static const QUMethod slot_12 = {"setup_GUI", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "show_cell(int)", &slot_0, QMetaData::Public },
	{ "show_speed(const QString&)", &slot_1, QMetaData::Public },
	{ "update_screen()", &slot_2, QMetaData::Public },
	{ "help()", &slot_3, QMetaData::Public },
	{ "cancel()", &slot_4, QMetaData::Public },
	{ "accept()", &slot_5, QMetaData::Public },
	{ "plot_type()", &slot_6, QMetaData::Public },
	{ "animate()", &slot_7, QMetaData::Public },
	{ "update_cell(const QString&)", &slot_8, QMetaData::Public },
	{ "update_id(const QString&)", &slot_9, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_10, QMetaData::Public },
	{ "create_gui()", &slot_11, QMetaData::Private },
	{ "setup_GUI()", &slot_12, QMetaData::Private }
    };
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QUMethod signal_1 = {"isClosed", 0, 0 };
    static const QUMethod signal_2 = {"returnPressed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Private },
	{ "isClosed()", &signal_1, QMetaData::Private },
	{ "returnPressed()", &signal_2, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"RunDetails_F", parentObject,
	slot_tbl, 13,
	signal_tbl, 3,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_RunDetails_F.setMetaObject( metaObj );
    return metaObj;
}

void* RunDetails_F::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "RunDetails_F" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL clicked
void RunDetails_F::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL isClosed
void RunDetails_F::isClosed()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

// SIGNAL returnPressed
void RunDetails_F::returnPressed()
{
    activate_signal( staticMetaObject()->signalOffset() + 2 );
}

bool RunDetails_F::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: show_cell((int)static_QUType_int.get(_o+1)); break;
    case 1: show_speed((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_screen(); break;
    case 3: help(); break;
    case 4: cancel(); break;
    case 5: accept(); break;
    case 6: plot_type(); break;
    case 7: animate(); break;
    case 8: update_cell((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_id((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 11: create_gui(); break;
    case 12: setup_GUI(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool RunDetails_F::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    case 1: isClosed(); break;
    case 2: returnPressed(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool RunDetails_F::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool RunDetails_F::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
