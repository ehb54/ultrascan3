/****************************************************************************
** US_Hydrodyn_Saxs_Hplc_Fit_Global meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_fit_global.h'
**
** Created: Tue Feb 25 10:28:33 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_hplc_fit_global.h"
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

const char *US_Hydrodyn_Saxs_Hplc_Fit_Global::className() const
{
    return "US_Hydrodyn_Saxs_Hplc_Fit_Global";
}

QMetaObject *US_Hydrodyn_Saxs_Hplc_Fit_Global::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Hplc_Fit_Global( "US_Hydrodyn_Saxs_Hplc_Fit_Global", &US_Hydrodyn_Saxs_Hplc_Fit_Global::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Hplc_Fit_Global::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Fit_Global", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Hplc_Fit_Global::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Fit_Global", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Hplc_Fit_Global::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"update_enables", 0, 0 };
    static const QUMethod slot_1 = {"restore", 0, 0 };
    static const QUMethod slot_2 = {"undo", 0, 0 };
    static const QUMethod slot_3 = {"lm", 0, 0 };
    static const QUMethod slot_4 = {"gsm_sd", 0, 0 };
    static const QUMethod slot_5 = {"gsm_ih", 0, 0 };
    static const QUMethod slot_6 = {"gsm_cg", 0, 0 };
    static const QUMethod slot_7 = {"ga", 0, 0 };
    static const QUMethod slot_8 = {"grid", 0, 0 };
    static const QUMethod slot_9 = {"stop", 0, 0 };
    static const QUMethod slot_10 = {"cancel", 0, 0 };
    static const QUMethod slot_11 = {"help", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_12 = {"closeEvent", 1, param_slot_12 };
    static const QMetaData slot_tbl[] = {
	{ "update_enables()", &slot_0, QMetaData::Private },
	{ "restore()", &slot_1, QMetaData::Private },
	{ "undo()", &slot_2, QMetaData::Private },
	{ "lm()", &slot_3, QMetaData::Private },
	{ "gsm_sd()", &slot_4, QMetaData::Private },
	{ "gsm_ih()", &slot_5, QMetaData::Private },
	{ "gsm_cg()", &slot_6, QMetaData::Private },
	{ "ga()", &slot_7, QMetaData::Private },
	{ "grid()", &slot_8, QMetaData::Private },
	{ "stop()", &slot_9, QMetaData::Private },
	{ "cancel()", &slot_10, QMetaData::Private },
	{ "help()", &slot_11, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_12, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Hplc_Fit_Global", parentObject,
	slot_tbl, 13,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Hplc_Fit_Global.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Hplc_Fit_Global::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Hplc_Fit_Global" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Hplc_Fit_Global::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_enables(); break;
    case 1: restore(); break;
    case 2: undo(); break;
    case 3: lm(); break;
    case 4: gsm_sd(); break;
    case 5: gsm_ih(); break;
    case 6: gsm_cg(); break;
    case 7: ga(); break;
    case 8: grid(); break;
    case 9: stop(); break;
    case 10: cancel(); break;
    case 11: help(); break;
    case 12: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Hplc_Fit_Global::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Hplc_Fit_Global::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Hplc_Fit_Global::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
