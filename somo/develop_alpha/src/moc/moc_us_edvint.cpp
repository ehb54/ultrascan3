/****************************************************************************
** EditIntVeloc_Win meta object code from reading C++ file 'us_edvint.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_edvint.h"
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

const char *EditIntVeloc_Win::className() const
{
    return "EditIntVeloc_Win";
}

QMetaObject *EditIntVeloc_Win::metaObj = 0;
static QMetaObjectCleanUp cleanUp_EditIntVeloc_Win( "EditIntVeloc_Win", &EditIntVeloc_Win::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString EditIntVeloc_Win::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "EditIntVeloc_Win", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString EditIntVeloc_Win::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "EditIntVeloc_Win", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* EditIntVeloc_Win::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = EditData_Win::staticMetaObject();
    static const QUMethod slot_0 = {"movie", 0, 0 };
    static const QUMethod slot_1 = {"next_step", 0, 0 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_3 = {"get_x", 1, param_slot_3 };
    static const QUMethod slot_4 = {"sub_baseline", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"calc_sum", 2, param_slot_5 };
    static const QUMethod slot_6 = {"load_base", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"load_base", 1, param_slot_7 };
    static const QUMethod slot_8 = {"subtract_residuals", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"fit_ls", 1, param_slot_9 };
    static const QUMethod slot_10 = {"calc_integral", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"setFringeTolerance", 1, param_slot_11 };
    static const QUMethod slot_12 = {"setup_GUI", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "movie()", &slot_0, QMetaData::Public },
	{ "next_step()", &slot_1, QMetaData::Public },
	{ "help()", &slot_2, QMetaData::Public },
	{ "get_x(const QMouseEvent&)", &slot_3, QMetaData::Public },
	{ "sub_baseline()", &slot_4, QMetaData::Public },
	{ "calc_sum(int,int)", &slot_5, QMetaData::Public },
	{ "load_base()", &slot_6, QMetaData::Public },
	{ "load_base(const char*)", &slot_7, QMetaData::Public },
	{ "subtract_residuals()", &slot_8, QMetaData::Public },
	{ "fit_ls(double)", &slot_9, QMetaData::Public },
	{ "calc_integral()", &slot_10, QMetaData::Public },
	{ "setFringeTolerance(double)", &slot_11, QMetaData::Public },
	{ "setup_GUI()", &slot_12, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"EditIntVeloc_Win", parentObject,
	slot_tbl, 13,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_EditIntVeloc_Win.setMetaObject( metaObj );
    return metaObj;
}

void* EditIntVeloc_Win::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "EditIntVeloc_Win" ) )
	return this;
    return EditData_Win::qt_cast( clname );
}

bool EditIntVeloc_Win::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: movie(); break;
    case 1: next_step(); break;
    case 2: help(); break;
    case 3: get_x((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 4: sub_baseline(); break;
    case 5: calc_sum((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 6: load_base(); break;
    case 7: load_base((const char*)static_QUType_charstar.get(_o+1)); break;
    case 8: subtract_residuals(); break;
    case 9: fit_ls((double)static_QUType_double.get(_o+1)); break;
    case 10: calc_integral(); break;
    case 11: setFringeTolerance((double)static_QUType_double.get(_o+1)); break;
    case 12: setup_GUI(); break;
    default:
	return EditData_Win::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool EditIntVeloc_Win::qt_emit( int _id, QUObject* _o )
{
    return EditData_Win::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool EditIntVeloc_Win::qt_property( int id, int f, QVariant* v)
{
    return EditData_Win::qt_property( id, f, v);
}

bool EditIntVeloc_Win::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
