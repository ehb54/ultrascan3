/****************************************************************************
** US_Diagnostics meta object code from reading C++ file 'us_diagnostics.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_diagnostics.h"
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

const char *US_Diagnostics::className() const
{
    return "US_Diagnostics";
}

QMetaObject *US_Diagnostics::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Diagnostics( "US_Diagnostics", &US_Diagnostics::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Diagnostics::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Diagnostics", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Diagnostics::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Diagnostics", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Diagnostics::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"load", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"load", 1, param_slot_1 };
    static const QUMethod slot_2 = {"quit", 0, 0 };
    static const QUMethod slot_3 = {"print_delta", 0, 0 };
    static const QUMethod slot_4 = {"print_scan", 0, 0 };
    static const QUMethod slot_5 = {"plot_graph", 0, 0 };
    static const QUMethod slot_6 = {"convert", 0, 0 };
    static const QUMethod slot_7 = {"help", 0, 0 };
    static const QUMethod slot_8 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_9 = {"closeEvent", 1, param_slot_9 };
    static const QMetaData slot_tbl[] = {
	{ "load()", &slot_0, QMetaData::Public },
	{ "load(const QString&)", &slot_1, QMetaData::Public },
	{ "quit()", &slot_2, QMetaData::Public },
	{ "print_delta()", &slot_3, QMetaData::Public },
	{ "print_scan()", &slot_4, QMetaData::Public },
	{ "plot_graph()", &slot_5, QMetaData::Public },
	{ "convert()", &slot_6, QMetaData::Public },
	{ "help()", &slot_7, QMetaData::Public },
	{ "setup_GUI()", &slot_8, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_9, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Diagnostics", parentObject,
	slot_tbl, 10,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Diagnostics.setMetaObject( metaObj );
    return metaObj;
}

void* US_Diagnostics::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Diagnostics" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Diagnostics::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load(); break;
    case 1: load((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: quit(); break;
    case 3: print_delta(); break;
    case 4: print_scan(); break;
    case 5: plot_graph(); break;
    case 6: convert(); break;
    case 7: help(); break;
    case 8: setup_GUI(); break;
    case 9: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Diagnostics::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Diagnostics::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Diagnostics::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
