/****************************************************************************
** US_Fefit_FitControl meta object code from reading C++ file 'us_fefit.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_fefit.h"
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

const char *US_Fefit_FitControl::className() const
{
    return "US_Fefit_FitControl";
}

QMetaObject *US_Fefit_FitControl::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Fefit_FitControl( "US_Fefit_FitControl", &US_Fefit_FitControl::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Fefit_FitControl::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Fefit_FitControl", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Fefit_FitControl::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Fefit_FitControl", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Fefit_FitControl::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"update_constrained_button", 0, 0 };
    static const QUMethod slot_3 = {"update_unconstrained_button", 0, 0 };
    static const QUMethod slot_4 = {"update_autoconverge_button", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_5 = {"closeEvent", 1, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Public },
	{ "help()", &slot_1, QMetaData::Public },
	{ "update_constrained_button()", &slot_2, QMetaData::Public },
	{ "update_unconstrained_button()", &slot_3, QMetaData::Public },
	{ "update_autoconverge_button()", &slot_4, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_5, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Fefit_FitControl", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Fefit_FitControl.setMetaObject( metaObj );
    return metaObj;
}

void* US_Fefit_FitControl::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Fefit_FitControl" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Fefit_FitControl::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: help(); break;
    case 2: update_constrained_button(); break;
    case 3: update_unconstrained_button(); break;
    case 4: update_autoconverge_button(); break;
    case 5: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Fefit_FitControl::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Fefit_FitControl::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Fefit_FitControl::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
