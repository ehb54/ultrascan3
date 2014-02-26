/****************************************************************************
** US_ShowDetails meta object code from reading C++ file 'us_showdetails.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_showdetails.h"
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

const char *US_ShowDetails::className() const
{
    return "US_ShowDetails";
}

QMetaObject *US_ShowDetails::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ShowDetails( "US_ShowDetails", &US_ShowDetails::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ShowDetails::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ShowDetails", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ShowDetails::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ShowDetails", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ShowDetails::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"show_dataset", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"show_dataset", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_3 = {"closeEvent", 1, param_slot_3 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Public },
	{ "show_dataset()", &slot_1, QMetaData::Public },
	{ "show_dataset(int)", &slot_2, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_3, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ShowDetails", parentObject,
	slot_tbl, 4,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ShowDetails.setMetaObject( metaObj );
    return metaObj;
}

void* US_ShowDetails::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ShowDetails" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_ShowDetails::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: show_dataset(); break;
    case 2: show_dataset((int)static_QUType_int.get(_o+1)); break;
    case 3: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ShowDetails::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_ShowDetails::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_ShowDetails::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
