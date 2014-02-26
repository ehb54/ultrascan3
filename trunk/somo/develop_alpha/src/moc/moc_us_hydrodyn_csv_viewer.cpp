/****************************************************************************
** US_Hydrodyn_Csv_Viewer meta object code from reading C++ file 'us_hydrodyn_csv_viewer.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_csv_viewer.h"
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

const char *US_Hydrodyn_Csv_Viewer::className() const
{
    return "US_Hydrodyn_Csv_Viewer";
}

QMetaObject *US_Hydrodyn_Csv_Viewer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Csv_Viewer( "US_Hydrodyn_Csv_Viewer", &US_Hydrodyn_Csv_Viewer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Csv_Viewer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Csv_Viewer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Csv_Viewer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Csv_Viewer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Csv_Viewer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"sort_column", 1, param_slot_1 };
    static const QUMethod slot_2 = {"cancel", 0, 0 };
    static const QUMethod slot_3 = {"help", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_4 = {"closeEvent", 1, param_slot_4 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "sort_column(int)", &slot_1, QMetaData::Private },
	{ "cancel()", &slot_2, QMetaData::Private },
	{ "help()", &slot_3, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_4, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Csv_Viewer", parentObject,
	slot_tbl, 5,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Csv_Viewer.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Csv_Viewer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Csv_Viewer" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Csv_Viewer::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: sort_column((int)static_QUType_int.get(_o+1)); break;
    case 2: cancel(); break;
    case 3: help(); break;
    case 4: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Csv_Viewer::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Csv_Viewer::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Csv_Viewer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
