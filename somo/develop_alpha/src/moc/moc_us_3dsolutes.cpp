/****************************************************************************
** US_3d_Solutes meta object code from reading C++ file 'us_3dsolutes.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_3dsolutes.h"
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

const char *US_3d_Solutes::className() const
{
    return "US_3d_Solutes";
}

QMetaObject *US_3d_Solutes::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_3d_Solutes( "US_3d_Solutes", &US_3d_Solutes::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_3d_Solutes::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_3d_Solutes", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_3d_Solutes::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_3d_Solutes", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_3d_Solutes::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"setup", 0, 0 };
    static const QUMethod slot_3 = {"set_mw1", 0, 0 };
    static const QUMethod slot_4 = {"set_mw2", 0, 0 };
    static const QUMethod slot_5 = {"set_s1", 0, 0 };
    static const QUMethod slot_6 = {"set_s2", 0, 0 };
    static const QUMethod slot_7 = {"set_D1", 0, 0 };
    static const QUMethod slot_8 = {"set_D2", 0, 0 };
    static const QUMethod slot_9 = {"set_f1", 0, 0 };
    static const QUMethod slot_10 = {"set_f2", 0, 0 };
    static const QUMethod slot_11 = {"set_ff01", 0, 0 };
    static const QUMethod slot_12 = {"set_ff02", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"set_zAdjust", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"set_resolutionAdjust", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"set_alphaAdjust", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"set_betaAdjust", 1, param_slot_16 };
    static const QUMethod slot_17 = {"plot_residuals", 0, 0 };
    static const QUMethod slot_18 = {"plot", 0, 0 };
    static const QUMethod slot_19 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_20 = {"closeEvent", 1, param_slot_20 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Public },
	{ "help()", &slot_1, QMetaData::Public },
	{ "setup()", &slot_2, QMetaData::Private },
	{ "set_mw1()", &slot_3, QMetaData::Private },
	{ "set_mw2()", &slot_4, QMetaData::Private },
	{ "set_s1()", &slot_5, QMetaData::Private },
	{ "set_s2()", &slot_6, QMetaData::Private },
	{ "set_D1()", &slot_7, QMetaData::Private },
	{ "set_D2()", &slot_8, QMetaData::Private },
	{ "set_f1()", &slot_9, QMetaData::Private },
	{ "set_f2()", &slot_10, QMetaData::Private },
	{ "set_ff01()", &slot_11, QMetaData::Private },
	{ "set_ff02()", &slot_12, QMetaData::Private },
	{ "set_zAdjust(double)", &slot_13, QMetaData::Private },
	{ "set_resolutionAdjust(double)", &slot_14, QMetaData::Private },
	{ "set_alphaAdjust(double)", &slot_15, QMetaData::Private },
	{ "set_betaAdjust(double)", &slot_16, QMetaData::Private },
	{ "plot_residuals()", &slot_17, QMetaData::Private },
	{ "plot()", &slot_18, QMetaData::Public },
	{ "setup_GUI()", &slot_19, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_20, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_3d_Solutes", parentObject,
	slot_tbl, 21,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_3d_Solutes.setMetaObject( metaObj );
    return metaObj;
}

void* US_3d_Solutes::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_3d_Solutes" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_3d_Solutes::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: help(); break;
    case 2: setup(); break;
    case 3: set_mw1(); break;
    case 4: set_mw2(); break;
    case 5: set_s1(); break;
    case 6: set_s2(); break;
    case 7: set_D1(); break;
    case 8: set_D2(); break;
    case 9: set_f1(); break;
    case 10: set_f2(); break;
    case 11: set_ff01(); break;
    case 12: set_ff02(); break;
    case 13: set_zAdjust((double)static_QUType_double.get(_o+1)); break;
    case 14: set_resolutionAdjust((double)static_QUType_double.get(_o+1)); break;
    case 15: set_alphaAdjust((double)static_QUType_double.get(_o+1)); break;
    case 16: set_betaAdjust((double)static_QUType_double.get(_o+1)); break;
    case 17: plot_residuals(); break;
    case 18: plot(); break;
    case 19: setup_GUI(); break;
    case 20: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_3d_Solutes::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_3d_Solutes::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_3d_Solutes::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
