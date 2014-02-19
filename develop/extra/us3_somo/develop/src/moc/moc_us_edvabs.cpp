/****************************************************************************
** EditAbsVeloc_Win meta object code from reading C++ file 'us_edvabs.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_edvabs.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QMouseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *EditAbsVeloc_Win::className() const
{
    return "EditAbsVeloc_Win";
}

QMetaObject *EditAbsVeloc_Win::metaObj = 0;
static QMetaObjectCleanUp cleanUp_EditAbsVeloc_Win( "EditAbsVeloc_Win", &EditAbsVeloc_Win::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString EditAbsVeloc_Win::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "EditAbsVeloc_Win", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString EditAbsVeloc_Win::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "EditAbsVeloc_Win", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* EditAbsVeloc_Win::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = EditData_Win::staticMetaObject();
    static const QUMethod slot_0 = {"help", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"get_x", 1, param_slot_1 };
    static const QUMethod slot_2 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_3 = {"next_step", 0, 0 };
    static const QUMethod slot_4 = {"subtract_residuals", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"fit_ls", 1, param_slot_5 };
    static const QUMethod slot_6 = {"update_oldscan", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "help()", &slot_0, QMetaData::Public },
	{ "get_x(const QMouseEvent&)", &slot_1, QMetaData::Public },
	{ "setup_GUI()", &slot_2, QMetaData::Public },
	{ "next_step()", &slot_3, QMetaData::Public },
	{ "subtract_residuals()", &slot_4, QMetaData::Public },
	{ "fit_ls(double)", &slot_5, QMetaData::Public },
	{ "update_oldscan()", &slot_6, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"EditAbsVeloc_Win", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_EditAbsVeloc_Win.setMetaObject( metaObj );
    return metaObj;
}

void* EditAbsVeloc_Win::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "EditAbsVeloc_Win" ) )
	return this;
    return EditData_Win::qt_cast( clname );
}

bool EditAbsVeloc_Win::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: help(); break;
    case 1: get_x((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 2: setup_GUI(); break;
    case 3: next_step(); break;
    case 4: subtract_residuals(); break;
    case 5: fit_ls((double)static_QUType_double.get(_o+1)); break;
    case 6: update_oldscan(); break;
    default:
	return EditData_Win::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool EditAbsVeloc_Win::qt_emit( int _id, QUObject* _o )
{
    return EditData_Win::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool EditAbsVeloc_Win::qt_property( int id, int f, QVariant* v)
{
    return EditData_Win::qt_property( id, f, v);
}

bool EditAbsVeloc_Win::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
