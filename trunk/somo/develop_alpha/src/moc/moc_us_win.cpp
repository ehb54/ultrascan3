/****************************************************************************
** UsWin meta object code from reading C++ file 'us_win.h'
**
** Created: Wed Dec 4 19:14:47 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_win.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *UsWin::className() const
{
    return "UsWin";
}

QMetaObject *UsWin::metaObj = 0;
static QMetaObjectCleanUp cleanUp_UsWin( "UsWin", &UsWin::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString UsWin::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "UsWin", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString UsWin::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "UsWin", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* UsWin::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_Widgets::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"display_help", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"data_control", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"launch", 1, param_slot_2 };
    static const QUMethod slot_3 = {"quit", 0, 0 };
    static const QUMethod slot_4 = {"about", 0, 0 };
    static const QUMethod slot_5 = {"credits", 0, 0 };
    static const QUMethod slot_6 = {"close_splash", 0, 0 };
    static const QUMethod slot_7 = {"export_V", 0, 0 };
    static const QUMethod slot_8 = {"export_E", 0, 0 };
    static const QUMethod slot_9 = {"print_V", 0, 0 };
    static const QUMethod slot_10 = {"print_E", 0, 0 };
    static const QUMethod slot_11 = {"report_V", 0, 0 };
    static const QUMethod slot_12 = {"report_E", 0, 0 };
    static const QUMethod slot_13 = {"report_EP", 0, 0 };
    static const QUMethod slot_14 = {"report_MC", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "QProcess", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"closeAttnt", 2, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_16 = {"closeEvent", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_17 = {"resizeEvent", 1, param_slot_17 };
    static const QMetaData slot_tbl[] = {
	{ "display_help(int)", &slot_0, QMetaData::Private },
	{ "data_control(int)", &slot_1, QMetaData::Private },
	{ "launch(int)", &slot_2, QMetaData::Private },
	{ "quit()", &slot_3, QMetaData::Private },
	{ "about()", &slot_4, QMetaData::Private },
	{ "credits()", &slot_5, QMetaData::Private },
	{ "close_splash()", &slot_6, QMetaData::Private },
	{ "export_V()", &slot_7, QMetaData::Private },
	{ "export_E()", &slot_8, QMetaData::Private },
	{ "print_V()", &slot_9, QMetaData::Private },
	{ "print_E()", &slot_10, QMetaData::Private },
	{ "report_V()", &slot_11, QMetaData::Private },
	{ "report_E()", &slot_12, QMetaData::Private },
	{ "report_EP()", &slot_13, QMetaData::Private },
	{ "report_MC()", &slot_14, QMetaData::Private },
	{ "closeAttnt(QProcess*,QString)", &slot_15, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_16, QMetaData::Private },
	{ "resizeEvent(QResizeEvent*)", &slot_17, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"explain", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "explain(const QString&)", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"UsWin", parentObject,
	slot_tbl, 18,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_UsWin.setMetaObject( metaObj );
    return metaObj;
}

void* UsWin::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "UsWin" ) )
	return this;
    return US_Widgets::qt_cast( clname );
}

// SIGNAL explain
void UsWin::explain( const QString& t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool UsWin::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: display_help((int)static_QUType_int.get(_o+1)); break;
    case 1: data_control((int)static_QUType_int.get(_o+1)); break;
    case 2: launch((int)static_QUType_int.get(_o+1)); break;
    case 3: quit(); break;
    case 4: about(); break;
    case 5: credits(); break;
    case 6: close_splash(); break;
    case 7: export_V(); break;
    case 8: export_E(); break;
    case 9: print_V(); break;
    case 10: print_E(); break;
    case 11: report_V(); break;
    case 12: report_E(); break;
    case 13: report_EP(); break;
    case 14: report_MC(); break;
    case 15: closeAttnt((Q3Process*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2)); break;
    case 16: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 17: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return US_Widgets::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool UsWin::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: explain((const QString&)static_QUType_QString.get(_o+1)); break;
    default:
	return US_Widgets::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool UsWin::qt_property( int id, int f, QVariant* v)
{
    return US_Widgets::qt_property( id, f, v);
}

bool UsWin::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
