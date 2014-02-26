/****************************************************************************
** US_Hydrodyn_SaxsOptions meta object code from reading C++ file 'us_hydrodyn_saxs_options.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_options.h"
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

const char *US_Hydrodyn_SaxsOptions::className() const
{
    return "US_Hydrodyn_SaxsOptions";
}

QMetaObject *US_Hydrodyn_SaxsOptions::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_SaxsOptions( "US_Hydrodyn_SaxsOptions", &US_Hydrodyn_SaxsOptions::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_SaxsOptions::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SaxsOptions", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_SaxsOptions::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_SaxsOptions", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_SaxsOptions::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"sas_options_saxs", 0, 0 };
    static const QUMethod slot_2 = {"sas_options_sans", 0, 0 };
    static const QUMethod slot_3 = {"sas_options_curve", 0, 0 };
    static const QUMethod slot_4 = {"sas_options_bead_model", 0, 0 };
    static const QUMethod slot_5 = {"sas_options_hydration", 0, 0 };
    static const QUMethod slot_6 = {"sas_options_guinier", 0, 0 };
    static const QUMethod slot_7 = {"sas_options_xsr", 0, 0 };
    static const QUMethod slot_8 = {"sas_options_misc", 0, 0 };
    static const QUMethod slot_9 = {"sas_options_experimental", 0, 0 };
    static const QUMethod slot_10 = {"cancel", 0, 0 };
    static const QUMethod slot_11 = {"help", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_12 = {"closeEvent", 1, param_slot_12 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "sas_options_saxs()", &slot_1, QMetaData::Private },
	{ "sas_options_sans()", &slot_2, QMetaData::Private },
	{ "sas_options_curve()", &slot_3, QMetaData::Private },
	{ "sas_options_bead_model()", &slot_4, QMetaData::Private },
	{ "sas_options_hydration()", &slot_5, QMetaData::Private },
	{ "sas_options_guinier()", &slot_6, QMetaData::Private },
	{ "sas_options_xsr()", &slot_7, QMetaData::Private },
	{ "sas_options_misc()", &slot_8, QMetaData::Private },
	{ "sas_options_experimental()", &slot_9, QMetaData::Private },
	{ "cancel()", &slot_10, QMetaData::Private },
	{ "help()", &slot_11, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_12, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_SaxsOptions", parentObject,
	slot_tbl, 13,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_SaxsOptions.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_SaxsOptions::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_SaxsOptions" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_SaxsOptions::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: sas_options_saxs(); break;
    case 2: sas_options_sans(); break;
    case 3: sas_options_curve(); break;
    case 4: sas_options_bead_model(); break;
    case 5: sas_options_hydration(); break;
    case 6: sas_options_guinier(); break;
    case 7: sas_options_xsr(); break;
    case 8: sas_options_misc(); break;
    case 9: sas_options_experimental(); break;
    case 10: cancel(); break;
    case 11: help(); break;
    case 12: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_SaxsOptions::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_SaxsOptions::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_SaxsOptions::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
